#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

bool person_alike(cv::Rect bbox){
    bool shaped_as_person = bbox.height > bbox.width;
    float area = bbox.height * bbox.width;
    bool is_small = area < 3000;
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

void process_frame(cv::Mat frame, cv::Mat* frame_proc_ptr, cv::Mat* frame_final_ptr){
    cv::Mat frame_proc, frame_final;

    frame_final = frame.clone();

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
        double area = cv::contourArea(contour);
        if (area > 100) {  // filter small blobs
            cv::Rect bbox = cv::boundingRect(contour);
            if (person_alike(bbox)){
                // draw ROI
                cv::rectangle(frame_proc, bbox, cv::Scalar(125, 125, 125), 4); 
                cv::circle(frame_final,	center_of_bbox(bbox), 1, cv::Scalar(0, 0, 255), 3);
            }
        }
    }

    *frame_proc_ptr = frame_proc;
    *frame_final_ptr = frame_final;
}

int main() {
    std::string videoPath = "./data/actions2.mpg";

    // Open the video file
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open the video file." << std::endl;
        return -1;
    }

    // Window to show the video
    cv::namedWindow("Video with detection", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Video processed", cv::WINDOW_AUTOSIZE);

    cv::Mat frame, frame_proc, frame_final;
    while (true) {
        // Read next frame
        cap >> frame;

        // Check if frame is empty (end of video)
        if (frame.empty())
            break;

        process_frame(frame, &frame_proc, &frame_final);

        // Display the frame
        cv::imshow("Video with detection", frame_final);
        cv::imshow("Video processed", frame_proc);

        // Wait for 30ms and break if 'q' is pressed
        if (cv::waitKey(30) == 'q')
            break;
    }

    // Release resources
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
