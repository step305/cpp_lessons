#include <iostream>
#include "thread"
#include <signal.h>
#include "circular_fifo.h"
#include <chrono>
#include "sstream"

circ_fifo::CircularFifo <std::string, 100> queuePrint(true);
std::atomic<bool> quitPrint;

long long unsigned get_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

auto t0 = get_us();

int printThread() {
    std::string string_to_print;
    std::cout << "Print thread started!!" << std::endl;
    while (true) {
        if (quitPrint) {
            break;
        }
        if (queuePrint.pop(string_to_print)) {
            std::cout << std::to_string(get_us() - t0) << "us: " << string_to_print << std::endl;
        }
    }
    std::cout << "Print thread finished!" << std::endl;
    return 0;
}

int cameraThread() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if (!queuePrint.push("Camera thread thread started!"))
        std::cerr << "error pushing string from Camera thread thread!" << std::endl;
    while (true) {
        if (quitPrint)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (!queuePrint.push(std::to_string(get_us() - t0) + "us: Camera thread thread going!!"))
            std::cerr << "error pushing cycled string from Camera thread thread!" << std::endl;
    }
    return 0;
}

int orbDetectorThread() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if (!queuePrint.push("ORB detector thread started!"))
        std::cerr << "error pushing string from ORB detector thread!" << std::endl;
    while (true) {
        if (quitPrint)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (!queuePrint.push(std::to_string(get_us() - t0) + "us: ORB detector thread thread going!!"))
            std::cerr << "error pushing cycled string from ORB detector thread thread!" << std::endl;
    }
    return 0;
}

int main() {
    std::thread print_thread( printThread );
    std::thread camera_thread( cameraThread );
    std::thread orb_detector_thread( orbDetectorThread );

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if (!queuePrint.push("Idle thread working!"))
        std::cerr << "error pushing string from idle thread!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    quitPrint = true;
    if (!queuePrint.push("Fake message to stop blocking thread!"))
        std::cerr << "error pushing second string from idle thread!" << std::endl;

    if (camera_thread.joinable())
        camera_thread.join();
    if (orb_detector_thread.joinable())
        orb_detector_thread.join();
    if (print_thread.joinable())
        print_thread.join();

    return 0;
}
