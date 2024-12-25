import numpy as np
import matplotlib.pyplot as plt
import colorsys

dimension = 2048
center = (dimension - 1) * 0.5

image = np.full((dimension, dimension, 4), 255, dtype=np.uint8)

for y in range(dimension):
	for x in range(dimension):
		distance = ((x - center) ** 2 + (y - center) ** 2) ** 0.5

		if distance > center:
			image[y, x] = [0, 0, 0, 0]
			continue

		hue = (np.arctan2(y - center, x - center) + np.pi) / (2 * np.pi)
		brightness = distance / center

		r, g, b = colorsys.hsv_to_rgb(hue, brightness, 1.0)
		r = int(r * 255)
		g = int(g * 255)
		b = int(b * 255)

		image[y, x] = [r, g, b, 255]

plt.imsave("tex/var/cs_hs.png", image)
