import os
from PIL import Image, ImageOps

def rgb_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def process():
    os.chdir(r"e:\project\esp32c3_supermini_tft_screen\esp32_c3_st7789_clock_ui_demo_v13")
    img_path = r"C:\Users\phamt\.gemini\antigravity\brain\1a8788f2-4ab6-419d-b20b-11754e5ae30e\media__1780036077150.png"
    img = Image.open(img_path).convert("RGBA")
    
    bg = Image.new("RGBA", img.size, (255, 255, 255, 255))
    diff = Image.alpha_composite(bg, img).convert("L")
    inv = ImageOps.invert(diff)
    bbox = inv.getbbox()
    
    if bbox:
        x1, y1, x2, y2 = bbox
        x1 = max(0, x1 - 20)
        y1 = max(0, y1 - 20)
        x2 = min(img.width, x2 + 20)
        y2 = min(img.height, y2 + 20)
        img_cropped = img.crop((x1, y1, x2, y2))
    else:
        img_cropped = img

    new_h = 100
    aspect_ratio = img_cropped.width / img_cropped.height
    new_w = int(new_h * aspect_ratio)
    
    if new_w % 2 != 0:
        new_w += 1
        
    img_scaled = img_cropped.resize((new_w, new_h), Image.Resampling.LANCZOS)
    
    # Base frame (Left Down, Right Up)
    f_left = Image.new("RGBA", (new_w, new_h), (255, 255, 255, 255))
    f_left.paste(img_scaled, (0, 0), img_scaled)
    
    # We define paw regions to isolate the movement without touching the face/body
    # Left paw region: x(0 to 65), y(40 to 100)
    # Right paw region: x(113 to 178), y(40 to 100)
    paw_w = 65
    paw_h = 60
    y_split = 40
    
    left_down_paw = f_left.crop((0, y_split, paw_w, y_split + paw_h))
    right_up_paw = f_left.crop((new_w - paw_w, y_split, new_w, y_split + paw_h))
    
    left_up_paw = ImageOps.mirror(right_up_paw)
    right_down_paw = ImageOps.mirror(left_down_paw)
    
    f_idle = f_left.copy()
    f_idle.paste(left_up_paw, (0, y_split))
    
    f_right = f_left.copy()
    f_right.paste(left_up_paw, (0, y_split))
    f_right.paste(right_down_paw, (new_w - paw_w, y_split))
    
    f_both = f_left.copy()
    f_both.paste(right_down_paw, (new_w - paw_w, y_split))
    
    frames = [f_idle, f_left, f_right, f_both]
    
    with open("bongo_sprites.h", "w") as f:
        f.write(f"// Generated Bongo Cat Sprites {new_w}x{new_h}\n")
        f.write("#include <Arduino.h>\n\n")
        f.write(f"#define BONGO_WIDTH {new_w}\n")
        f.write(f"#define BONGO_HEIGHT {new_h}\n\n")
        
        for i, frame in enumerate(frames):
            f.write(f"const uint16_t bongo_frame_{i}[] PROGMEM = {{\n")
            pixels = frame.convert("RGB").load()
            for y in range(new_h):
                for x in range(new_w):
                    r, g, b = pixels[x, y]
                    color = rgb_to_rgb565(r, g, b)
                    f.write(f"0x{color:04X},")
                f.write("\n")
            f.write("};\n\n")
            
        f.write("const uint16_t* const bongo_frames[] PROGMEM = {\n")
        f.write("  bongo_frame_0, bongo_frame_1, bongo_frame_2, bongo_frame_3\n")
        f.write("};\n")

if __name__ == "__main__":
    process()
