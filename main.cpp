#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

int main() {
    cv::Mat img = cv::imread("example.jpg");
    if (img.empty()) {
        std::cerr << "Failed to read image\n";
        return -1;
    }
    // std::cout << img << std::endl;
    cv::imshow("Original image", img);
    cv::waitKey(0);
    cv::destroyAllWindows();

    cv::Mat img_transformed;

    // increase brightness
    img.convertTo(img_transformed, -1, 1, 170);

    // cut out part of image
    img_transformed = img_transformed(cv::Range(150, 300), cv::Range::all());

    // blur
    blur(img_transformed, img_transformed, cv::Size(9, 9), cv::Point(-1,-1));

    // show final transformed image
    cv::imshow("Transformed image", img_transformed);

    // std::cout << img_transformed << std::endl;

    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}
