#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

constexpr double VIDEO_SAVE_RESIZE_COEF = 0.5;
constexpr int MIN_AREA_PIX = 100;
constexpr int MAX_AREA_PIX = 3000;
constexpr const char* input_video_path = "./data/actions2.mpg";

struct SavedVideoParams {
    int frame_width;
    int frame_height;
    double fps;
};

bool person_alike(const cv::Rect& bbox){
    const bool shaped_as_person = bbox.height > bbox.width;
    const int area = static_cast<int>(bbox.area());
    const bool is_small = area > MIN_AREA_PIX && area < MAX_AREA_PIX;
    return shaped_as_person && is_small;
}

cv::Point center_of_bbox(const cv::Rect& bbox){
    int x, y;
    x = bbox.x + (bbox.width / 2);
    y = bbox.y + (bbox.height / 2);
    return cv::Point(x, y);
}

void process_frame(cv::Mat& frame, cv::Mat& frame_proc){
    // to back and white
    cv::cvtColor(frame, frame_proc, cv::COLOR_BGR2GRAY);

    // blur
    blur(frame_proc, frame_proc, cv::Size(12, 12), cv::Point(-1,-1));

    // Threshold to ensure binary image
    cv::threshold(frame_proc, frame_proc, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::bitwise_not(frame_proc, frame_proc);

    // Morphological filtering
    cv::morphologyEx(frame_proc, frame_proc, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1,-1), 2);

    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(frame_proc, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


    // Loop over contours to get bounding boxes
    for (const auto& contour : contours) {
        cv::Rect bbox = cv::boundingRect(contour);
        if (person_alike(bbox)){
            // draw ROI
            cv::rectangle(frame_proc, bbox, cv::Scalar(125, 125, 125), 4); 
            cv::circle(frame, center_of_bbox(bbox), 1, cv::Scalar(0, 0, 255), 5);
        }
    }
}

struct SavedVideoParams get_params(const cv::VideoCapture& cap){ 
    struct SavedVideoParams params;
    params.frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH) * VIDEO_SAVE_RESIZE_COEF);
    params.frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT) * VIDEO_SAVE_RESIZE_COEF);
    params.fps = cap.get(cv::CAP_PROP_FPS);
    return params;
}

bool initialize_writers(const struct SavedVideoParams& video_params, cv::VideoWriter& writer, cv::VideoWriter& writer_proc){

    // writer for original video with marks
    writer.open(
        "./output/output.mp4", 
        cv::VideoWriter::fourcc('m', 'p', '4', 'v'), // MP4 codec
        video_params.fps, 
        cv::Size(video_params.frame_width, video_params.frame_height)
    );

    if (!writer.isOpened()) {
        std::cerr << "Error: Cannot open the video writer.\n" << std::endl;
        return false;
    }


    // writer for processed video
    writer_proc.open(
        "./output/output_proc.mp4", 
        cv::VideoWriter::fourcc('m', 'p', '4', 'v'), // MP4 codec
        video_params.fps, 
        cv::Size(video_params.frame_width, video_params.frame_height)
    );


    if (!writer_proc.isOpened()) {
        std::cerr << "Error: Cannot open the video writer.\n" << std::endl;
        return false;
    }

    // if success return true
    return true;
}

void write_to_file(const cv::Mat& frame, const struct SavedVideoParams& video_params, cv::VideoWriter& writer, bool is_grey){
    cv::Mat frame_tmp;
    cv::resize(frame, frame_tmp, cv::Size(video_params.frame_width, video_params.frame_height));
    if (is_grey){
        cv::cvtColor(frame_tmp, frame_tmp, cv::COLOR_GRAY2BGR);
    }
    writer.write(frame_tmp);
}

int main() {
    // Open the video file
    cv::VideoCapture cap(input_video_path);
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open the video file." << std::endl;
        return -1;
    }

    // for saving video
    struct SavedVideoParams video_params = get_params(cap);
    cv::VideoWriter writer, writer_proc;
    bool writer_init_is_successful = initialize_writers(video_params, writer, writer_proc);
    if (!writer_init_is_successful){
        std::cerr << "Error: Writers init wasn't successful." << std::endl;
        return -1;
    }
    
    // Window to show the video
    cv::namedWindow("Video with detection", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Video processed", cv::WINDOW_AUTOSIZE);

    cv::Mat frame, frame_proc;
    while (true) {
        // Read next frame
        cap >> frame;

        // Check if frame is empty (end of video)
        if (frame.empty())
            break;

        process_frame(frame, frame_proc);

        // Display the frame
        cv::imshow("Video with detection", frame);
        cv::imshow("Video processed", frame_proc);

        write_to_file(frame, video_params, writer, false);
        write_to_file(frame_proc, video_params, writer_proc, true);

        // Wait for 30ms and break if 'q' is pressed
        if (cv::waitKey(30) == 'q')
            break;
    }

    // Release resources
    cap.release();
    writer.release();
    writer_proc.release();
    cv::destroyAllWindows();
    return 0;
}
