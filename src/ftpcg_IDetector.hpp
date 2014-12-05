#ifndef FTPCG_IDETECTOR_HPP
#define FTPCG_IDETECTOR_HPP

#include "ftpcg.hpp"

namespace ftpcg {

class CriticalState;

class IDetector {
public:
  virtual ~IDetector() {}
  virtual void setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b) = 0;
  virtual const std::size_t getWindowSize() = 0;
  virtual bool isDetectIter(std::size_t iter) = 0;
  virtual const DetectTime getDetectTime() = 0;
  virtual bool isError(const RCP<CriticalState>& critState, bool errInjected) = 0;
  virtual void describe(std::ostream& out) const = 0;
};

}

#endif
