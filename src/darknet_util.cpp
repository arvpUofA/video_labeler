/**
 * @file darknet_util.cpp
 * @author Jacky Chung
 * @date 19 March 2019
 * @brief Implementation for the DarknetUtil class
 *
 */

#include "darknet_util.h"

namespace au_vision {

DarknetUtil::DarknetUtil(const std::string &yamlName) {
  imageToProcess_.data = nullptr;
  yoloInit(yamlName);
}

DarknetUtil::~DarknetUtil() {
  if (&net_) {
    free_network(net_);
  }

  if (imageToProcess_.data) {
    free_image(imageToProcess_);
  }
}

std::vector<std::string> DarknetUtil::getClassNames() { return classNames_; }

void DarknetUtil::yoloInit(const std::string &yamlName) {
  std::cout << "[yolo_detector] yoloInit()." << std::endl;

  char *cfg;
  char *weights;
  char **names;

  const std::string packagePath = ".";
  const std::string yamlFile = packagePath + "/params/" + yamlName;
  std::cout << "Yaml file: " << yamlFile << std::endl;
  const YAML::Node conf = YAML::LoadFile(yamlFile);
  const auto classNames =
      yamlLoad<std::vector<std::string>>("class_names", conf);
  const float thresh = yamlLoad<float>("object_threshold", conf);
  const auto weightsModel = yamlLoad<std::string>("weights_model", conf);
  const auto cfgModel = yamlLoad<std::string>("cfg_model", conf);

  const std::string weightsPath = packagePath + "/weights/" + weightsModel;
  const std::string configPath = packagePath + "/cfg/" + cfgModel;

  // make sure each class has a name, height and width
  if (!classNames.empty()) {
    numYoloClasses_ = static_cast<int>(classNames.size());
    classNames_ = classNames;
    classedBoxes_.resize(classNames.size());
    names = new char *[numYoloClasses_];
  } else {
    std::cout << "Each class must have a name" << std::endl;
    return;
  }

  weights = new char[weightsPath.length() + 1];
  strcpy(weights, weightsPath.c_str());
  cfg = new char[configPath.length() + 1];
  strcpy(cfg, configPath.c_str());

  // Get classes.
  for (int i = 0; i < numYoloClasses_; i++) {
    //+1 is for null char
    char *name = new char[classNames[i].length() + 1];
    strcpy(name, classNames[i].c_str());
    names[i] = name;
  }

  // Load network.
  loadNetwork(cfg, weights, thresh, names, numYoloClasses_, 0.5f, 0.4f);

  // Free allocated memory
  for (int i = 0; i < numYoloClasses_; ++i) {
    delete[] names[i];
  }
  delete[] names;

  delete[] weights;
  delete[] cfg;
}

template <typename T>
T DarknetUtil::yamlLoad(const std::string &key, const YAML::Node &node) {
  if (node[key]) {
    return node[key].as<T>();
  } else {
    throw std::invalid_argument("Cannot read yaml with key: " + key);
  }
}

void DarknetUtil::loadNetwork(char *cfgfile, char *weightfile, float thresh,
                              char **names, int classes, float hier,
                              float nms) {
  thresh_ = thresh;
  hier_ = hier;
  nms_ = nms;

  net_ = parse_network_cfg_custom(cfgfile, 1, 1);  // set batch=1
  load_weights(&net_, weightfile);
  network_size_ = cv::Size(net_.w, net_.h);
}

image DarknetUtil::matToImage(const cv::Mat &src) {
  // Special case: return empty
  if (src.empty()) {
    image out = make_empty_image(0, 0, 0);
    return out;
  }

  // new models do not letterbox the image
  cv::Mat resized_image;
  cv::resize(src, resized_image, network_size_);
  // Convert cv::Mat to image
  // cv::Mat is in uchar whereas image is in floats
  // src.data is a 1D array where each element alterates between the color
  // channels ie. index 0, 1, 2 is BGR of the pixel
  image out = make_image(resized_image.cols, resized_image.rows,
                         resized_image.channels());
  // cv::split seperates src into its three color channels
  std::vector<cv::Mat> channels;
  cv::split(resized_image, channels);
  // out.data is a 1D array of size height*width*channels where each third of
  // the array contains values of the same channel
  long int image_size = resized_image.cols * resized_image.rows;
  for (int i = 0; i < channels.size(); ++i) {
    long int image_offset = image_size * i;
    channels[i].convertTo(channels[i], CV_32FC1, 1.0 / 255);
    memcpy(out.data + image_offset, channels[i].data,
           image_size * sizeof(float));
  }
  return out;
}

std::vector<std::vector<YoloBox>> DarknetUtil::runYolo(
    const cv::Mat &fullFrame) {
  // Ensure network was loaded
  assert(&net_);

  // Convert image to darknet format
  if (imageToProcess_.data) {
    free_image(imageToProcess_);
  }
  imageToProcess_ = matToImage(fullFrame);

  network_predict(net_, imageToProcess_.data);

  int detectionCount = 0;
  detection *detectionResults = get_network_boxes(
      &net_, net_.w, net_.h, thresh_, hier_, nullptr, 1, &detectionCount, 0);

  // Remove detections which overlap too much
  if (nms_ > 0) {
    do_nms_sort(detectionResults, detectionCount, numYoloClasses_, nms_);
  }

  // Convert results to YoloBox objects
  std::vector<YoloBox> candidates;
  for (int i = 0; i < detectionCount; ++i) {
    YoloBox c_box;

    // Classify each detection
    c_box.whichClass = getYoloClass(detectionResults[i], numYoloClasses_);
    c_box.prob = detectionResults[i].prob[c_box.whichClass];

    if (c_box.prob) {
      box b = detectionResults[i].bbox;

      // taken exactly from image.c draw_detections
      int left = (b.x - b.w / 2.) * fullFrame.cols;
      int right = (b.x + b.w / 2.) * fullFrame.cols;
      int top = (b.y - b.h / 2.) * fullFrame.rows;
      int bot = (b.y + b.h / 2.) * fullFrame.rows;

      if (left < 0) left = 0;
      if (right > fullFrame.cols - 1) right = fullFrame.cols - 1;
      if (top < 0) top = 0;
      if (bot > fullFrame.rows - 1) bot = fullFrame.rows - 1;

      c_box.x = left;
      c_box.y = top;
      c_box.w = right - left;
      c_box.h = bot - top;

      // Add to vector
      candidates.push_back(c_box);
    }
  }

  // Free detections and images
  free_detections(detectionResults, detectionCount);
  free_image(imageToProcess_);
  imageToProcess_.data = nullptr;

  // Clear categories
  for (int i = 0; i < numYoloClasses_; i++) {
    classedBoxes_[i].clear();
  }

  // categorize bounding boxes by class
  if (!candidates.empty()) {
    // display number of objects
    std::cout << "# Objects: " << candidates.size() << std::endl;
    for (YoloBox &candidate : candidates) {
      for (int j = 0; j < numYoloClasses_; j++) {
        if (candidate.whichClass == j) {
          classedBoxes_[j].push_back(candidate);

          // display confidence of each detection
          std::cout << classNames_[j].c_str() << " (" << candidate.prob * 100 << "%)" << std::endl;
        }
      }
    }
  }
  return classedBoxes_;
}

int DarknetUtil::getYoloClass(const detection &d, int num_classes) const {
  // The index with the maximum probability is the class of the
  // detection

  float max_prob = 0;
  int max_index = 0;

  for (int c = 0; c < num_classes; c++) {
    if (d.prob[c] > max_prob) {
      max_prob = d.prob[c];
      max_index = c;
    }
  }

  return max_index;
}

}  // namespace au_vision
