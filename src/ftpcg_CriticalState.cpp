#include "ftpcg.hpp"
#include "ftpcg_CriticalState.hpp"

namespace ftpcg {

CriticalState::CriticalState(const RCP<MV>& x, const RCP<MV>& r, const RCP<MV>& p, Scalar *prho, const RCP<MV>& residuals, const RCP<MV> Ap) : x(x), r(r), p(p), prho(prho), residuals(residuals), Ap(Ap) {
  allStates.push_back(x);
  allStates.push_back(r);
  allStates.push_back(p);
  allStates.push_back(residuals); 
}

}
