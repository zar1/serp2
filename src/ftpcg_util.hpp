#ifndef FTPCG_UTIL_HPP
#define FTPCG_UTIL_HPP

#include <time.h>

#include "ftpcg.hpp"

namespace ftpcg {

double getCSVEntryD(const char* filename, std::size_t row, std::size_t col);
int getCSVEntryI(const char* filename, std::size_t row, std::size_t col);
std::string getCSVEntryS(const char* filename, std::size_t row, std::size_t col);

std::size_t getCSVNRows(const char* filename);

RCP<MV> newCorrespondingVector(const Teuchos::RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A);

void dump(const RCP<MV>& v, const std::string& name);

void dump(const MV& v, const std::string& name);

void dump(Scalar s, const std::string& name);

//void timeDiff(const struct timespec *start, const struct timespec *stop, struct timespec *diff);


}

#endif
