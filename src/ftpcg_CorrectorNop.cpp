#include "ftpcg_CorrectorNop.hpp"

namespace ftpcg {

CorrectorNop::CorrectorNop() {}

bool CorrectorNop::isPreserveIter(std::size_t iter) {
  return false;
}

void CorrectorNop::initProtection(const RCP<CriticalState>& critState) {}
void CorrectorNop::protectState(const RCP<CriticalState>& critState, std::size_t effectiveIter) {}
void CorrectorNop::restoreState(const RCP<CriticalState>& critState, std::size_t& effectiveIter, std::size_t& pendingReworkIters) {
  pendingReworkIters = 0;
}
void CorrectorNop::finalizeProtection(const RCP<CriticalState>& critState) {}

void CorrectorNop::describe(std::ostream& out) const {
  out << "Nop";
}

}
