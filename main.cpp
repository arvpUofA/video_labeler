/*
 * Video labeling with KCF tracking
 * args: input_image_folder output_file(optional)
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
cv::Mat getFrame(const int i);
static void onMouseCb(int event, int x, int y, int, void* );
void addInfoPanel(cv::Mat &frame, const int curr_frame,
                  const int total_frames, const cv::Rect roi1,
                  const bool show_panel);
void displayHelp();
cv::Rect operator *(const float s, const cv::Rect r1);
cv::Rect operator +(const cv::Rect r1, const cv::Rect r2);

/*
 * Globals
 * =======
 */
std::vector<std::string> filenames; // image files
// ROI stuff
bool roi_selection_flag = false;    // set to true in mouse clicked and selecting
cv::Rect roi_selection;             // selected ROI
cv::Point roi_origin;               // starting point of ROI
// image
cv::Mat image;                      // cloned frame for processing
cv::Mat frame;                      // raw image frame
// tracking stuff
int track_object = 0;               // handle tracking status

/*
 * Main
 */
int main(int argc, char *argv[])
{
    std::string output_file_name = "ground_truth.txt";
    std::cout << "Labeling application started" << std::endl;

    /*
     * KCF Params
     */
    bool KCF_HOG = false;           // HOG feature is pretty bad
    bool KCF_FIXEDWINDOW = false;   // fixed window performance bad
    bool KCF_MULTISCALE = true;
    bool KCF_LAB = false;

    /*
     * read CLI parameters
     */
    if(argc < 2)
    { // read the input image folder name
        std::cerr << "Usage: video_labeler input_folder <output_file_name>" << std::endl;
        exit(1);
    }
    if(argc > 2)
    {
        output_file_name = argv[2];
    }

    /*
     * read image files
     */
    QDir image_folder(argv[1]);
    image_folder.setNameFilters(QStringList() << "*.jpg" << "*.png");
    QStringList fileList = image_folder.entryList();
    for(QStringList::iterator it = fileList.begin(); it != fileList.end(); it++)
    {
        std::string filename = image_folder.absolutePath().toStdString() +
                "/" + it->toStdString();
        filenames.push_back(filename);
    }
    if(filenames.empty())
    {
        std::cerr << "ERROR: No input images found." << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "Loaded " << filenames.size() << " images" << std::endl;
    }

    /*
     * show help
     */
    displayHelp();

    /*
     * initialize opencv window
     */
    cv::namedWindow("Labeling", 0);
    cv::setMouseCallback("Labeling", onMouseCb, 0);

    /*
     * start tracking
     */
    int frame_index = 0;
    frame = getFrame(frame_index);
    std::cout << "The resolution of video is : " << frame.cols << "x" << frame.rows << std::endl;

    std::vector<cv::Rect> rectangles;   // stores all the rois
    std::vector<bool> keyframes;
    cv::Scalar color, color_green(0,255,0), color_blue(255,0,0), color_red(0,0,255);
    bool continuous_play = true;
    int wait_time = 50;
    bool continue_video = true;
    bool show_info_panel = true;
    cv::Rect result;

    std::cout << "Please select a rect as a tracking object and press ENTER!" << std::endl;
    // process all frames
    while(frame_index < filenames.size() && continue_video)
    {
        //-- Step 1: select tracking box from first frame
        while(1)
        {
            char c = (char) cv::waitKey(wait_time);
            // spaccebar to pause/play
            if(c == 32)
                continuous_play = !continuous_play;

            if(!continuous_play)
                frame = getFrame(frame_index);

            // clone frame to image
            frame.copyTo(image);

            // draw on image
            cv::rectangle(image, cv::Point(roi_selection.x, roi_selection.y),
                          cv::Point(roi_selection.x + roi_selection.width, roi_selection.y + roi_selection.height),
                          color_red, 2);
            addInfoPanel(image, frame_index, filenames.size(), roi_selection, show_info_panel);
            cv::imshow("Labeling", image);

            // handle keyboard input
            if(c == 27)
            { // esc key to terminate
                std::cout << "Tracking is terminated!" << std::endl;
                return 0;
            }
            if(c == 13)
            { // ENTER key to move forward with ROI
                std::cout << "The tracking object is selected!" << std::endl;
                std::cout << "\t" << "tracking object:" << std::endl;
                std::cout << "\t" << cv::Point(roi_selection.x, roi_selection.y) << std::endl;
                std::cout << "\t" << cv::Point(roi_selection.x + roi_selection.width, roi_selection.y + roi_selection.height) << std::endl;
                rectangles.push_back(roi_selection);
                keyframes.push_back(true); // the first frame is a key frame
                break;
            }

        }

        //-- Step 2: start tracking
        KCFTracker tracker(KCF_HOG, KCF_FIXEDWINDOW, KCF_MULTISCALE, KCF_LAB);
        // initialize with first frame and position of the object
        tracker.init(roi_selection, frame);

        int step = 1;
        frame_index += step;
        char c2 = -1;

        while(continue_video)
        {
            // get frame
            frame = getFrame(frame_index);
            if(rectangles.size()>frame_index)
            { // already computed
                result = rectangles[frame_index];
            }
            else
            {
                cv::Rect prev_rect = rectangles[frame_index-1];
                if(prev_rect.x == 0 && prev_rect.y == 0 && prev_rect.width == 0 && prev_rect.height == 0)
                {
                    result = cv::Rect(0,0,0,0);
                }
                else
                {
                    result = tracker.update(frame);
                }
                rectangles.push_back(result);
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
            addInfoPanel(frame, frame_index, filenames.size(), result, show_info_panel);
            cv::imshow("Labeling", frame);

            // wait key and process keys
            c2 = (char) cv::waitKey(wait_time);

            if(c2 == 27) // terminate
            {
                std::cout << "Tracking is terminated!" << std::endl;
                continue_video = false;
            }

            if(c2 == 'r') // reset
            {
                cv::setMouseCallback("Labeling", onMouseCb, 0);
                while(1)
                {
                    char c = (char) cv::waitKey(5);
                    frame.copyTo(image);
                    cv::rectangle(image, cv::Point(roi_selection.x, roi_selection.y),
                                  cv::Point(roi_selection.x + roi_selection.width, roi_selection.y + roi_selection.height),
                                  color_red, 2);
                    addInfoPanel(image, frame_index, filenames.size(), roi_selection, show_info_panel);
                    cv::imshow("Labeling", image);

                    if(c == 13)
                    { // wait for RETURN
                        // initialize tracker
                        tracker.init(roi_selection, frame);
                        rectangles[frame_index] = roi_selection;
                        keyframes[frame_index] = true;
                        // remove following rectangles
                        rectangles.erase(rectangles.begin() + frame_index + 1, rectangles.end());
                        keyframes.erase(keyframes.begin() + frame_index + 1);
                        break;
                    }
                }
                step = 0;
            }

            if(c2 == 'w') // non-destructive correction
            {
                cv::setMouseCallback("Labeling", onMouseCb, 0);
                while(1)
                {
                    char c = (char) cv::waitKey(5);
                    frame.copyTo(image);
                    cv::rectangle(image, cv::Point(roi_selection.x, roi_selection.y),
                                  cv::Point(roi_selection.x + roi_selection.width, roi_selection.y + roi_selection.height),
                                  color_red, 2);
                    addInfoPanel(image, frame_index, filenames.size(), roi_selection, show_info_panel);
                    cv::imshow("Labeling", image);

                    if(c == 13)
                    { // wait for RETURN
                        rectangles[frame_index] = roi_selection;
                        keyframes[frame_index] = true;

                        break;
                    }
                }
                step = 0;
            }

            if(c2 == 'x') // frame without object
            {
                rectangles[frame_index] = cv::Rect(0,0,0,0);
            }

            if(c2 == 'b') // move to beginning
            {
                frame_index = 0;
            }

            if(c2 == 'i') // interpolate interval
            {
                // compute over under (keyframes)
                int over = frame_index, under = frame_index;
                while(over<keyframes.size())
                {
                    if(keyframes[over])
                        break;
                    over++;
                }
                while(under>0)
                {
                    if(keyframes[under])
                        break;
                    under--;
                }

                if(over<(int)keyframes.size() && under>=0)
                {
                    float jump = over - under;
                    for(int i=under+1; i<over; i++)
                    {
                        rectangles[i] = ((float)(jump-(i-under))/jump) * rectangles[under]
                                         + ((float)(jump-(over-i))/jump) * rectangles[over];
                    }
                }
            }

            if(c2 == 's') // smooth
            {
                std::vector<cv::Rect> temp_rectangles = rectangles;
                for(size_t i = 1; i < temp_rectangles.size()-1; i++)
                {
                    if(keyframes[i]) continue; // don't smooth keyframes
                    temp_rectangles[i] = 0.5*rectangles[i] + 0.25*rectangles[i-1] + 0.25*rectangles[i+1];
                }
                if(frame_index>1 && !keyframes[frame_index-1])
                {
                    temp_rectangles[frame_index-1] = 0.75*rectangles[frame_index-1] + 0.25*rectangles[frame_index-2];
                }
                rectangles = temp_rectangles;
            }

            if(c2 == 'k') // mark keyframe
            {
                keyframes[frame_index] = true;
            }

            if(c2 == 32) // play/pause
            {
                continuous_play = !continuous_play;
                if(continuous_play)
                    step = 1;
                else
                    step = 0;
            }

            if(c2 == 'h') //  next frame
            {
                if(!continuous_play)
                {
                    step += 1;
                }
            }

            if(c2 == 'j') // previous frame
            {
                if(!continuous_play)
                {
                    step -= 1;
                }
            }

            if(c2 == 'y') // speed down video
            {
                wait_time += 10;
                wait_time == std::min(wait_time, 1000);
            }

            if(c2 == 'u') // speed up video
            {
                wait_time -= 10;
                wait_time = std::max(wait_time, 10);
            }

            if(c2 == 't') // toggle info
            {
                show_info_panel = !show_info_panel;
            }

            if(c2 == -1 && !continuous_play) // no key pressed and paused
            {
                step = 0;
            }

            frame_index += step;

            if(frame_index >= filenames.size()) // video complete
            {
                frame_index = filenames.size() - 1;
                std::cout << "Video is complete. please press esc to end labeling or review it" << std::endl;
                step = 0;
                continuous_play = 0;
            }
        }
    }

    /*
     * write output file
     */
    std::ofstream ofile;
    ofile.open(output_file_name.c_str());
    for(size_t i = 0; i < rectangles.size(); i++)
    {
        ofile << rectangles[i].x << " " << rectangles[i].y << " " <<
                 rectangles[i].width << " " << rectangles[i].height <<
                 std::endl;
    }
    ofile.close();

    return 0;

}

/**
 * @brief getFrame  gets frame using image index
 * @param i         image index
 * @return          cv::Mat frame
 */
cv::Mat getFrame(const int i)
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
static void onMouseCb(int event, int x, int y, int, void* )
{
    if(roi_selection_flag)
    {
        roi_selection.x = MIN(x, roi_origin.x);
        roi_selection.y = MIN(y, roi_origin.y);
        roi_selection.width = std::abs(x - roi_origin.x);
        roi_selection.height = std::abs(y - roi_origin.y);
        roi_selection &= cv::Rect(0, 0, image.cols, image.rows);
    }

    switch(event)
    {
    case CV_EVENT_LBUTTONDOWN:
        roi_origin = cv::Point(x,y);
        roi_selection = cv::Rect(x,y,0,0);
        roi_selection_flag = true;
        break;
    case CV_EVENT_LBUTTONUP:
        roi_selection_flag = false;
        if(roi_selection.width > 0 && roi_selection.height > 0)
        {   // if an roi is selection, init tracking flag
            track_object = -1;
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
void addInfoPanel(cv::Mat &frame, const int curr_frame,
                  const int total_frames, const cv::Rect roi1,
                  const bool show_panel)
{
    if(!show_panel) return;

    // formatting
    cv::Scalar color_gray = cv::Scalar(200,200,200);
    cv::Scalar color_white = cv::Scalar(255,255,255);
    float panel_transparancy = 1.2;
    int line_position = 25;
    double font_scale = 0.75;
    cv::Rect panel_position = cv::Rect(0,0,250,70);
    int line_jump = 35 * font_scale;

    // draw a semi transparent panel
    frame(panel_position) = frame(panel_position) / panel_transparancy;

    // frame info
    std::string output_text = "INFO | " +
            std::to_string(curr_frame) + "/" + std::to_string(total_frames);
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
 * @brief displayHelp   display help
 */
void displayHelp()
{
    std::cout << "CONTROLS" << std::endl;
    std::cout << "\tPause/play: space bar" << std::endl;
    std::cout << "\tGo forwards/backwards on frame: h | j" << std::endl;
    std::cout << "\tSlow/Speed up video y | u" << std::endl;
    std::cout << "\tFinish labeling: esc" << std::endl;
    std::cout << "\tInterpolate interval: i" << std::endl;
    std::cout << "\tSmooth all: s" << std::endl;
    std::cout << "\tMark frame as keyframe: k" << std::endl;
    std::cout << "\tNon-destructive marking: w" << std::endl;
    std::cout << "\tReset(destructive marking): r" << std::endl;
    std::cout << "\tMark frame without object: x" << std::endl;
    std::cout << "\tMove to beginning: b" << std::endl;
    std::cout << "\tShow/hide info panel: t" << std::endl;
}

cv::Rect operator *(const float s, const cv::Rect r1)
{
    cv::Rect r2;
    r2.x = r1.x*s;
    r2.y = r1.y*s;
    r2.width = r1.width*s;
    r2.height = r1.height*s;
    return r2;
}

cv::Rect operator +(const cv::Rect r1, const cv::Rect r2)
{
    cv::Rect r3;
    r3.x = r2.x + r1.x;
    r3.y = r2.y + r1.y;
    r3.width = r2.width + r1.width;
    r3.height = r2.height + r1.height;
    return r3;
}
