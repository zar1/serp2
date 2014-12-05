#ifndef FTPCG_CORRECTORNOP_HPP
#define FTPCG_COORECTORNOP_HPP

#include "ftpcg.hpp"
#include "ftpcg_ICorrector.hpp"

namespace ftpcg {

class CriticalState;

class CorrectorNop : public ICorrector{
public:
  CorrectorNop();

  bool isPreserveIter(std::size_t iter);
  void initProtection(const RCP<CriticalState>& critState);
  void protectState(const RCP<CriticalState>& critState, std::size_t effectiveIter);
  void restoreState(const RCP<CriticalState>& critState, std::size_t& effectiveIter, std::size_t& pendingReworkIters);
  void finalizeProtection(const RCP<CriticalState>& critState);
  void describe(std::ostream& out) const;
    
};

}
#endif
