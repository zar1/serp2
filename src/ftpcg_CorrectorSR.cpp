#include "ftpcg_CorrectorSR.hpp"
#include "ftpcg_CriticalState.hpp"

namespace ftpcg {


CorrectorSR::CorrectorSR(std::size_t cpInterval, bool restartOnly) :
  cpInterval_(cpInterval),
  restartOnly_(restartOnly),
  lastCPIter_(0) {}

bool CorrectorSR::isPreserveIter(std::size_t iter) {
  return (iter == 0) || (!restartOnly_ && (iter % cpInterval_ == 0));
}

void CorrectorSR::initProtection(const RCP<CriticalState>& critState) {
  std::vector<RCP<MV> > allStates = critState->allStates;
  for(std::vector<RCP<MV> >::iterator it = allStates.begin(); it != allStates.end(); ++it) {
    
    (*it)->Tpetra_Vector_gds_alloc();
    (*it)->Tpetra_Vector_gds_put();
    (*it)->Tpetra_Vector_gds_version_inc();
    
  }
  lastCPIter_ = 0;
 
  
}

  void CorrectorSR::protectState(const RCP<CriticalState>& critState, std::size_t effectiveIter) {
  
    
    critState->x->Tpetra_Vector_gds_put();
    critState->x->Tpetra_Vector_gds_version_inc();
    critState->p->Tpetra_Vector_gds_put();
    critState->p->Tpetra_Vector_gds_version_inc();
    critState->r->Tpetra_Vector_gds_put();
    critState->r->Tpetra_Vector_gds_version_inc();
    prevRho_ = *critState->prho;
    critState->residuals->Tpetra_Vector_gds_put();
    critState->residuals->Tpetra_Vector_gds_version_inc();
    lastCPIter_ = effectiveIter;
    
  
  }

  void CorrectorSR::restoreState(const RCP<CriticalState>& critState, std::size_t& effectiveIter, std::size_t& pendingReworkIters) {
  
    std::vector<RCP<MV> > allStates = critState->allStates;
    for(std::vector<RCP<MV> >::iterator it = allStates.begin(); it != allStates.end(); ++it) {
      
      (*it)->Tpetra_Vector_gds_get();
      (*it)->Tpetra_Vector_gds_version_inc();
      
    }
    *critState->prho = prevRho_;
    pendingReworkIters = effectiveIter - lastCPIter_;
    effectiveIter = lastCPIter_;
  
  }

  void CorrectorSR::finalizeProtection(const RCP<CriticalState>& critState) {
    std::vector<RCP<MV> > allStates = critState->allStates;
    for(std::vector<RCP<MV> >::iterator it = allStates.begin(); it != allStates.end(); ++it) {
      (*it)->Tpetra_Vector_gds_free();
    
    }
  }

  void CorrectorSR::describe(std::ostream& out) const {
  
    if (restartOnly_) {
      out << "restart";
    } else {
      out << "SR," << cpInterval_;
    }
  
  }
  
}
