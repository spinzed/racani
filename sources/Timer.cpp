#include "Timer.h"

#include <sys/time.h>
#include <string>
#include <stdio.h>
#include <iostream>

Timer::Timer() {
    reset();
}

Timer Timer::start() {
    return Timer();
}

void Timer::reset() {
    gettimeofday(&t1, NULL);
}

double Timer::elapsed() {
    gettimeofday(&t2, NULL);
    double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;    // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;        // us to ms
    return elapsedTime;
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
