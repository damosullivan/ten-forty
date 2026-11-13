"""Generate the 25x25 launcher menu icon: 10 over 40, hour bold, minutes regular.

Rendered from the same Iosevka files the face uses and thresholded to one bit so
the digits stay crisp at launcher size.

The digits sit on an opaque white tile rather than on transparency. The launcher
draws the icon into the row it is highlighting, and that row is not the same
colour on every platform — white on the rectangles, a dark bar on chalk and
gabbro, black on flint. Ink on transparency disappears on at least one of them,
and a one-bit icon with alpha came out either inverted or as a plain white box;
a filled tile reads the same everywhere and is how the stock Pebble icons are
drawn. Keep the background opaque.
"""
from PIL import Image, ImageDraw, ImageFont

ROOT = "/home/damien/src/ten-forty/ten-forty"
FONTS = ROOT + "/resources/fonts"
OUT = ROOT + "/resources/images/menu-icon.png"

TOP, BOTTOM, SIZE, GAP = "10", "40", 14, 2


def ink(text, weight):
    font = ImageFont.truetype("%s/font-%s.ttf" % (FONTS, weight), SIZE)
    canvas = Image.new("L", (SIZE * 4, SIZE * 3), 0)
    ImageDraw.Draw(canvas).text((SIZE, SIZE), text, font=font, fill=255)
    canvas = canvas.point(lambda v: 255 if v >= 128 else 0)
    return canvas.crop(canvas.getbbox())


def main():
    a, b = ink(TOP, "bold"), ink(BOTTOM, "regular")
    width = max(a.width, b.width)
    glyphs = Image.new("L", (width, a.height + GAP + b.height), 0)
    glyphs.paste(a, ((width - a.width) // 2, 0))
    glyphs.paste(b, ((width - b.width) // 2, a.height + GAP))

    icon = Image.new("RGBA", (25, 25), (255, 255, 255, 255))
    tile = Image.new("RGBA", glyphs.size, (0, 0, 0, 255))
    icon.paste(tile, ((25 - glyphs.width) // 2, (25 - glyphs.height) // 2), glyphs)
    icon.save(OUT)
    print("%s  %dx%d ink on 25x25" % (OUT, glyphs.width, glyphs.height))


if __name__ == "__main__":
    main()
