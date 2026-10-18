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
    # sprite sheet: 4 frames side by side (32x16 each), the flame flickers between them
    frame_w, frame_h = 32 * SS, 16 * SS
    frames = 4
    sheet = Image.new("RGBA", (frame_w * frames, frame_h), (0, 0, 0, 0))
    # (length, thickness) changes per frame so the flame "breathes"
    frame_shapes = [(1.00, 1.00), (0.80, 0.90), (0.92, 1.05), (0.70, 0.85)]
    for index, (length_scale, thick_scale) in enumerate(frame_shapes):
        img = Image.new("RGBA", (frame_w, frame_h), (0, 0, 0, 0))
        layers = [
            ((255, 90, 20, 200), 1.00, 1.00),
            ((255, 170, 40, 230), 0.75, 0.70),
            ((255, 240, 160, 255), 0.45, 0.40),
        ]
        W, H = frame_w, frame_h
        for color, length, thick in layers:
            d = ImageDraw.Draw(img)
            tip_x = W - W * length * length_scale
            half = (H / 2) * thick * thick_scale * 0.9
            pts = [(W, H / 2 - half), (W, H / 2 + half), (tip_x, H / 2)]
            d.polygon(pts, fill=color)
            d.ellipse([W - half * 1.2, H / 2 - half, W + half * 0.8, H / 2 + half], fill=color)
        img = img.filter(ImageFilter.GaussianBlur(SS * 0.8))
        sheet.paste(img, (index * frame_w, 0))
    finish(sheet, (32 * frames, 16), "ship_flame.png")


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


# ---------------------------------------------------------------- bomb power-up
def make_powerup_bomb():
    W = H = 48 * SS
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    c = W / 2

    # orange glow behind the bomb so it stands out from the asteroids
    glow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    gd = ImageDraw.Draw(glow)
    gd.ellipse([c - 21 * SS, c - 19 * SS, c + 21 * SS, c + 23 * SS], fill=(255, 140, 30, 150))
    img.alpha_composite(glow.filter(ImageFilter.GaussianBlur(5 * SS)))

    d = ImageDraw.Draw(img)
    body_cx, body_cy, r = c, c + 4 * SS, 14 * SS

    # bomb body with a light from the top-left
    d.ellipse([body_cx - r, body_cy - r, body_cx + r, body_cy + r], fill=(40, 40, 52, 255), outline=(230, 235, 255, 255), width=int(1.5 * SS))
    d.ellipse([body_cx - r * 0.6, body_cy - r * 0.65, body_cx - r * 0.1, body_cy - r * 0.2], fill=(120, 125, 150, 255))

    # cap and fuse
    d.rectangle([body_cx - 4 * SS, body_cy - r - 4 * SS, body_cx + 4 * SS, body_cy - r + 2 * SS], fill=(150, 150, 165, 255), outline=(230, 235, 255, 255), width=SS)
    d.line([(body_cx + 1 * SS, body_cy - r - 4 * SS), (body_cx + 6 * SS, body_cy - r - 10 * SS), (body_cx + 11 * SS, body_cy - r - 11 * SS)], fill=(200, 170, 120, 255), width=int(1.5 * SS))

    # spark at the end of the fuse
    sx, sy = body_cx + 12 * SS, body_cy - r - 12 * SS
    for angle in range(0, 360, 45):
        a = math.radians(angle)
        length = (5 if angle % 90 == 0 else 3) * SS
        d.line([(sx, sy), (sx + math.cos(a) * length, sy + math.sin(a) * length)], fill=(255, 230, 90, 255), width=SS)
    d.ellipse([sx - 2 * SS, sy - 2 * SS, sx + 2 * SS, sy + 2 * SS], fill=(255, 255, 220, 255))

    finish(img, (48, 48), "powerup_bomb.png")


# ---------------------------------------------------------------- enemy UFO (flying saucer)
def make_ufo():
    W, H = 72 * SS, 40 * SS
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx = W / 2

    # glass dome
    d.ellipse([cx - 14 * SS, 3 * SS, cx + 14 * SS, 27 * SS], fill=(90, 230, 160, 230), outline=(200, 255, 220, 255), width=SS)
    d.ellipse([cx - 8 * SS, 7 * SS, cx - 1 * SS, 13 * SS], fill=(220, 255, 235, 255))

    # saucer body
    d.ellipse([3 * SS, 15 * SS, W - 3 * SS, 33 * SS], fill=(120, 60, 150, 255), outline=(230, 190, 255, 255), width=int(1.5 * SS))
    d.ellipse([10 * SS, 15 * SS, W - 10 * SS, 24 * SS], fill=(170, 100, 200, 255))

    # bottom part
    d.ellipse([cx - 16 * SS, 26 * SS, cx + 16 * SS, 37 * SS], fill=(80, 40, 100, 255), outline=(230, 190, 255, 255), width=SS)

    # lights around the body
    for i in range(5):
        x = 14 * SS + i * (W - 28 * SS) / 4
        d.ellipse([x - 2.5 * SS, 24 * SS - 2.5 * SS, x + 2.5 * SS, 24 * SS + 2.5 * SS], fill=(255, 230, 90, 255))

    finish(img, (72, 40), "ufo.png")


