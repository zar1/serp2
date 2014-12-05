#ifndef FTPCG_ICORRECTOR_HPP
#define FTPCG_ICORRECTOR_HPP

#include "ftpcg.hpp"

namespace ftpcg {

class CriticalState;

class ICorrector {
public:
  virtual ~ICorrector() {}
  virtual bool isPreserveIter(std::size_t iter) = 0;
  //GDS_alloc, initial put
  virtual void initProtection(const RCP<CriticalState>& critState) = 0;
  //put
  virtual void protectState(const RCP<CriticalState>& critState, std::size_t effectiveIter) = 0;
  //get
  virtual void restoreState(const RCP<CriticalState>& critState, std::size_t& effectiveIter, std::size_t& pendingReworkIters) = 0;
  //free
  virtual void finalizeProtection(const RCP<CriticalState>& critState) = 0;
  virtual void describe(std::ostream& out) const = 0;
    
};

}
#endif
