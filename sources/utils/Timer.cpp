#include "utils/Timer.h"
#include <chrono>
#include <iostream>
#include <string>

Timer::Timer() { reset(); }

Timer Timer::start() { return Timer(); }

void Timer::reset() {
    // Capture the current time point using high-resolution clock
    t1 = std::chrono::high_resolution_clock::now();
}

double Timer::elapsed() {
    // Get the current time point and compute the difference in milliseconds
    t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTime = t2 - t1;
    return elapsedTime.count();
}

void Timer::printElapsed(std::string message) {
    double elapsedTime = elapsed();
    size_t index = message.find("$");
    if (index == std::string::npos) {
        message += std::to_string(elapsedTime) + "ms";
    } else {
        message.replace(index, 1, std::to_string(elapsedTime) + "ms");
    }
    std::cout << message << std::endl;
}
