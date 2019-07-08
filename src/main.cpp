#include "main.h"

int main(int argc, char** argv ) {
    bool KCF_HOG = false;           // HOG feature is pretty bad
    bool KCF_FIXEDWINDOW = false;   // fixed window performance bad
    bool KCF_MULTISCALE = true;
    bool KCF_LAB = false;

    if(argc <= 2) { // read the input image folder name
        std::cerr << "Usage: ./VideoLabeler input_folder {class number}" << std::endl;
        exit(1);
    }

    std::vector<std::string> split_string;
    std::vector<std::string> image_paths;
  
    std::string image_directory = argv[1]; // relative path from current directory
    std::string main_class_number = argv[2];
    std::string secondary_class_number = (argc > 3) ? argv[3] : "0";

    // gets the directory name of the image_directory
    boost::split(split_string, image_directory, boost::is_any_of("/"));
    std::string image_directory_name = split_string.back();

    std::string output_file_name = main_class_number + "-" +
                                secondary_class_number + "-" + 
                                image_directory_name + ".txt";

    std::cout << "Image directory: " << image_directory << std::endl;
    std::cout << "Output file: " << output_file_name << std::endl;

    /*=============================== misc ==============================*/ 
    cv::Scalar color;

    getImagePaths(image_paths, image_directory);
    size_t frame_index = 0;
    size_t image_count = image_paths.size();
    
    displayHelp();
    if (image_paths.empty()) {
        std::cerr << "ERROR: No input images found." << std::endl;
        exit(1);
    } else {
        std::cout << "Loaded " << image_count << " images" << std::endl;
    }

    /*==================== handle displaying of images ====================*/ 
    cv::namedWindow("Labeler", cv::WINDOW_AUTOSIZE );

    cv::Mat image; // frame is the current image, image is a temp used for drawing cursors
    frame = getFrame(image_paths, frame_index);
    std::cout << "The resolution of video is : " << frame.cols << "x" << frame.rows << std::endl;

    std::vector<cv::Rect> rois;   // stores all the rois

    cv::Rect result;
    bool show_info_panel = true;

    int wait_time = 50; // for cv::waitKey
    char key_press = -1;
    bool finished = false;
    bool continuous_play = false;
    int step = 0;

      bool data_read = false; // flag for pre-existing data

    // load pre existing rois if they exist
    std::ifstream infile(output_file_name.c_str());
    if(infile.good()) {
        float x, y, w, h;
        std::string file_dir;
        while (infile >> x >> y >> w >> h >> file_dir) {
            data_read = true;
            rois.push_back(convert_yolo_cv(x, y, w, h, frame.cols, frame.rows));
        }
        infile.close();
        if (data_read) {
            std::cout << "Pre-existing YOLO data read from " << output_file_name << std::endl;
        }
        // Start at last labeled image
        frame_index = rois.size()-1;
    }
    KCFTracker tracker(KCF_HOG, KCF_FIXEDWINDOW, KCF_MULTISCALE, KCF_LAB);

    /*========================== main loop ================================*/ 
    while (!finished) {
        frame = getFrame(image_paths, frame_index);
        if (rois.size() > frame_index) { // already computed
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
                clampRoi(result);
            }
            rois.push_back(result);
        }
        
        cv::rectangle(frame, cv::Point(result.x, result.y),
            cv::Point(result.x + result.width, result.y + result.height),
            color_green, 2, 8);
        addInfoPanel(frame, frame_index, image_count, result, show_info_panel);

        cv::imshow("Labeler", frame);

    /*==================== process key presses ====================*/ 
        key_press = static_cast<char>(cv::waitKey(wait_time));
        if (key_press == 27) { // finish labelling
            std::cout << "Tracking is terminated!" << std::endl;
            saveFile(output_file_name, image_paths, image_directory, image_directory_name, rois);
            return 0;
        }
        
        if (key_press == 'r') { // reset
            cv::setMouseCallback("Labeler", onMouseCb, nullptr);
            bool making_roi = false;

            while(true) {
                cv::waitKey(5); // used as a delay
                frame.copyTo(image);
                cv::rectangle(image, cv::Point(roi_selection.x, roi_selection.y),
                                cv::Point(roi_selection.x + roi_selection.width, roi_selection.y + roi_selection.height),
                                color_red, 2);
                addInfoPanel(image, frame_index, image_count, roi_selection, show_info_panel);
                drawCrosshairs(image);
                cv::imshow("Labeler", image);

                if(roi_selection_flag) {
                    making_roi = true;
                }
                else if(!roi_selection_flag && making_roi && roi_selection.width > 0) {
                    // initialize tracker
                    tracker.init(roi_selection, frame);
                    rois[frame_index] = roi_selection;
                    // remove following rois
                    long index_diff = static_cast<long>(frame_index);
                    rois.erase(rois.begin() + index_diff + 1, rois.end());
                    break;
                }
            }
            step = 0;
        }
        
        if (key_press == 'w') { // non-destructive correction
            cv::setMouseCallback("Labeler", onMouseCb, nullptr);
            bool making_roi = false;

            while(1) {
                cv::waitKey(5); // used as a delay
                frame.copyTo(image);
                cv::rectangle(image, cv::Point(roi_selection.x, roi_selection.y),
                                cv::Point(roi_selection.x + roi_selection.width, roi_selection.y + roi_selection.height),
                                color_red, 2);
                addInfoPanel(image, frame_index, image_count, roi_selection, show_info_panel);
                drawCrosshairs(image);
                cv::imshow("Labeler", image);

                if (roi_selection_flag) {
                    making_roi = true;
                } else if(!roi_selection_flag && making_roi) {
                    rois[frame_index] = roi_selection;
                
                    break;
                }
            }
            track_object = false;
            step = 0;
        }

        if (key_press == 'x') { // remove roi from frame
            rois[frame_index] = cv::Rect(0,0,0,0);
            track_object = false;
            step = 0;
        }

        if (key_press == 'b') { // move to beginning
            frame_index = 0;
            track_object = false;
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
            // do not go to next frame unless roi has been added to rois vector
            if(!continuous_play && (rois.size() > frame_index)) {
                step = 1;
            }
        }

        if (key_press == 'h') { // previous frame
        // do not go to end of video when rois are not filled
        // when pressing back from first frame of video
            if(!continuous_play &&
                !(frame_index == 0 && rois.size() != image_count)
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
            saveFile(output_file_name, image_paths, image_directory, image_directory_name, rois);
        }

        if (key_press == -1 && !continuous_play) {// no key pressed and paused
            step = 0;
        }

        frame_index += step;
        if (frame_index >= image_count) { // video complete
            frame_index = image_count - 1;
            std::cout << "Video is complete. please press esc to end labeling or review it" << std::endl;
            track_object = false;
            step = 0;
            continuous_play = 0;
        }
    }

    return 0;
}