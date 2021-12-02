import cv2, os, sys

IMAGE_DIR_PATH = sys.argv[-2]
IMAGE_OUTPUT_DIR_PATH = sys.argv[-1]
FILES_IN_DIR = os.listdir(IMAGE_DIR_PATH)

for file in FILES_IN_DIR:

    print(file)

    image = cv2.imread(IMAGE_DIR_PATH + '/' + file, cv2.IMREAD_GRAYSCALE)

    output_string = "{};{}\n".format(image.shape[1], image.shape[0])

    for y in range(image.shape[0]):

        for x in range(image.shape[1]):

            val = (image[y][x] / 255)

            if (val > .5):
                val = 1
            else:
                val = 0

            output_string += "{};".format(val)
    
    output = open(IMAGE_OUTPUT_DIR_PATH + '/' + file + '.txt', 'w')
    output.write(output_string + '\n')
    output.close()