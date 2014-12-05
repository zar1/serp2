#include "ftpcg_util.hpp"
#include "ftpcg_CriticalState.hpp"
#include "ftpcg_DetectorABFT.hpp"


namespace ftpcg {

DetectorABFT::DetectorABFT(Scalar tol, std::size_t freq) :
  tol_(tol),
  freq_(freq) {
}

void DetectorABFT::setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b) {
  A_ = A;
  b_ = b;
  residDenom_ = A->getFrobeniusNorm() * b->norm2();
  Ax_ = newCorrespondingVector(A_);
  addVec_ = newCorrespondingVector(A_);
}

const std::size_t DetectorABFT::getWindowSize() {
  return 0;
}

bool DetectorABFT::isDetectIter(std::size_t iter) {
  return (iter > 1) && (iter % freq_ == 0);
}

const DetectTime DetectorABFT::getDetectTime() {
  return DETECT_EARLY;
}

bool DetectorABFT::isError(const RCP<CriticalState>& critState, bool errInjected) {
  RCP<MV> p = critState->p;
  RCP<MV> Ap = critState->Ap;
  pAp_ = p->dot(*Ap);
  if (pAp_ / (p->norm2() * Ap->norm2()) > tol_) {
    return true;
  }
  A_->apply(*critState->x, *Ax_);
  //addVec_->setScalar(0.0);
  //addVec_->update(1.0, *critState->r, 0.0);
  *addVec_ = *critState->r;
  addVec_->update(1.0, *Ax_, -1.0, *b_, 1.0);
  if (addVec_->norm2() / residDenom_ > tol_) {
    return true;
  }
  return false;
}

void DetectorABFT::describe(std::ostream& out) const {
  out << "ABFT," << freq_ << "," << tol_;
  //out << "ABFT," << tol_; //XXX temporarily changed for tol parameter sweep
}

}