# ---------------------------------------------------------------- enemy bullet (red glow)
def make_enemy_bullet():
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
                px[x, y] = (255, int(70 + 150 * core), int(60 + 150 * core), int(min(255, 255 * (glow + core))))
    finish(img, (16, 16), "enemy_bullet.png")


# ---------------------------------------------------------------- explosion (8-frame animation)
def make_explosion():
    frame = 64 * SS
    frames = 8
    rng = random.Random(99)
    sheet = Image.new("RGBA", (frame * frames, frame), (0, 0, 0, 0))
    c = frame / 2

    # fixed random layout shared by all frames, so the animation moves smoothly
    puffs = [(rng.uniform(0, 2 * math.pi), rng.uniform(0.0, 1.0), rng.uniform(0.55, 1.0)) for _ in range(9)]
    sparks = [(rng.uniform(0, 2 * math.pi), rng.uniform(0.7, 1.0)) for _ in range(12)]
    debris = [(rng.uniform(0, 2 * math.pi), rng.uniform(0.6, 1.0)) for _ in range(8)]

    fire_colors = [(255, 255, 235), (255, 235, 130), (255, 160, 45), (225, 70, 30), (110, 60, 50)]

    def fire_color(t):
        pos = min(t * (len(fire_colors) - 1), len(fire_colors) - 1.001)
        i = int(pos)
        return lerp(fire_colors[i], fire_colors[i + 1], pos - i)

    for index in range(frames):
        t = index / (frames - 1)              # 0 = start, 1 = end
        fade = 1.0 - t
        img = Image.new("RGBA", (frame, frame), (0, 0, 0, 0))
        d = ImageDraw.Draw(img)

        # smoke puffs (late frames): grey clouds that spread and fade
        if t > 0.3:
            smoke_t = (t - 0.3) / 0.7
            for angle, offset, size in puffs:
                dist = frame * (0.10 + 0.22 * smoke_t) * offset
                x, y = c + math.cos(angle) * dist, c + math.sin(angle) * dist
                r = frame * 0.13 * size * (0.8 + 0.6 * smoke_t)
                d.ellipse([x - r, y - r, x + r, y + r], fill=(80, 72, 70, int(170 * (1 - smoke_t))))

        # rock debris flying away (under the fire, so it only shows once it leaves the fireball)
        for angle, speed in debris:
            dist = frame * 0.46 * t * speed
            x, y = c + math.cos(angle) * dist, c + math.sin(angle) * dist
            r = frame * 0.03 * (1.1 - t * 0.6)
            d.ellipse([x - r, y - r, x + r, y + r], fill=(150, 130, 110, int(255 * fade)))

        # fireball made of puffs: grows fast, then shrinks and darkens
        growth = math.sin(min(1.0, t * 1.6) * math.pi / 2)
        shrink = 1.0 - max(0.0, t - 0.5) * 1.2
        for angle, offset, size in puffs:
            dist = frame * 0.15 * growth * offset
            x, y = c + math.cos(angle) * dist, c + math.sin(angle) * dist
            r = frame * 0.19 * size * (0.4 + 0.6 * growth) * max(0.15, shrink)
            col = fire_color(min(1.0, t * 1.1 + offset * 0.15))
            d.ellipse([x - r, y - r, x + r, y + r], fill=col + (int(255 * fade ** 0.6),))

        # hot white core in the first frames
        if t < 0.45:
            core_r = frame * 0.12 * (1 - t / 0.45)
            d.ellipse([c - core_r, c - core_r, c + core_r, c + core_r], fill=(255, 255, 245, 255))

        # sparks: bright streaks shooting outwards (early frames)
        if t < 0.6:
            for angle, speed in sparks:
                inner = frame * (0.08 + 0.55 * t) * speed
                outer = inner + frame * 0.10 * (1 - t)
                d.line([(c + math.cos(angle) * inner, c + math.sin(angle) * inner),
                        (c + math.cos(angle) * outer, c + math.sin(angle) * outer)],
                       fill=(255, 230, 140, int(255 * (1 - t / 0.6))), width=int(1.6 * SS))

        img = img.filter(ImageFilter.GaussianBlur(SS * 0.6))
        sheet.paste(img, (index * frame, 0))
    finish(sheet, (64 * frames, 64), "explosion.png")

make_ship()
make_flame()
make_asteroid("asteroid_large.png", 11, 128)
make_asteroid("asteroid_medium.png", 23, 64)
make_asteroid("asteroid_small.png", 37, 32)
make_bullet()
make_cursor()
make_powerup_bomb()
make_ufo()
make_enemy_bullet()
make_explosion()
