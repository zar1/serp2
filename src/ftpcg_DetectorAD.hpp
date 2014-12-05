#ifndef FTPCG_DETECTORAD_HPP
#define FTPCG_DETECTORAD_HPP

#include "ftpcg.hpp"
#include "ftpcg_ADetectorResid.hpp"

namespace ftpcg {

class DetectorAD : public ADetectorResid {

private:
  std::size_t windowSize_;

  bool isErrorResid(const RCP<MV>& resid);
public:
  DetectorAD(std::size_t matrixNumber, std::size_t nMatrices, std::size_t methodNumber, Sensitivity sense);
  const std::size_t getWindowSize();
  void describe(std::ostream& out) const;
};

}

#endif
