#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

void process_frame(cv::Mat frame, cv::Mat* frame_proc_ptr){
    cv::Mat frame_proc;
    // blur
    blur(frame, frame_proc, cv::Size(9, 9), cv::Point(-1,-1));

    *frame_proc_ptr = frame_proc;
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
    cv::namedWindow("Video", cv::WINDOW_AUTOSIZE);

    cv::Mat frame, frame_proc;
    while (true) {
        // Read next frame
        cap >> frame;

        // Check if frame is empty (end of video)
        if (frame.empty())
            break;

        process_frame(frame, &frame_proc);

        // Display the frame
        cv::imshow("Video", frame);
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
