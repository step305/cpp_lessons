//
// Created by step305 on 25.07.2021.
//

#ifndef SLAM_LOGGER_UTILS_H
#define SLAM_LOGGER_UTILS_H

#include <chrono>
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"

#define FPS_CALC_PERIOD 10

extern const char color_fmt_red[];
extern const char color_fmt_blue[];
extern const char color_fmt_green[];
extern const char color_fmt_yellow[];
extern const char color_fmt_reset[];

extern const std::string FMT_BLUE;
extern const std::string FMT_YELLOW;
extern const std::string FMT_RED;
extern const std::string FMT_GREEN;
extern const std::string FMT_RESET;

long long unsigned get_us();
void get_descriptor(cv::Mat& mat, int row, uchar *a);
void get_descriptor32(cv::Mat& mat, int row, int32_t *a);
float calc_fps(std::chrono::system_clock::time_point &last_calc, uint16_t &frame_counter);
std::string num_to_string(float val, int precision);
std::string num_to_string(int val);

#endif //SLAM_LOGGER_UTILS_H
