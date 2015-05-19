import sys
import numpy
import glob
import random
import cv2

def getFiles(path) :
  return glob.glob(str(path) + "/*.jpg")

def chooseRandom(files) :
  return random.choice(files)

def resizeToWidth(width, img) :
  return cv2.resize(img, (width, img.shape[0]))

def resizeChin(width, img) :
  imgWidth = img.shape[1]
  imgHeight = img.shape[0]
  blankSpaceWidth = (width - imgWidth) / 2
  chin = numpy.zeros((imgHeight, width, 3), numpy.uint8)
  chin[0:imgHeight, blankSpaceWidth:blankSpaceWidth+imgWidth] = img
  return chin

if __name__ == "__main__" :
  
  if len(sys.argv) != 6 :
    print "Usage: python concat-faces.py <num_faces> <foreheads_path> <noses_path> <chins_path> <output_path>"
    print "where: "
    print "- num_faces is the number of faces to generate,"
    print "- foreheads_path, noses_path and chins_path are paths to folders containing face parts,"
    print "- output_path is a place where generated faces would be saved."

  else :
    numFaces = sys.argv[1]

    foreheads = getFiles(sys.argv[2])
    noses = getFiles(sys.argv[3])
    chins = getFiles(sys.argv[4])
    outputPath = sys.argv[5]

    for i in range(0, int(numFaces)) :
      print "Generating face #{0}".format(str(i))
      forehead = cv2.imread(chooseRandom(foreheads))
      nose = cv2.imread(chooseRandom(noses))
      chin = cv2.imread(chooseRandom(chins))

      maxWidth = max(forehead.shape[1], nose.shape[1], chin.shape[1])

      foreheadResized = resizeToWidth(maxWidth, forehead)
      noseResized = resizeToWidth(maxWidth, nose)
      chinResized = resizeChin(maxWidth, chin)

      upperFace = numpy.concatenate((foreheadResized, noseResized), axis = 0)
      face = numpy.concatenate((upperFace, chinResized), axis = 0)

      cv2.imwrite(outputPath + "/face_{0}.jpg".format(str(i)), face)