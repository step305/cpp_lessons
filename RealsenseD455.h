//
// Created by driver on 05.11.2021.
//

#ifndef SLAM_LOGGER_REALSENSED455_H
#define SLAM_LOGGER_REALSENSED455_H

#pragma once
#include <string>
#include <map>
#include <librealsense2/rs.hpp>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include "main.h"

#define images_queue_len    32
#define imu_queue_len       200

typedef struct {
    cv::Mat frame;
    long long unsigned ts;
} ImageMessageStruct;

typedef struct {
    float rate[3];
    long long unsigned ts;
} RealsenseIMUMessageStruct;

extern circ_fifo::CircularFifo <ImageMessageStruct, images_queue_len> queueImages;
extern circ_fifo::CircularFifo <RealsenseIMUMessageStruct, imu_queue_len> queueIMU;

int realsenseStreamThread();

#endif //SLAM_LOGGER_REALSENSED455_H
