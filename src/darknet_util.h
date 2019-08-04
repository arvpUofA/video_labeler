/**
 * @file darknet_util.h
 * @author Jacky Chung
 * @date 19 March 2019
 * @brief Header for the DarknetUtil class
 *
 * Uses darknet to run inference on trained yolo model and output detected
 * objects
 */

#include <yaml-cpp/yaml.h>
#include <opencv2/opencv.hpp>
#include <cassert>

#include <darknet.h>
extern "C" {
#include <box.h>
#include <cost_layer.h>
#include <image.h>
#include <network.h>
#include <parser.h>
#include <region_layer.h>
#include <utils.h>
#include <layer.h>
}

#ifndef AU_VISION_DARKNET_UTIL_H
#define AU_VISION_DARKNET_UTIL_H

namespace au_vision {

// Type for bounding boxes of detected objects.
struct YoloBox {
  int x, y, w, h;
  float prob;
  int whichClass;
};

class DarknetUtil {
 public:
  DarknetUtil(const std::string& yamlName);
  ~DarknetUtil();
  /**
   * @brief runYolo, runs YOLO inference on input frame
   */
  std::vector<std::vector<YoloBox> > runYolo(const cv::Mat& fullFrame);
  std::vector<std::string> getClassNames();

 protected:
 private:
  int numYoloClasses_;
  std::vector<std::string> classNames_;
  std::vector<std::vector<YoloBox> > classedBoxes_;  // Divided into classes

  network net_;
  cv::Size network_size_;
  image imageToProcess_;
  float hier_;
  float thresh_;
  float nms_;

  /*
   * @brief
   * @param[node]
   */
  template <typename T>
  T yamlLoad(const std::string& key, const YAML::Node& node);
  /**
   * @brief Initialize darknet network of yolo.
   * @param[in] cfgfile location of darknet's cfg file describing the layers of
   * the network.
   * @param[in] weightfile location of darknet's weights file setting the
   * weights of the network.
   * @param[in] thresh threshold of the object detection (0 < thresh < 1).
   */
  void loadNetwork(char* cfgfile, char* weightfile, float thresh, char** names,
                   int classes, float hier, float nms);

  image matToImage(const cv::Mat& src);

  /**
   * @brief yoloInit, Loads trained weights and initialize the yolo network.
   * @param nodeHandle, node handle for ros process
   */
  void yoloInit(const std::string& yamlName);

  /**
   * @breif Returns the most likely class of a detection candidate
   */
  int getYoloClass(const detection& d, int num_classes) const;
};

}  // namespace au_vision

#endif  // AU_VISION_DARKNET_UTIL_H
