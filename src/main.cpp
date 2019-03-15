/*
 * Video labeling with KCF tracking
 * args: input_image_folder <class_num>.txt output_file(optional) label_format(optional) output_format(optional)
 * default: yolo format
 */

/*
 * Bounding Box Formats: This script is uses cv::Rect objects (top left x, top 
 * left y, width and height) and converts these into a cv::rectangle format 
 * (topleft XY, bottomright XY) in order to draw bounding boxes.
 *
 * CV Format - topLeft X, topLeft Y, width, height (all in raw pixel values)
 * YOLO Format - center X, center Y, box width, box height (all in percentages of image size)
 *  
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <QDir>
#include <QStringList>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "tracker/kcftracker.hpp"

/*
 * Function prototypes
 * ===================
 */
cv::Mat getFrame(size_t i);
static void onMouseCb(int event, int x, int y, int, void* );
void addInfoPanel(cv::Mat &frame, size_t curr_frame,
                  size_t total_frames, const cv::Rect roi1,
                  const bool show_panel);
void drawCrosshairs(cv::Mat &frame);
void displayHelp();
void saveFile(std::string file_name, std::vector<cv::Rect> rois);
cv::Rect operator *(const float s, const cv::Rect r1);
cv::Rect operator +(const cv::Rect r1, const cv::Rect r2);
cv::Rect convert_yolo_cv(const float x, const float y, const float w,
                         const float h, const float X, const float Y);


/*
 * Constants
 * =========
 */

const cv::Scalar color_green(0,255,0);
const cv::Scalar color_blue(255,0,0);
const cv::Scalar color_red(0,0,255);
const cv::Scalar color_black(0, 0, 0);
const cv::Scalar color_gray(200,200,200);
const cv::Scalar color_white(255,255,255);

/*
 * Globals
 * =======
 */
static std::vector<std::string> filenames; // image files
// ROI stuff
static bool roi_selection_flag = false;    // set to true in mouse clicked and selecting
static cv::Rect roi_selection;             // selected ROI
static cv::Point roi_origin;               // starting point of ROI
static cv::Point mouse_pos;
// image
static cv::Mat image;                      // cloned frame for processing
static cv::Mat frame;                      // raw image frame
// tracking stuff
static bool track_object = false;               // handle tracking status
static std::string image_directory;               // image directory passed by argc
static std::string label_format = "yolo";         // label format
static std::string output_format = "yolo";

/*
 * Main
 */
