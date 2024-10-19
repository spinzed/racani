#pragma once

#include <chrono>
#include <string>

class Timer {
  public:
    static Timer start();
    void reset();
    double elapsed();
    void printElapsed(std::string message);

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;
    Timer();
};
