import cv2, sys, numpy

INPUT_IMAGE_PATH = sys.argv[-2]
OUTPUT_IMAGE_PATH = sys.argv[-1]

image_file = open(INPUT_IMAGE_PATH, 'r')

image_file_contents = image_file.read()

image_size = image_file_contents.split('\n')[0].split(';')
image_size[0] = int(image_size[0])
image_size[1] = int(image_size[1])

output_image = numpy.zeros((image_size[1], image_size[0], 1), dtype = numpy.uint8)

output_image_pixdata = image_file_contents.split('\n')[1].split(';')

x, y = 0, 0

for pair in range(len(output_image_pixdata) // 2 // 2):

    pix_value = int(output_image_pixdata[(pair * 2) + 0])
    pix_run_length = int(output_image_pixdata[(pair * 2) + 1])

    for pix_run in range(pix_run_length):

        if ((x) >= (image_size[0] / 2)):

            x = 0
            y += 1

        output_image[y][(x * 2) + 0] = pix_value
        output_image[y][(x * 2) + 1] = pix_value

        x += 1
    
    x += 1

image_file.close()

cv2.imwrite(OUTPUT_IMAGE_PATH, output_image)