int main(int argc, char *argv[]) {
  std::string output_file_name = "ground_truth.txt";

  bool KCF_HOG = false;           // HOG feature is pretty bad
  bool KCF_FIXEDWINDOW = false;   // fixed window performance bad
  bool KCF_MULTISCALE = true;
  bool KCF_LAB = false;

  if(argc <= 2) { // read the input image folder name
    std::cerr << "Usage: video_labeler input_folder {class number} \
      {label_format} (Default YOLO) \
      {output_format} (Default YOLO)" << std::endl;
    exit(1);
  }
  image_directory = argv[1];
  output_file_name = argv[2];
  output_file_name = boost::algorithm::ends_with(output_file_name, ".txt")
                       ? output_file_name : output_file_name + ".txt";
  label_format = (argc > 3) ? argv[3] : label_format;
  output_format = (argc > 4) ? argv[4] : output_format;

  std::cout << "Image directory: " << image_directory << std::endl;
  std::cout << "Output file: " << output_file_name << std::endl;
  std::cout << "Label format: " << label_format << std::endl;
  std::cout << "Output format: " << output_format << std::endl;

  // read image files
  QDir image_folder(argv[1]);
  image_folder.setNameFilters(QStringList() << "*.jpg" << "*.png");
  QStringList fileList = image_folder.entryList();
  for (QStringList::iterator it = fileList.begin(); it != fileList.end(); it++) {
    std::string filename = image_folder.absolutePath().toStdString() +
                           "/" + it->toStdString();
    filenames.push_back(filename);
  }
  size_t filenames_size = filenames.size();
  if (filenames.empty()) {
    std::cerr << "ERROR: No input images found." << std::endl;
    exit(1);
  } else {
    std::cout << "Loaded " << filenames_size << " images" << std::endl;
  }

  displayHelp();

  cv::namedWindow("Labeling", 0);
  cv::setMouseCallback("Labeling", onMouseCb, nullptr);

  size_t frame_index = 0; // index of current frame
  frame = getFrame(frame_index);
  std::cout << "The resolution of video is : " << frame.cols << "x" << frame.rows << std::endl;

  std::vector<cv::Rect> rois;   // stores all the rois
  std::vector<bool> keyframes;
  cv::Scalar color;
  bool continuous_play = false;
  int wait_time = 50;
  bool continue_video = true;
  bool show_info_panel = true;
  cv::Rect result;
  bool data_read = false; // flag for pre-existing data

  // load pre existing rois if they exist
  std::ifstream infile(output_file_name.c_str());
  if(infile.good()) {
    // Label format is cv::Rect
    if (label_format.compare("cv") == 0) {
      int x, y, w, h;
      while(infile >> x >> y >> w >> h) {
        data_read = true;
        rois.push_back(cv::Rect(x,y, w, h));
        keyframes.push_back(true);
      }
      infile.close();
      if (data_read) {
        std::cout << "Pre-existing CV data read from " << output_file_name << std::endl;
      }
    } else { // Label format is YOLO
      float x, y, w, h;
      std::string file_dir;
      while(infile >> x >> y >> w >> h >> file_dir) {
        data_read = true;
        rois.push_back(convert_yolo_cv(x, y, w, h, frame.cols, frame.rows));
        keyframes.push_back(true);
      }
      infile.close();
      if(data_read) {
        std::cout << "Pre-existing YOLO data read from " << output_file_name << std::endl;
      }
    }
    // Start at last labeled image
    frame_index = rois.size()-1;
  }

  // process all frames
  while(frame_index < filenames_size && continue_video) {
    KCFTracker tracker(KCF_HOG, KCF_FIXEDWINDOW, KCF_MULTISCALE, KCF_LAB);
    // only run kcf tracker if last entry is a valid, existing ROI
    if (data_read && (rois.size() > 0 && rois.back().area() != 0)) {
      tracker.init(rois.back(), frame);
    }

    char key_press = -1;
    int step = 0;
    while(continue_video) {
      frame = getFrame(frame_index);
      if(rois.size() > frame_index) { // already computed
        result = rois[frame_index];
      } else {
        // get the last roi if there is atleast one roi in rois
        cv::Rect prev_rect = (rois.size() > 0) ? rois[frame_index-1] : cv::Rect(0, 0, 0, 0);
        // if the previous roi has a width and height of 0
        if ((prev_rect.area() == 0) ||
             !track_object
        ) {
          result = cv::Rect(0,0,0,0);
        } else { // use tracker
          result = tracker.update(frame);
          // assert ROI values in range of image
          result.x = std::max(0, result.x);
          result.width = ((result.x + result.width) > frame.cols) ?
              frame.cols - result.x : result.width;
          result.y = std::max(0, result.y);
          result.height = (result.y + result.height) > frame.rows ?
              frame.rows - result.y : result.height;
        }
        rois.push_back(result);
        keyframes.push_back(false);
      }

      // draw roi
      if(keyframes[frame_index])
        color = color_blue;
      else
        color = color_green;

      cv::rectangle(frame, cv::Point(result.x, result.y),
                    cv::Point(result.x + result.width, result.y + result.height),
                    color, 2, 8);
      addInfoPanel(frame, frame_index, filenames_size, result, show_info_panel);
      cv::imshow("Labeling", frame);

      // process keypress
      key_press = static_cast<char>(cv::waitKey(wait_time));
      if (key_press == 27) { // finish labelling
        std::cout << "Tracking is terminated!" << std::endl;
        continue_video = false;
      }

      if (key_press == 'r') { // reset
        cv::setMouseCallback("Labeling", onMouseCb, nullptr);
        bool making_roi = false;

        while(true) {
          cv::waitKey(5); // used as a delay
          frame.copyTo(image);
          cv::rectangle(image, cv::Point(roi_selection.x, roi_selection.y),
                        cv::Point(roi_selection.x + roi_selection.width, roi_selection.y + roi_selection.height),
                        color_red, 2);
          addInfoPanel(image, frame_index, filenames_size, roi_selection, show_info_panel);
          drawCrosshairs(image);
          cv::imshow("Labeling", image);

          if(roi_selection_flag) {
            making_roi = true;
          }
          else if(!roi_selection_flag && making_roi && roi_selection.width > 0) {
            // initialize tracker
            tracker.init(roi_selection, frame);
            rois[frame_index] = roi_selection;
            keyframes[frame_index] = true;
            // remove following rois
            long index_diff = static_cast<long>(frame_index);
            rois.erase(rois.begin() + index_diff + 1, rois.end());
            keyframes.erase(keyframes.begin() + index_diff + 1);
            break;
          }
        }
        step = 0;
      }

      if (key_press == 'w') { // non-destructive correction
        cv::setMouseCallback("Labeling", onMouseCb, nullptr);
        bool making_roi = false;

        while(1) {
          cv::waitKey(5); // used as a delay
          frame.copyTo(image);
          cv::rectangle(image, cv::Point(roi_selection.x, roi_selection.y),
                        cv::Point(roi_selection.x + roi_selection.width, roi_selection.y + roi_selection.height),
                        color_red, 2);
          addInfoPanel(image, frame_index, filenames.size(), roi_selection, show_info_panel);
          drawCrosshairs(image);
          cv::imshow("Labeling", image);

          if(roi_selection_flag) {
            making_roi = true;
          }
          else if(!roi_selection_flag && making_roi) {
            rois[frame_index] = roi_selection;
            keyframes[frame_index] = true;

            break;
          }
        }
        track_object = false;
        step = 0;
      }

      if (key_press == 'x') { // frame without object
        rois[frame_index] = cv::Rect(0,0,0,0);
        track_object = false;
      }

      if (key_press == 'b') { // move to beginning
        frame_index = 0;
        track_object = false;
      }

      if (key_press == 'i') { // interpolate interval
        // compute over under (keyframes)
        size_t over = frame_index, under = frame_index;

        while(over < keyframes.size()) {
          if (keyframes[over])
            break;
          over++;
        }
        while(under > 0) {
          if (keyframes[under])
            break;
          under--;
        }

        if (over < keyframes.size()) {
          float jump = over - under;
          for(size_t i=under+1; i<over; i++) {
            rois[i] = (static_cast<float>(jump-(i-under))/jump) * rois[under]
                            + (static_cast<float>(jump-(over-i))/jump) * rois[over];
          }
        }
      }

      if (key_press == 's') { // smooth
        std::vector<cv::Rect> temp_rois = rois;
        for(size_t i = 1; i < temp_rois.size()-1; i++) {
          if(keyframes[i]) continue; // don't smooth keyframes
          temp_rois[i] = 0.5*rois[i] + 0.25*rois[i-1] + 0.25*rois[i+1];
        }
        if (frame_index>1 && !keyframes[frame_index-1]) {
          temp_rois[frame_index-1] = 0.75*rois[frame_index-1] + 0.25*rois[frame_index-2];
        }
        rois = temp_rois;
      }

      if (key_press == 'k') { // mark keyframe
        keyframes[frame_index] = true;
      }

      if (key_press == 32) { // play/pause
        continuous_play = !continuous_play;
        if(continuous_play && (rois.size() > frame_index)) {
          step = 1;
        } else {
          step = 0;
        }
      }

      if (key_press == 'j') { //  next frame
        // do not go to next frame unless roi add to rois vector
        if(!continuous_play && (rois.size() > frame_index)) {
          step = 1;
        }
      }

      if (key_press == 'h') { // previous frame
        // do not go to end of video when rois are not filled
        // when pressing back from first frame of video
        if(!continuous_play &&
           !(frame_index == 0 && rois.size() != filenames_size)
        ) {
          track_object = false;
          step = -1;
        }
      }

      if (key_press == 'y') { // speed down video
        wait_time = std::min(wait_time + 10, 1000);
      }

      if (key_press == 'u') { // speed up video
        wait_time = std::max(wait_time - 10, 10);
      }

      if (key_press == 't') { // toggle info
        show_info_panel = !show_info_panel;
      }

      if (key_press == 'z') { // save to file
        if (rois.size() > 0) {
          saveFile(output_file_name.c_str(), rois);
        } else {
          std::cout << "No ROIs to save" << std::endl;
        }
      }

      if (key_press == -1 && !continuous_play) {// no key pressed and paused
        step = 0;
      }

      frame_index += step;

      if (frame_index >= filenames.size()) { // video complete
        frame_index = filenames.size() - 1;
        std::cout << "Video is complete. please press esc to end labeling or review it" << std::endl;
        track_object = false;
        step = 0;
        continuous_play = 0;
      }
    }
  }
  // save rois
  saveFile(output_file_name.c_str(), rois);

  return 0;

}

