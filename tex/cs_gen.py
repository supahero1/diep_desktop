from PIL import Image


def run():
	width = 256
	height = 256
	pixel_data = [(0, 0, 0)] * (width * height)


	for x in range(width):
		for y in range(height):
			pixel_data[y * width + x] = (x, x, x)

	image = Image.new("RGB", (width, height))
	image.putdata(pixel_data)
	image.save("tex/var/cs_b.png")


	for x in range(width):
		for y in range(height):
			pixel_data[y * width + x] = (x, 255, 255)

	image = Image.new("RGB", (256, 256))
	image.putdata(pixel_data)
	image.save("tex/var/cs_red.png")


	for x in range(width):
		for y in range(height):
			pixel_data[y * width + x] = (255, x, 255)

	image = Image.new("RGB", (256, 256))
	image.putdata(pixel_data)
	image.save("tex/var/cs_green.png")


	for x in range(width):
		for y in range(height):
			pixel_data[y * width + x] = (255, 255, x)

	image = Image.new("RGB", (256, 256))
	image.putdata(pixel_data)
	image.save("tex/var/cs_blue.png")


	a_pixel_data = [(0, 0, 0)] * (8 * 8)

	for x in range(8):
		x_xor = x & 1
		for y in range(8):
			y_xor = y & 1
			if x_xor ^ y_xor == 0:
				a_pixel_data[y * 8 + x] = (170, 170, 170)
			else:
				a_pixel_data[y * 8 + x] = (113, 113, 113)

	image = Image.new("RGB", (8, 8))
	image.putdata(a_pixel_data)
	image.save("tex/var/rect8_t.png")


	a_pixel_data = [(0, 0, 0)] * (128 * 128)

	for x in range(128):
		x_xor = x & 1
		for y in range(128):
			y_xor = y & 1
			if x_xor ^ y_xor == 0:
				a_pixel_data[y * 128 + x] = (170, 170, 170)
			else:
				a_pixel_data[y * 128 + x] = (113, 113, 113)

	image = Image.new("RGB", (128, 128))
	image.putdata(a_pixel_data)
	image.save("tex/var/rect128_t.png")


	for x in range(width):
		x_xor = (x // 4) & 1
		for y in range(height):
			y_xor = (y // 64) & 1
			if x_xor ^ y_xor == 0:
				pixel_data[y * width + x] = (170, 170, 170)
			else:
				pixel_data[y * width + x] = (113, 113, 113)

	image = Image.new("RGB", (width, height))
	image.putdata(pixel_data)
	image.save("tex/var/cs_t.png")


	image_path = "tex/const/circle.png"
	image = Image.open(image_path)
	image = image.convert("RGBA")
	pixels = list(image.getdata())

	for y in range(image.height):
		y_xor = (y // 64) & 1
		for x in range(image.width):
			x_xor = (x // 64) & 1
			idx = y * image.width + x
			if pixels[idx][3] != 0:
				if x_xor ^ y_xor == 0:
					pixels[idx] = (170, 170, 170)
				else:
					pixels[idx] = (113, 113, 113)

	result_image = Image.new("RGBA", image.size)
	result_image.putdata(pixels)
	result_image.save("tex/var/circle_t.png")


	pixels = [(0, 0, 0, 0)] * (width * height)

	for x in range(width):
		for y in range(height):
			pixels[y * width + x] = (255, 255, 255, x)

	image = Image.new("RGBA", (256, 256))
	image.putdata(pixels)
	image.save("tex/var/t_mask.png")


if __name__ == "__main__":
	run()
