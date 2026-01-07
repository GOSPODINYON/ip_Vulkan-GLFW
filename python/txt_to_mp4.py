import cv2
import numpy as np

TEXT_FILE = "input.txt"
OUT_VIDEO = "output.mp4"
WIDTH, HEIGHT = 640, 480
FPS = 30

with open(TEXT_FILE, "rb") as f:
    data = f.read() + b"\x00\x00\x00\x00"  # EOF marker

bits = ''.join(f'{byte:08b}' for byte in data)

fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter(OUT_VIDEO, fourcc, FPS, (WIDTH, HEIGHT))

bit_index = 0
while bit_index < len(bits):
    frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)

    for y in range(HEIGHT):
        for x in range(WIDTH):
            for c in range(3):
                if bit_index < len(bits):
                    frame[y, x, c] |= int(bits[bit_index])
                    bit_index += 1

    out.write(frame)

out.release()
print("TXT закодирован в MP4")
