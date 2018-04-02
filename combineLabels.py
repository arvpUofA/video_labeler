import os

''' ####### INSTRUCTIONS #######
- this script assumes the image files are sorted via their name
- place images to label in folder called images
- begin create labels files starting with 0.txt and incrementing by one
  for eah new label
- run python combineLabels.py
- rename images folder, delete 0-[num_classes].txt from root dir
- begin again
- to sanity check data move into data folder in rotation_augment script
  root folder: https://github.com/wagonhelm/rotation_augment
'''

cwd = os.getcwd()
images = os.listdir(cwd+'/images')
images = [i.split('.')[0] for i in images]
images = sorted(images)
# X width, Y height
img_size = [1920,1080]
num_classes = 6

def convert_2_yolo(coords):
    # Yolo labels are [center x, center y, width height] w.r.t to image size
    x_center = coords[0] + coords[2]/2
    y_center = coords[1] + coords[3]/2
    coords[0] = x_center / img_size[0]
    coords[1] = y_center / img_size[1]
    coords[2] = coords[2] / img_size[0]
    coords[3] = coords[3] / img_size[1]

# Labels are in Top Left X, Top Left Y, Box Width, Box Height
labels = [[None] for i in range(num_classes)]

for i in range(num_classes):
    cur_class = open(str(i)+'.txt','r')
    label = [i.strip() for i in cur_class]
    label = [i.split(' ') for i in label]
    for line in label:
        for ii in range(4):
            line[ii] = int(line[ii])
    labels[i] = label
    cur_class.close()
     
for i in labels:
    for line in i:
        line = convert_2_yolo(line)

for i in range(len(images)):
    label = open(cwd+'/images/'+images[i]+'.txt','w')
    for class_num in range(num_classes):
        string = "{} {:.6f} {:.6f} {:.6f} {:.6f}".format(
                  class_num,
                  labels[class_num][i][0],
                  labels[class_num][i][1],
                  labels[class_num][i][2],
                  labels[class_num][i][3])
        label.write(string+'\n')
    label.close()
