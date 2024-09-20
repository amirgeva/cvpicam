#pragma once

#include <memory>
#include <functional>
#include <opencv2/core.hpp>

namespace cvpicam
{

    class ICamera
    {
    public:
        typedef double timestamp_t;
        typedef std::function<void(timestamp_t,cv::Mat)> frame_callback;

        virtual ~ICamera() = default;

        virtual cv::Mat get_latest_frame() = 0;
        virtual void add_frame_callback(frame_callback cb) = 0;

        static std::shared_ptr<ICamera> create();
    };



}