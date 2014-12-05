/*@HEADER
// ***********************************************************************
//
//       Ifpack2: Tempated Object-Oriented Algebraic Preconditioner Package
//                 Copyright (2009) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
//@HEADER
*/

//-----------------------------------------------------
// Ifpack2::ILUT is a translation of the Aztec ILUT
// implementation. The Aztec ILUT implementation was
// written by Ray Tuminaro.
// See notes in the Ifpack2::ILUT::Compute method.
// ABW.
//------------------------------------------------------

#ifndef FTPCG_PREC_PRECOMPUTED_HPP
#define FTPCG_PREC_PRECOMPUTED_HPP

#include "Ifpack2_ConfigDefs.hpp"
#include "Ifpack2_Preconditioner.hpp"
#include "Ifpack2_Condest.hpp"
#include "Ifpack2_Heap.hpp"
#include "Ifpack2_Parameters.hpp"

#include <Teuchos_Assert.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_Time.hpp>
#include <Teuchos_TypeNameTraits.hpp>
#include <Teuchos_ScalarTraits.hpp>

#include <Tpetra_Map_decl.hpp>

#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

#include "ftpcg.hpp"

namespace Teuchos {
  // forward declaration
  class ParameterList;
}

namespace ftpcg {

  /// \class ILUT
  /// \brief ILUT incomplete factorization of a Tpetra sparse matrix.
  ///
  /// This class computes an ILUT sparse incomplete factorization with
  /// specified fill and drop tolerance, of a given sparse matrix
  /// represented as a Tpetra::RowMatrix.
  ///
  /// \warning If the matrix is distributed over multiple MPI
  ///   processes, this class will not work correctly by itself.  You
  ///   must use it as a subdomain solver inside of a domain
  ///   decomposition method like AdditiveSchwarz (which see).  If you
  ///   use Factory to create an ILUT preconditioner, the Factory will
  ///   automatically wrap ILUT in AdditiveSchwarz for you, if the
  ///   matrix's communicator contains multiple processes.
  ///
  /// See the documentation of setParameters() for a list of valid
  /// parameters.
class PrecPrecomputed :
    virtual public Ifpack2::Preconditioner<Scalar,
                                           Ordinal,
                                           Ordinal,
                                           Node>
{
public:
  //! \name Typedefs
  //@{

  //! The type of the entries of the input MatrixType.
  //typedef typename MatrixType::scalar_type scalar_type;

  //! Preserved only for backwards compatibility.  Please use "scalar_type".
  //TEUCHOS_DEPRECATED typedef typename MatrixType::scalar_type Scalar;


  //! The type of local indices in the input MatrixType.
  //typedef typename MatrixType::local_ordinal_type local_ordinal_type;

  //! Preserved only for backwards compatibility.  Please use "local_ordinal_type".
  //TEUCHOS_DEPRECATED typedef typename MatrixType::local_ordinal_type LocalOrdinal;


  //! The type of global indices in the input MatrixType.
  //typedef typename MatrixType::global_ordinal_type global_ordinal_type;

  //! Preserved only for backwards compatibility.  Please use "global_ordinal_type".
  //TEUCHOS_DEPRECATED typedef typename MatrixType::global_ordinal_type GlobalOrdinal;


  //! The type of the Kokkos Node used by the input MatrixType.
  //typedef typename MatrixType::node_type node_type;

  //! Preserved only for backwards compatibility.  Please use "node_type".
  //TEUCHOS_DEPRECATED typedef typename MatrixType::node_type Node;


  //! The type of the magnitude (absolute value) of a matrix entry.
  typedef typename Teuchos::ScalarTraits<Scalar>::magnitudeType magnitude_type;

  //typedef magnitude_type CondestType;

  //! Preserved only for backwards compatibility.  Please use "magnitude_type".
  //TEUCHOS_DEPRECATED typedef typename Teuchos::ScalarTraits<scalar_type>::magnitudeType magnitudeType;

  //@}
  //! \name Constructors and Destructors
  //@{

  /// \brief Constructor
  ///
  /// \param A [in] The sparse matrix to factor, as a
  ///   Tpetra::RowMatrix.  (Tpetra::CrsMatrix inherits from this, so
  ///   you may use a Tpetra::CrsMatrix here instead.)
  ///
  /// The factorization will <i>not</i> modify the input matrix.  It
  /// stores the L and U factors in the incomplete factorization
  /// separately.
  explicit PrecPrecomputed(const Teuchos::RCP<const Tpetra::RowMatrix<Scalar,Ordinal,Ordinal,Node> > &A);

  //! Destructor
  virtual ~PrecPrecomputed();

  //@}
  //! \name Methods for setting up and computing the incomplete factorization
  //@{

  //! Set parameters for the preconditioner.
  /**
    <ul>
     <li> "fact: ilut level-of-fill" (int)<br>
     <li> "fact: drop tolerance" (magnitude_type)<br>
     <li> "fact: absolute threshold" (magnitude_type)<br>
     <li> "fact: relative threshold" (magnitude_type)<br>
     <li> "fact: relax value" (magnitude_type)<br>
    </ul>
  */
  void setParameters (const Teuchos::ParameterList& params);

  /// \brief Clear any previously computed factors.
  ///
  /// You may call this before calling compute().  The compute()
  /// method will call this automatically if it has not yet been
  /// called.  If you call this after calling compute(), you must
  /// recompute the factorization (by calling compute() again) before
  /// you may call apply().
  void initialize ();

  //! Returns \c true if the preconditioner has been successfully initialized.
  inline bool isInitialized() const {
    return(IsInitialized_);
  }

  //! Compute factors L and U using the specified diagonal perturbation thresholds and relaxation parameters.
  /*! This function computes the ILUT factors L and U using the current:
    <ol>
    <li> Value for the drop tolerance
    <li> Value for the level of fill
    <li> Value for the \e a \e priori diagonal threshold values.
    </ol>
   */
  void compute();

  //! If compute() is completed, this query returns true, otherwise it returns false.
  inline bool isComputed() const {
    return(IsComputed_);
  }

  //@}

  //! @name Methods implementing Tpetra::Operator.
  //@{

  //! Returns the result of a ILUT forward/back solve on a Tpetra::MultiVector X in Y.
  /*!
    \param
    X - (In) A Tpetra::MultiVector of dimension NumVectors to solve for.
    \param
    Y - (Out) A Tpetra::MultiVector of dimension NumVectors containing result.
  */
  void apply(
      const Tpetra::MultiVector<Scalar,Ordinal,Ordinal,Node>& X,
            Tpetra::MultiVector<Scalar,Ordinal,Ordinal,Node>& Z,
            Teuchos::ETransp mode = Teuchos::NO_TRANS,
               Scalar alpha = Teuchos::ScalarTraits<Scalar>::one(),
               Scalar beta = Teuchos::ScalarTraits<Scalar>::zero()) const;

  //! Tpetra::Map representing the domain of this operator.
  const Teuchos::RCP<const Tpetra::Map<Ordinal,Ordinal,Node> >& getDomainMap() const;

  //! Tpetra::Map representing the range of this operator.
  const Teuchos::RCP<const Tpetra::Map<Ordinal,Ordinal,Node> >& getRangeMap() const;

  //! Whether this object's apply() method can apply the transpose (or conjugate transpose, if applicable).
  bool hasTransposeApply() const;

  //@}

  //@{
  //! \name Mathematical functions.

  //! Returns the result of a ILUT forward/back solve on a Tpetra::MultiVector X in Y.
  /*!
    \param
    X - (In) A Tpetra::MultiVector of dimension NumVectors to solve for.
    \param
    Y - (Out) A Tpetra::MultiVector of dimension NumVectors containing result.
  */
  template<class DomainScalar, class RangeScalar>
  void applyTempl(
      const Tpetra::MultiVector<DomainScalar,Ordinal,Ordinal,Node>& X,
            Tpetra::MultiVector<RangeScalar,Ordinal,Ordinal,Node>& Z,
            Teuchos::ETransp mode = Teuchos::NO_TRANS,
               RangeScalar alpha = Teuchos::ScalarTraits<Scalar>::one(),
               RangeScalar beta = Teuchos::ScalarTraits<Scalar>::zero()) const;

  //! Computes the estimated condition number and returns the value.
  magnitude_type computeCondEst(Ifpack2::CondestType CT = Ifpack2::Cheap,
                               Ordinal MaxIters = 1550,
                               magnitude_type Tol = 1e-9,
                               const Teuchos::Ptr<const Tpetra::RowMatrix<Scalar,Ordinal,Ordinal,Node> > &Matrix_in = Teuchos::null);

  //! Returns the computed estimated condition number, or -1.0 if no computed.
  magnitude_type getCondEst() const { return Condest_; }

  //! Returns the Tpetra::BlockMap object associated with the range of this matrix operator.
  const Teuchos::RCP<const Teuchos::Comm<int> > & getComm() const;

  //! Returns a reference to the matrix to be preconditioned.
  Teuchos::RCP<const Tpetra::RowMatrix<Scalar,Ordinal,Ordinal,Node> > getMatrix() const;

  //! Returns a reference to the L factor.
  const Teuchos::RCP<const OP> getL() const { return L_; }

  //! Returns a reference to the U factor.
  const Teuchos::RCP<const OP> getU() const { return U_; }

  //! Returns the number of calls to Initialize().
  int getNumInitialize() const;

  //! Returns the number of calls to Compute().
  int getNumCompute() const;

  //! Returns the number of calls to apply().
  int getNumApply() const;

  //! Returns the time spent in Initialize().
  double getInitializeTime() const;

  //! Returns the time spent in Compute().
  double getComputeTime() const;

  //! Returns the time spent in apply().
  double getApplyTime() const;

  /*
  //! The level of fill.
  inline magnitude_type getLevelOfFill() const {
    return(LevelOfFill_);
  }

  //! Get absolute threshold value
  inline magnitude_type getAbsoluteThreshold() const {
    return(Athresh_);
  }

  //! Get relative threshold value
  inline magnitude_type getRelativeThreshold() const {
    return(Rthresh_);
  }

  //! Get the relax value
  inline magnitude_type getRelaxValue() const {
    return(RelaxValue_);
  }
  */

  //! Gets the dropping tolerance
  inline magnitude_type getDropTolerance() const {
    return(DropTolerance_);
  }

  //! Returns the number of nonzero entries in the global graph.
  Ifpack2::global_size_t getGlobalNumEntries() const;

  //! Returns the number of nonzero entries in the local graph.
  size_t getNodeNumEntries() const;

  // @}

  //! @name Overridden from Teuchos::Describable
  //@{

  /** \brief Return a simple one-line description of this object. */
  std::string description() const;

  /** \brief Print the object with some verbosity level to an FancyOStream object. */
  void describe(Teuchos::FancyOStream &out, const Teuchos::EVerbosityLevel verbLevel=Teuchos::Describable::verbLevel_default) const;

  //@}

private:
  typedef Teuchos::ScalarTraits<Scalar> STS;
  typedef Teuchos::ScalarTraits<magnitude_type> STM;
  typedef typename Teuchos::Array<Ordinal>::size_type size_type;

  //@{ Internal methods

  //! Copy constructor (declared private and undefined; may not be used)
  PrecPrecomputed(const PrecPrecomputed& RHS);

  //! operator= (declared private and undefined; may not be used)
  PrecPrecomputed& operator=(const PrecPrecomputed& RHS);

  //@}
  // \name Internal data
  //@{

  //! reference to the matrix to be preconditioned.
  const Teuchos::RCP<const Tpetra::RowMatrix<Scalar,Ordinal,Ordinal,Node> > A_;
  //! L factor
  //Teuchos::RCP<OP> L_;
  //! U factor
  Teuchos::RCP<OP> U_;
  Teuchos::RCP<OP> L_;
  Teuchos::RCP<MV> Y_;
  //Teuchos::RCP<Tpetra::Map<Ordinal, Ordinal, Node> > mapVec_;
  //@}
  // \name Parameters (set by the input ParameterList)
  //@{

  //magnitude_type Athresh_; //!< Absolute threshold
  //magnitude_type Rthresh_; //!< Relative threshold
  //magnitude_type RelaxValue_; //!< Relax value
  //magnitude_type LevelOfFill_; //!< Max fill level
  //! Discard all elements below this tolerance
  magnitude_type DropTolerance_;
  std::string precFileName_;
  //! Condition number estimate
  magnitude_type Condest_;

  //@}
  // \name Other internal data
  //@{

  //! Total time in seconds for all successful calls to initialize().
  double InitializeTime_;
  //! Total time in seconds for all successful calls to compute().
  double ComputeTime_;
  //! Total timer in seconds for all successful calls to apply().
  mutable double ApplyTime_;
  //! The number of successful calls to initialize().
  int NumInitialize_;
  //! The number of successful call to compute().
  int NumCompute_;
  //! The number of successful call to apply().
  mutable int NumApply_;
  //! \c true if \c this object has been initialized
  bool IsInitialized_;
  //! \c true if \c this object has been computed
  bool IsComputed_;
  //@}
}; // class ILUT

} // namespace Ifpack2

#endif /* IFPACK2_ILUT_HPP */
