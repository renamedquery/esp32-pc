import cv2, sys, numpy

INPUT_IMAGE_PATH = sys.argv[-2]
OUTPUT_IMAGE_PATH = sys.argv[-1]

image_file = open(INPUT_IMAGE_PATH, 'r')

image_file_contents = image_file.read()

image_size = image_file_contents.split('\n')[0].split(';')
image_size[0] = int(image_size[0])
image_size[1] = int(image_size[1])

output_image = numpy.zeros((image_size[1], image_size[0], 3), dtype = numpy.uint8)

for y in range(image_size[1]):

    row = image_file_contents.split('\n')[y + 1].split(';')

    for x in range(image_size[0]):

        pix_val_byte = row[x]

        # decode the bytes to get the actual non bw value
        r = pix_val_byte
        b = pix_val_byte
        g = pix_val_byte

        output_image[y][x] = [r, g, b]

image_file.close()

cv2.imwrite(OUTPUT_IMAGE_PATH, output_image)