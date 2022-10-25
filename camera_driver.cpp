//
// Created by step305 on 25.07.2021.
//

#include "camera_driver.h"

//#define CALIBR_CAMERA

//RGB camera
cv::Mat camMtx = (cv::Mat1d(3, 3) << 377.636383056641, 0, 313.422821044922, 0, 377.265106201172, 242.470718383789, 0, 0, 1);
cv::Mat distCoeffs = (cv::Mat1d(1, 5) << -0.0598541013896465, 0.0736653730273247, -0.000628226727712899, 0.000477661902550608, -0.0230451133102179);

bool response_comparator(const cv::KeyPoint& p1, const cv::KeyPoint& p2)
{
    return p1.response > p2.response;
}

int ORBdetectorStreamThread() {
    printer.print(FMT_YELLOW + "ORBThread:: Started!" + FMT_RESET);
    auto last_print = std::chrono::system_clock::now();
    uint16_t fps_cnt = 0;
    float fps;

    cv::Mat empty_frame;
    cv::Mat frame(640, 480, cv::DataType<float>::type);
    cv::Mat frame_gray;
    long long unsigned frame_ts;

    printer.print(FMT_YELLOW + "ORBThread:: ORB detector thread started." +FMT_RESET);

    //cv::Ptr<cv::cuda::ORB> orb_detector = cv::cuda::ORB::create(ORB_POINTS);
    cv::Ptr<cv::ORB> orb_detector_cpu = cv::ORB::create(ORB_POINTS);

#ifdef CALIBR_CAMERA
    int img_cnt = 0;
    int skip_cnt = 0;
    char nam_img[] = "calibr/img_000000000000.bmp";
#endif
    int frames_cnt = 0;
    int skip_frames = 0;

    while (!quitApp) {
        ImageMessageStruct image_msg;
        if (!queueImages.pop(image_msg)) {
            continue;
        }

        frame = image_msg.frame;
        frame_ts = image_msg.ts;

        if (frame.empty()){
            continue;
        }

        fps = calc_fps(last_print, fps_cnt);
        if (fps >= 0.0f) {
            printer.print(FMT_YELLOW + "ORBThread:: frames: " + num_to_string(fps, 1) + "fps" + FMT_RESET);
        }

        cameraStarted = true;

        //cv::cuda::GpuMat gpu_frame1, gpu_frame2;
        //cv::cuda::GpuMat gpu_gray_frame;

        //gpu_frame2.upload(frame);

        cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);

        //cv::cuda::GpuMat d_keypoints;
        //cv::cuda::GpuMat d_descriptors, d_descriptors_32F;

        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        std::vector<cv::Point2f> dist_points;
        std::vector<cv::Point2f> undist_points;

        orb_detector_cpu->detect(frame_gray, keypoints);
        if (keypoints.empty()) {
            CAMMessageStruct msg = {frame, descriptors, undist_points, frame_ts};
#ifdef ENABLE_ORB_QUEUE
            if (!queueCamera.push(msg)) {
                printer.printError("ORBThread:: Error!:: Queue full!");
                exit_flag = 1;
                break;
            }
#endif
            continue;
        }

        std::vector<cv::KeyPoint> keypoints_srt;
        std::sort(keypoints.begin(), keypoints.end(),
                  [](const cv::KeyPoint &a, const cv::KeyPoint &b) { return a.response > b.response; });

        for ( auto & keypoint : keypoints ){
            bool too_close = false;
            bool swapped = false;
            for ( auto & keypoint_srt : keypoints_srt )
                if (ORB_CLOSENESS_THRESHOLD > cv::norm(keypoint.pt - keypoint_srt.pt)) {
                    if (keypoint.response > keypoint_srt.response) {
                        std::swap(keypoint, keypoint_srt);
                        swapped = true;
                    } else {
                        too_close = true;
                    }
                }
            if (!too_close && !swapped)
                keypoints_srt.push_back(keypoint);
        }

        cv::KeyPointsFilter::retainBest(keypoints_srt, ORB_MAX_POINTS);

        //d_keypoints.upload(keypoints_srt);

        orb_detector_cpu->compute(frame, keypoints_srt, descriptors);

        //d_descriptors.download(descriptors);

        cv::KeyPoint::convert(keypoints_srt, dist_points);
        cv::undistortPoints(dist_points, undist_points, camMtx, distCoeffs);

        std::vector<cv::KeyPoint> keypoints_fin;
        cv::KeyPoint::convert(dist_points, keypoints_fin);

        //cv::drawKeypoints(frame, keypoints_fin, frame);
#ifdef CALIBR_CAMERA
        if (skip_cnt == 0) {
            sprintf(nam_img, "calibr/img_%d.bmp", img_cnt);
            img_cnt++;
            skip_cnt = 15;
            cv::imwrite(nam_img, frame);
            std::cout << color_fmt_red << "ORBThread:: Save frame:: #" << img_cnt << color_fmt_reset << std::endl;
        } else {
            skip_cnt--;
        }
#endif

        CAMMessageStruct msg = {frame, descriptors, undist_points, frame_ts};
#ifdef ENABLE_ORB_QUEUE
        if (queueCamera.push(msg) == false) {
            printer.printError("ORBThread:: Error!:: Queue full!");
            exit_flag = 1;
            quitCamera = true;
            break;
        }
#endif
#ifdef DEBUG_IMSHOW
        cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
        cv::imshow("MyWindow", frame);
        cv::waitKey(1);
#endif
    }
    printer.print(FMT_YELLOW + "ORBThread:: " + num_to_string(frames_cnt) + "total" + FMT_RESET);
    printer.print(FMT_YELLOW + "ORBThread:: Finished!" + FMT_RESET);
    return 0;
}
