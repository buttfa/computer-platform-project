with open("output.bin", "rb") as f:
    data = f.read()

# 每4个字节（即32位）一组：
for i in range(0, len(data), 4):
    chunk = data[i:i+4]
    bits = ''.join(format(b, '08b') for b in chunk)
    print(f"{i:08X}: {bits}")