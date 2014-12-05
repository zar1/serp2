#ifndef FTPCG_INJECTORWORSTROW_HPP
#define FTPCG_INJECTORWORSTROW_HPP

#include <Tpetra_Vector_decl.hpp>

#include "ftpcg.hpp"
#include "ftpcg_IInjector.hpp"

namespace ftpcg {

class CriticalState;

class InjectorWorstRow : public IInjector {
private:
  Ordinal injectionRow_;
  std::size_t vectorInd_;
  Scalar errorFactor_;
  std::size_t injectionIter_;
  RCP<Tpetra::Vector<Scalar, Ordinal, Ordinal, Node> > corruptVec_;
public:
  InjectorWorstRow(std::size_t vectorInd, std::size_t injectionIter_, Scalar errorFactor);
  void setA(const RCP<const OP>& A, int matrixNumber);
  const bool isNop();
  bool isInjectIter(std::size_t iter);
  void inject(const RCP<CriticalState>& critState);
  void describe(std::ostream& out) const;
};

}

#endif
