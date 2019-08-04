#include <iostream>
#include <algorithm>
#include <memory>
#include <fstream>
#include <chrono>
#include <thread>

#include <experimental/filesystem> // c++ 17
#include <boost/algorithm/string.hpp>

#include <opencv2/opencv.hpp>
#include "kcftracker.hpp"
#include "darknet_util.h"

namespace fs = std::experimental::filesystem;

// constants
const cv::Scalar color_green(0,255,0);
const cv::Scalar color_blue(255,0,0);
const cv::Scalar color_red(0,0,255);
const cv::Scalar color_black(0, 0, 0);
const cv::Scalar color_gray(200,200,200);
const cv::Scalar color_white(255,255,255);

// globabls
// mouse callback
static bool roi_selection_flag = false;         // set to true in mouse clicked and selecting
static cv::Rect roi_selection;                  // selected ROI
static cv::Point roi_origin;                    // starting point of ROI
static cv::Point mouse_pos;
// tracking
bool track_object = false;
// images
static cv::Mat frame;                           // use frame size to clamp ROI

void clampRoi(cv::Rect &roi) {
    roi.x = std::max(0, roi.x);
    roi.width = ((roi.x + roi.width) > frame.cols) ?
        frame.cols - roi.x : roi.width;
    roi.y = std::max(0, roi.y);
    roi.height = (roi.y + roi.height) > frame.rows ?
        frame.rows - roi.y : roi.height;
}

void getImagePaths(std::vector<std::string> &image_paths, const std::string &directory) {
    for (auto& p: fs::directory_iterator(directory)) {
        image_paths.push_back(p.path().string());
    }

    std::sort(image_paths.begin(), image_paths.end());
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
        clampRoi(roi_selection);
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
            // if an roi is selected, init tracking flag
            track_object = roi_selection.width > 0 && roi_selection.height > 0;
            break;
        }
}

/**
 * @breif drawCrosshairs  draws crosshair at cursor position
 * @param frame           cv::Mat to add crosshair to
 */
void drawCrosshairs(cv::Mat &frame) {
    if(not roi_selection_flag) {
        cv::line(frame, cv::Point(mouse_pos.x, 0), cv::Point(mouse_pos.x, frame.rows), color_black, 2);
        cv::line(frame, cv::Point(0, mouse_pos.y), cv::Point(frame.cols, mouse_pos.y), color_black, 2);
    }
}
/**
 * @brief getFrame  gets frame using image index
 * @param image_paths   vector of image path names
 * @param i             image index
 * @return              cv::Mat frame
 */
cv::Mat getFrame(std::vector<std::string> const &image_paths, size_t i) {
  cv::Mat frame = cv::imread(image_paths[i]);
  return frame;
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
 * @brief saveFile      save roi to file
 * @param output_file     output file
 * @param rois    vector of ROIs
 */
void saveFile(std::string const &output_file, std::vector<std::string> const &image_names,
            std::string &image_directory, std::string &image_directory_name,
            std::vector<cv::Rect> rois) {
    std::ofstream ofile;
    ofile.open(output_file.c_str());

    if (image_directory[image_directory.size()-1] == '/') {        // If argc[1] has a trailing "/" remove it
      image_directory.pop_back();
    }

    for (size_t i = 0; i < rois.size(); i++) {
      std::vector<std::string> splitString;
      const std::string fileName(image_names[i]);
      boost::split(splitString, fileName, boost::is_any_of("/"));

      // ROI in YOLO format + image directory
      ofile << static_cast<double>(rois[i].x+(rois[i].width/2))/frame.cols << " " <<
            static_cast<double>(rois[i].y+(rois[i].height/2))/frame.rows << " " <<
            static_cast<double>(rois[i].width)/frame.cols << " " <<
            static_cast<double>(rois[i].height)/frame.rows << " "  <<
            image_directory_name << "/" <<
            splitString.back() <<
            std::endl;
    }
    ofile.close();
    std::cout << "YOLO Data saved to " << output_file << std::endl;
}

/**
 * @brief converts from yolo format to open cv
 */
cv::Rect convert_yolo_cv (const float x, const float y, const float w,
                          const float h, const float X, const float Y) {
    cv::Rect r;
    r.x = static_cast<int>((x - (w/2))*X);
    r.y = static_cast<int>((y - (h/2))*Y);
    r.width = static_cast<int>(w * X);
    r.height = static_cast<int>(h * Y);
    return r;
}

/**
 * @brief displayHelp   display help
 */
void displayHelp() {
    std::cout << "CONTROLS" << std::endl;
    std::cout << "\tPause/play: space bar" << std::endl;
    std::cout << "\tSlow/Speed up video y | u" << std::endl;
    std::cout << "\tFinish labeling: esc" << std::endl;
    std::cout << "\tMark frame as keyframe: k" << std::endl;
    std::cout << "\tSave labels to file: z" << std::endl;

    std::cout << "RECOMMENDED CONTROLS BELOW:" << std::endl;
    std::cout << "\tGo backwards/forwards on frame: h | j" << std::endl;
    std::cout << "\tNon-destructive marking: w" << std::endl;
    std::cout << "\tReset(destructive marking on future frames): r" << std::endl;
    std::cout << "\tUse YOLO model to predict bound (single): q" << std::endl;
    std::cout << "\tToggle YOLO prediction on each new frame (auto): a" << std::endl;
    std::cout << "\tMark frame without object: x" << std::endl;
    std::cout << "\tMove to beginning: b" << std::endl;
    std::cout << "\tShow/hide info panel: t" << std::endl;
}