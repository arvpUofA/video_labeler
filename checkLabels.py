import numpy as np 
import cv2
import os, sys
from pathlib import Path

class LabelChecker:
    def __init__(self):
        self.rootPath = Path.cwd()
        self.directories = ['badData', 'goodData']
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
        labels = (self.dirDict[path] / textFile).open()
        labels = [i.strip() for i in labels]
        labels = [i.split() for i in labels]
        labels = np.asarray(labels)
        classes = labels[:,0].astype(np.int)
        labels = labels[:,1:].astype(np.float)
        return image, sizeHW, classes, labels
    
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
        IMAGE ALLOWING YOU TO REMOVE IMPROPERLY LABELED IMAGES IF MOVE IS TRUE
        IT WILL MOVE CHECKED DATA INTO SORTED FOLDERS IN ROOT DIRECTORY'''
        print('Press ESC to quit')
        filelist = self.getImageFileList(path)
        move = input('Do you want to seperate sorted data? (Y/N): ').lower()     
        if move == 'y':
            try:
                self.dirDict['goodData'].mkdir()
            except Exception as e:
                print(e.args[0])
            try:
                self.dirDict['badData'].mkdir()
            except Exception as e:
                print(e.args[0])
            print('Press Spacebar to move image into badData dir')
            print('Press any other key to keep data')
        
        for fileName in filelist:
            img, sizeHW, classes, labels, = self.loadImage(fileName, path)
            key,img = self.drawBox(img, sizeHW, classes, labels, window=True)
            image = self.dirDict[path] / fileName
            label = self.dirDict[path] / (fileName.split('.')[0]+'.txt')
            
            if key == 32 and move == 'y':
                print('Removing', fileName)
                image.replace(self.dirDict['badData'] / image.name)
                label.replace(self.dirDict['badData'] / label.name)
            elif key == 27:
                break
            elif move =='y':
                image.replace(self.dirDict['goodData'] / image.name)
                label.replace(self.dirDict['goodData'] / label.name)
        cv2.destroyAllWindows() 
        
                 
def main():
    labelChecker = LabelChecker()

    directoryName = 'images'    
    if len(sys.argv) > 1:
        directoryName = sys.argv[1]

    labelChecker.dirDict[directoryName] = labelChecker.rootPath / directoryName

    if labelChecker.dirDict[directoryName].is_dir():
        labelChecker.checkLabels(path=directoryName)
            
main()
