#ifndef FTPCG_INJECTORFLIP_HPP
#define FTPCG_INJECTORFLIP_HPP

#include <Tpetra_Vector_decl.hpp>

#include "ftpcg.hpp"
#include "ftpcg_IInjector.hpp"

namespace ftpcg {

class CriticalState;

class InjectorFlip : public IInjector {
private:
  Ordinal injectionRow_;
  std::size_t vectorInd_;
  std::size_t flippedBit_;
  std::size_t injectionIter_;
  std::size_t rowImpactIndex_;
  double rowImpactPerc_;
  int matrixNum_;
  
  //RCP<Tpetra::Vector<Scalar, Ordinal, Ordinal, Node> > corruptVec_;
public:
  InjectorFlip(std::size_t vectorInd, std::size_t injectionIter, std::size_t rowImpactIndex, std::size_t flippedBit);
  void setA(const RCP<const OP>& A, int matrixNum);
  const bool isNop();
  bool isInjectIter(std::size_t iter);
  void inject(const RCP<CriticalState>& critState);
  void describe(std::ostream& out) const;
};

}

#endif
