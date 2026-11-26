#!/usr/bin/env python3
# Moemon Music Studio: the program's icon (the user's GREAT BALL, icon.png) -> src/icon.h (the window's icon,
# 64x64 RGBA for SDL_SetWindowIcon) and src/icon.ico (the Windows exe's, every size Explorer asks for, through
# src/icon.rc). Both are checked in, as the fonts are; run this again after changing icon.png. Needs Pillow.
import os
from PIL import Image

here = os.path.dirname(os.path.abspath(__file__))
src = Image.open(os.path.join(here, "icon.png")).convert("RGBA")
# square, centred, with the picture's own transparent edge
side = max(src.size)
sq = Image.new("RGBA", (side, side), (0, 0, 0, 0))
sq.paste(src, ((side - src.width) // 2, (side - src.height) // 2))

N = 64
small = sq.resize((N, N), Image.LANCZOS)
px = small.tobytes()
with open(os.path.join(here, "src", "icon.h"), "w") as f:
    f.write("// Moemon Music Studio: the window's icon, 64x64 RGBA (gen_icon.py from icon.png)\n")
    f.write("#define STUDIO_ICON_SIZE %d\n" % N)
    f.write("static const unsigned char k_studioIcon[%d] = {\n" % len(px))
    for i in range(0, len(px), 24):
        f.write("    " + ",".join(str(b) for b in px[i:i + 24]) + ",\n")
    f.write("};\n")
sq.save(os.path.join(here, "src", "icon.ico"), sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
print("src/icon.h, src/icon.ico")
