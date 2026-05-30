from PIL import Image, ImageOps

def process():
    img_path = r"C:\Users\phamt\.gemini\antigravity\brain\1a8788f2-4ab6-419d-b20b-11754e5ae30e\media__1780036077150.png"
    img = Image.open(img_path).convert("RGBA")
    
    # Bounding box for the cat to center it. The image is ~1024x636.
    # Cat is roughly in the center. Let's crop it tightly.
    # Using the coordinates from the previous script
    # Let's actually find the bounding box dynamically by looking at non-white pixels
    # But wait, it's a line art with a white background, so the background IS non-transparent!
    # Let's convert to grayscale and invert, then get bbox
    bg = Image.new("RGBA", img.size, (255, 255, 255, 255))
    diff = Image.alpha_composite(bg, img).convert("L")
    # Invert so black lines become white
    inv = ImageOps.invert(diff)
    bbox = inv.getbbox()
    
    if bbox:
        # Pad the bbox a bit
        x1, y1, x2, y2 = bbox
        x1 = max(0, x1 - 20)
        y1 = max(0, y1 - 20)
        x2 = min(img.width, x2 + 20)
        y2 = min(img.height, y2 + 20)
        img_cropped = img.crop((x1, y1, x2, y2))
    else:
        img_cropped = img

    # Scale to width 240
    aspect_ratio = img_cropped.height / img_cropped.width
    new_h = int(240 * aspect_ratio)
    img_scaled = img_cropped.resize((240, new_h), Image.Resampling.LANCZOS)
    
    # Frame 2: Left hand hits (Original image scaled)
    # We place it on a 240x240 canvas, centered
    f2 = Image.new("RGBA", (240, 240), (255, 255, 255, 0))
    y_offset = (240 - new_h) // 2
    f2.paste(img_scaled, (0, y_offset), img_scaled)
    
    # Frame 3: Right hand hits (Flipped horizontally)
    f3_img = ImageOps.mirror(img_scaled)
    f3 = Image.new("RGBA", (240, 240), (255, 255, 255, 0))
    f3.paste(f3_img, (0, y_offset), f3_img)
    
    # Frame 1: Idle (Both hands up)
    # The right half of the image has the hand UP.
    # We will copy the right half of the scaled image and mirror it to the left half.
    # Wait, the center of the cat might not be exactly at width/2.
    # Let's just find the center. For a 240 width image, center is 120.
    idle_img = img_scaled.copy()
    right_half = idle_img.crop((120, 0, 240, new_h))
    left_half = ImageOps.mirror(right_half)
    idle_img.paste(left_half, (0, 0))
    
    f1 = Image.new("RGBA", (240, 240), (255, 255, 255, 0))
    f1.paste(idle_img, (0, y_offset), idle_img)
    
    # White background for all frames
    bg1 = Image.new("RGBA", (240, 240), (255, 255, 255, 255))
    bg1.paste(f1, (0, 0), f1)
    bg2 = Image.new("RGBA", (240, 240), (255, 255, 255, 255))
    bg2.paste(f2, (0, 0), f2)
    bg3 = Image.new("RGBA", (240, 240), (255, 255, 255, 255))
    bg3.paste(f3, (0, 0), f3)
    
    # Save as PNGs
    bg1.save(r"e:\project\esp32c3_supermini_tft_screen\frame1.png")
    bg2.save(r"e:\project\esp32c3_supermini_tft_screen\frame2.png")
    bg3.save(r"e:\project\esp32c3_supermini_tft_screen\frame3.png")

if __name__ == "__main__":
    process()
