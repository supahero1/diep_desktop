from PIL import Image

def run():
	size = 128
	pixel_data = [(0, 0, 0)] * (size * size)

	for y in range(0, size):
		for x in range(0, size):
			idx = y * size + x
			if (x >= 63 and x <= 64) or (y >= 63 and y <= 64):
				pixel_data[idx] = (0, 0, 0)
			else:
				pixel_data[idx] = (255, 255, 255)

	image = Image.new("RGB", (size, size))
	image.putdata(pixel_data)
	image.save("tex/var/bg_tile.png")

if __name__ == "__main__":
	run()
