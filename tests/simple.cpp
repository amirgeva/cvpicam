#include <iostream>
#include <opencv2/opencv.hpp>
#include <cvpicam.h>

int main(int argc, char* argv[])
{
    cv::Mat last_frame;
    auto camera = cvpicam::ICamera::create();
    camera->add_frame_callback([&last_frame](const cvpicam::ICamera::timestamp_t ts, const cv::Mat& image)
    {
        // Low latency callback!
        // Save image and process in other thread
        last_frame=image;
        std::cout << "Received image at " << ts << std::endl;
    });
    std::cin.ignore();
    if (!last_frame.empty())
        cv::imwrite("frame.png",last_frame);
    return 0;
}
