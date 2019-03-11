import os, sys

''' ####### INSTRUCTIONS #######
Usage: python2 <image directory name> <num of classes>
Output: - will not create any labels if bounding box = [0,0,0,0]
        - creates individual train files for each image

'''

cwd = os.getcwd()

if len(sys.argv) == 3:
    directory_name = sys.argv[1]
    num_classes = int(sys.argv[2])
else:
    print("Usage python {image directory name} {num of classes}")
    exit()

choice = ''
# Remove pre existing labels
for files in os.listdir(os.path.join(cwd, directory_name)):
    if files.endswith(".txt"):
        while choice.lower() not in ['n', 'y']:
            choice = raw_input("Removing pre-exisitng combined labels?[y/n] ").lower()

            if choice.lower() == 'y':
                for files2 in os.listdir(os.path.join(cwd, directory_name)):
                    if files2.endswith(".txt"):
                        os.remove(os.path.join(cwd, directory_name, files2))
                break
            elif choice.lower() == 'n':    
                print("Script will not work with pre-existing labels")
                exit()
        
image_names = os.listdir(os.path.join(cwd, directory_name))
image_names = [i for i in image_names]
image_names = sorted(image_names)
# Labels are in YOLO format
labels = [[None] for i in range(num_classes)]

for i in range(num_classes):
    class_label_file = open(str(i)+'.txt','r')

    # label is a list of
    # ['0', '0', '0', '0', 'pathGateDice/00812.jpg']
    class_labels = [l.strip().split(' ') for l in class_label_file]
    for label in class_labels:
        for j in range(4):
            label[j] = float(label[j])

    # Assert the file still exists.
    # File not existing should never be possible, but in case it is
    to_remove = []
    for label in class_labels:
        if (not os.path.isfile(os.path.join(cwd, label[-1]))):
            print("could not find" + label[-1] + " did you remove it?")
            to_remove.append(label)
    for removed in to_remove:
        class_labels.remove(removed)
    
    labels[i] = class_labels
    class_label_file.close()

for i in range(len(image_names)):
    image_name = image_names[i]
    label_strings = []
    for class_num in range(num_classes):
        total = sum(labels[class_num][i][0:3])
        if total != 0.0:
            label_string = "{} {:.6f} {:.6f} {:.6f} {:.6f}\n".format(
                    class_num,
                    labels[class_num][i][0],
                    labels[class_num][i][1],
                    labels[class_num][i][2],
                    labels[class_num][i][3])
            label_strings.append(label_string)
    # remove the image if there are no labels
    if len(label_strings) == 0:
        print(image_name + ' has no label and is removed')
        os.remove(os.path.join(cwd, directory_name, image_name))
    else:
        label_file = open(os.path.join(cwd, directory_name, image_name.split('.')[0] + '.txt'),'w')
        for l in label_strings:
            label_file.write(l)
        label_file.close()

