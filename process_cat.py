import os
from PIL import Image, ImageOps, ImageDraw

def process():
    img_path = r"C:\Users\phamt\.gemini\antigravity\brain\41947823-f92b-478d-86ff-1135266acf7d\media__1779987783548.png"
    if not os.path.exists(img_path):
        print("Image not found")
        return
        
    img = Image.open(img_path).convert("RGBA")
    
    # Kích thước màn hình
    TARGET_W = 240
    TARGET_H = 120
    
    # Crop hình con mèo (Cắt bỏ viền không cần thiết)
    # Ảnh gốc: 1024x636
    # Trọng tâm mèo nằm ở phần dưới
    cat_box = (150, 150, 950, 600)
    img_cropped = img.crop(cat_box)
    
    # Cắt tay phải (đang giơ lên có măng cụt)
    right_up_arm_box = (650 - 150, 250 - 150, 900 - 150, 500 - 150) # Box tương đối
    right_up_arm = img_cropped.crop(right_up_arm_box)
    
    # Cắt tay trái (đang đập xuống chuột)
    left_down_arm_box = (100 - 150, 300 - 150, 450 - 150, 600 - 150) # Box tương đối (cần chỉnh lại toạ độ tuyệt đối)
    left_down_arm_box = (150, 350, 450, 600) # Toạ độ trên img gốc
    
    # Lưu ra file test để check
    img_cropped.save("test_cropped.png")
    
if __name__ == "__main__":
    process()
