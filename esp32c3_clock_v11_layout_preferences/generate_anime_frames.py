import os
import sys

try:
    from PIL import Image, ImageSequence
except ImportError:
    print("Vui lòng cài đặt Pillow: pip install Pillow")
    sys.exit(1)

import urllib.request
import io

def generate_frames():
    print("Downloading Bad Apple GIF...")
    # URL to a small Bad Apple GIF (or any short loop anime GIF)
    # Using a 64x64 or 128x128 GIF is best. Here we use a generic placeholder URL or a known small GIF
    # Since finding a direct Bad Apple GIF link that is guaranteed to be small is tricky, 
    # we will use a reliable small GIF or prompt the user to provide one.
    
    # Let's download a small sample Bad Apple GIF (this is a known Tenor GIF, but tenor links expire).
    # Instead, we will ask the user to place a file named "bad_apple.gif" in the same folder.
    
    gif_path = "bad_apple.gif"
    if not os.path.exists(gif_path):
        print(f"Error: Khong tim thay file {gif_path}!")
        print("Vui long tai 1 file GIF hoat hinh (VD: bad apple loop) va doi ten thanh 'bad_apple.gif' de cung thu muc voi script nay.")
        sys.exit(1)

    print("Opening GIF...")
    img = Image.open(gif_path)
    
    target_width = 120
    target_height = 120
    
    frames = []
    
    for i, frame in enumerate(ImageSequence.Iterator(img)):
        if i >= 30: # Limit to 30 frames to save flash memory
            break
            
        # Convert to black and white, resize
        f = frame.copy().convert("L")
        f = f.resize((target_width, target_height), Image.Resampling.LANCZOS)
        f = f.point(lambda p: 255 if p > 128 else 0, mode='1')
        
        # Convert to byte array
        byte_array = []
        width, height = f.size
        
        for y in range(height):
            for x in range(0, width, 8):
                byte_val = 0
                for bit in range(8):
                    if x + bit < width:
                        pixel = f.getpixel((x + bit, y))
                        if pixel > 0: # White pixel
                            byte_val |= (1 << (7 - bit))
                byte_array.append(byte_val)
                
        frames.append(byte_array)
        
    print(f"Processed {len(frames)} frames.")
    
    with open("anime_frames.h", "w") as out:
        out.write("#ifndef ANIME_FRAMES_H\n#define ANIME_FRAMES_H\n\n")
        out.write("#include <Arduino.h>\n\n")
        out.write(f"const int ANIME_FRAME_WIDTH = {target_width};\n")
        out.write(f"const int ANIME_FRAME_HEIGHT = {target_height};\n")
        out.write(f"const int ANIME_FRAME_COUNT = {len(frames)};\n\n")
        
        for i, frame_data in enumerate(frames):
            out.write(f"const unsigned char PROGMEM anime_frame_{i}[] = {{\n")
            for j, b in enumerate(frame_data):
                out.write(f"0x{b:02X}, ")
                if (j + 1) % 16 == 0:
                    out.write("\n")
            out.write("};\n\n")
            
        out.write("const unsigned char* const anime_frames[] PROGMEM = {\n")
        for i in range(len(frames)):
            out.write(f"  anime_frame_{i},\n")
        out.write("};\n\n")
        
        out.write("#endif // ANIME_FRAMES_H\n")
        
    print("Da tao xong anime_frames.h!")

if __name__ == "__main__":
    generate_frames()
