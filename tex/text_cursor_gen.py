from PIL import Image

def run():
	size = 256
	ratio = 6
	thickness = 12
	x_thickness = thickness * ratio
	pixel_data = [(0, 0, 0)] * (size * size)

	for y in range(0, size):
		for x in range(0, size):
			idx = y * size + x
			if y < thickness or y >= size - thickness or x < x_thickness or x >= size - x_thickness:
				pixel_data[idx] = (0, 0, 0)
			else:
				pixel_data[idx] = (255, 255, 255)

	image = Image.new("RGB", (size, size))
	image.putdata(pixel_data)
	image.save("tex/var/text_cursor.png")

if __name__ == "__main__":
	run()
