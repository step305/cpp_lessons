#include "RealsenseD455.h"
#include "RealsenseUtils.h"

#define DEFAULT_SCALE_FACTOR (0.5f * 0.005f)

int realsenseStreamThread()
try {
    uint16_t gyro_counter = 0;
    uint16_t frame_counter = 0;
    float fps_gyro, fps_frame;
    auto last_print_frame = std::chrono::system_clock::now();
    auto last_print_gyro = std::chrono::system_clock::now();
    int skip_frames = 0;
    bool skip_imu_packet = false;

    std::string serial;
    if (!device_with_streams({ RS2_STREAM_GYRO  }, serial)) {
        printer.printError("realsenseThread:: Not found supported Realsense camera!");
        exit_flag = 1;
        return EXIT_SUCCESS;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    printer.print(FMT_BLUE + "realsenseThread:: Started!" + FMT_RESET);

    rs2::pipeline pipe_temp;
    rs2::pipeline_profile selection = pipe_temp.start();
    rs2::device selected_device = selection.get_device();
    auto depth_sensor = selected_device.first<rs2::depth_sensor>();

    if (depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED)) {
        depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f); // Disable emitter
    }
    if (depth_sensor.supports(RS2_OPTION_LASER_POWER)) {
        depth_sensor.set_option(RS2_OPTION_LASER_POWER, 0.f); // Disable laser
    }
    pipe_temp.stop();

    rs2::pipeline pipe;
    printer.print(FMT_BLUE + "realsenseThread:: Realsense camera configured. Laser set to off." + FMT_RESET);

    // Create a configuration for configuring the pipeline with a non default profile
    rs2::config cfg;
    if (!serial.empty())
        cfg.enable_device(serial);
    // Add gyro stream
    cfg.enable_stream(RS2_STREAM_GYRO, RS2_FORMAT_MOTION_XYZ32F, IMU_FPS);
    // Enable image stream
    cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, IMAGES_FPS);

    // Define frame callback
    // The callback is executed on a sensor thread and can be called simultaneously from multiple sensors
    // Therefore any modification to common memory should be done under lock
    std::mutex data_mutex;

    rs2_vector dat_prev = {0.0f, 0.0f, 0.0f};

    auto camera_callback = [&](const rs2::frame& frame) {
        std::lock_guard<std::mutex> lock(data_mutex);
        rs2_vector dat;
        RealsenseIMUMessageStruct imu_frame;
        ImageMessageStruct image_frame;

        if (auto frame_motion = frame.as<rs2::motion_frame>()) {
            fps_gyro = calc_fps(last_print_gyro, gyro_counter);
            if (fps_gyro >= 0.0f) {
                printer.print(FMT_BLUE + "realsenseThread:: gyro: " + num_to_string(fps_gyro, 1) + "fps" + FMT_RESET);
            }
            dat = frame_motion.get_motion_data();
            imu_frame.rate[2] = (dat.x + dat_prev.x)*DEFAULT_SCALE_FACTOR;
            imu_frame.rate[0] = (dat.y + dat_prev.y)*DEFAULT_SCALE_FACTOR;
            imu_frame.rate[1] = (dat.z + dat_prev.z)*DEFAULT_SCALE_FACTOR;
            dat_prev = dat;
            imu_frame.ts = get_us();

            if (!skip_imu_packet) {
                skip_imu_packet = true;

#ifdef ENABLE_IMU_QUEUE
                if (!queueIMU.push(imu_frame)) {
                    printer.printError("realsenseThread:: Error!:: IMU queue full!!");
                    exit_flag = 1;
                }
#endif
            } else {
                skip_imu_packet = false;
            }
        } else if (auto frame_image = frame.as<rs2::frameset>()) {
            fps_frame = calc_fps(last_print_frame, frame_counter);
            if (fps_frame >= 0.0f) {
                printer.print(FMT_BLUE + "realsenseThread:: video: " + num_to_string(fps_frame, 1) + "fps" + FMT_RESET);
            }
            rs2::video_frame img = frame_image.get_color_frame();
            const int w = img.get_width();
            const int h = img.get_height();
            if ((h > 0) && (w >0)) {
                cv::Mat image(cv::Size(w, h), CV_8UC3, (void *) img.get_data());
                image_frame.frame = image;
                image_frame.ts = get_us();

                if (skip_frames < FRAMES_TO_SKIP) {
                    skip_frames++;
                } else {
                    skip_frames = 0;
#ifdef ENABLE_FRAME_QUEUE
                    if (!queueImages.push(image_frame)) {
                        printer.printError("realsenseThread:: Error!:: Image queue full!!");
                        exit_flag = 1;
                    }
#endif
                }
            }
        }
    };

    // Start streaming through the callback
    rs2::pipeline_profile profiles = pipe.start(cfg, camera_callback);

    printer.print(FMT_BLUE + "realsenseThread:: Camera configured!" + FMT_RESET);

    while (!quitApp) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // send empty frame to unlock waiting thread
    ImageMessageStruct image_frame;
    queueImages.push(image_frame);
    printer.print(FMT_BLUE + "realsenseThread:: Finished!" + FMT_RESET);
    return 0;
}

catch (const rs2::error & e) {
    printer.printError("realsenseThread:: RealSense error calling " + std::string(e.get_failed_function()) +
        "(" + std::string(e.get_failed_args()) + ") " + std::string(e.what()));
    exit_flag = 1;
    return -1;
}

catch (const std::exception& e) {
    printer.printError("realsenseThread::" + std::string(e.what()));
    exit_flag = 1;
    return -2;
}
