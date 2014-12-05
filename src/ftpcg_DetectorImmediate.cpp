#include "ftpcg_DetectorImmediate.hpp"

namespace ftpcg {

DetectorImmediate::DetectorImmediate() {}

void DetectorImmediate::setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b) {}

const std::size_t DetectorImmediate::getWindowSize() {
  return 0;
}

bool DetectorImmediate::isDetectIter(std::size_t iter) {
  return true;
}

const DetectTime DetectorImmediate::getDetectTime() {
  return DETECT_LATE;
}

bool DetectorImmediate::isError(const RCP<CriticalState>& critState, bool errInjected) {
  return errInjected;
}

void DetectorImmediate::describe(std::ostream& out) const {
  out << "Immediate";
}

}
