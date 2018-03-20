# Video labeler

Video labeling software with KCF tracker.
Output format: x y width height

_Note:_ x y indicate the coordinates of the top left corner

## Usage

`./video_labeler path_to_images <output_file>`

Draw the initial ROI and press **Enter**. Wherever the tracker fails, press **r** and redraw that frame. Once everything is good, press **Esc** to abort and save ROI file.

If the output_file specified already contains ROIs, these will be loaded up into the labeler.

*Note: the tool requires a starting ROI on the first frame. If there is no object for tagging the first frame, create an arbitrary ROI and then run the tracker. Later use **x** to remove ROI's where no objects exist.*

### Controls

Use the mouse to draw bounding rectangles for the desired object while the video.
is paused. ROIs are confirmed by pressing the <kbd>enter</kbd> key.

The general principle of the application is to manually draw ROIs as little as
possible. This can be done by using _keyframes_ wherein the KCF tracker attempt
to interpolate ROIs in the intervals between them.

#### Functional Controls
|       Key        |    Action    |
|:----------------:|:-------------|
| <kbd>space</kbd>             | Pause/Play |
| <kbd>y</kbd><kbd>u</kbd>     | Slow/Speed up video |
| <kbd>h</kbd><kbd>j</kbd>     | Go backwards/forwards one frame |
| <kbd>x</kbd>     | Mark frame without object |
| <kbd>k</kbd>     | Mark frame as keyframe |
| <kbd>w</kbd>     | Non-destructive marking. Draws ROI on frame but doesn't update tracker |
| <kbd>r</kbd>     | Reset (destructive marking). Draws ROI on frame and updates tracker for following frames |
| <kbd>i</kbd>     | Interpolates interval between keyframes, overrides tracking |
| <kbd>s</kbd>     | Smooth all rois, doesn't move keyframes |
| <kbd>z</kbd>     | Save current ROIs to file |
| <kbd>esc</kbd>   | Finish labeling |

#### Additional Controls
|       Key        |    Action    |
|:----------------:|:-------------|
| <kbd>b</kbd>     | Move to beginning |
| <kbd>t</kbd>     | Show/Hide info panel |

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
