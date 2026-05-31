import os
from PIL import Image

def rgb_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def generate():
    os.chdir(r"e:\project\esp32c3_supermini_tft_screen\esp32_c3_st7789_clock_ui_demo_v11")
    
    cat = Image.open('cat.png').convert("RGBA").crop((0, 0, 800, 450))
    kb = Image.open('keyboard.png').convert("RGBA").crop((0, 0, 800, 450))
    paw_l = Image.open('paw-left.png').convert("RGBA").crop((0, 0, 800, 450))
    paw_r = Image.open('paw-right.png').convert("RGBA").crop((0, 0, 800, 450))
    
    # Kích thước target: 200 x 112 (tỉ lệ 800x450)
    # Resize sau khi composite để tránh nhoè alpha
    W = 200
    H = 112
    
    frames = []
    
    for i in range(4):
        base = Image.new("RGBA", (800, 450), (0, 0, 0, 255)) # Nền đen
        base.paste(cat, (0, 0), cat)
        base.paste(kb, (0, 0), kb)
        
        left_down = (i == 1 or i == 3)
        right_down = (i == 2 or i == 3)
        
        if left_down:
            base.paste(paw_l, (0, 0), paw_l)
        if right_down:
            base.paste(paw_r, (0, 0), paw_r)
            
        # Resize
        frame = base.resize((W, H), Image.LANCZOS)
        frames.append(frame)
        frame.save(f"frame_{i}.png")
        
    # Tạo bongo_sprites.h
    with open("bongo_sprites.h", "w") as f:
        f.write(f"// Generated Bongo Cat Sprites {W}x{H}\n")
        f.write("#include <Arduino.h>\n\n")
        f.write(f"#define BONGO_WIDTH {W}\n")
        f.write(f"#define BONGO_HEIGHT {H}\n\n")
        
        for i in range(4):
            f.write(f"const uint16_t bongo_frame_{i}[] PROGMEM = {{\n")
            pixels = frames[i].load()
            for y in range(H):
                for x in range(W):
                    r, g, b, a = pixels[x, y]
                    if a == 0:
                        r, g, b = 0, 0, 0
                    color = rgb_to_rgb565(r, g, b)
                    f.write(f"0x{color:04X},")
                f.write("\n")
            f.write("};\n\n")
            
        f.write("const uint16_t* const bongo_frames[] PROGMEM = {\n")
        f.write("  bongo_frame_0, bongo_frame_1, bongo_frame_2, bongo_frame_3\n")
        f.write("};\n")

if __name__ == "__main__":
    generate()
