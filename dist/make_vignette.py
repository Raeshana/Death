#makes dist/vignette.png (256x240), using only Python's standard library.
#run from the Death folder:  python3 make_vignette.py

import math
import struct
import zlib

W = 256
H = 240
levels = [0, 30, 60, 95, 135, 175]  #alpha steps, darkest at the edges
bayer = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]

rows = b""
for y in range(H):
    row = bytearray([0])  #filter type 0 for this row
    for x in range(W):
        dx = (x + 0.5 - W / 2) / (W / 2)
        dy = (y + 0.5 - H * 0.52) / (H / 2)
        d = math.sqrt(dx * dx + dy * dy)
        t = (d - 0.55) / (1.35 - 0.55)
        t = max(0.0, min(1.0, t))
        f = t * (len(levels) - 1)
        lo = int(f)
        fr = f - lo
        if lo >= len(levels) - 1:
            a = levels[-1]
        elif fr * 16 > bayer[y % 4][x % 4] + 0.5:
            a = levels[lo + 1]
        else:
            a = levels[lo]
        row += bytes([5, 6, 10, a])
    rows += bytes(row)


def chunk(kind, data):
    out = struct.pack(">I", len(data)) + kind + data
    return out + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)


png = b"\x89PNG\r\n\x1a\n"
png += chunk(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 6, 0, 0, 0))
png += chunk(b"IDAT", zlib.compress(rows, 9))
png += chunk(b"IEND", b"")

with open("dist/vignette.png", "wb") as f:
    f.write(png)
print("wrote dist/vignette.png")
