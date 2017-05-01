# Video labeler

Video labeling software with KCF tracker. 
Output format: x y width height

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

## Build

Requirements QT5 and Opencv3. This project was built using ros-kinetic-opencv3. Modify opencv path in Qt project file for a different opencv locaation.
Use QTCreator to build (prefered) or do it manually

```
mkdir build && cd $_
qmake ../
make
```
