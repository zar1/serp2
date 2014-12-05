#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_Comm.hpp>
#include <Teuchos_CommHelpers.hpp>

#include "ftpcg_InjectorWorstRow.hpp"
#include "ftpcg_CriticalState.hpp"

namespace ftpcg {

InjectorWorstRow::InjectorWorstRow(std::size_t vectorInd, std::size_t injectionIter, Scalar errorFactor = 4.0) :
  vectorInd_(vectorInd),
  injectionIter_(injectionIter),
  errorFactor_(errorFactor) {
}

void InjectorWorstRow::setA(const RCP<const OP>& A, int matrixNumber) {
  //TODO injectionRow is max norm of rows of A
  Scalar rowNorm, localMaxNorm=0.0, maxNorm;
  Ordinal localMaxRow;
  Teuchos::ArrayView<const Ordinal> indices;
  Teuchos::ArrayView<const Scalar> values;
  Teuchos::ArrayView<const Scalar>::iterator valIter;
  std::size_t nNodeRows = A->getNodeNumRows();
  Ordinal indexBase = A->getIndexBase();
  int i;

  for (i = 0; i < nNodeRows; ++i) {
    A->getLocalRowView(i, indices, values);
    rowNorm = 0.0;
    for (valIter = values.begin(); valIter !=values.end(); ++valIter) {
      rowNorm += (*valIter) * (*valIter);
    } 
    if (rowNorm > localMaxNorm) {
      localMaxNorm = rowNorm;
      localMaxRow = indexBase + i;
    }
  }
  
  RCP<const Teuchos::Comm<Ordinal> > commA = A->getComm();
  Teuchos::reduceAll(*commA, Teuchos::MaxValueReductionOp<Ordinal, Scalar>(), 1, &localMaxNorm, &maxNorm);
  if (localMaxNorm < maxNorm) localMaxRow = A->getGlobalNumRows();
  Teuchos::reduceAll(*commA, Teuchos::MinValueReductionOp<Ordinal, Ordinal>(), 1, &localMaxRow, &injectionRow_); 
  
  corruptVec_ = rcp(new Tpetra::Vector<Scalar, Ordinal, Ordinal, Node>(A->getMap(), true));
  corruptVec_->putScalar(1.0);
  corruptVec_->replaceGlobalValue(injectionRow_, errorFactor_);
}

const bool InjectorWorstRow::isNop() {
  return errorFactor_ - 1.0 < 1e-6;
}

bool InjectorWorstRow::isInjectIter(std::size_t iter) {
  return iter == injectionIter_;
}

void InjectorWorstRow::inject(const RCP<CriticalState>& critState) {
  RCP<MV> victim = critState->allStates[vectorInd_];
  victim->elementWiseMultiply(1.0, *corruptVec_, *victim, 0.0);
}

void InjectorWorstRow::describe(std::ostream& out) const {
  out << injectionIter_ << "," << vectorInd_ << "," << errorFactor_;
}

}
