#include "ftpcg_DetectorIgnore.hpp"

namespace ftpcg {

DetectorIgnore::DetectorIgnore() {}

void DetectorIgnore::setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b) {}

const std::size_t DetectorIgnore::getWindowSize() {
  return 0;
}

bool DetectorIgnore::isDetectIter(std::size_t iter) {
  return false;
}

const DetectTime DetectorIgnore::getDetectTime() {
  return DETECT_LATE;
}

bool DetectorIgnore::isError(const RCP<CriticalState>& critState, bool errInjected) {
  return false;
}

void DetectorIgnore::describe(std::ostream& out) const {
  out << "Ignore";
}

}
