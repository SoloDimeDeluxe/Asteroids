"""Generates the original sprites for the Asteroids TP (all drawn from code)."""
import math
import random
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

OUT = Path(sys.argv[1])
OUT.mkdir(parents=True, exist_ok=True)
SS = 4  # supersampling factor for smooth edges


def finish(img, size, name):
    img = img.resize((size[0], size[1]), Image.LANCZOS)
    img.save(OUT / name)
    print("saved", name, img.size)


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(len(a)))


# ---------------------------------------------------------------- ship (points right)
def make_ship():
    W, H = 64 * SS, 64 * SS
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    s = SS

    def P(pts):
        return [(x * s, y * s) for x, y in pts]

    # back wings
    wing_top = P([(30, 24), (14, 6), (6, 8), (16, 26)])
    wing_bot = P([(30, 40), (14, 58), (6, 56), (16, 38)])
    for wing in (wing_top, wing_bot):
        d.polygon(wing, fill=(70, 90, 130, 255), outline=(170, 200, 240, 255), width=2 * s)

    # main hull
    hull = P([(62, 32), (40, 22), (16, 24), (10, 32), (16, 40), (40, 42)])
    d.polygon(hull, fill=(150, 165, 190, 255), outline=(230, 240, 255, 255), width=2 * s)

    # upper light stripe on the hull
    d.polygon(P([(58, 31), (40, 24), (20, 26), (20, 29), (40, 28)]), fill=(200, 212, 230, 255))

    # red side markings
    d.polygon(P([(22, 26), (30, 25), (28, 28), (22, 28)]), fill=(210, 60, 60, 255))
    d.polygon(P([(22, 38), (30, 39), (28, 36), (22, 36)]), fill=(210, 60, 60, 255))

    # cockpit
    d.ellipse(P([(34, 28), (48, 36)]), fill=(60, 200, 255, 255), outline=(20, 60, 110, 255), width=s)
    d.ellipse(P([(37, 29), (42, 31)]), fill=(200, 245, 255, 255))

    # engine nozzle
    d.rectangle(P([(8, 29), (12, 35)]), fill=(60, 60, 70, 255), outline=(150, 150, 160, 255), width=s)

    finish(img, (64, 64), "ship.png")


# ---------------------------------------------------------------- engine flame (points left)
def make_flame():
    W, H = 32 * SS, 16 * SS
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    layers = [
        ((255, 90, 20, 200), 1.00, 1.00),
        ((255, 170, 40, 230), 0.75, 0.70),
        ((255, 240, 160, 255), 0.45, 0.40),
    ]
    for color, length, thick in layers:
        d = ImageDraw.Draw(img)
        tip_x = W - W * length
        half = (H / 2) * thick
        pts = [(W, H / 2 - half), (W, H / 2 + half), (tip_x, H / 2)]
        d.polygon(pts, fill=color)
        d.ellipse([W - half * 1.2, H / 2 - half, W + half * 0.8, H / 2 + half], fill=color)
    img = img.filter(ImageFilter.GaussianBlur(SS * 0.8))
    finish(img, (32, 16), "ship_flame.png")


# ---------------------------------------------------------------- asteroids
def make_asteroid(name, seed, size):
    rng = random.Random(seed)
    W = H = size * SS
    cx = cy = W / 2
    base_r = W * 0.46
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # irregular rocky outline
    points = []
    vertices = 14
    for i in range(vertices):
        angle = 2 * math.pi * i / vertices
        r = base_r * rng.uniform(0.78, 1.0)
        points.append((cx + math.cos(angle) * r, cy + math.sin(angle) * r))

    mask = Image.new("L", (W, H), 0)
    ImageDraw.Draw(mask).polygon(points, fill=255)

    # shading: light from the top-left
    rock = Image.new("RGBA", (W, H))
    px = rock.load()
    light = (126, 112, 98)
    dark = (52, 45, 40)
    for y in range(H):
        for x in range(W):
            t = ((x - cx) + (y - cy)) / (2 * base_r) + 0.5
            t = min(max(t, 0.0), 1.0)
            noise = rng.uniform(-6, 6)
            c = lerp(light, dark, t)
            px[x, y] = (int(min(255, max(0, c[0] + noise))),
                        int(min(255, max(0, c[1] + noise))),
                        int(min(255, max(0, c[2] + noise))), 255)
    img.paste(rock, (0, 0), mask)

    d = ImageDraw.Draw(img)
    # craters (spread out: each one keeps distance from the others)
    craters = []
    attempts = 0
    while len(craters) < 5 and attempts < 500:
        attempts += 1
        r = base_r * rng.uniform(0.09, 0.18)
        dist = base_r * rng.uniform(0.1, 0.6)
        a = rng.uniform(0, 2 * math.pi)
        x, y = cx + math.cos(a) * dist, cy + math.sin(a) * dist
        if any(math.hypot(x - ox, y - oy) < (r + orr) * 1.4 for ox, oy, orr in craters):
            continue
        craters.append((x, y, r))
    for x, y, r in craters:
        d.ellipse([x - r, y - r, x + r, y + r], fill=(40, 34, 30, 255))
        d.ellipse([x - r * 0.75 + r * 0.2, y - r * 0.75 + r * 0.2, x + r * 0.75 + r * 0.2, y + r * 0.75 + r * 0.2],
                  fill=(78, 68, 60, 255))

    # cut craters that leak outside the rock shape, then draw the outline on top
    alpha = img.split()[3]
    img.putalpha(Image.composite(alpha, Image.new("L", (W, H), 0), mask))
    d = ImageDraw.Draw(img)
    line_width = max(2, int(1.5 * SS * size / 64))
    d.line(points + [points[0]], fill=(170, 155, 140, 255), width=line_width, joint="curve")

    finish(img, (size, size), name)


# ---------------------------------------------------------------- bullet (glow)
def make_bullet():
    W = H = 16 * SS
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    px = img.load()
    c = W / 2
    for y in range(H):
        for x in range(W):
            dist = math.hypot(x - c, y - c) / c
            if dist <= 1.0:
                core = max(0.0, 1.0 - dist / 0.35)
                glow = max(0.0, 1.0 - dist) ** 2
                r = int(120 + 135 * core)
                g = int(230 + 25 * core)
                b = 255
                a = int(min(255, 255 * (glow + core)))
                px[x, y] = (r, g, b, a)
    finish(img, (16, 16), "bullet.png")


# ---------------------------------------------------------------- crosshair cursor
def make_cursor():
    W = H = 32 * SS
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    c = W / 2
    color = (90, 255, 160, 255)
    r = 11 * SS
    lw = int(1.6 * SS)
    # circle in 4 arcs with gaps
    for start in (20, 110, 200, 290):
        d.arc([c - r, c - r, c + r, c + r], start, start + 50, fill=color, width=lw)
    # cross lines
    gap, length = 4 * SS, 15 * SS
    d.line([(c - length, c), (c - gap, c)], fill=color, width=lw)
    d.line([(c + gap, c), (c + length, c)], fill=color, width=lw)
    d.line([(c, c - length), (c, c - gap)], fill=color, width=lw)
    d.line([(c, c + gap), (c, c + length)], fill=color, width=lw)
    d.ellipse([c - 1.5 * SS, c - 1.5 * SS, c + 1.5 * SS, c + 1.5 * SS], fill=color)
    finish(img, (32, 32), "cursor.png")


make_ship()
make_flame()
make_asteroid("asteroid_large.png", 11, 128)
make_asteroid("asteroid_medium.png", 23, 64)
make_asteroid("asteroid_small.png", 37, 32)
make_bullet()
make_cursor()
