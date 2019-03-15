# Video labeler

Video labeling software with KCF tracker.  
Default In/Out Label Format: YOLO  
Compatible ROI Formats: yolo, cv  
yolo format: `class, centerX, centerY, box width, box height` (all in percentages of image size)  
cv format: `top left X, top left Y, width, height` (all in pixel values)  
*Note: Labeler assumes that all images are the same size

## Getting Started
#### Requirements
* Qt 5.X.X (comes with Qt Creator download)
* OpenCV 3.X.X
* [QT Creator](https://www.qt.io/download)

After downloading the repository, open the project in Qt Creator. Then apply the following configurations:
1. Change the Qt version to 5.X.X in `Projects > Manage Kits... > Build & Run > Desktop` (where Desktop is usually the default kit)
2. Edit the video_labeler.pro file so that INCLUDEPATH adds your opencv3 include directories. For the opencv3 version that comes with kinetic, this is `/opt/ros/kinetic/include/<OPEN_CV_VERSION>/opencv`. Check your directories to include the correct version.
3. Build
4. Run the `video_labeler` executable in the build directory according to the usage guide

## Usage

`./video_labeler path_to_images {class_number}`

*Note: class_number must be a int and all classes must be continuous for combineLabels.py to work

## Optional Usage

`./video_labeler path_to_images {class_number} {input_format} {output_format}`

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
| <kbd>t</kbd>     | Show/Hide info panel |
| <kbd>Esc</kbd>   | Finish labeling |
| <b>Key</b>       | <b>Action (These actions also stops the tracker on the next frame)</b> |
| <kbd>h</kbd>     | Go backwards |
| <kbd>w</kbd>     | Non-destructive marking. Draws ROI on frame but stops tracker |
| <kbd>x</kbd>     | Mark frame without object |
| <kbd>b</kbd>     | Move to beginning |


<i>Interpolation is not recommended in the current video labeler state</i>

The general principle of the application is to manually draw ROIs as little as
possible. This can be done by using _keyframes_ wherein the KCF tracker attempt
to interpolate ROIs in the intervals between them. 

<i>Interpolation is not recommended in the current video labeler state</i>

#### Additional Controls
|       Key        |    Action    |
|:----------------:|:-------------|
| <kbd>space</kbd>             | Pause/Play |
| <kbd>k</kbd>     | Mark frame as keyframe |
| <kbd>i</kbd>     | Interpolates interval between keyframes, overrides tracking |
| <kbd>s</kbd>     | Smooth all rois, doesn't move keyframes |
| <kbd>z</kbd>     | Save current ROIs to file |


## Combining and checking labels

To combine all class labels into an individual file run  

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

`ffmpeg -i <video file> -r 5/1 <path_to_images>/<data_folder_name>$filename%05d.jpg`
