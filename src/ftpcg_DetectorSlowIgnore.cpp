#include "ftpcg_DetectorSlowIgnore.hpp"

namespace ftpcg {

DetectorSlowIgnore::DetectorSlowIgnore() {}

void DetectorSlowIgnore::setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b) {}

const std::size_t DetectorSlowIgnore::getWindowSize() {
  return 0;
}

bool DetectorSlowIgnore::isDetectIter(std::size_t iter) {
  //The only difference between this and DetectorIgnore is this function
  return true;
}

const DetectTime DetectorSlowIgnore::getDetectTime() {
  return DETECT_LATE;
}

bool DetectorSlowIgnore::isError(const RCP<CriticalState>& critState, bool errInjected) {
  return false;
}

void DetectorSlowIgnore::describe(std::ostream& out) const {
  out << "Ignore (slow)";
}

}
