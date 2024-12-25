from PIL import Image

def run():
	size = 4
	pixel_data = [(255, 255, 255)] * (size * size)

	image = Image.new("RGB", (size, size))
	image.putdata(pixel_data)
	image.save("tex/var/rect.png")

if __name__ == "__main__":
	run()
