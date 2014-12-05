#ifndef FTPCG_CLOCK_HPP
#define FTPCG_CLOCK_HPP

#include <time.h>
#include <iostream>

namespace ftpcg {

class Clock {
private:
  struct timespec time_start_, time_stop_, time_acc_;
  bool isRunning_;

public:
  Clock();
  void start();
  void stop();
  void reset();
  bool isRunning();
  std::ostream& report(std::ostream& out) const;
};

inline void Clock::start() {
  isRunning_ = true;
  clock_gettime(CLOCK_MONOTONIC, &time_start_);
}

inline void Clock::stop() {
  clock_gettime(CLOCK_MONOTONIC, &time_stop_);
  isRunning_ = false;
  int carry = 0;
  long nsec = time_acc_.tv_nsec + time_stop_.tv_nsec - time_start_.tv_nsec;
  if (nsec < 0) {
    carry = -1;
    time_acc_.tv_nsec = nsec + 1e9;
  } else if (nsec > 1e9) {
    carry = 1;
    time_acc_.tv_nsec = nsec - 1e9;
  } else {
    time_acc_.tv_nsec = nsec;
  }
  time_acc_.tv_sec += time_stop_.tv_sec - time_start_.tv_sec + carry;
}

}

#endif
