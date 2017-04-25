# Video labeler

Video labeling software with KCF tracker.

## Usage

`./video_labeler path_to_images <output_file>`

To generate images from a video, use ffmpeg `sudo apt-get install ffmpeg`:

`ffmpeg -i <video file> -r 1/1 <path_to_images>/$filename%04d.jpg`


