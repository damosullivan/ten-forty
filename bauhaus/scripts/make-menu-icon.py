"""Generate the 25x25 launcher menu icon: the dial, at launcher size.

The icon is the face in miniature: a ring, ticks, the 12 triangle and two
batons at 10:10. It sits on an opaque white tile for the same reason Ten Forty's
does — the launcher highlights rows in different colours on different
platforms, and ink on transparency disappears on at least one of them. Keep the
background opaque.
"""
from PIL import Image, ImageDraw
import math
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "resources/images/menu-icon.png")
SIZE = 25
SS = 4  # supersample, then threshold

W = SIZE * SS


def px(v):
    return int(round(v * SS))


def main():
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    image = Image.new("RGBA", (W, W), (255, 255, 255, 255))
    draw = ImageDraw.Draw(image)
    black = (0, 0, 0, 255)

    cx = cy = SIZE / 2
    r = 10.0

    draw.ellipse([px(cx - r), px(cy - r), px(cx + r), px(cy + r)],
                 outline=black, width=SS)

    # Ticks: 1px wide at 1x, 6 and 9 a little longer, none at 3, triangle at 12.
    for i in range(1, 12):
        if i == 3:
            continue
        angle = math.radians(i * 30)
        inner = 5.0 if i in (6, 9) else 6.0
        outer = 9.0
        x1, y1 = cx + inner * math.sin(angle), cy - inner * math.cos(angle)
        x2, y2 = cx + outer * math.sin(angle), cy - outer * math.cos(angle)
        draw.line([px(x1), px(y1), px(x2), px(y2)], fill=black, width=SS)

    # Triangle: 4px wide, tip 2px inside the ring.
    tip = cy - 8.4
    base = cy - 5.6
    draw.polygon([(px(cx), px(tip)), (px(cx - 1.6), px(base)),
                  (px(cx + 1.6), px(base))], fill=black)

    # Hands at 10:10.
    def hand(angle_deg, length, width):
        a = math.radians(angle_deg)
        x = cx + length * math.sin(a)
        y = cy - length * math.cos(a)
        draw.line([px(cx), px(cy), px(x), px(y)], fill=black,
                  width=int(round(width * SS)))
        draw.ellipse([px(x - width / 2), px(y - width / 2),
                      px(x + width / 2), px(y + width / 2)], fill=black)

    hand(305, 5.0, 1.8)
    hand(60, 8.0, 1.0)
    draw.ellipse([px(cx - 1.2), px(cy - 1.2), px(cx + 1.2), px(cy + 1.2)],
                 fill=black)

    icon = image.resize((SIZE, SIZE), Image.LANCZOS)
    icon = icon.point(lambda v: 255 if v >= 128 else 0).convert("RGBA")
    # Thresholding drops the alpha channel to 0/255; rebuild the opaque tile.
    white = Image.new("RGBA", (SIZE, SIZE), (255, 255, 255, 255))
    white.paste(icon, (0, 0), icon)
    white.save(OUT)
    print("%s  %dx%d" % (OUT, SIZE, SIZE))


if __name__ == "__main__":
    main()
