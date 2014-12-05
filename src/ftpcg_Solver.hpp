#ifndef FTPCG_SOLVER_HPP
#define FTPCG_SOLVER_HPP

#include <time.h>

#include <Teuchos_DefaultMpiComm.hpp>
#include <Teuchos_FancyOStream.hpp>

#include <Tpetra_Map_decl.hpp>
#include <MatrixMarket_Tpetra.hpp>

#include <Ifpack2_ILUT_decl.hpp>

#include "ftpcg.hpp"
#include "ftpcg_IDetector.hpp"
#include "ftpcg_ICorrector.hpp"
#include "ftpcg_IInjector.hpp"
#include "ftpcg_Clock.hpp"

namespace ftpcg {

class Solver {

private:

  RCP<Teuchos::FancyOStream> ErrOut_;

  RCP<const Teuchos::Comm<int> > comm_;

  //RCP<Tpetra::Map<Ordinal, Ordinal, Node> > mapVec_;
  RCP<Tpetra::Map<Ordinal, Ordinal, Node> > mapScalar_;
  RCP<Tpetra::Map<Ordinal, Ordinal, Node> > mapResids_;

  RCP<MV> x_;
  RCP<MV> b_;
  RCP<MV> r_;
  RCP<MV> p_;
  RCP<MV> Ap_;
  RCP<MV> z_;
  Scalar rho_;
  RCP<MV> residuals_;
  RCP<const OP> A_;

  RCP<Ifpack2::Preconditioner<Scalar, Ordinal, Ordinal, Node> > precL_;
  bool hasPrec_;

  RCP<CriticalState> critState_;

  std::string path_;
  std::string matrixName_;
  int matrixNum_;
  Scalar dropTol_;

  RCP<IDetector> detector_;
  std::size_t detector_window_;
  bool hasDetector_;

  RCP<ICorrector> corrector_;
  bool hasCorrector_;

  RCP<IInjector> injector_;
  bool hasInjector_;

  std::size_t maxIters_;
  std::size_t iter_;
  Scalar tol_;

  std::size_t tp_, fp_, fn_;
  std::size_t injIter_;
  std::size_t tpIter_;
  bool converged_;
  Scalar finalResid_;
  bool escaped_;

  //RCP<Teuchos::Time> timer_;
  //struct timespec timeStart_, timeStop_, timeDiff_;
  //double timeElapsed_;
  Clock clkOverall_, clkPreserve_, clkDetect_, clkRestore_, clkRework_;

  void print(const RCP<MV>& v);
  void print(const RCP<OP>& M);

  //bool doErrCheck(bool& errInjected, bool& errUncaught, Scalar norm2, std::size_t& effectiveIter, std::vector<Scalar>& caughtResids, std::vector<std::size_t>& caughtIters, const Teuchos::ArrayView<Scalar>& rho);
  bool doErrCheck(bool& errInjected, bool& errUncaught, Scalar norm2, std::size_t& effectiveIter, std::size_t& pendingReworkIters, std::vector<Scalar>& caughtResids, std::vector<std::size_t>& caughtIters, Scalar& rho);
  void doCorAndInj(bool& errInjected, bool& errUncaught, std::size_t effectiveIter);
  void doSetup();
  void iterate(bool logResid, std::ostream& logOut);

public:

  Solver(const RCP<const Teuchos::Comm<int> >& comm);
  ~Solver();

  void setMaxIters(std::size_t maxIters);
  void setTol(Scalar tol);

  void setA(const std::string& path, const std::string& matrixName, int matrixNum);
  void setM(Scalar dropTol, bool useCholinc = true);

  void setDetector(const RCP<IDetector>& d);
  void setCorrector(const RCP<ICorrector>& c);
  void setInjector(const RCP<IInjector>& i);
  
  void run();
  void run(std::ostream& logOut);

  void report(std::ostream& logOut);
};

}


#endif
