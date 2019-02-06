import numpy as np 
import cv2
import os, sys
from pathlib import Path

class LabelChecker:
    def __init__(self):
        self.rootPath = Path.cwd()
        self.directories = []
        self.dirDict = {}
        for i in self.directories:
            self.dirDict[i] = self.rootPath / i

    def getImageFileList(self, path):
        '''RETURNS A LIST OF ALL FILES IN /DATA DIR THAT DO NOT END IN .TXT'''
        imageFileList = os.listdir(self.dirDict[path].as_posix())
        imageFileList = [i for i in imageFileList if i[-3:] != 'txt']
        return imageFileList
    
    def loadImage(self, fileName, path): 
        '''TAKES IN IMAGE FILENAME AND RETURNS IMAGE, OBJECT SIZE (HxW), 
        CLASSES AND LABELS'''
        image = cv2.imread((self.dirDict[path] / fileName).as_posix())
        sizeHW = [image.shape[0], image.shape[1]]
        textFile = fileName.split('.')[0]+'.txt'
        
        hasLabel = True
        try:
            labels = (self.dirDict[path] / textFile).open()
        except:
            hasLabel = False

        if hasLabel:    
            labels = [i.strip() for i in labels]
            labels = [i.split() for i in labels]
            labels = np.asarray(labels)
            classes = labels[:,0].astype(np.int)
            labels = labels[:,1:].astype(np.float)

            return image, sizeHW, classes, labels
        else:
            return image, sizeHW, [], []

    def drawBox(self, img, sizeHW, classes, labels, window):
        '''TAKES IN LABELS IN YOLO FORMAT AND CREATES ANNOTATED IMAGE'''
        font = cv2.FONT_HERSHEY_SIMPLEX
        for i in range(len(classes)):
            x = (labels[i][0] - labels[i][2]/2) * sizeHW[1]
            y = (labels[i][1] - labels[i][3]/2) * sizeHW[0]
            w = labels[i][2] * sizeHW[1]
            h = labels[i][3] * sizeHW[0]
            cv2.rectangle(img,(int(x),int(y)),(int(x+w),
                           int(y+h)),(0,0,255),2)
            cv2.putText(img,str(int(classes[i])),(int(x + w/2),
                            int(y+h/2)), font, 1,(0,0,255),
                            1,cv2.LINE_AA)
        if window:
            cv2.imshow("annotated_img", img)
            key = cv2.waitKey(0)
            return key, img
        else:
            return img
    
    def checkLabels(self, path):
        '''LOOPS THROUGH DATA IN YOUR DATA FOLDER GIVING A PREVIEW OF LABELED
        IMAGE ALLOWING YOU TO REMOVE IMPROPERLY LABELED IMAGES'''
        print('RUNNING THIS WILL MODIFY YOUR {images} folder(in same directory of this script)')
        filelist = self.getImageFileList(path)
        move = int(input('0:remove labels with no classes\n'
                         '1:remove images and labels with space, any other key to continue \n'
                         '2:genrate train.txt:\n'
                         '[0/1/2]: '))    
        if move == 0:
            print('Hold space bar down, all images with labels of 0 will be removed')
        elif move == 1:
            print('Press space bar to remove image, any other key to continue, ESC to quit')
        elif move == 2:
            allLabelFile = open(os.getcwd() + '/train.txt','w')

        filelist.sort();
        for fileName in filelist:
            if move == 2:
                allLabelString = 'arvpData/images/train/'+fileName+'\n'
                allLabelFile.write(allLabelString)
                continue
            img, sizeHW, classes, labels, = self.loadImage(fileName, path)
            print fileName
            imagePath = str(self.dirDict[path] / fileName)
            
            if (move == 0): 
                if len(classes) == 0:
                    if os.path.isfile(imagePath):
                        os.remove(imagePath)
                    else:
                        print(imagePath, "not found")
                continue
            else:
                key,img = self.drawBox(img, sizeHW, classes, labels, window=True)


            # label to remove
            label = str(self.dirDict[path] / (fileName.split('.')[0]+'.txt'))
            if (move == 1) and key == 32: 
                if os.path.isfile(imagePath) and os.path.isfile(label):
                    os.remove(imagePath)
                    os.remove(label)
                else:
                    print(imagePath, "not found")
            elif key == 27:
                break                        

        cv2.destroyAllWindows() 
        if move == 2:            
            allLabelFile.close()
                 
def main():
    labelChecker = LabelChecker()

    directoryName = 'images'    
    if len(sys.argv) > 1:
        directoryName = sys.argv[1]
    else:
        print("Usage python {imageWithLabels directoy name}\nMust be in same directory as this script")
        exit()
    labelChecker.dirDict[directoryName] = labelChecker.rootPath / directoryName

    if labelChecker.dirDict[directoryName].is_dir():
        labelChecker.checkLabels(path=directoryName)
            
main()
