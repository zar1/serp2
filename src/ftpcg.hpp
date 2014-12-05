#ifndef FTPCG_HPP
#define FTPCG_HPP

#include <cstddef>
#include <iostream>
#include <mpi.h>

#include <Teuchos_RCPDecl.hpp>

#include <Tpetra_VectorGDS.hpp>
#include <Tpetra_CrsMatrix.hpp>

#include <Kokkos_DefaultNode.hpp>

#define STR_FTPCG_RESOURCE_PREFIX TOSTRING(FTPCG_RESOURCE_PREFIX)
#define TOSTRING(__x__) STRINGIFY(__x__)
#define STRINGIFY(__x__) #__x__


namespace ftpcg {

using Teuchos::RCP;
using Teuchos::rcp;

typedef double Scalar;
typedef int Ordinal;
typedef typename Kokkos::DefaultNode::DefaultNodeType Node;
typedef typename Tpetra::CrsMatrix<Scalar, Ordinal, Ordinal, Node> OP;
typedef typename Tpetra::VectorGDS<Scalar, Ordinal, Ordinal, Node> MV;
//typedef typename Tpetra::MultiVector<Scalar, Ordinal, Ordinal, Node> MV;

enum DetectTime {
  DETECT_EARLY,
  DETECT_LATE
};

}
#endif
