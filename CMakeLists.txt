cmake_minimum_required(VERSION 3.16)
project(cvpicam)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_library(rpicam_app_so rpicam_app.so REQUIRED)
find_library(libcam_so libcamera.so REQUIRED)

add_library(cvpicam STATIC src/cvpicam.cpp include/cvpicam.h)
target_include_directories(cvpicam
PUBLIC
    include/
PRIVATE
    /usr/include/libcamera
    /usr/include/rpicam-apps
)

target_link_libraries(cvpicam PUBLIC
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    ${rpicam_app_so}
    ${libcam_so}
)


add_executable(simple tests/simple.cpp)
target_link_libraries(simple PRIVATE cvpicam)
