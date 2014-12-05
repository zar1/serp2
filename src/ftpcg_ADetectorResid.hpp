#ifndef FTPCG_ADETECTORRESID_HPP
#define FTPCG_ADETECTORRESID_HPP

#include "ftpcg.hpp"
#include "ftpcg_IDetector.hpp"

namespace ftpcg {

class CriticalState;

class ADetectorResid : public IDetector {
public:
  enum Sensitivity {
    LOW = 2,
    MED = 1,
    HIGH = 0
  };
protected:
  Scalar param_;
  std::size_t matrixNumber_;
  std::size_t nMatrices_;
  std::size_t methodNumber_;
  Sensitivity sensitivity_;
  const char* paramFile_;
  
  void init();
  const std::string getSensitivityString() const;

private:
  virtual bool isErrorResid(const RCP<MV>& resid) = 0;
  ADetectorResid();

public:
  virtual ~ADetectorResid() {}
  ADetectorResid(std::size_t matrixNumber, std::size_t nMatrices, std::size_t methodNumber, Sensitivity sense, const char* paramFile); 
  void setState(const RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A, const RCP<MV>& b);
  bool isDetectIter(std::size_t iter);
  const DetectTime getDetectTime();
  bool isError(const RCP<CriticalState>& critState, bool errInjected);
};

}

#endif
