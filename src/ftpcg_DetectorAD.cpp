#include <cmath>

#include "ftpcg.hpp"
#include "ftpcg_ADetectorResid.hpp"
#include "ftpcg_DetectorAD.hpp"

namespace ftpcg {

bool DetectorAD::isErrorResid(const RCP<MV>& resid) {
  //Teuchos::ArrayRCP<const Scalar> residWindow = resid->getData();
  Teuchos::ArrayRCP<const Scalar> residWindow = resid->getData(0);
  Scalar avg = 0;
  for (std::size_t i = 1; i < windowSize_; ++i) avg += residWindow[i];
  return residWindow[0] > avg / (windowSize_ - 1);
}

DetectorAD::DetectorAD(std::size_t matrixNumber, std::size_t nMatrices, std::size_t methodNumber, Sensitivity sense) :
  ADetectorResid(matrixNumber, nMatrices, methodNumber, sense, "AD_window_size")
{
  init();
  windowSize_ = ceil(param_) + 1;
}


const std::size_t DetectorAD::getWindowSize() {
  return windowSize_;
}

void DetectorAD::describe(std::ostream& out) const {
  out << "AD," << getSensitivityString();
}

}
