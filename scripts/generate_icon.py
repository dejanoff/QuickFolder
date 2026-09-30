import os
from PIL import Image, ImageDraw

def create_quickfolder_icon(size):
    # RGBA image
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    scale = size / 64.0

    # Folder body coordinates
    # Back tab
    tab_x0, tab_y0 = int(6 * scale), int(10 * scale)
    tab_x1, tab_y1 = int(28 * scale), int(20 * scale)
    draw.rounded_rectangle([tab_x0, tab_y0, tab_x1, tab_y1], radius=int(3 * scale), fill=(218, 160, 40, 255))

    # Main folder back
    back_x0, back_y0 = int(6 * scale), int(16 * scale)
    back_x1, back_y1 = int(58 * scale), int(54 * scale)
    draw.rounded_rectangle([back_x0, back_y0, back_x1, back_y1], radius=int(4 * scale), fill=(235, 185, 60, 255))

    # Front folder flap
    front_x0, front_y0 = int(4 * scale), int(24 * scale)
    front_x1, front_y1 = int(60 * scale), int(56 * scale)
    draw.rounded_rectangle([front_x0, front_y0, front_x1, front_y1], radius=int(4 * scale), fill=(255, 208, 80, 255), outline=(200, 150, 30, 255), width=max(1, int(1.5 * scale)))

    # Inward arrow (pointing down-right into folder)
    # Circle badge in bottom-right corner
    badge_r = int(14 * scale)
    badge_cx = int(44 * scale)
    badge_cy = int(42 * scale)
    draw.ellipse([badge_cx - badge_r, badge_cy - badge_r, badge_cx + badge_r, badge_cy + badge_r], fill=(0, 120, 215, 255), outline=(255, 255, 255, 255), width=max(1, int(1.5 * scale)))

    # Downward/inward arrow inside badge
    arrow_color = (255, 255, 255, 255)
    aw = max(1, int(2.5 * scale))
    # Vertical line of arrow
    draw.line([badge_cx, badge_cy - int(7 * scale), badge_cx, badge_cy + int(5 * scale)], fill=arrow_color, width=aw)
    # Arrow head
    draw.line([badge_cx - int(5 * scale), badge_cy + int(1 * scale), badge_cx, badge_cy + int(6 * scale)], fill=arrow_color, width=aw)
    draw.line([badge_cx + int(5 * scale), badge_cy + int(1 * scale), badge_cx, badge_cy + int(6 * scale)], fill=arrow_color, width=aw)

    return img

def main():
    os.makedirs('res', exist_ok=True)
    sizes = [16, 24, 32, 48, 64, 128, 256]
    images = [create_quickfolder_icon(s) for s in sizes]
    images[0].save('res/QuickFolder.ico', format='ICO', sizes=[(s, s) for s in sizes], append_images=images[1:])
    print("Generated res/QuickFolder.ico successfully.")

if __name__ == '__main__':
    main()
