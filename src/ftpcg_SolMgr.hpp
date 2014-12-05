#ifndef FTPCG_SOLMGR_HPP
#define FTPCG_SOLMGR_HPP

#include "ftpcg.hpp"
#include "ftpcg_ADetectorResid.hpp"

namespace ftpcg {

class IInjector;
class IDetector;
class ICorrector;
class Solver;

struct SolMgrTrial {

public:

  RCP<IInjector> injector;
  RCP<ICorrector> corrector;
  RCP<IDetector> detector;

  SolMgrTrial(RCP<IInjector> i, RCP<ICorrector> c, RCP<IDetector> d);

};

class SolMgr {

private:

  typedef typename std::vector<std::string> VecStr;

  RCP<const Teuchos::Comm<int> > comm_;

  Teuchos::CommandLineProcessor clProc_;

  int matrixNum_;
  std::string matrixName_;
  int expectedIters_;
  int maxIters_;

  int methodNum_;
  bool hasPrec_;
  Scalar dropThresh_;
  bool useCholinc_;

  std::string injectIters_;
  std::string injectVec_;
  std::string injectRowImpact_;
  std::string injectBit_;
  std::string correctType_;
  std::string detectType_;

  int nTrials_;
  int startingTrialNo_;
  bool logResid_;
  bool doInject_;
  bool lrdsInject_;

  std::string logFileName_;
  std::ofstream logFile_;
  std::ostream* outStream_;

  std::vector<RCP<IInjector> > injectors_;
  std::vector<RCP<ICorrector> > correctors_;
  std::vector<RCP<IDetector> > detectors_;

  std::vector<RCP<SolMgrTrial> > trials_;

  bool isParsed_;

  ADetectorResid::Sensitivity getSensitivity(std::string param);

  void parseList(const std::string& in, VecStr& out, VecStr& outParams);
  
  void makeInjectors();
  void makeDetectors();
  void makeCorrectors();

public:
  explicit SolMgr(const RCP<const Teuchos::Comm<int> >& comm);
  ~SolMgr();

  int parseCL(int argc, char** argv);
  void runTrials();

};

}

#endif
