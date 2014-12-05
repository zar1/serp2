#include "ftpcg.hpp"
#include "ftpcg_ADetectorResid.hpp"
#include "ftpcg_DetectorMD.hpp"

namespace ftpcg {

bool DetectorMD::isErrorResid(const RCP<MV>& resid) {
  //Teuchos::ArrayRCP<const Scalar> residWindow = resid->getData();
  Teuchos::ArrayRCP<const Scalar> residWindow = resid->getData(0);
  return residWindow[0] > residWindow[1] * param_; 
}

DetectorMD::DetectorMD(std::size_t matrixNumber, std::size_t nMatrices, std::size_t methodNumber, Sensitivity sense) :
  ADetectorResid(matrixNumber, nMatrices, methodNumber, sense, "MD_tol")
{
  init();
}


const std::size_t DetectorMD::getWindowSize() {
  return 2;
}

void DetectorMD::describe(std::ostream& out) const {
  out << "MD," << getSensitivityString();
}

}
