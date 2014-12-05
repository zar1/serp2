#ifndef FTPCG_IDETECTORIGNORE_HPP
#define FTPCG_IDETECTORIGNORE_HPP

#include "ftpcg.hpp"
#include "ftpcg_IDetector.hpp"

namespace ftpcg {

class CriticalState;

class DetectorIgnore : public IDetector {
public:
  DetectorIgnore();
  void setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b);
  const std::size_t getWindowSize();
  bool isDetectIter(std::size_t iter);
  const DetectTime getDetectTime();
  bool isError(const RCP<CriticalState>& critState, bool errInjected);
  void describe(std::ostream& out) const;
};

}

#endif
