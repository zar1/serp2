#include <iostream>
#include <iomanip>
#include "ftpcg_Clock.hpp"

namespace ftpcg {

Clock::Clock() {
  reset();
}

void Clock::reset() {
  time_acc_.tv_sec = 0;
  time_acc_.tv_nsec = 0;
  isRunning_ = false;
}


bool Clock::isRunning() {
  return isRunning_;
}

std::ostream& Clock::report(std::ostream& out) const {
  out << time_acc_.tv_sec << "." << std::setfill('0') << std::setw(9) 
    << time_acc_.tv_nsec;
  return out;
}

}
