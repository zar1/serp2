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
// See notes below, in the Ifpack2::ILUT::Compute method.
// ABW.
//------------------------------------------------------


#include <MatrixMarket_Tpetra.hpp> 
#include <Tpetra_RowMatrixTransposer_decl.hpp>
#include "ftpcg_PrecPrecomputed.hpp"
#include "ftpcg_util.hpp"

namespace ftpcg {

  namespace {

    /// \brief Default drop tolerance for ILUT.
    ///
    /// \tparam ScalarType The "scalar type"; the type of entries in
    ///   the input sparse matrix to ILUT.  This is the same as the
    ///   scalar_type typedef of ILUT.
    ///
    /// \warning This is an implementation detail of Ifpack2.  Do NOT
    ///   depend on this function or use it in your code.  It may go
    ///   away entirely or change interface or behavior without
    ///   warning.
    ///
    /// This function preserves the previous default drop tolerance
    /// (1e-12, independent of scalar type), thus ensuring backwards
    /// compatibility for the common case of ScalarType=double.
    /// However, it provides a more reasonable default for other
    /// scalar types of possibly lower or higher precision than
    /// double.
    ///
    /// This function is templated on ScalarType, rather than its
    /// magnitude type, so that we can handle complex numbers
    /// specially if desired.
    ///
    /// In order to override the default, just specialize this
    /// function for your particular ScalarType.
    typename Teuchos::ScalarTraits<Scalar>::magnitudeType
    ilutDefaultDropTolerance () {
      using Teuchos::as;
      typedef Teuchos::ScalarTraits<Scalar> STS;
      typedef typename STS::magnitudeType magnitude_type;
      typedef Teuchos::ScalarTraits<magnitude_type> STM;

      // 1/2.  Hopefully this can be represented in magnitude_type.
      const magnitude_type oneHalf = STM::one() / (STM::one() + STM::one());

      // The min ensures that in case magnitude_type has very low
      // precision, we'll at least get some value strictly less than
      // one.
      return std::min (as<magnitude_type> (1000) * STS::magnitude (STS::eps ()), oneHalf);
    }