/**
 * @brief getFrame  gets frame using image index
 * @param i         image index
 * @return          cv::Mat frame
 */
cv::Mat getFrame(size_t i)
{
  cv::Mat frame = cv::imread(filenames[i]);
  return frame;
}

/**
 * @brief onMouseCb mouse callback for opencv window
 * @param event     mouse event
 * @param x         mouse x
 * @param y         mouse y
 */
static void onMouseCb(int event, int x, int y, int, void* ) {
  if (roi_selection_flag) {
    roi_selection.x = MIN(x, roi_origin.x);
    roi_selection.y = MIN(y, roi_origin.y);
    roi_selection.width = std::abs(x - roi_origin.x);
    roi_selection.height = std::abs(y - roi_origin.y);
    roi_selection &= cv::Rect(0, 0, image.cols, image.rows);
  } else {
    mouse_pos.x = x;
    mouse_pos.y = y;
  }

  switch(event) {
    case CV_EVENT_LBUTTONDOWN:
      roi_origin = cv::Point(x,y);
      roi_selection = cv::Rect(x,y,0,0);
      roi_selection_flag = true;
      break;
    case CV_EVENT_LBUTTONUP:
      roi_selection_flag = false;
      // if an roi is selection, init tracking flag
      if(roi_selection.width > 0 && roi_selection.height > 0) {
        track_object = true;
      }
      break;
  }
}

