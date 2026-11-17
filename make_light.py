import math, struct, zlib
W = 96
H = 64
levels = [0, 40, 80, 120, 160]
bayer = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]
rows = b""
for y in range(H):
    row = bytearray([0])
    for x in range(W):
        dx = (x + 0.5 - W / 2) / (W / 2)
        dy = (y + 0.5 - H / 2) / (H / 2)
        d = math.sqrt(dx * dx + dy * dy)
        t = max(0.0, min(1.0, (1.0 - d) * 1.6))
        f = t * (len(levels) - 1)
        lo = int(f)
        fr = f - lo
        if lo >= len(levels) - 1:
            a = levels[-1]
        elif fr * 16 > bayer[y % 4][x % 4] + 0.5:
            a = levels[lo + 1]
        else:
            a = levels[lo]
        row += bytes([255, 252, 240, a])
    rows += bytes(row)
def chunk(kind, data):
    out = struct.pack(">I", len(data)) + kind + data
    return out + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
png = b"\x89PNG\r\n\x1a\n"
png += chunk(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 6, 0, 0, 0))
png += chunk(b"IDAT", zlib.compress(rows, 9))
png += chunk(b"IEND", b"")
with open("dist/light.png", "wb") as f:
    f.write(png)
print("wrote dist/light.png")
