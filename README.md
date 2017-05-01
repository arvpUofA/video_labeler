# Video labeler

Video labeling software with KCF tracker.

## Usage

`./video_labeler path_to_images <output_file>`

Draw the initial ROI and press **Enter**. Wherever the tracker fails, press **r** and redraw that frame. Once everything is good, press **Esc** to abort and save ROI file.

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

*Everytime an ROI needs to be drawn, press ENTER to continue*

### Image generation

To generate images from a video, use ffmpeg `sudo apt-get install ffmpeg`:

`ffmpeg -i <video file> -r 5/1 <path_to_images>/$filename%04d.jpg`

## Build

Requirements QT5 and Opencv3. This project was built using ros-kinetic-opencv3. Modify opencv path in Qt project file for a different opencv locaation.
Use QTCreator to build (prefered) or do it manually

```
mkdir build && cd $_
qmake ../
make
```
