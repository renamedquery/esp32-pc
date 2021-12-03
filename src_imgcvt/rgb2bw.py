import cv2, os, sys

IMAGE_DIR_PATH = sys.argv[-2]
IMAGE_OUTPUT_DIR_PATH = sys.argv[-1]
FILES_IN_DIR = os.listdir(IMAGE_DIR_PATH)
FRAMES_PER_FRAME_FILE = 16

for file_i in range(len(FILES_IN_DIR) // FRAMES_PER_FRAME_FILE):

    file_i_name = ("0" * ((9 - 0) - len(str(file_i)))) + str(file_i)

    output = open(IMAGE_OUTPUT_DIR_PATH + '/' + file_i_name + '.jpg.esp32binimg', 'wb')

    file = FILES_IN_DIR[file_i]

    for file_j in range(FRAMES_PER_FRAME_FILE):

        file = FILES_IN_DIR[(file_i * FRAMES_PER_FRAME_FILE) + file_j]

        print(IMAGE_DIR_PATH + '/' + file)

        image = cv2.imread(IMAGE_DIR_PATH + '/' + file, cv2.IMREAD_GRAYSCALE)

        image = cv2.resize(image, [image.shape[0] // 3, image.shape[1] // 3])

        eol = 2

        for y in range(image.shape[0]):

            lastval = -1
            lastval_count = 0
            lastval_written = 0

            for x in range(image.shape[1]):

                val = (image[y][x] / 255)

                if (val > .5):
                    val = 1
                else:
                    val = 0

                if (x == 0):
                    lastval = val
                
                if (val == lastval):
                    lastval_count += 1
                    lastval_written = 0
                else:
                    output.write(lastval.to_bytes(1, 'big'))
                    output.write(lastval_count.to_bytes(1, 'big'))
                    lastval_count = 0
                    lastval_written = 1
                
                lastval = val
            
            if (lastval_written == 0):

                output.write(lastval.to_bytes(1, 'big'))
                output.write(lastval_count.to_bytes(1, 'big'))
            
    output.close()