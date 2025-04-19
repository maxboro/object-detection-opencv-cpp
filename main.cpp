#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#define VIDEO_SAVE_RESIZE_COEF 0.5
#define MIN_AREA_PIX 100
#define MAX_AREA_PIX 3000

struct SavedVideoParams {
    int frame_width;
    int frame_height;
    double fps;
};

bool person_alike(cv::Rect bbox){
    bool shaped_as_person = bbox.height > bbox.width;
    float area = bbox.height * bbox.width;
    bool is_small = area > MIN_AREA_PIX && area < MAX_AREA_PIX;
    if (shaped_as_person && is_small){
        return true;
    } else {
        return false;
    }
}

cv::Point center_of_bbox(cv::Rect bbox){
    int x, y;
    x = bbox.x + (bbox.width / 2);
    y = bbox.y + (bbox.height / 2);
    return cv::Point(x, y);
}

void process_frame(const cv::Mat& frame, cv::Mat& frame_proc){
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

struct SavedVideoParams get_params(cv::VideoCapture* cap_ptr){ 
    struct SavedVideoParams params;
    params.frame_width = static_cast<int>(cap_ptr->get(cv::CAP_PROP_FRAME_WIDTH) * VIDEO_SAVE_RESIZE_COEF);
    params.frame_height = static_cast<int>(cap_ptr->get(cv::CAP_PROP_FRAME_HEIGHT) * VIDEO_SAVE_RESIZE_COEF);
    params.fps = cap_ptr->get(cv::CAP_PROP_FPS);
    return params;
}

bool initialize_writers(struct SavedVideoParams* video_params_ptr, cv::VideoWriter* writer_ptr, cv::VideoWriter* writer_proc_ptr){

    // writer for original video with marks
    cv::VideoWriter writer(
        "./output/output.mp4", 
        cv::VideoWriter::fourcc('m', 'p', '4', 'v'), // MP4 codec
        video_params_ptr->fps, 
        cv::Size(video_params_ptr->frame_width, video_params_ptr->frame_height)
    );

    if (!writer.isOpened()) {
        std::cerr << "Error: Cannot open the video writer.\n" << std::endl;
        return false;
    }


    // writer for processed video
    cv::VideoWriter writer_proc(
        "./output/output_proc.mp4", 
        cv::VideoWriter::fourcc('m', 'p', '4', 'v'), // MP4 codec
        video_params_ptr->fps, 
        cv::Size(video_params_ptr->frame_width, video_params_ptr->frame_height)
    );


    if (!writer_proc.isOpened()) {
        std::cerr << "Error: Cannot open the video writer.\n" << std::endl;
        return false;
    }

    *writer_ptr = writer;
    *writer_proc_ptr = writer_proc;
    // if success return true
    return true;
}

void write_to_file_main(cv::Mat frame_final, struct SavedVideoParams* video_params_ptr, cv::VideoWriter* writer_ptr){
    cv::resize(frame_final, frame_final, cv::Size(video_params_ptr->frame_width, video_params_ptr->frame_height));
    writer_ptr->write(frame_final);
}

void write_to_file_proc(cv::Mat frame_proc, struct SavedVideoParams* video_params_ptr, cv::VideoWriter* writer_proc_ptr){
    cv::resize(frame_proc, frame_proc, cv::Size(video_params_ptr->frame_width, video_params_ptr->frame_height));
    cv::cvtColor(frame_proc, frame_proc, cv::COLOR_GRAY2BGR);
    writer_proc_ptr->write(frame_proc);
}

int main() {
    std::string videoPath = "./data/actions2.mpg";

    // Open the video file
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open the video file." << std::endl;
        return -1;
    }

    // for saving video
    struct SavedVideoParams video_params = get_params(&cap);
    cv::VideoWriter writer, writer_proc;
    bool writer_init_is_successful = initialize_writers(&video_params, &writer, &writer_proc);
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

        write_to_file_main(frame, &video_params, &writer);
        write_to_file_proc(frame_proc, &video_params, &writer_proc);

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