/**
 * @brief addInfoPanel  add text stats to opencv image
 * @param frame         cv::Mat to add stats to
 * @param curr_frame    current frame number
 * @param total_frames  total number of frames
 * @param roi1          selected roi
 * @param show_panel    flag to show this info
 */
void addInfoPanel(cv::Mat &frame, size_t curr_frame,
                  size_t total_frames, const cv::Rect roi1,
                  const bool show_panel) {
  if(!show_panel) return;

  double panel_transparancy = 1.2;
  int line_position = 25;
  double font_scale = 0.75;
  cv::Rect panel_position = cv::Rect(0,0,250,70);
  int line_jump = static_cast<int>(35 * font_scale);

  // draw a semi transparent panel
  frame(panel_position) = frame(panel_position) / panel_transparancy;

  // frame info (0 indexed)
  std::string output_text = "INFO | " +
                            std::to_string(curr_frame) + "/" + std::to_string(total_frames -1);
  cv::putText(frame, output_text, cv::Point2f(5, line_position), 4,
              font_scale, color_gray, 2);
  // roi info
  output_text = "Rectangle: " + std::to_string(roi1.x) + "," +
                std::to_string(roi1.y) + "," +
                std::to_string(roi1.width) + "," +
                std::to_string(roi1.height);
  line_position += line_jump;
  cv::putText(frame, output_text, cv::Point2f(5, line_position), 4, font_scale, color_gray, 2);
}

/**
 * @breif drawCrosshairs  draws crosshair at cursor position
 * @param frame           cv::Mat to add crosshair to
 */
