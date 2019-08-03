/**
 * @file yolo_torpedo_detector.cpp
 * @author Jacky Chung
 * @date 29 Jun 2019
 * @brief Implementation for the YoloTorpedoDetector class
 *
 */

#include <au_vision/yolo/yolo_torpedo_detector.h>

namespace au_vision {

YoloTorpedoDetector::YoloTorpedoDetector(ros::NodeHandle &nh,
                                         ros::NodeHandle &private_nh,
                                         std::string detectorType)
    : Detector(nh, private_nh, detectorType), darknetUtil("yolo_torpedo.yaml") {
  detectorType_ = "yolo_torpedo_detector";
  classNames_ = darknetUtil.getClassNames();
}

YoloTorpedoDetector::~YoloTorpedoDetector() {}

std::vector<au_core::Roi> YoloTorpedoDetector::detect(
    const cv::Mat &frame, const au_core::CameraInfo &cameraInfo) {
  // Vector to be returned
  std::vector<au_core::Roi> roiArray;

  std::vector<std::vector<YoloBox> > classedBoxes_ = darknetUtil.runYolo(frame);
  // Convert YoloBoxes for ROS
  for (int i = 0; i < classedBoxes_.size(); i++) {
    std::string className = classNames_[i];

    for (auto box : classedBoxes_[i]) {
      // Calculate position and dimensions

      auto w = (unsigned int)box.w;
      auto h = (unsigned int)box.h;

      au_core::Roi roi;
      roi.topLeft.x = box.x;
      roi.topLeft.y = box.y;
      roi.width = w;
      roi.height = h;

      roi.tags.push_back(className);
      roiArray.push_back(roi);
    }
  }

  return roiArray;
}

}  // namespace au_vision
