/**
 * @file yolo_torpedo_detector.h
 * @author Jacky Chung
 * @date 29 June 2019
 * @brief Header for the YoloTorpedoDetector class
 *
 * Uses darknet to run inference on trained yolo model and output detected
 * objects
 */

#include <au_core/vision_util.h>
#include <au_vision/detector.h>
#include <au_vision/yolo/darknet_util.h>
#include <ros/package.h>

#include <opencv2/opencv.hpp>

#pragma once

namespace au_vision {

class YoloTorpedoDetector : public Detector {
 public:
  YoloTorpedoDetector(ros::NodeHandle& nh, ros::NodeHandle& private_nh,
                      std::string detectorType = "yolo_torpedo_detector");
  ~YoloTorpedoDetector();

 protected:
  std::vector<au_core::Roi> detect(const cv::Mat& frame,
                                   const au_core::CameraInfo& cameraInfo);

 private:
  DarknetUtil darknetUtil;
  std::vector<std::string> classNames_;
};

}  // namespace au_vision