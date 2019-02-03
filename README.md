# Video labeler

Video labeling software with KCF tracker.
Default In/Out Label Format: YOLO
Compatible ROI Formats: yolo, cv
yolo format: class, centerX, centerY, box width, box height (all in percentages of image size)
cv format: top left X, top left Y, width, height (all in pixel values)

## Known Bugs

If your output files last label has no boxes/or a zero (0 0 0 0) the program will crash

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

`./video_labeler path_to_images <class_number>`

*Note: do not use any file extensions for the class number

## Optional Usage

`./video_labeler path_to_iamges <class_number> <input_format> <output_format>`

Draw the initial ROI and press <kbd>Enter</kbd>. Wherever the tracker fails, press
<kbd>r</kbd> and redraw that frame. Once everything is good, press <kbd>Esc</kbd> to abort and save ROI file.

If the output_file specified already contains ROIs, these will be loaded up into the labeler.

*Note: the tool requires a starting ROI on the first frame. If there is no object
for tagging the first frame, create an arbitrary ROI and then run the tracker.
Later use <kbd>x</kbd> to remove ROI's where no objects exist.*

## Controls

Use the mouse to draw bounding rectangles for the desired object while the video.
is paused. ROIs are confirmed by pressing the <kbd>enter</kbd> key.

The general principle of the application is to manually draw ROIs as little as
possible. This can be done by using _keyframes_ wherein the KCF tracker attempt
to interpolate ROIs in the intervals between them.

#### Functional Controls
|       Key        |    Action    |
|:----------------:|:-------------|
| <kbd>space</kbd>             | Pause/Play |
| <kbd>y</kbd> <kbd>u</kbd>    | Slow/Speed up video |
| <kbd>h</kbd> <kbd>j</kbd>    | Go backwards/forwards one frame |
| <kbd>x</kbd>     | Mark frame without object |
| <kbd>k</kbd>     | Mark frame as keyframe |
| <kbd>w</kbd>     | Non-destructive marking. Draws ROI on frame but doesn't update tracker |
| <kbd>r</kbd>     | Reset (destructive marking). Draws ROI on frame and updates tracker for following frames |
| <kbd>i</kbd>     | Interpolates interval between keyframes, overrides tracking |
| <kbd>s</kbd>     | Smooth all rois, doesn't move keyframes |
| <kbd>z</kbd>     | Save current ROIs to file |
| <kbd>Esc</kbd>   | Finish labeling |

#### Additional Controls
|       Key        |    Action    |
|:----------------:|:-------------|
| <kbd>b</kbd>     | Move to beginning |
| <kbd>t</kbd>     | Show/Hide info panel |




## Image generation

To generate images from a video, use ffmpeg `sudo apt-get install ffmpeg`:

`ffmpeg -i <video file> -r 5/1 <path_to_images>/<data_folder_name>$filename%05d.jpg`
