#include <limits>

#include <Teuchos_OpaqueWrapper.hpp>
#include <Teuchos_DefaultMpiComm.hpp>
#include <Teuchos_Array.hpp>
#include <Teuchos_ParameterList.hpp>
#include <Teuchos_ScalarTraits.hpp>
#include <Teuchos_FancyOStream.hpp>
#include <Teuchos_Comm.hpp>
#include <Teuchos_VerboseObject.hpp>

#include <Kokkos_DefaultNode.hpp>

#include <Tpetra_Map_decl.hpp>
#include <Tpetra_VectorGDS.hpp>
#include <Tpetra_CrsMatrix.hpp>
#include <MatrixMarket_Tpetra.hpp> 
#include <Ifpack2_ILUT_decl.hpp>
#include <Ifpack2_Factory.hpp>
#include <Ifpack2_AdditiveSchwarz_decl.hpp>

#include "ftpcg.hpp"
#include "ftpcg_Solver.hpp"
#include "ftpcg_CriticalState.hpp"
#include "ftpcg_PrecPrecomputed.hpp"
#include "ftpcg_util.hpp"

//#include <valgrind/callgrind.h>

namespace ftpcg {

void Solver::print(const RCP<MV>& v) {
  v->describe(*ErrOut_, Teuchos::VERB_EXTREME);
}

void Solver::print(const RCP<OP>& m) {
  m->describe(*ErrOut_, Teuchos::VERB_EXTREME); }


void Solver::doSetup() {

  x_ = newCorrespondingVector(A_);
  b_ = newCorrespondingVector(A_);
  r_ = newCorrespondingVector(A_); p_ = newCorrespondingVector(A_);
  Ap_ = newCorrespondingVector(A_);
  z_ = newCorrespondingVector(A_);

  b_->putScalar(1.0);

}

bool isNewError(Scalar norm2, std::size_t effectiveIter, const std::vector<Scalar>& caughtResids, const std::vector<std::size_t>& caughtIters) {
  std::vector<std::size_t>::const_iterator iterIter = caughtIters.begin();
  for (std::vector<Scalar>::const_iterator iterResid = caughtResids.begin(); iterResid != caughtResids.end(); ++iterResid) {
    if (*iterResid == norm2 && *iterIter == effectiveIter) return false;
    ++iterIter;
  }
  return true;
}

//bool Solver::doErrCheck(bool& errInjected, bool& errUncaught, Scalar norm2, std::size_t& effectiveIter, std::vector<Scalar>& caughtResids, std::vector<std::size_t>& caughtIters, const Teuchos::ArrayView<Scalar>& rho) {
bool Solver::doErrCheck(bool& errInjected, bool& errUncaught, Scalar norm2, std::size_t& effectiveIter, std::size_t& pendingReworkIters, std::vector<Scalar>& caughtResids, std::vector<std::size_t>& caughtIters, Scalar& rho) {
  bool reworkRequested = false;
  if (pendingReworkIters == 1) {
    clkRework_.stop();
    //std::cout << "##" << 1 << std::endl;
    pendingReworkIters = 0;
  } else if (pendingReworkIters > 1) {
    //std::cout << "##" << pendingReworkIters << std::endl;
    --pendingReworkIters;
  }
  if (detector_->isDetectIter(iter_)) {
    clkDetect_.start();
    bool detected = detector_->isError(critState_, errInjected);
    clkDetect_.stop();
    if (detected) {
      if (errUncaught) {
        tpIter_ = iter_;
        errUncaught = false;
      }
      if (isNewError(norm2, effectiveIter, caughtResids, caughtIters)) {
        if (errInjected) {
          ++tp_;
        } else {
          ++fp_;
        }
        std::vector<RCP<MV> > allStates = critState_->allStates;
        for(std::vector<RCP<MV> >::iterator it = allStates.begin(); it != allStates.end(); ++it) {
          (*it)->Tpetra_Vector_gds_raise_error(0);
          (*it)->Tpetra_Vector_gds_fence();
        }
        reworkRequested = true;
      } 
    } else if (errInjected) {
      ++fn_;
    }
    errInjected = false;
  }
  
  if (MV::rollback_pending) {
    clkRestore_.start();
    caughtResids.push_back(norm2);
    caughtIters.push_back(effectiveIter);
    corrector_->restoreState(critState_, effectiveIter, pendingReworkIters);
    clkRestore_.stop();
    if (pendingReworkIters > 0 && (!clkRework_.isRunning())) {
      clkRework_.start();
    }
    MV::rollback_pending = false; 
    //rho_->get1dCopy(rho, 1);
    //rho = rho_->get1dView()[0];
  }
  return reworkRequested;
}

void Solver::doCorAndInj(bool& errInjected, bool& errUncaught, std::size_t effectiveIter) {
  if (corrector_->isPreserveIter(iter_)) {
    clkPreserve_.start();
    corrector_->protectState(critState_, effectiveIter);
    clkPreserve_.stop();
  }

  if (hasInjector_) {
    if (injector_->isInjectIter(iter_)) {
      injector_->inject(critState_);
      if (!injector_->isNop()) {
        errInjected = true;
        errUncaught = true;
        injIter_ = iter_;
      }
    }
  }
}

void Solver::iterate(bool logResid, std::ostream& logOut) {

  //TODO reset method that restores A and b to whatever they initially were
  Scalar alpha, beta, rHz, pAp, rhoTmp;

  const Scalar one = Teuchos::ScalarTraits<Scalar>::one();
  const Scalar zero = Teuchos::ScalarTraits<Scalar>::zero();

  Scalar norm2;

  RCP<MV> Ax = newCorrespondingVector(A_);
  RCP<MV> y = newCorrespondingVector(A_);
  RCP<MV> realR = newCorrespondingVector(A_);

  Teuchos::ArrayRCP<Scalar> residView = residuals_->get1dViewNonConst();
  Scalar inf = std::numeric_limits<Scalar>::infinity();

  std::vector<Scalar> caughtResids;
  std::vector<std::size_t> caughtIters;

  for (int i = 0; i < detector_window_; ++i) residView[i] = inf;

  DetectTime detectTime = detector_->getDetectTime();
  bool errInjected = false;
  bool errUncaught = false;
  std::size_t effectiveIter = 0;
  std::size_t pendingReworkIters = 0;

  converged_ = false;
  tp_ = fp_ = fn_ = 0;
  iter_ = 0;
  injIter_ = 0;
  tpIter_ = 0;

  clkOverall_.reset();
  clkPreserve_.reset();
  clkDetect_.reset();
  clkRestore_.reset();
  clkRework_.reset();

  if (logResid) logOut << "RESID" << std::endl;

  rho_ = one;
  x_->putScalar(0.0);
  p_->putScalar(0.0);

  A_->apply(*x_, *Ax);
  r_->update(one, *b_, -1 * one, *Ax, zero);
  norm2 = r_->norm2();

  corrector_->initProtection(critState_);

  //CALLGRIND_START_INSTRUMENTATION;
  //clock_gettime(CLOCK_MONOTONIC, &timeStart_);
  clkOverall_.start();

  while (iter_ < maxIters_) {

      //std::cout << "iter: " << iter_ << ", normr: " << norm2 << std::endl;
      if (logResid) logOut << norm2 << std::endl;

      iter_++;
      //dump(iter_, "iter");
      effectiveIter++;

      if (hasPrec_) {
        precL_->apply(*r_, *z_);
      } else {
        //z_->update(one, *r_, zero); 
        *z_ = *r_;
      }
      //dump(z_, "z");
      rhoTmp = rho_;
      //dump(rhoTmp, "rhoTmp");
      rho_ = r_->dot(*z_);
      beta = rho_ / rhoTmp;
      //dump(beta[0], "beta");
      p_->update(one, *z_, beta);
      //dump(p_, "p");
      if (detectTime == DETECT_EARLY) {
        if (doErrCheck(errInjected, errUncaught, norm2, effectiveIter, pendingReworkIters, caughtResids, caughtIters, rho_)) { 
          if (effectiveIter < 1) {
            if (hasPrec_) {
              precL_->apply(*r_, *z_);
            } else {
              //z_->update(one, *r_, zero); 
              *z_ = *r_;
            }
            rhoTmp = rho_;
            rho_ = r_->dot(*z_);
            beta = rho_ / rhoTmp;
            p_->update(one, *z_, beta);
          }
        }
        doCorAndInj(errInjected, errUncaught, effectiveIter);
      }
      //p and Ap should be orthogonal here
      A_->apply(*p_, *Ap_);
      //dump(Ap_, "Ap");
      pAp = p_->dot(*Ap_);
      //dump(pAp[0], "pAp");
      alpha = rho_ / pAp;
      //dump(alpha[0], "alpha");
      x_->update(alpha, *p_, one);
      //dump(x_, "x");
      r_->update(-1 * alpha, *Ap_, one);
      //dump(r_, "r");
      norm2 = r_->norm2();

      for (int i = detector_window_ - 1; i >= 1; --i) residView[i] = residView[i-1];
      residView[0] = norm2;

      if (norm2 < tol_) {
        converged_ = true;
        break;
      }

      if (detectTime == DETECT_LATE) {
        doErrCheck(errInjected, errUncaught, norm2, effectiveIter, pendingReworkIters, caughtResids, caughtIters, rho_); 
        doCorAndInj(errInjected, errUncaught, effectiveIter);
      }


    } 
    //clock_gettime(CLOCK_MONOTONIC, &timeStop_);
    clkOverall_.stop();
    //CALLGRIND_STOP_INSTRUMENTATION;
    if (logResid) {
      //logOut << norm2[0] << std::endl;
      logOut << norm2 << std::endl;
      logOut << "STOP" << std::endl;
    }
    corrector_->finalizeProtection(critState_);
    //timeDiff(&timeStart_, &timeStop_, &timeDiff_);
    A_->apply(*x_, *Ax);
    realR->update(one, *b_, -1 * one, *Ax, zero);
    norm2 = realR->norm2();
    finalResid_ = norm2;
    escaped_ = finalResid_ > tol_;

}

Solver::Solver(const RCP<const Teuchos::Comm<int> >& comm) : comm_(comm), maxIters_(10000), tol_(1e-6), hasPrec_(false), hasDetector_(false), hasCorrector_(false), hasInjector_(false), tp_(0), fp_(0), fn_(0) {
  ErrOut_ = Teuchos::VerboseObjectBase::getDefaultOStream();
  mapScalar_ = rcp(new Tpetra::Map<Ordinal, Ordinal, Node>(1, 0, comm, Tpetra::LocallyReplicated));
  //timer_ = rcp(new Teuchos::Time("kernel", false));
}

Solver::~Solver() {
}

void Solver::setMaxIters(std::size_t maxIters) { maxIters_ = maxIters; }

void Solver::setTol(Scalar tol) { tol_ = tol; }

void Solver::setA(const std::string& path, const std::string& matrixName, int matrixNum) {
  matrixName_ = matrixName;
  matrixNum_ = matrixNum;
  path_ = path;
  std::string fullPath(path);
  fullPath += ("/" + matrixName + ".mtx");
  std::cout << fullPath << std::endl;
  A_ = Tpetra::MatrixMarket::Reader<OP>::readSparseFile(fullPath, comm_, Kokkos::DefaultNode::getDefaultNode());
  //mapVec_ = rcp(new Tpetra::Map<Ordinal, Ordinal, Node>(A_->getGlobalNumRows(), 0, comm_, Tpetra::GloballyDistributed,  Kokkos::DefaultNode::getDefaultNode()));
  doSetup();
}

void Solver::setM(Scalar dropTol, bool useCholinc) {
  Teuchos::ParameterList params;
  dropTol_ = dropTol;
  if (useCholinc) {
    params.set("fact: drop tolerance", dropTol);
    precL_ = rcp(new PrecPrecomputed(A_));
    params.set("prec: path", path_);
    params.set("prec: matrix name", matrixName_);
  } else {
    //params.set("inner preconditioner name", "DENSE");
    //params.set("schwarz: combine mode", "ADD");
    //params.set("schwarz: overlap level", A_->getNodeNumRows());
    Ifpack2::Factory factory;
    precL_ = factory.create("RELAXATION", A_);
    params.set("relaxation: type", "Symmetric Gauss-Seidel");
    params.set("relaxation: sweeps", 1);
    //precL_ = factory.create("RILUK", A_);
    //params.set("fact: ilut level-of-fill", 10.0);
    //params.set("fact: ilut drop tolerance", 1e-10);
    //precL_ = factory.create("RELAXATION", A_, A_->getNodeNumRows());
    //precL_= rcp(new Ifpack2::AdditiveSchwarz<OP, Ifpack2::Preconditioner<Scalar,
    //  Ordinal, Ordinal, Node> >(A_));
  }

  precL_->setParameters(params);
  precL_->initialize();
  precL_->compute();
  
  hasPrec_ = true;
}


void Solver::setDetector(const RCP<IDetector>& d) {
  detector_ = d;
  detector_window_ = d->getWindowSize();
  detector_window_ = detector_window_ < 1 ? 1 : detector_window_;
  //detector_->setState(Teuchos::rcp_dynamic_cast<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node>(A_), b_);
  detector_->setState(A_, b_);
  hasDetector_ = true;
  mapResids_ = rcp(new Tpetra::Map<Ordinal, Ordinal, Node>(detector_window_, 0, comm_, Tpetra::LocallyReplicated));
  residuals_ = rcp(new MV(mapResids_, 1));
  critState_ = rcp(new CriticalState(x_, r_, p_, &rho_, residuals_, Ap_));
}

void Solver::setCorrector(const RCP<ICorrector>& c) {
  corrector_ = c;
  hasCorrector_ = true;
}

void Solver::setInjector(const RCP<IInjector>& i) {
  injector_ = i;
  injector_->setA(A_, matrixNum_);
  hasInjector_ = true;
}

void Solver::run() {
  iterate(false, std::cout);
}

void Solver::run(std::ostream& logOut) {
  iterate(true, logOut);
}

void Solver::report(std::ostream& logOut) {
  logOut << matrixName_ << ",";  
  if (hasPrec_) { 
    logOut << dropTol_;
  } else {
    logOut << "None";
  }
  logOut << std::endl;
  if (hasDetector_) {
    detector_->describe(logOut);
    logOut << std::endl;
  } else {
    logOut << "None" << std::endl;
  }
  if (hasCorrector_) {
    corrector_->describe(logOut);
    logOut << std::endl;
  } else {
    logOut << "None" << std::endl;
  }
  if (hasInjector_) {
    injector_->describe(logOut);
    logOut << std::endl;
  } else {
    logOut << "None" << std::endl;
  }
  logOut << converged_ << "," << iter_ << ",";
  clkOverall_.report(logOut) << std::endl;
  clkPreserve_.report(logOut) << ",";
  clkDetect_.report(logOut) << ",";
  clkRestore_.report(logOut) << ",";
  clkRework_.report(logOut) << std::endl;
  logOut << escaped_ << "," << finalResid_ << std::endl;
  logOut << tp_ << "," << fp_ << "," << fn_ << "," << ((int)tpIter_ - (int)injIter_) << std::endl;
}

}
