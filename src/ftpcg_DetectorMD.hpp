#ifndef FTPCG_DETECTORMD_HPP
#define FTPCG_DETECTORMD_HPP

#include "ftpcg.hpp"
#include "ftpcg_ADetectorResid.hpp"

namespace ftpcg {

class DetectorMD : public ADetectorResid {

private:
  bool isErrorResid(const RCP<MV>& resid);
public:
  DetectorMD(std::size_t matrixNumber, std::size_t nMatrices, std::size_t methodNumber, Sensitivity sense);
  const std::size_t getWindowSize();
  void describe(std::ostream& out) const;
};

}

#endif
