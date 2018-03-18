# Video labeler

Video labeling software with KCF tracker. 
Output format: x y width height

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

`./video_labeler path_to_images <output_file>`

Draw the initial ROI and press **Enter**. Wherever the tracker fails, press **r** and redraw that frame. Once everything is good, press **Esc** to abort and save ROI file. 

If the output_file specified already contains ROIs, these will be loaded up into the labeler.

*Note: the tool requires a starting ROI on the first frame. If there is no object for tagging the first frame, create an arbitrary ROI and then run the tracker. Later use **x** to remove ROI's where no objects exist.*

### Controls

* Pause/play: space bar
* Go forwards/backwards on frame: h | j
* Slow/Speed up video y | u
* Finish labeling: esc
* Move to beginning: b
* Show/hide info panel: t
* Non-destructive marking: w (draws ROI on frame but doesn't update tracker)
* Reset(destructive marking): r (draws ROI on frame and updates tracker for following frames)
* Mark frame without object: x
* Mark frame as keyframe: k
* Iterpolate interval: i (interpolates between keyframes, overrides tracking)
* Smooth all: s (smoothes rois, doesn't move keyframes)
* Save current ROIs to file: z

*Everytime an ROI needs to be drawn, press ENTER to continue*

### Image generation

To generate images from a video, use ffmpeg `sudo apt-get install ffmpeg`:

`ffmpeg -i <video file> -r 5/1 <path_to_images>/$filename%05d.jpg`