    // Full specialization for Scalar = double.
    // This specialization preserves ILUT's previous default behavior.
    /*
    Teuchos::ScalarTraits<double>::magnitudeType
    ilutDefaultDropTolerance<double> () {
      return 1e-12;
    }
    */

  } // namespace (anonymous)

//Definitions for the ILUT methods:

//==============================================================================
PrecPrecomputed::PrecPrecomputed(const Teuchos::RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A) :
  A_ (A),
  //Athresh_ (Teuchos::ScalarTraits<magnitude_type>::zero ()),
  //Rthresh_ (Teuchos::ScalarTraits<magnitude_type>::one ()),
  //RelaxValue_ (Teuchos::ScalarTraits<magnitude_type>::zero ()),
  //LevelOfFill_ (Teuchos::ScalarTraits<magnitude_type>::one ()),
  //DropTolerance_ (ilutDefaultDropTolerance<scalar_type> ()),
  DropTolerance_ (0.01),
  Condest_ (-Teuchos::ScalarTraits<magnitude_type>::one ()),
  InitializeTime_(0.0),
  ComputeTime_(0.0),
  ApplyTime_(0.0),
  NumInitialize_(0),
  NumCompute_(0),
  NumApply_(0),
  IsInitialized_ (false),
  IsComputed_ (false)
{
  TEUCHOS_TEST_FOR_EXCEPTION(
    A_.is_null (), std::runtime_error,
    "ftpcg::PrecPrecomputed: Input matrix is null.");
}

//==========================================================================
PrecPrecomputed::~PrecPrecomputed() {
}

//==========================================================================
void PrecPrecomputed::setParameters(const Teuchos::ParameterList& params) {
  using Teuchos::as;
  using Teuchos::Exceptions::InvalidParameterName;
  using Teuchos::Exceptions::InvalidParameterType;

  std::stringstream fileNameStream;
  // Default values of the various parameters.
  //magnitude_type fillLevel = STM::one ();
  //magnitude_type absThresh = STM::zero ();
  //magnitude_type relThresh = STM::one ();
  //magnitude_type relaxValue = STM::zero ();
  //magnitude_type dropTol = ilutDefaultDropTolerance<scalar_type> ();
  magnitude_type dropTol = 0.01;
  std::string path;
  std::string matrixName;
  std::string dropTolString;

  /*
  try {
    fillLevel = params.get<magnitude_type> ("fact: ilut level-of-fill");
  }
  catch (InvalidParameterType&) {
    // Try double, for backwards compatibility.
    // The cast from double to magnitude_type must succeed.
    fillLevel = as<magnitude_type> (params.get<double> ("fact: ilut level-of-fill"));
  }
  catch (InvalidParameterName&) {
    // Accept the default value.
  }

  // FIXME (mfh 28 Nov 2012) This doesn't make any sense.  Isn't zero
  // fill allowed?  The exception test doesn't match the exception
  // message.  However, I'm leaving it alone for now, so as not to
  // change current behavior.
  TEUCHOS_TEST_FOR_EXCEPTION(fillLevel <= 0.0, std::runtime_error,
    "Ifpack2::ILUT::SetParameters ERROR, level-of-fill must be >= 0.");

  try {
    absThresh = params.get<magnitude_type> ("fact: absolute threshold");
  }
  catch (InvalidParameterType&) {
    // Try double, for backwards compatibility.
    // The cast from double to magnitude_type must succeed.
    absThresh = as<magnitude_type> (params.get<double> ("fact: absolute threshold"));
  }
  catch (InvalidParameterName&) {
    // Accept the default value.
  }

  try {
    relThresh = params.get<magnitude_type> ("fact: relative threshold");
  }
  catch (InvalidParameterType&) {
    // Try double, for backwards compatibility.
    // The cast from double to magnitude_type must succeed.
    relThresh = as<magnitude_type> (params.get<double> ("fact: relative threshold"));
  }
  catch (InvalidParameterName&) {
    // Accept the default value.
  }

  try {
    relaxValue = params.get<magnitude_type> ("fact: relax value");
  }
  catch (InvalidParameterType&) {
    // Try double, for backwards compatibility.
    // The cast from double to magnitude_type must succeed.
    relaxValue = as<magnitude_type> (params.get<double> ("fact: relax value"));
  }
  catch (InvalidParameterName&) {
    // Accept the default value.
  }
  */

  try {
    dropTol = params.get<magnitude_type> ("fact: drop tolerance");
  }
  catch (InvalidParameterType&) {
    // Try double, for backwards compatibility.
    // The cast from double to magnitude_type must succeed.
    dropTol = as<magnitude_type> (params.get<double> ("fact: drop tolerance"));
  }
  catch (InvalidParameterName&) {
    // Accept the default value.
  }
  
  path = params.get<std::string> ("prec: path");
  matrixName = params.get<std::string> ("prec: matrix name");
  
  int nTolZeros = -log10(dropTol) -1;
  std::string tolZeros = std::string(nTolZeros, '0');
  fileNameStream << path << "/cholinc/" << matrixName << "cholinc_R" << tolZeros << "1.mtx"; 
  precFileName_ = fileNameStream.str();
  
  

  // "Commit" the values only after validating all of them.  This
  // ensures that there are no side effects if this routine throws an
  // exception.

  // mfh 28 Nov 2012: The previous code would not assign Athresh_,
  // Rthresh_, RelaxValue_, or DropTolerance_ if the read-in value was
  // -1.  I don't know if keeping this behavior is correct, but I'll
  // keep it just so as not to change previous behavior.

  /*
  LevelOfFill_ = fillLevel;
  if (absThresh != -STM::one ()) {
    Athresh_ = absThresh;
  }
  if (relThresh != -STM::one ()) {
    Rthresh_ = relThresh;
  }
  if (relaxValue != -STM::one ()) {
    RelaxValue_ = relaxValue;
  }
  if (dropTol != -STM::one ()) {
    DropTolerance_ = dropTol;
  }
  */
}

//==========================================================================
const Teuchos::RCP<const Teuchos::Comm<int> > &
PrecPrecomputed::getComm() const{
  return A_->getComm ();
}

//==========================================================================
Teuchos::RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >
PrecPrecomputed::getMatrix() const {
  return A_;
}

//==========================================================================
const Teuchos::RCP<const Tpetra::Map<Ordinal, Ordinal, Node> >&
PrecPrecomputed::getDomainMap() const
{
  return A_->getDomainMap();
}

//==========================================================================
const Teuchos::RCP<const Tpetra::Map<Ordinal, Ordinal, Node> >&
PrecPrecomputed::getRangeMap() const
{
  return A_->getRangeMap();
}

//==============================================================================
bool PrecPrecomputed::hasTransposeApply() const {
  return true;
}

//==========================================================================
int PrecPrecomputed::getNumInitialize() const {
  return(NumInitialize_);
}

//==========================================================================
int PrecPrecomputed::getNumCompute() const {
  return(NumCompute_);
}

//==========================================================================
int PrecPrecomputed::getNumApply() const {
  return(NumApply_);
}

//==========================================================================
double PrecPrecomputed::getInitializeTime() const {
  return(InitializeTime_);
}

//==========================================================================
double PrecPrecomputed::getComputeTime() const {
  return(ComputeTime_);
}

//==========================================================================
double PrecPrecomputed::getApplyTime() const {
  return(ApplyTime_);
}

//==========================================================================
Ifpack2::global_size_t PrecPrecomputed::getGlobalNumEntries() const {
  //return(L_->getGlobalNumEntries() + U_->getGlobalNumEntries());
  return U_->getGlobalNumEntries() * 2;
}

//==========================================================================
size_t PrecPrecomputed::getNodeNumEntries() const {
  return U_->getNodeNumEntries() * 2;
}

//=============================================================================
typename PrecPrecomputed::magnitude_type
PrecPrecomputed::
computeCondEst (Ifpack2::CondestType CT,
                Ordinal MaxIters,
                magnitude_type Tol,
                const Teuchos::Ptr<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> > &matrix) {

  if (! isComputed ()) {
    return -STM::one ();
  }
  // NOTE: this is computing the *local* condest
  if (Condest_ == -STM::one ()) {
    Condest_ = Ifpack2::Condest(*this, CT, MaxIters, Tol, matrix);
  }
  return(Condest_);
}

//==========================================================================
void PrecPrecomputed::initialize() {
  Teuchos::Time timer ("ILUT::initialize");
  {
    Teuchos::TimeMonitor timeMon (timer);

    // clear any previous allocation
    IsInitialized_ = false;
    IsComputed_ = false;
    L_ = Teuchos::null;
    U_ = Teuchos::null;
    //mapVec_ = rcp(new Tpetra::Map<Ordinal, Ordinal, Node>(A_->getGlobalNumRows(), 0, A_->getComm(), Tpetra::GloballyDistributed,  Kokkos::DefaultNode::getDefaultNode()));
    //Y_ = rcp(new MV(mapVec_, 1));
    Y_ = newCorrespondingVector(A_);

    // check only in serial if the matrix is square
    TEUCHOS_TEST_FOR_EXCEPTION(
      getComm ()->getSize () == 1 && A_->getNodeNumRows() != A_->getNodeNumCols(),
      std::runtime_error, "Ifpack2::ILUT::initialize: In serial or one-process "
      "mode, the input matrix must be square.");
  }
  IsInitialized_ = true;
  ++NumInitialize_;
  InitializeTime_ += timer.totalElapsedTime ();
}

typename Teuchos::ScalarTraits<Scalar>::magnitudeType scalar_mag(const Scalar& s)
{
  return Teuchos::ScalarTraits<Scalar>::magnitude(s);
}

//==========================================================================
void PrecPrecomputed::compute() {
  using Teuchos::Array;
  using Teuchos::ArrayRCP;
  using Teuchos::ArrayView;
  using Teuchos::as;
  using Teuchos::rcp;
  using Teuchos::reduceAll;

  //--------------------------------------------------------------------------
  // Ifpack2::ILUT is a translation of the Aztec ILUT implementation. The Aztec
  // ILUT implementation was written by Ray Tuminaro.
  //
  // This isn't an exact translation of the Aztec ILUT algorithm, for the
  // following reasons:
  // 1. Minor differences result from the fact that Aztec factors a MSR format
  // matrix in place, while the code below factors an input CrsMatrix which
  // remains untouched and stores the resulting factors in separate L and U
  // CrsMatrix objects.
  // Also, the Aztec code begins by shifting the matrix pointers back
  // by one, and the pointer contents back by one, and then using 1-based
  // Fortran-style indexing in the algorithm. This Ifpack2 code uses C-style
  // 0-based indexing throughout.
  // 2. Aztec stores the inverse of the diagonal of U. This Ifpack2 code
  // stores the non-inverted diagonal in U.
  // The triangular solves (in Ifpack2::ILUT::apply()) are performed by
  // calling the Tpetra::CrsMatrix::solve method on the L and U objects, and
  // this requires U to contain the non-inverted diagonal.
  //
  // ABW.
  //--------------------------------------------------------------------------

  // Don't count initialization in the compute() time.
  if (! isInitialized ()) {
    initialize ();
  }

  Teuchos::Time timer ("ILUT::compute");
  { // Timer scope for timing compute()
    Teuchos::TimeMonitor timeMon (timer, true);
    /*
    const scalar_type zero = STS::zero ();
    const scalar_type one  = STS::one ();

    const local_ordinal_type myNumRows = A_->getNodeNumRows ();
    L_ = rcp (new MatrixType (A_->getRowMap (), A_->getColMap (), 0));
    U_ = rcp (new MatrixType (A_->getRowMap (), A_->getColMap (), 0));

    // CGB: note, this caching approach may not be necessary anymore
    // We will store ArrayView objects that are views of the rows of U, so that
    // we don't have to repeatedly retrieve the view for each row. These will
    // be populated row by row as the factorization proceeds.
    Array<ArrayView<const local_ordinal_type> > Uindices (myNumRows);
    Array<ArrayView<const scalar_type> >       Ucoefs (myNumRows);

    // If this macro is defined, files containing the L and U factors
    // will be written. DON'T CHECK IN THE CODE WITH THIS MACRO ENABLED!!!
    // #define IFPACK2_WRITE_FACTORS
#ifdef IFPACK2_WRITE_FACTORS
    std::ofstream ofsL("L.tif.mtx", std::ios::out);
    std::ofstream ofsU("U.tif.mtx", std::ios::out);
#endif

    // mfh 28 Nov 2012: The code here uses double for fill calculations.
    // This really has nothing to do with magnitude_type.  The point is
    // to avoid rounding and overflow for integer calculations.  That
    // should work just fine for reasonably-sized integer values, so I'm
    // leaving it.  However, the fill level parameter is a
    // magnitude_type, so I do need to cast to double.  Typical values
    // of the fill level are small, so this should not cause overflow.

    // Calculate how much fill will be allowed in addition to the space that
    // corresponds to the input matrix entries.
    double local_nnz = static_cast<double>(A_->getNodeNumEntries());
    double fill;
    {
      const double fillLevel = as<double> (getLevelOfFill ());
      fill = ((fillLevel - 1) * local_nnz) / (2 * myNumRows);
    }

    // std::ceil gives the smallest integer larger than the argument.
    // this may give a slightly different result than Aztec's fill value in
    // some cases.
    double fill_ceil=std::ceil(fill);

    // Similarly to Aztec, we will allow the same amount of fill for each
    // row, half in L and half in U.
    size_type fillL = static_cast<size_type>(fill_ceil);
    size_type fillU = static_cast<size_type>(fill_ceil);

    Array<scalar_type> InvDiagU (myNumRows, zero);

    Array<local_ordinal_type> tmp_idx;
    Array<scalar_type> tmpv;

    enum { UNUSED, ORIG, FILL };
    local_ordinal_type max_col = myNumRows;

    Array<int> pattern(max_col, UNUSED);
    Array<scalar_type> cur_row(max_col, zero);
    Array<magnitude_type> unorm(max_col);
    magnitude_type rownorm;
    Array<local_ordinal_type> L_cols_heap;
    Array<local_ordinal_type> U_cols;
    Array<local_ordinal_type> L_vals_heap;
    Array<local_ordinal_type> U_vals_heap;

    // A comparison object which will be used to create 'heaps' of indices
    // that are ordered according to the corresponding values in the
    // 'cur_row' array.
    greater_indirect<scalar_type,local_ordinal_type> vals_comp(cur_row);

    // =================== //
    // start factorization //
    // =================== //

    ArrayRCP<local_ordinal_type> ColIndicesARCP;
    ArrayRCP<scalar_type>       ColValuesARCP;
    if (! A_->supportsRowViews ()) {
      const size_t maxnz = A_->getNodeMaxNumRowEntries ();
      ColIndicesARCP.resize (maxnz);
      ColValuesARCP.resize (maxnz);
    }

    for (local_ordinal_type row_i = 0 ; row_i < myNumRows ; ++row_i) {
      ArrayView<const local_ordinal_type> ColIndicesA;
      ArrayView<const scalar_type> ColValuesA;
      size_t RowNnz;

      if (A_->supportsRowViews ()) {
        A_->getLocalRowView (row_i, ColIndicesA, ColValuesA);
        RowNnz = ColIndicesA.size ();
      }
      else {
        A_->getLocalRowCopy (row_i, ColIndicesARCP (), ColValuesARCP (), RowNnz);
        ColIndicesA = ColIndicesARCP (0, RowNnz);
        ColValuesA = ColValuesARCP (0, RowNnz);
      }

      // Always include the diagonal in the U factor. The value should get
      // set in the next loop below.
      U_cols.push_back(row_i);
      cur_row[row_i] = zero;
      pattern[row_i] = ORIG;

      size_type L_cols_heaplen = 0;
      rownorm = STM::zero ();
      for (size_t i = 0; i < RowNnz; ++i) {
        if (ColIndicesA[i] < myNumRows) {
          if (ColIndicesA[i] < row_i) {
            add_to_heap(ColIndicesA[i], L_cols_heap, L_cols_heaplen);
          }
          else if (ColIndicesA[i] > row_i) {
            U_cols.push_back(ColIndicesA[i]);
          }

          cur_row[ColIndicesA[i]] = ColValuesA[i];
          pattern[ColIndicesA[i]] = ORIG;
          rownorm += scalar_mag(ColValuesA[i]);
        }
      }

      // Alter the diagonal according to the absolute-threshold and
      // relative-threshold values. If not set, those values default
      // to zero and one respectively.
      const magnitude_type rthresh = getRelativeThreshold();
      const scalar_type& v = cur_row[row_i];
      cur_row[row_i] = as<scalar_type> (getAbsoluteThreshold() * IFPACK2_SGN(v)) + rthresh*v;

      size_type orig_U_len = U_cols.size();
      RowNnz = L_cols_heap.size() + orig_U_len;
      rownorm = getDropTolerance() * rownorm/RowNnz;

      // The following while loop corresponds to the 'L30' goto's in Aztec.
      size_type L_vals_heaplen = 0;
      while (L_cols_heaplen > 0) {
        local_ordinal_type row_k = L_cols_heap.front();

        scalar_type multiplier = cur_row[row_k] * InvDiagU[row_k];
        cur_row[row_k] = multiplier;
        magnitude_type mag_mult = scalar_mag(multiplier);
        if (mag_mult*unorm[row_k] < rownorm) {
          pattern[row_k] = UNUSED;
          rm_heap_root(L_cols_heap, L_cols_heaplen);
          continue;
        }
        if (pattern[row_k] != ORIG) {
          if (L_vals_heaplen < fillL) {
            add_to_heap(row_k, L_vals_heap, L_vals_heaplen, vals_comp);
          }
          else if (L_vals_heaplen==0 ||
                   mag_mult < scalar_mag(cur_row[L_vals_heap.front()])) {
            pattern[row_k] = UNUSED;
            rm_heap_root(L_cols_heap, L_cols_heaplen);
            continue;
          }
          else {
            pattern[L_vals_heap.front()] = UNUSED;
            rm_heap_root(L_vals_heap, L_vals_heaplen, vals_comp);
            add_to_heap(row_k, L_vals_heap, L_vals_heaplen, vals_comp);
          }
        }

        */
        /* Reduce current row */
        /*

        ArrayView<const local_ordinal_type>& ColIndicesU = Uindices[row_k];
        ArrayView<const scalar_type>& ColValuesU = Ucoefs[row_k];
        size_type ColNnzU = ColIndicesU.size();

        for(size_type j=0; j<ColNnzU; ++j) {
          if (ColIndicesU[j] > row_k) {
            scalar_type tmp = multiplier * ColValuesU[j];
            local_ordinal_type col_j = ColIndicesU[j];
            if (pattern[col_j] != UNUSED) {
              cur_row[col_j] -= tmp;
            }
            else if (scalar_mag(tmp) > rownorm) {
              cur_row[col_j] = -tmp;
              pattern[col_j] = FILL;
              if (col_j > row_i) {
                U_cols.push_back(col_j);
              }
              else {
                add_to_heap(col_j, L_cols_heap, L_cols_heaplen);
              }
            }
          }
        }

        rm_heap_root(L_cols_heap, L_cols_heaplen);
      }//end of while(L_cols_heaplen) loop


      // Put indices and values for L into arrays and then into the L_ matrix.

      //   first, the original entries from the L section of A:
      for (size_type i = 0; i < ColIndicesA.size (); ++i) {
        if (ColIndicesA[i] < row_i) {
          tmp_idx.push_back(ColIndicesA[i]);
          tmpv.push_back(cur_row[ColIndicesA[i]]);
          pattern[ColIndicesA[i]] = UNUSED;
        }
      }

      //   next, the L entries resulting from fill:
      for (size_type j = 0; j < L_vals_heaplen; ++j) {
        tmp_idx.push_back(L_vals_heap[j]);
        tmpv.push_back(cur_row[L_vals_heap[j]]);
        pattern[L_vals_heap[j]] = UNUSED;
      }

      // L has a one on the diagonal, but we don't explicitly store it.
      // If we don't store it, then the Tpetra/Kokkos kernel which performs
      // the triangular solve can assume a unit diagonal, take a short-cut
      // and perform faster.

      L_->insertLocalValues (row_i, tmp_idx (), tmpv ());
#ifdef IFPACK2_WRITE_FACTORS
      for (size_type ii = 0; ii < tmp_idx.size (); ++ii) {
        ofsL << row_i << " " << tmp_idx[ii] << " " << tmpv[ii] << std::endl;
      }
#endif

      tmp_idx.clear();
      tmpv.clear();

      // Pick out the diagonal element, store its reciprocal.
      if (cur_row[row_i] == zero) {
        std::cerr << "Ifpack2::ILUT::Compute: zero pivot encountered! Replacing with rownorm and continuing...(You may need to set the parameter 'fact: absolute threshold'.)" << std::endl;
        cur_row[row_i] = rownorm;
      }
      InvDiagU[row_i] = one / cur_row[row_i];

      // Non-inverted diagonal is stored for U:
      tmp_idx.push_back(row_i);
      tmpv.push_back(cur_row[row_i]);
      unorm[row_i] = scalar_mag(cur_row[row_i]);
      pattern[row_i] = UNUSED;

      // Now put indices and values for U into arrays and then into the U_ matrix.
      // The first entry in U_cols is the diagonal, which we just handled, so we'll
      // start our loop at j=1.

      size_type U_vals_heaplen = 0;
      for(size_type j=1; j<U_cols.size(); ++j) {
        local_ordinal_type col = U_cols[j];
        if (pattern[col] != ORIG) {
          if (U_vals_heaplen < fillU) {
            add_to_heap(col, U_vals_heap, U_vals_heaplen, vals_comp);
          }
          else if (U_vals_heaplen!=0 && scalar_mag(cur_row[col]) >
                   scalar_mag(cur_row[U_vals_heap.front()])) {
            rm_heap_root(U_vals_heap, U_vals_heaplen, vals_comp);
            add_to_heap(col, U_vals_heap, U_vals_heaplen, vals_comp);
          }
        }
        else {
          tmp_idx.push_back(col);
          tmpv.push_back(cur_row[col]);
          unorm[row_i] += scalar_mag(cur_row[col]);
        }
        pattern[col] = UNUSED;
      }

      for(size_type j=0; j<U_vals_heaplen; ++j) {
        tmp_idx.push_back(U_vals_heap[j]);
        tmpv.push_back(cur_row[U_vals_heap[j]]);
        unorm[row_i] += scalar_mag(cur_row[U_vals_heap[j]]);
      }

      unorm[row_i] /= (orig_U_len + U_vals_heaplen);

      U_->insertLocalValues(row_i, tmp_idx(), tmpv() );
#ifdef IFPACK2_WRITE_FACTORS
      for(int ii=0; ii<tmp_idx.size(); ++ii) {
        ofsU <<row_i<< " " <<tmp_idx[ii]<< " " <<tmpv[ii]<< std::endl;
      }
#endif
      tmp_idx.clear();
      tmpv.clear();

      U_->getLocalRowView(row_i, Uindices[row_i], Ucoefs[row_i] );

      L_cols_heap.clear();
      U_cols.clear();
      L_vals_heap.clear();
      U_vals_heap.clear();
    } // end of for(row_i) loop

    // FIXME (mfh 03 Apr 2013) Do we need to supply a domain and range Map?
    L_->fillComplete();
    U_->fillComplete();
    */
    U_ = Tpetra::MatrixMarket::Reader<OP>::readSparseFile(precFileName_, getComm(), Kokkos::DefaultNode::getDefaultNode());
    Tpetra::RowMatrixTransposer<Scalar, Ordinal, Ordinal, Node, Kokkos::DefaultKernels<Scalar, Ordinal, Node>::SparseOps> trans(U_.getConst());
    L_ = trans.createTranspose();
    //std::cout << "###" << U_->isUpperTriangular() << std::endl;
    
  }
  ComputeTime_ += timer.totalElapsedTime ();
  IsComputed_ = true;
  ++NumCompute_;
}

//==========================================================================
void PrecPrecomputed::apply(
           const Tpetra::MultiVector<Scalar, Ordinal, Ordinal, Node>& X,
                 Tpetra::MultiVector<Scalar, Ordinal, Ordinal, Node>& Z,
                 Teuchos::ETransp mode,
                 Scalar alpha,
                 Scalar beta) const
{
  applyTempl<Scalar,Scalar>(X, Z, mode, alpha, beta);
}

//==========================================================================
template <class DomainScalar, class RangeScalar>
void PrecPrecomputed::applyTempl(
           const Tpetra::MultiVector<DomainScalar, Ordinal, Ordinal, Node>& X,
                 Tpetra::MultiVector<RangeScalar, Ordinal, Ordinal, Node>& Z,
                 Teuchos::ETransp mode,
               RangeScalar alpha,
               RangeScalar beta) const
{
  using Teuchos::RCP;
  using Teuchos::rcp;
  typedef Tpetra::MultiVector<DomainScalar, Ordinal, Ordinal, Node> MV;

  Teuchos::Time timer ("ILUT::apply");
  { // Timer scope for timing apply()
    Teuchos::TimeMonitor timeMon (timer, true);

    TEUCHOS_TEST_FOR_EXCEPTION(
      ! isComputed (), std::runtime_error,
      "Ifpack2::ILUT::apply: You must call compute() to compute the incomplete "
      "factorization, before calling apply().");

    TEUCHOS_TEST_FOR_EXCEPTION(
      X.getNumVectors() != Z.getNumVectors(), std::runtime_error,
      "Ifpack2::ILUT::apply: X and Y must have the same number of columns.  "
      "X has " << X.getNumVectors () << " columns, but Y has "
      << Z.getNumVectors () << " columns.");

    TEUCHOS_TEST_FOR_EXCEPTION(
      beta != STS::zero (), std::logic_error,
      "Ifpack2::ILUT::apply: This method does not currently work when beta != 0.");

    // If X and Y are pointing to the same memory location,
    // we need to create an auxiliary vector, Xcopy
    RCP<const MV> Xcopy;
    if (X.getLocalMV ().getValues () == Z.getLocalMV ().getValues ()) {
      Xcopy = rcp (new MV (X));
    }
    else {
      Xcopy = rcpFromRef (X);
      //ZACH always taken
    }

    //if (mode == Teuchos::NO_TRANS) { // Solve L U Y = X
      /*
      L_->template localSolve<RangeScalar,DomainScalar> (*Xcopy, Y, Teuchos::NO_TRANS);
      U_->template localSolve<RangeScalar,RangeScalar> (Y, Y, Teuchos::NO_TRANS);
      */
      
      //dump(X, "##X");
      //dump(*Xcopy, "##Xcopy");
      //ZACH the problem is here: This actually needs to be a global triangular solve
      L_->template localSolve<RangeScalar,DomainScalar> (*Xcopy, *Y_, Teuchos::NO_TRANS);
      //dump(*Y_, "##Y");
      U_->template localSolve<RangeScalar,RangeScalar> (*Y_, Z, Teuchos::NO_TRANS);
      
      
      //U_->template localSolve<RangeScalar,RangeScalar> (*Xcopy, Z, Teuchos::NO_TRANS);
      //dump(Z, "##Z");
      //U_->template localSolve<RangeScalar,DomainScalar> (*Xcopy, Y, Teuchos::NO_TRANS);
      //U_->template localSolve<RangeScalar,RangeScalar> (Y, Y, Teuchos::TRANS);
      
    //}
    //else { // Solve U^* L^* Y = X
      /*
      U_->template localSolve<RangeScalar,DomainScalar> (*Xcopy, Y, mode);
      L_->template localSolve<RangeScalar,RangeScalar> (Y, Y, mode);
      */
    //}

    if (alpha != STS::one ()) {
      Z.scale (alpha);
    }
  }
  ++NumApply_;
  ApplyTime_ += timer.totalElapsedTime ();
}

//=============================================================================
std::string PrecPrecomputed::description() const {
  std::ostringstream oss;
  oss << Teuchos::Describable::description();
  if (isInitialized()) {
    if (isComputed()) {
      oss << "{status: [initialized, computed]";
    }
    else {
      oss << "{status: [initialized, not computed]";
    }
  }
  else {
    oss << "{status: [not initialized, not computed]";
  }
  oss << ", global number of rows: " << A_->getGlobalNumRows()
      << ", global number of columns: " << A_->getGlobalNumCols()
      << "}";
  return oss.str();
}

//=============================================================================
void PrecPrecomputed::describe(Teuchos::FancyOStream &out, const Teuchos::EVerbosityLevel verbLevel) const {
  using std::endl;
  using std::setw;
  using Teuchos::VERB_DEFAULT;
  using Teuchos::VERB_NONE;
  using Teuchos::VERB_LOW;
  using Teuchos::VERB_MEDIUM;
  using Teuchos::VERB_HIGH;
  using Teuchos::VERB_EXTREME;

  const Teuchos::EVerbosityLevel vl = (verbLevel == VERB_DEFAULT) ? VERB_LOW : verbLevel;
  Teuchos::OSTab tab (out);
  //    none: print nothing
  //     low: print O(1) info from node 0
  //  medium:
  //    high:
  // extreme:
  if (vl != VERB_NONE && getComm ()->getRank () == 0) {
    out << this->description() << endl;
    out << endl;
    out << "===============================================================================" << endl;
    //out << "Level-of-fill      = " << getLevelOfFill()       << endl;
    //out << "Absolute threshold = " << getAbsoluteThreshold() << endl;
    //out << "Relative threshold = " << getRelativeThreshold() << endl;
    //out << "Relax value        = " << getRelaxValue()        << endl;
    if   (Condest_ == -1.0) { out << "Condition number estimate       = N/A" << endl; }
    else                    { out << "Condition number estimate       = " << Condest_ << endl; }
    if (isComputed()) {
      out << "Number of nonzeros in A         = " << A_->getGlobalNumEntries() << endl;
      out << "Number of nonzeros in L + U     = " << getGlobalNumEntries()
          << " ( = " << 100.0 * (double)getGlobalNumEntries() / (double)A_->getGlobalNumEntries() << " % of A)" << endl;
      out << "nonzeros / rows                 = " << 1.0 * getGlobalNumEntries() / U_->getGlobalNumRows() << endl;
    }
    out << endl;
    out << "Phase           # calls    Total Time (s) " << endl;
    out << "------------    -------    ---------------" << endl;
    out << "initialize()    " << setw(7) << getNumInitialize() << "    " << setw(15) << getInitializeTime() << endl;
    out << "compute()       " << setw(7) << getNumCompute()    << "    " << setw(15) << getComputeTime()    << endl;
    out << "apply()         " << setw(7) << getNumApply()      << "    " << setw(15) << getApplyTime()      << endl;
    out << "==============================================================================="                << endl;
    out << endl;
  }
}


}//namespace Ifpack2


