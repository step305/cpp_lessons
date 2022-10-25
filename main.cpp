#include "main.h"
#include "camera_driver.h"
#include "RealsenseD455.h"
#include "camera_driver.h"


circ_fifo::CircularFifo <ImageMessageStruct, images_queue_len> queueImages(true);
circ_fifo::CircularFifo <CAMMessageStruct, camera_queue_len> queueCamera(true);
circ_fifo::CircularFifo <RealsenseIMUMessageStruct, imu_queue_len> queueIMU(true);

std::atomic <bool> quitApp;
std::atomic <bool> cameraStarted;

Printer::Printer printer = Printer::Printer();

volatile sig_atomic_t exit_flag;

void exit_catch(int sig) {
    printer.printError("Logger:: User stop requested!");
    exit_flag = 1;
}

void exit_cath_init() {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = exit_catch;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
}

int main() {
    exit_cath_init();

    std::thread camera_thread( realsenseStreamThread );
    std::thread orb_detector_thread( ORBdetectorStreamThread );
    std::thread printer_thread( &Printer::Printer::printLoop, printer );
    std::thread error_thread( &Printer::Printer::printErrorLoop, printer );

    printer.print("Thread safe app.");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if (!printer.print("Idle thread working!"))
        printer.printError("error pushing string from idle thread!");


    while (true) {
        if (exit_flag) {
            quitApp = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            printer.stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            break;
        }
    }

    if (camera_thread.joinable())
        camera_thread.join();
    if (orb_detector_thread.joinable())
        orb_detector_thread.join();
    if (printer_thread.joinable())
        printer_thread.join();
    if (error_thread.joinable())
        error_thread.join();

    return 0;
}
