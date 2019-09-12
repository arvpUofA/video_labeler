# Video labeler

Video labeling software with KCF tracker and YOLO detector.
Default In/Out Label Format: YOLO  
yolo format: `class, centerX, centerY, box width, box height` (all in percentages of image size)  
*Note: Labeler assumes that all images are the same size

## Getting Started
#### Build
After downloading the repository, open the project in Qt Creator. Then apply the following configurations:
1. `cmake .`
2. `make`

Note: YOLO is very slow without a GPU. Edit the CMake file to enable GPU functionality if desired. This required CUDA >=10.

## Usage

`./VideoLabeler path_to_images {class_number}`

*Note: class_number must be a int and all classes must be continuous for combineLabels.py to work
*Note: dump a yolo.weights into ./weights, a yolo.cfg into ./cfg, and yolo.yaml into ./params to use assisted labelling with YOLO. If a .yaml file is not detected, YOLO will be disabled. The .yaml file must be named "yolo.yaml". The class number used to start the labeller must correspond to the class number in the .yaml file.


## Optional Usage

`./VideoLabeler path_to_images {class_number} {secondary_class_number}`

## Labeling Data

If the output_file specified already contains ROIs, these will be loaded up into the labeler.

Drawing ROIs is done clicking and holding the <kbd>LMB</kbd> on the image. This is enabled after pressing <kbd>w</kbd> or <kbd>r</kbd>

<kbd>r</kbd> Will draw an ROI on the current frame and start tracking for following frames. <b>If the following frames have ANY ROIs already set, those ROIs will be deleted.</b>

<kbd>w</kbd> Will draw an ROI but the following frames will not track. This does not affect any other frames.

The following keys will stop the tracking until <kbd>r</kbd> is used again: <kbd>w</kbd> <kbd>x</kbd> <kbd>b</kbd> <kbd>h</kbd>  
 
#### Recommended Controls
|       Key        |    Action    |
|:----------------:|:-------------|
| <kbd>y</kbd> <kbd>u</kbd>  | Increase/Decrease up delay between frames |
| <kbd>h</kbd> <kbd>j</kbd>    | Go backwards/forwards one frame |
| <kbd>r</kbd>     | Reset (destructive marking). Draws ROI on frame and starts tracker for following frames |
| <kbd>q</kbd>     | Use YOLO model to predict bound (single) |
| <kbd>a</kbd>     | Toggle YOLO prediction on each new frame (auto) |
| <kbd>t</kbd>     | Show/Hide info panel |
| <kbd>Esc</kbd>   | Finish labeling |
| <b>Key</b>       | <b>Action (These actions also stops the tracker on the next frame)</b> |
| <kbd>w</kbd>     | Non-destructive marking. Draws ROI on frame but stops tracker |
| <kbd>x</kbd>     | Mark frame without object |
| <kbd>b</kbd>     | Move to beginning |

#### Additional Controls
|       Key        |    Action    |
|:----------------:|:-------------|
| <kbd>space</kbd>             | Pause/Play |
| <kbd>z</kbd>     | Save current ROIs to file |

Note: try not to hold <kbd>j</kbd> (next frame) while YOLO auto labelling is enabled. The keyboard buffer build up, causing YOLO (and the GUI) to lag.

## Combining and checking labels

To combine all class labels into an individual file  

`python combineLabels {image_directory} {num_of_classes}`  

Once all labels are combined you can check the bounding boxes using  

`python checkLabels {image_directory}`  

Choose option 0 to check labels, and remove any wrongly labeled files.  Warning this will completely remove the image and it's associated labels.
Removing labels (as well as the images) will prevent the labeller from working on the existing images

## Generating train.txt file
    
`python checkLabels {image_directory}`  

Choose option 1

## Image generation

To generate images from a video, use ffmpeg `sudo apt-get install ffmpeg`:

Create directory to store images
`mkdir <image_folder_name>`

Generate images from video at 5hz
`ffmpeg -i <video file> -r 5 <image_folder_name>/<image_folder_name>%05d.jpg`
