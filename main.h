//
// Created by sanch on 24.10.2022.
//

#ifndef SIMPLE_MAIN_H
#define SIMPLE_MAIN_H

#include <iostream>
#include "thread"
#include <csignal>
#include "circular_fifo.h"
#include <chrono>
#include "sstream"
#include "utils.h"
#include "Printer.h"

// ORB detector
#define ORB_POINTS                  1000
#define ORB_CLOSENESS_THRESHOLD     8.0f
#define ORB_MAX_POINTS              100
#define DEBUG_IMSHOW
//#define ENABLE_ORB_QUEUE

//intel camera and IMU
#define IMAGES_FPS    30
#define IMU_FPS       200
#define FRAMES_TO_SKIP 0
//#define ENABLE_IMU_QUEUE
#define ENABLE_FRAME_QUEUE
#define FPS (IMAGES_FPS / (FRAMES_TO_SKIP+1))

extern volatile sig_atomic_t exit_flag;
extern std::atomic <bool> quitApp;

#endif //SIMPLE_MAIN_H
