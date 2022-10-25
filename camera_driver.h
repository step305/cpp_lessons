//
// Created by step305 on 24.10.2022.
//

#ifndef SIMPLE_CAMERA_DRIVER_H
#define SIMPLE_CAMERA_DRIVER_H

#include <cstdio>
#include <iostream>
#include <deque>
#include <cstdlib>
#include <string>
#include <numeric>
#include <chrono>
#include <ctime>
#include <opencv4/opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>

#include "opencv2/core/core.hpp"
//#include "opencv2/cudabgsegm.hpp"
//#include "opencv2/core/cuda.hpp"
//#include "opencv2/core/cuda_stream_accessor.hpp"
//#include "opencv2/cudafeatures2d.hpp"

//#include "opencv2/cudaimgproc.hpp"
//#include "opencv2/cudaarithm.hpp"
//#include "opencv2/cudafilters.hpp"
//#include "opencv2/cudawarping.hpp"

#include <vector>
#include "circular_fifo.h"
#include "RealsenseD455.h"
#include "utils.h"
#include "thread"
#include <csignal>
#include "main.h"

typedef struct {
    cv::Mat frame;
    cv::Mat descriptors;
    std::vector<cv::Point2f>  points;
    long long unsigned ts;
} CAMMessageStruct;

#define camera_queue_len 32

extern std::atomic<bool> cameraStarted;
extern circ_fifo::CircularFifo <CAMMessageStruct,camera_queue_len> queueCamera;

int ORBdetectorStreamThread();

#endif //SIMPLE_CAMERA_DRIVER_H
