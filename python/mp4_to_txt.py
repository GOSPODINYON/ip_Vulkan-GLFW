import cv2

VIDEO_FILE = "output.mp4"
OUT_TEXT = "decoded.txt"

cap = cv2.VideoCapture(VIDEO_FILE)
bits = ""

while True:
    ret, frame = cap.read()
    if not ret:
        break

    for y in range(frame.shape[0]):
        for x in range(frame.shape[1]):
            for c in range(3):
                bits += str(frame[y, x, c] & 1)

cap.release()

data = bytearray()
for i in range(0, len(bits), 8):
    byte = bits[i:i+8]
    if len(byte) < 8:
        break
    b = int(byte, 2)
    data.append(b)
    if data[-4:] == b"\x00\x00\x00\x00":
        break

with open(OUT_TEXT, "wb") as f:
    f.write(data[:-4])

print("TXT восстановлен из MP4")

