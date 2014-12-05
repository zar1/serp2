#ifndef FTPCG_IINJECTOR_HPP
#define FTPCG_IINJECTOR_HPP

#include "ftpcg.hpp"

namespace ftpcg {

class CriticalState;

class IInjector {
public:
  virtual ~IInjector() {}
  virtual void setA(const RCP<const OP>& A, int matrixNum) = 0;
  virtual const bool isNop() = 0;
  virtual bool isInjectIter(std::size_t iter) = 0;
  virtual void inject(const RCP<CriticalState>& critState) = 0;
  virtual void describe(std::ostream& out) const = 0;
};

}

#endif
