#include "ftpcg.hpp"
#include "ftpcg_CriticalState.hpp"
#include "ftpcg_ADetectorResid.hpp"
#include "ftpcg_util.hpp"

namespace ftpcg {

ADetectorResid::ADetectorResid() {}

void ADetectorResid::init() {
  param_ = getCSVEntryD(paramFile_, methodNumber_ * nMatrices_ + matrixNumber_, sensitivity_);
}

void ADetectorResid::setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b) {}

const std::string ADetectorResid::getSensitivityString() const {
  switch (sensitivity_) {
    case LOW:
      return "LOW";
    case MED:
      return "MED";
    case HIGH:
      return "HIGH";
  }
  return "?";
}

ADetectorResid::ADetectorResid(std::size_t matrixNumber, std::size_t nMatrices, std::size_t methodNumber, Sensitivity sense, const char* paramFile) :
  matrixNumber_(matrixNumber),
  nMatrices_(nMatrices),
  methodNumber_(methodNumber),
  sensitivity_(sense),
  paramFile_(paramFile)
{
} 

bool ADetectorResid::isDetectIter(std::size_t iter) {
  return true;
}

const DetectTime ADetectorResid::getDetectTime() { return DETECT_LATE; }

bool ADetectorResid::isError(const RCP<CriticalState>& critState, bool errInjected) {
  return isErrorResid(critState->residuals);
}

}

