#include <iostream>
#include <mpi.h>
#include <Teuchos_OpaqueWrapper.hpp>
#include <Teuchos_RCPDecl.hpp>
#include <Teuchos_DefaultMpiComm.hpp>
#include <Tpetra_MultiVector.hpp>
#include <Tpetra_GDSOP.hpp>

#include "ftpcg.hpp"
#include "ftpcg_Solver.hpp"
#include "ftpcg_DetectorIgnore.hpp"
#include "ftpcg_CorrectorNop.hpp"
#include "ftpcg_InjectorWorstRow.hpp"
#include "ftpcg_SolMgr.hpp"

#include "ftpcg_util.hpp"


int main(int argc, char *argv[]) {
    using Teuchos::RCP;
    using Teuchos::rcp;
    using Teuchos::Comm;
    using Teuchos::MpiComm;
    using Teuchos::opaqueWrapper;

    int mpi_prov;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpi_prov);
    Tpetra::Tpetra_gds_init(&argc, &argv);

    RCP<const Comm<int> > comm = rcp(new MpiComm<int> (opaqueWrapper(MPI_COMM_WORLD)));

    ftpcg::SolMgr mgr(comm);
    if (!mgr.parseCL(argc, argv)) mgr.runTrials();
    Tpetra::Tpetra_gds_finalize();
    MPI_Finalize();

    return 0;
}
