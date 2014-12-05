#ifndef FTPCG_INJECTORNOP_HPP
#define FTPCG_INJECTORNOP_HPP

#include <Tpetra_Vector_decl.hpp>

#include "ftpcg.hpp"
#include "ftpcg_IInjector.hpp"

namespace ftpcg {

class CriticalState;

class InjectorNop : public IInjector {
private:
public:
  InjectorNop();
  void setA(const RCP<const OP>& A, int matrixNum);
  const bool isNop();
  bool isInjectIter(std::size_t iter);
  void inject(const RCP<CriticalState>& critState);
  void describe(std::ostream& out) const;
};

}

#endif
