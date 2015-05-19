import cv2
import sys
import glob
import sys
import os

def createOutputFoler(folderName) :
  print "Creating " + folderName
  if not os.path.exists(folderName) :
    os.makedirs(folderName)
  return folderName

if __name__ == "__main__" :
  
  if len(sys.argv) != 3 :
    print "Usage: python face_extractor.py <path_to_photos> <output_path>"
  
  else :
    # Haar classifiers were taken from http://alereimondo.no-ip.org/OpenCV/34
    mouthCascade = cv2.CascadeClassifier("./classifiers/Mouth.xml")
    eyesCascade = cv2.CascadeClassifier("./classifiers/parojos.xml")

    files = glob.glob(str(sys.argv[1]) + "/*.jpg")
    outputPath = sys.argv[2]

    nosesOutputFolder = createOutputFoler(outputPath + "/noses")
    mouthsOutputFolder = createOutputFoler(outputPath + "/mouths")
    foreheadsOutputFolder = createOutputFoler(outputPath + "/foreheads")

    mouthNumber = 0
    fileNumber = 0

    for file in files:
      image = cv2.imread(file)
      gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

      mouths = mouthCascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5,
        minSize=(30, 30),
      )
      eyes = eyesCascade.detectMultiScale (
        gray,
        scaleFactor=1.1,
        minNeighbors=5
      )

      print "Found {0} eyes and {1} mouths".format(len(eyes), len(mouths))

      mouthLeft = 5
      mouthRight = 5
      eyeTop = 5
      eyeBottom = 5

      for (x, y, w, h) in eyes :
        noseEyeImage = image[y-eyeTop:y+(2.5*h)+eyeBottom, x-(0.25*w):x+(1.25*w)]
        imageTop = max(0, y-(4*h))
        foreheadHairImage = image[imageTop:y-eyeTop, x-(0.25*w):x+(1.25*w)]
        fileNumber += 1
        cv2.imwrite(nosesOutputFolder + "/eye_nose_{0}.jpg".format(str(fileNumber)), noseEyeImage)
        cv2.imwrite(foreheadsOutputFolder + "/forehead_{0}.jpg".format(str(fileNumber)), foreheadHairImage)

      for (x, y, w, h) in mouths :
        image = image[y:y+1.5*h, x-0.7*w:x+1.7*w]
        mouthNumber += 1
        cv2.imwrite(mouthsOutputFolder + "/mouth_{0}.jpg".format(str(mouthNumber)), image)