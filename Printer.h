//
// Created by sanch on 24.10.2022.
//

#ifndef SIMPLE_PRINTER_H
#define SIMPLE_PRINTER_H

#include "circular_fifo.h"
#include <iostream>
#include "utils.h"
#include "thread"
#include "chrono"
#include "atomic"
#include "mutex"

namespace Printer {
    class Printer {
    public:
        circ_fifo::CircularFifo <std::string, 100> *queuePrint;
        circ_fifo::CircularFifo <std::string, 100> *queuePrintError;

        long long unsigned t_program_start;
        std::atomic <bool> *quit;

        Printer() {
            this -> t_program_start = get_us();
            this -> quit = new std::atomic <bool> (false);
            this -> queuePrint = new circ_fifo::CircularFifo<std::string, 100>(true);
            this -> queuePrintError = new circ_fifo::CircularFifo<std::string, 100>(true);
        }

        bool print(const std::string &str) const {
            return this -> queuePrint -> push(std::to_string(get_us() - this -> t_program_start) + "us: "  + str);
        }

        bool printError(const std::string &str) const {
            return this -> queuePrintError -> push(std::to_string(get_us() - this -> t_program_start) + "us: " + str);
        }

        int printLoop() const{
            std::string string_to_print;
            std::cout << "Print thread started!!" << std::endl;
            while (true) {
                if (*(this -> quit)) {
                    break;
                }
                if (this -> queuePrint->pop(string_to_print)) {
                    std::cout << string_to_print << std::endl;
                }
            }
            std::cout << "Print thread finished!" << std::endl;
            return 0;
        }

        int printErrorLoop() const{
            std::string string_to_print;
            while (true) {
                if (*(this -> quit)) {
                    break;
                }
                if (this -> queuePrintError->pop(string_to_print)) {
                    std::cerr << color_fmt_red << string_to_print << color_fmt_reset << std::endl;
                }
            }
            return 0;
        }

        void stop() const {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            *(this -> quit) = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            this -> printError(".");
            this -> print("..");
        }
    };
}

extern Printer::Printer printer;

#endif //SIMPLE_PRINTER_H
