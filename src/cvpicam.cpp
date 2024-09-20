#include <cvpicam.h>
#include <libcamera-apps/core/libcamera_app.h>
#include <libcamera-apps/core/still_options.hpp>

namespace cvpicam
{
    class CameraApp : public RPiCamApp
    {
    public:
        CameraApp()
        : RPiCamApp(std::make_unique<StillOptions>())
        {
            StillOptions* opts = static_cast<StillOptions*>(GetOptions());
            const char* app_name="cvpicam";
            opts->Parse(1,(char**)&app_name);
        }

        static CameraApp& instance()
        {
            static CameraApp app;
            return app;
        }
    };


    class Camera : public ICamera
    {
        bool                            m_Terminating = false;
        std::mutex                      m_Mutex;
        timestamp_t                     m_LatestTimestamp;
        cv::Mat                         m_LatestFrame;
        std::vector<frame_callback>     m_Callbacks;
        std::thread                     m_EventThread;
        typedef std::chrono::high_resolution_clock clock;
        clock::time_point               m_StartTime;

        void event_thread()
        {
            try
            {
                auto& app=CameraApp::instance();
                app.OpenCamera();
                //app.ConfigureViewfinder();
                //app.ConfigureStill(RPiCamApp::FLAG_STILL_BGR);
                app.ConfigureZsl();
                app.StartCamera();
                m_StartTime=clock::now();
                //auto stream = app.ViewfinderStream();
                RPiCamApp::Stream* stream = app.StillStream();
                while (!m_Terminating)
                {
                    RPiCamApp::Msg msg = app.Wait();
                    if (msg.type == RPiCamApp::MsgType::Timeout)
                    {
                        app.StopCamera();
                        app.StartCamera();
                        continue;
                    }
                    if (msg.type == RPiCamApp::MsgType::Quit)
                        break;
                    else if (msg.type != RPiCamApp::MsgType::RequestComplete)
                    {
                        std::cerr << "Unknown message.  Aborting\n";
                        break;
                    }
                    CompletedRequestPtr& payload = std::get<CompletedRequestPtr>(msg.payload);
                    timestamp_t ts = 1.0e-6*std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - m_StartTime).count();
                    if (payload->buffers.count(stream)==0)
                    {
                        std::cerr << "No buffer found.\n";
                    }
                    else
                    {
                        libcamera::FrameBuffer* fb = payload->buffers[stream];
                        BufferReadSync r(&app, fb);
                        const std::vector<libcamera::Span<uint8_t>> mem = r.Get();
                        StreamInfo info = app.GetStreamInfo(stream);
                        uint8_t* data = static_cast<uint8_t*>(mem[0].data());
                        int flags = CV_8UC1;
                        if (info.pixel_format == libcamera::formats::RGB888)
                            flags = CV_8UC3;
                        image_captured(ts, cv::Mat(info.height, info.width, flags, data, info.stride).clone());
                    }
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << "Exception: " << e.what() << std::endl;
            }
        }

        void image_captured(timestamp_t ts, cv::Mat image)
        {
            for(auto& cb : m_Callbacks)
                cb(ts, image);
            std::scoped_lock<std::mutex> guard(m_Mutex);
            m_LatestTimestamp=ts;
            m_LatestFrame=image;
        }

        void stop()
        {
            m_Terminating=true;
            if (m_EventThread.joinable())
            {
                m_EventThread.join();
            }
        }
    public:
        Camera()
        : m_EventThread([this]{event_thread();})
        {}

        ~Camera()
        {
            stop();
        }

        virtual cv::Mat get_latest_frame() override
        {
            cv::Mat res;
            {
                std::scoped_lock<std::mutex> guard(m_Mutex);
                res = m_LatestFrame;
            }
            return res;
        }
        virtual void add_frame_callback(frame_callback cb) override
        {
            m_Callbacks.push_back(cb);
        }
    };


    std::shared_ptr<ICamera> ICamera::create()
    {
        return std::make_shared<Camera>();
    }

}

