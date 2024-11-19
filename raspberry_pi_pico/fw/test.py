
value = [[0.6, 0.1, 0.3] for _ in range(25)]
max_brightness = 31
_brightness_bits = int(0.5 * max_brightness)
start_of_frame = [0]*4
end_of_frame = [0]*5
             # SSSBBBBB (start, brightness)
brightness = 0b11100000 | _brightness_bits
pixels = [[int(255*v) for v in p] for p in value]
pixels = [[brightness, b, g, r] for r, g, b in pixels]
pixels = [i for p in pixels for i in p]
data = start_of_frame + pixels + end_of_frame
max_line_length = 16
while len(data) > 0:
    line = data[:max_line_length]
    data = data[max_line_length:]
    print('"' + ''.join(f'\\x{d:02x}' for d in line) + '"')

