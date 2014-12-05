#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_Comm.hpp>
#include <Teuchos_CommHelpers.hpp>
#include <Teuchos_OrdinalTraits.hpp>

#include <Tpetra_Map_decl.hpp>

#include "ftpcg_InjectorFlip.hpp"
#include "ftpcg_CriticalState.hpp"
#include "ftpcg_util.hpp"

namespace ftpcg {

InjectorFlip::InjectorFlip(std::size_t vectorInd, std::size_t injectionIter,std::size_t rowImpactIndex, std::size_t flippedBit) :
  vectorInd_(vectorInd),
  injectionIter_(injectionIter),
  flippedBit_(flippedBit),
  rowImpactIndex_(rowImpactIndex) {
  rowImpactPerc_ = getCSVEntryD("impact_rows_percentiles", rowImpactIndex_, 1);
}

void InjectorFlip::setA(const RCP<const OP>& A, int matrixNum) {
  matrixNum_ = matrixNum;
  injectionRow_ = getCSVEntryI("impact_rows", matrixNum_, rowImpactIndex_);
}

const bool InjectorFlip::isNop() {
  //return errorFactor_ - 1.0 < 1e-6;
  return false;
}

bool InjectorFlip::isInjectIter(std::size_t iter) {
  return iter == injectionIter_;
}

void InjectorFlip::inject(const RCP<CriticalState>& critState) {
  RCP<MV> victim = critState->allStates[vectorInd_];
  Ordinal localInd = victim->getMap()->getLocalElement(injectionRow_);
  if (!(localInd == Teuchos::OrdinalTraits<Ordinal>::invalid())) {
    Teuchos::ArrayRCP<Scalar> data = victim->getDataNonConst();
    //data[localInd] = 1; //DEBUG
    Scalar entry = data[localInd];
    Scalar old = entry;
    char *pentry = (char *)&entry;
    char corruptionByte = 1 << (flippedBit_ % 8);
    pentry[flippedBit_ / 8] ^= corruptionByte;
    data[localInd] = entry;
    //std::cout << "Was: " << old << " Is now: " << data[localInd] << std::endl;
  }
}

void InjectorFlip::describe(std::ostream& out) const {
  out << injectionIter_ << "," << vectorInd_ << "," << flippedBit_
    << "," << rowImpactPerc_;
}

}
