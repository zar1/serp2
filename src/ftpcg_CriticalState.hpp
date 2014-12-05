#ifndef FTPCG_CRITICALSTATE_HPP
#define FTPCG_CRITICALSTATE_HPP

#include <vector>

#include "ftpcg.hpp"

namespace ftpcg {

class CriticalState {
public:
  RCP<MV> x;
  RCP<MV> r;
  RCP<MV> p;
  Scalar *prho;
  RCP<MV> residuals;  
  std::vector<RCP<MV> > allStates;

  RCP<MV> Ap; //It should be noted that Ap isn't actually critical, but we put it here for convenience
  
  CriticalState(const RCP<MV>& x, const RCP<MV>& r, const RCP<MV>& p, Scalar *prho, const RCP<MV>& residuals, const RCP<MV> Ap);

};

}
#endif
