import os
import sys

''' ####### INSTRUCTIONS #######
Usage: python2 <image directory name> <num of classes>
Output: - will not create any labels if bounding box = [0,0,0,0]
	    - creates individual train files for each image

'''

cwd = os.getcwd()

if len(sys.argv) > 2:
    directoryName = sys.argv[1]
    num_classes = int(sys.argv[2])
else:
    print("Usage python <image directory name> <num of classes>")
    exit()

# Remove pre existing labels
for files in os.listdir(os.path.join(cwd, directoryName)):
    if files[-4:] == ".txt":

        choice = raw_input("Removing pre-exisitng combined labels,\
                   \n[n] to cancel or press any key to continue: ").lower()

        if choice != 'n':
            for files2 in os.listdir(os.path.join(cwd, directoryName)):
                if files2[-4:] == ".txt":
                    os.remove(os.path.join(cwd, directoryName, files2))
            break
        else:    
            print("Script will not work if pre-existing labels")
            exit()
        
images = os.listdir(os.path.join(cwd, directoryName))
images = [i.split('.')[0] for i in images]
images = sorted(images)
# Labels are in YOLO format
labels = [[None] for i in range(num_classes)]

for i in range(num_classes):
    cur_class = open(str(i)+'.txt','r')
    label = [c.strip() for c in cur_class]
    label = [l.split(' ') for l in label]
    for line in label:
        for ii in range(4):
            line[ii] = float(line[ii])

    # Assert the file still exists
    to_remove = []
    for line in label:
        if (not os.path.isfile(os.path.join(cwd, line[-1]))):
	    print("could not find" + line[-1] + " did you remove it?")
            to_remove.append(line)
    for removed in to_remove:
	label.remove(removed)
    labels[i] = label
    cur_class.close()
    


for i in range(len(images)):
    filename = images[i]+'.txt'
    label = open(os.path.join(cwd, directoryName, images[i]+'.txt'),'w')
    for class_num in range(num_classes):
        total = sum(labels[class_num][i][0:3])
        if total != 0.0:
            string = "{} {:.6f} {:.6f} {:.6f} {:.6f}".format(
                    class_num,
                    labels[class_num][i][0],
                    labels[class_num][i][1],
                    labels[class_num][i][2],
                    labels[class_num][i][3])
            label.write(string+'\n')
    label.close()
