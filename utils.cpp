//
// Created by step305 on 25.07.2021.
//

#include "utils.h"

const char color_fmt_red[] = "\033[1;31m";
const char color_fmt_blue[] = "\033[1;34m";
const char color_fmt_green[] = "\033[1;32m";
const char color_fmt_yellow[] = "\033[1;33m";
const char color_fmt_reset[] = "\033[0m";

const std::string FMT_BLUE = std::string(color_fmt_blue);
const std::string FMT_RED = std::string(color_fmt_red);
const std::string FMT_YELLOW = std::string(color_fmt_yellow);
const std::string FMT_GREEN = std::string(color_fmt_green);
const std::string FMT_RESET = std::string(color_fmt_reset);

long long unsigned get_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

float calc_fps(std::chrono::system_clock::time_point &last_calc, uint16_t &frame_counter) {
    auto t_now = std::chrono::system_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::seconds>(t_now - last_calc).count();
    if (t > FPS_CALC_PERIOD) {
        float fps = (float)frame_counter / (float) t * 10.0f;
        fps = (float)((int)(fps + 0.5f)) / 10.0f;
        frame_counter = 0;
        last_calc = t_now;
        return fps;
    } else {
        frame_counter++;
        return -1.0f;
    }
}

std::string num_to_string(float val, int precision) {
    std::string result = std::to_string(val);
    result = result.substr(0, result.find('.') + 1 + precision);
    return result;
}

std::string num_to_string(int val) {
    return std::to_string(val);
}

void get_descriptor(cv::Mat& mat, int row, uchar *a)
{
    auto* p = mat.ptr<uchar>(row);
    memcpy(a,p,sizeof(uchar) * mat.cols);
}

//Copy row of Mat to uchar array
void get_descriptor32(cv::Mat& mat, int row, int32_t *a)
{
    auto* p = mat.ptr<int32_t>(row);
    memcpy(a, p, sizeof(uchar) * mat.cols);
}
