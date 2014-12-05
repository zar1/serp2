#ifndef FTPCG_IDETECTORSLOWIGNORE_HPP
#define FTPCG_IDETECTORSLOWIGNORE_HPP

#include "ftpcg.hpp"
#include "ftpcg_IDetector.hpp"

namespace ftpcg {

class CriticalState;

class DetectorSlowIgnore : public IDetector {
public:
  DetectorSlowIgnore();
  void setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b);
  const std::size_t getWindowSize();
  bool isDetectIter(std::size_t iter);
  const DetectTime getDetectTime();
  bool isError(const RCP<CriticalState>& critState, bool errInjected);
  void describe(std::ostream& out) const;
};

}

#endif
