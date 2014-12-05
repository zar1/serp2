#ifndef FTPCG_CORRECTORSR_HPP
#define FTPCG_COORECTORSR_HPP

#include "ftpcg.hpp"
#include "ftpcg_ICorrector.hpp"

namespace ftpcg {

class CorrectorSR : public ICorrector {
private:
  std::size_t cpInterval_;
  bool restartOnly_;
  std::size_t lastCPIter_;
  Scalar prevRho_;
public:
  CorrectorSR(std::size_t cpInterval, bool restartOnly);

  bool isPreserveIter(std::size_t iter);
  void initProtection(const RCP<CriticalState>& critState);
  void protectState(const RCP<CriticalState>& critState, std::size_t effectiveIter);
  void restoreState(const RCP<CriticalState>& critState, std::size_t& effectiveIter, std::size_t& pendingReworkIters);
  void finalizeProtection(const RCP<CriticalState>& critState);
  void describe(std::ostream& out) const;
};

}
#endif
