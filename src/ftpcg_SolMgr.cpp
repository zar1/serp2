#include <Teuchos_CommandLineProcessor.hpp>

#include <ctime>
#include <unistd.h>
#include <algorithm>
#include <ctime>
#include <cstdlib>

#include "ftpcg.hpp"
#include "ftpcg_util.hpp"
#include "ftpcg_SolMgr.hpp"
//#include "ftpcg_InjectorWorstRow.hpp"
#include "ftpcg_InjectorFlip.hpp"
#include "ftpcg_InjectorNop.hpp"
#include "ftpcg_InjectorLRDS.hpp"
#include "ftpcg_CorrectorNop.hpp"
#include "ftpcg_CorrectorSR.hpp"
#include "ftpcg_DetectorIgnore.hpp"
#include "ftpcg_DetectorImmediate.hpp"
#include "ftpcg_DetectorMD.hpp"
#include "ftpcg_DetectorAD.hpp"
#include "ftpcg_DetectorABFT.hpp"
#include "ftpcg_DetectorSlowIgnore.hpp"
#include "ftpcg_Solver.hpp"

namespace ftpcg {

SolMgrTrial::SolMgrTrial(RCP<IInjector> i, RCP<ICorrector> c, RCP<IDetector> d)
    :
    injector(i),
    corrector(c),
    detector(d) {}

ADetectorResid::Sensitivity SolMgr::getSensitivity(std::string param) {
  if (param == "LOW") return ADetectorResid::LOW;
  if (param == "MED") return ADetectorResid::MED;
  return ADetectorResid::HIGH;
}

void SolMgr::parseList(const std::string& in, SolMgr::VecStr& out, SolMgr::VecStr& outParams) {
  std::string outBuf;
  std::size_t colonPos;
  std::stringstream inStream(in);
  while (std::getline(inStream, outBuf, ',')) {
    colonPos = outBuf.find(":");
    if (colonPos == std::string::npos) {
      out.push_back(outBuf);
      outParams.push_back("");
    } else {
      out.push_back(outBuf.substr(0, colonPos));
      outParams.push_back(outBuf.substr(colonPos + 1));
    }
  } 
}

void SolMgr::makeInjectors() {
  
  /*
  VecStr iter, sev, vec, dummy;
  RCP<IInjector> inj;

  parseList(injectIters_, iter, dummy);
  parseList(injectSev_, sev, dummy);
  parseList(injectVec_, vec, dummy);
  
  for (VecStr::iterator i = iter.begin(); i != iter.end(); ++i) {
    for (VecStr::iterator s = sev.begin(); s != sev.end(); ++s) {
      for (VecStr::iterator v = vec.begin(); v != vec.end(); ++v) {
        inj = rcp(new InjectorWorstRow(atoi(v->c_str()), atoi(i->c_str()), atof(s->c_str())));
        injectors_.push_back(inj);
      }
    }
  } 
  */
  if (doInject_) {
    VecStr iter, bit, rowInd, vec, dummy;
    RCP<IInjector> inj;

    parseList(injectIters_, iter, dummy);
    parseList(injectBit_, bit, dummy);
    parseList(injectRowImpact_, rowInd, dummy);
    parseList(injectVec_, vec, dummy);
    if (lrdsInject_) {
      for (VecStr::iterator i = iter.begin(); i != iter.end(); ++i) {
        injectors_.push_back(rcp(new InjectorLRDS(atoi(i->c_str()))));
      }
    } else {
  
      for (VecStr::iterator i = iter.begin(); i != iter.end(); ++i) {
        for (VecStr::iterator b = bit.begin(); b != bit.end(); ++b) {
          for (VecStr::iterator r = rowInd.begin(); r != rowInd.end(); ++r) {
            for (VecStr::iterator v = vec.begin(); v != vec.end(); ++v) {
              inj = rcp(new InjectorFlip(atoi(v->c_str()), atoi(i->c_str()), atoi(r->c_str()), atoi(b->c_str())));
              injectors_.push_back(inj);
              std::cout << "here" << std::endl;
            }
          }
        }
      } 
    }
  } else {
    injectors_.push_back(rcp(new InjectorNop()));
  }
}

void SolMgr::makeDetectors() {
  VecStr detTypes, params;
  RCP<IDetector> det;

  parseList(detectType_, detTypes, params);

  std::size_t nMatrices = getCSVNRows("matrices");

  VecStr::iterator p = params.begin();
  for (VecStr::iterator d = detTypes.begin(); d != detTypes.end(); ++d) {
    if (*d == "IGNORE") {
      det = rcp(new DetectorIgnore);
    } else if (*d == "IMMEDIATE") {
      det = rcp(new DetectorImmediate);
    } else if (*d == "MD") {
      det = rcp(new DetectorMD(matrixNum_, nMatrices, methodNum_, getSensitivity(*p)));
    } else if (*d == "AD") {
      det = rcp(new DetectorAD(matrixNum_, nMatrices, methodNum_, getSensitivity(*p)));
    } else if (*d == "ABFT") {
      //det = rcp(new DetectorABFT(1e-15, atoi(p->c_str()))); //TODO Decide that sensitivity less magically
      det = rcp(new DetectorABFT(1e-6, atoi(p->c_str()))); //TODO Maybe same as convergence?
      //det = rcp(new DetectorABFT(atof(p->c_str()), 1)); //TODO TEMPORARILY USING THE INPUT PARAMETER FOR SENSITIVITY, NOT FREQ!!!!
    } else if (*d == "ABFTTOL") {
      det = rcp(new DetectorABFT(atof(p->c_str()), 1));
    } else if (*d == "IGNOREDBG") {
      det = rcp(new DetectorSlowIgnore);
    }
    detectors_.push_back(det);
    ++p;
  }
}

void SolMgr::makeCorrectors() {
  VecStr corTypes, params;
  RCP<ICorrector> cor;

  parseList(correctType_, corTypes, params);
  
  VecStr::iterator p = params.begin();
  for (VecStr::iterator c = corTypes.begin(); c != corTypes.end(); ++c) {
    if (*c == "NOP") {
      cor = rcp(new CorrectorNop);
    } else if (*c == "RESTART") {
      cor = rcp(new CorrectorSR(0, true));
    } else if (*c == "SR") {
      cor = rcp(new CorrectorSR(atoi(p->c_str()), false));
    } 
    correctors_.push_back(cor);
    ++p;
  }
}

SolMgr::SolMgr(const RCP<const Teuchos::Comm<int> >& comm) :
  comm_(comm),
  isParsed_(false)
  {
  clProc_.throwExceptions(false);
  clProc_.setOption("mat_no", &matrixNum_);
  clProc_.setOption("method_no", &methodNum_);
  clProc_.setOption("enable_inject", "disable_inject", &doInject_);
  clProc_.setOption("lrds_inject", "flip_inject", &lrdsInject_);
  clProc_.setOption("inj_iter", &injectIters_);
  clProc_.setOption("inj_bit", &injectBit_);
  clProc_.setOption("inj_row_impact", &injectRowImpact_);
  clProc_.setOption("inj_vec", &injectVec_);
  clProc_.setOption("cor", &correctType_);
  clProc_.setOption("det", &detectType_);
  clProc_.setOption("ntrials", &nTrials_);
  clProc_.setOption("start_trial", &startingTrialNo_);
  clProc_.setOption("enable_log_resid", "disable_log_resid", &logResid_);
  clProc_.setOption("logfile", &logFileName_);
  clProc_.setOption("cholinc", "hajime", &useCholinc_);
  
}

SolMgr::~SolMgr() {
  if (isParsed_) {
    delete outStream_;
    if (logFile_.is_open()) logFile_.close();
  }
}

int SolMgr::parseCL(int argc, char* argv[]) {
  
  matrixNum_ = 0;
  methodNum_ = 1;
  doInject_ = false;
  lrdsInject_ = false;
  injectIters_ = "0";
  injectBit_ = "53";
  injectRowImpact_ = "0";
  injectVec_ = "2";
  //correctType_ = "NOP,RESTART,SR:1,SR:2,SR:4,SR:8"; 
  correctType_ = "NOP,RESTART,SR:1,SR:2,SR:3,SR:4"; 
  //detectType_ = "IGNORE,MD:LOW,MD:MED,MD:HIGH,AD:LOW,AD:MED,AD:HIGH,IMMEDIATE"
  //detectType_ = "IGNORE,MD:LOW,MD:MED,MD:HIGH,AD:LOW,AD:MED,AD:HIGH,ABFT:1,ABFT:2,ABFT:4,ABFT:8,IMMEDIATE";
  detectType_ = "IGNORE,MD:LOW,MD:MED,MD:HIGH,AD:LOW,AD:MED,AD:HIGH,ABFT:1,ABFT:2,ABFT:3,ABFT:4,IMMEDIATE";
  nTrials_ = 1;
  startingTrialNo_ = 0;
  logResid_ = true;
  logFileName_ = "STDOUT";
  expectedIters_ = 10000;
  maxIters_ = 10000;

  useCholinc_ = true;

  if (clProc_.parse(argc, argv) != clProc_.PARSE_SUCCESSFUL) return 1;
  isParsed_ = true;

  matrixName_ = getCSVEntryS("matrices", matrixNum_, 0);  
  if (methodNum_ >= 0) {
    dropThresh_ = getCSVEntryD("methods", methodNum_, 0);
    hasPrec_ = true;
    expectedIters_ = getCSVEntryD("expected_iters", matrixNum_, methodNum_);
    maxIters_ = 2 * expectedIters_;
  } else {
    hasPrec_ = false;
  }
  makeInjectors();
  makeCorrectors();
  makeDetectors();
 
  std::streambuf* obuf;
  if (logFileName_ == "STDOUT") {
    obuf = std::cout.rdbuf();
  } else {
    logFile_.open(logFileName_.c_str());
    obuf = logFile_.rdbuf();
  }
  outStream_ = new std::ostream(obuf);

  return 0; 
}

void SolMgr::runTrials() {
  char hostName[256];
  gethostname(hostName, 255);
  int trialNo = startingTrialNo_;
  std::stringstream logBuffer;
  std::stringstream reportBuffer;
  Solver sol(comm_);
  //Solver sol(comm_, std::cout);
  sol.setA(STR_FTPCG_RESOURCE_PREFIX "/matrices", matrixName_, matrixNum_);
  //sol.setMaxIters(maxIters_);
  sol.setMaxIters(1000); //TODO temporarly measure to explore other preconditioners
  if (hasPrec_) sol.setM(dropThresh_, useCholinc_);
  for (std::vector<RCP<IInjector> >::iterator i = injectors_.begin(); i != injectors_.end(); ++i) {
    for (std::vector<RCP<ICorrector> >::iterator c = correctors_.begin(); c != correctors_.end(); ++c) {
      for (std::vector<RCP<IDetector> >::iterator d = detectors_.begin(); d != detectors_.end(); ++d) {
        for (int run = 0; run < nTrials_; ++run) {
            trials_.push_back(rcp(new SolMgrTrial(*i, *c, *d)));
        }
      }
    }
  }

  int rank = comm_->getRank();
  unsigned int seed;
  if (rank == 0) seed = unsigned(std::time(0));
  comm_->broadcast(0, sizeof(seed), (char *)&seed);
  std::srand(seed);
  std::random_shuffle(trials_.begin(), trials_.end());
  //shuffle trials

  for (std::vector<RCP<SolMgrTrial> >::iterator t = trials_.begin(); t != trials_.end(); ++t) {
    sol.setInjector((*t)->injector);
    sol.setDetector((*t)->detector);
    sol.setCorrector((*t)->corrector);
    if (logResid_) { 
      sol.run(logBuffer);
    } else {
      sol.run();
    }
    if (rank == 0) {
      sol.report(reportBuffer);
      *outStream_ << trialNo << "," << std::time(0) << "," << hostName << std::endl;
      *outStream_ << reportBuffer.str();
      if (logFileName_ != "STDOUT") std::cout << reportBuffer.str() << std::endl;
      *outStream_ << logBuffer.str() << std::endl;
      reportBuffer.str("");
      logBuffer.str("");
    }
    ++trialNo;
  }
}
  

}
