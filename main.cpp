#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

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

    cv::Mat frame;
    while (true) {
        // Read next frame
        cap >> frame;

        // Check if frame is empty (end of video)
        if (frame.empty())
            break;

        // Display the frame
        cv::imshow("Video", frame);

        // Wait for 30ms and break if 'q' is pressed
        if (cv::waitKey(30) == 'q')
            break;
    }

    // Release resources
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
