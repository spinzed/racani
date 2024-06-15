#pragma once

#include <sys/time.h>
#include <string>

class Timer {
public:
    static Timer start();
    void reset();
    double elapsed();
    void printElapsed(std::string message);
private:
    struct timeval t1, t2;
    Timer();
};