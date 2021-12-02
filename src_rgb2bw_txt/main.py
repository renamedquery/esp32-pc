import cv2, os, sys

IMAGE_DIR_PATH = sys.argv[-2]
IMAGE_OUTPUT_DIR_PATH = sys.argv[-1]
FILES_IN_DIR = os.listdir(IMAGE_DIR_PATH)

for file in FILES_IN_DIR:

    print(IMAGE_DIR_PATH + '/' + file)

    image = cv2.imread(IMAGE_DIR_PATH + '/' + file, cv2.IMREAD_GRAYSCALE)

    image = cv2.resize(image, [image.shape[0] // 3, image.shape[1] // 3])

    output = open(IMAGE_OUTPUT_DIR_PATH + '/' + file + '.esp32binimg', 'wb')

    eol = 2

    for y in range(image.shape[0]):

        for x in range(image.shape[1]):

            val = (image[y][x] / 255)

            if (val > .5):
                val = 1
            else:
                val = 0

            output.write(val.to_bytes(1, 'big'))
        
        output.write(eol.to_bytes(1, 'big'))
    
    output.close()