#ifndef FTPCG_DETECTOR_ABFT_HPP
#define FTPCG_DETECTOR_ABFT_HPP

#include "ftpcg.hpp"
#include "ftpcg_IDetector.hpp"

namespace ftpcg {

class CriticalState;

class DetectorABFT : public IDetector {
private:
  RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> > A_;
  RCP<MV> b_;
  RCP<MV> Ax_;
  RCP<MV> addVec_;
  //Scalar pApRaw_;
  //Teuchos::ArrayView<Scalar>  pAp_;
  Scalar pAp_;
  Scalar residDenom_;
  Scalar tol_;
  std::size_t freq_;
public:
  DetectorABFT(Scalar tol, std::size_t freq);
  void setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b);
  const std::size_t getWindowSize();
  bool isDetectIter(std::size_t iter);
  const DetectTime getDetectTime();
  bool isError(const RCP<CriticalState>& critState, bool errInjected);
  void describe(std::ostream& out) const;
};

}

#endif