void drawCrosshairs(cv::Mat &frame) {
  if(not roi_selection_flag) {
    cv::line(image, cv::Point(mouse_pos.x, 0), cv::Point(mouse_pos.x, frame.rows), color_black, 2);
    cv::line(image, cv::Point(0, mouse_pos.y), cv::Point(frame.cols, mouse_pos.y), color_black, 2);
  }
}

/**
 * @brief displayHelp   display help
 */
void displayHelp() {
  std::cout << "CONTROLS" << std::endl;
  std::cout << "\tPause/play: space bar" << std::endl;
  std::cout << "\tSlow/Speed up video y | u" << std::endl;
  std::cout << "\tFinish labeling: esc" << std::endl;
  std::cout << "\tInterpolate interval: i" << std::endl;
  std::cout << "\tSmooth all: s" << std::endl;
  std::cout << "\tMark frame as keyframe: k" << std::endl;
  std::cout << "\tSave labels to file: z" << std::endl;

  std::cout << "RECOMMENDED CONTROLS BELOW:" << std::endl;
  std::cout << "\tGo backwards/forwards on frame: h | j" << std::endl;
  std::cout << "\tNon-destructive marking: w" << std::endl;
  std::cout << "\tReset(destructive marking): r" << std::endl;
  std::cout << "\tMark frame without object: x" << std::endl;
  std::cout << "\tMove to beginning: b" << std::endl;
  std::cout << "\tShow/hide info panel: t" << std::endl;
}

/**
 * @brief saveFile      save roi to file
 * @param file_name     output file
 * @param rois    vector of ROIs
 */
void saveFile(std::string file_name, std::vector<cv::Rect> rois) {
  std::ofstream ofile;
  ofile.open(file_name.c_str());

  // If label format is cv:Rect
  if (output_format.compare("cv") == 0) {
    for(size_t i = 0; i < rois.size(); i++) {
      ofile << rois[i].x << " " << rois[i].y << " " <<
            rois[i].width << " " << rois[i].height <<
            std::endl;
    }
    ofile.close();
    std::cout << "CV Data saved to " << file_name << std::endl;
  } else { // If label format is YOLO
    if (image_directory[image_directory.size()-1] == '/') {        // If argc[1] has a trailing "/" remove it
      image_directory.pop_back();
    }

    for (size_t i = 0; i < rois.size(); i++) {
      std::vector<std::string> splitString;
      const std::string fileName(filenames[i]);
      boost::split(splitString, fileName, boost::is_any_of("/"));

      // ROI in YOLO format + image directory
      ofile << static_cast<double>(rois[i].x+(rois[i].width/2))/frame.cols << " " <<
            static_cast<double>(rois[i].y+(rois[i].height/2))/frame.rows << " " <<
            static_cast<double>(rois[i].width)/frame.cols << " " <<
            static_cast<double>(rois[i].height)/frame.rows << " "  <<
            image_directory << "/" <<
            splitString[splitString.size()-1] <<
            std::endl;
    }
    ofile.close();
    std::cout << "YOLO Data saved to " << file_name << std::endl;
  }
}

cv::Rect operator *(const float s, const cv::Rect r1) {
  cv::Rect r2;
  r2.x = static_cast<int>(r1.x * s);
  r2.y = static_cast<int>(r1.y * s);
  r2.width = static_cast<int>(r1.width * s);
  r2.height = static_cast<int>(r1.height * s);
  return r2;
}

cv::Rect operator +(const cv::Rect r1, const cv::Rect r2) {
  cv::Rect r3;
  r3.x = r2.x + r1.x;
  r3.y = r2.y + r1.y;
  r3.width = r2.width + r1.width;
  r3.height = r2.height + r1.height;
  return r3;
}

cv::Rect convert_yolo_cv (const float x, const float y, const float w,
                          const float h, const float X, const float Y) {
  cv::Rect r;
  r.x = static_cast<int>((x - (w/2))*X);
  r.y = static_cast<int>((y - (h/2))*Y);
  r.width = static_cast<int>(w * X);
  r.height = static_cast<int>(h * Y);
  return r;
}
