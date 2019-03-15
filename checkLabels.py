import numpy as np 
import cv2
import os, sys

class LabelChecker:
    def __init__(self, image_directory_name):
        self.cwd = os.getcwd()
        self.image_directory_name = image_directory_name

    def getImageFileList(self, directory_name):
        '''RETURNS A LIST OF ALL FILES IN /DATA DIR THAT DO NOT END IN .TXT'''
        image_file_list = os.listdir(os.path.join(self.cwd, directory_name))
        image_file_list = [i for i in image_file_list if not i.endswith('txt')]
        return image_file_list
    
    def loadImage(self, file_name, directory_name): 
        '''TAKES IN IMAGE file_name AND RETURNS IMAGE, OBJECT SIZE (HxW), 
        CLASSES AND LABELS'''
        directory = os.path.join(self.cwd, directory_name)
        image = cv2.imread(os.path.join(directory, file_name))
        sizeHW = [image.shape[0], image.shape[1]]
        text_file = file_name.split('.')[0]+'.txt'
        
        hasLabel = True
        try:
            label_file = open(os.path.join(directory, text_file))
        except:
            hasLabel = False
        
        if hasLabel:    
            labels = [i.strip() for i in label_file]
            labels = [i.split() for i in labels]
            labels = np.asarray(labels)
            classes = labels[:,0].astype(np.int)
            labels = labels[:,1:].astype(np.float)
            label_file.close()
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
    
    def checkLabels(self, directory_name):
        '''LOOPS THROUGH DATA IN YOUR DATA FOLDER GIVING A PREVIEW OF LABELED
        IMAGE ALLOWING YOU TO REMOVE IMPROPERLY LABELED IMAGES'''
        print('RUNNING THIS WILL MODIFY YOUR {images} folder(in same directory of this script)')
        file_list = self.getImageFileList(directory_name)
        option = -1
        while option not in [0, 1]:
            option = int(input('0:remove images and labels with space, any other key to continue \n'
                            '1:genrate train.txt:\n'
                            '[0/1]: '))    
        if option == 0:
            print('Press \'x\' to remove image, any other key to continue, ESC to quit')
        elif option == 1:
            all_label_file = open(os.getcwd() + '/train.txt','w')

        file_list.sort();
        directory = os.path.join(self.cwd, directory_name)
        for file_name in file_list:
            if option == 1:
                label_string = 'arvpData/images/train/'+file_name+'\n'
                all_label_file.write(label_string)
                continue

            img, sizeHW, classes, labels, = self.loadImage(file_name, directory_name)
            image_path = os.path.join(directory, file_name)
            print(image_path)
            
            key,img = self.drawBox(img, sizeHW, classes, labels, window=True)

            # label file to remove
            label_file = os.path.join(directory, file_name.split('.')[0]+'.txt')
            if (option == 0) and key == ord('x'): 
                if os.path.isfile(image_path) and os.path.isfile(label_file):
                    print(image_path + " has been removed")
                    os.remove(image_path)
                    os.remove(label_file)
                else:
                    print image_path + "not found"
            elif key == 27:
                break                        

        cv2.destroyAllWindows() 
        if option == 1:            
            all_label_file.close()
                 
def main():
    if len(sys.argv) > 1:
        image_directory_name = sys.argv[1]
    else:
        print("Usage python checkLabels.py {image_with_labels directoy name}\nMust be in same directory as this script")
        exit()
    labelChecker = LabelChecker(image_directory_name)
    labelChecker.checkLabels(directory_name=image_directory_name)    

main()