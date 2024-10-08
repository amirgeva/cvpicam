# cvpicam
## OpenCV Wrapper for Raspberry pi libcamera

This is a small library to allow opencv apps to easily link to the libcamera API.
Since the API is very low level, 
and in many cases the developer simply wants an interface similar in 
simplicity to the OpenCV VideoCapture class, there's a need for a wrapper.

The simple usage example included is short:
```
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
```


                                                                        

