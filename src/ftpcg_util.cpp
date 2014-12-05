#include "ftpcg.hpp"
#include "ftpcg_util.hpp"

namespace ftpcg {

void openCSV(const char* filename, std::ifstream& csv) {
  std::string fullpath(STR_FTPCG_RESOURCE_PREFIX "/parameters/");
  csv.open((fullpath + filename + ".csv").c_str());
}

//TODO this ignores commas and newlines within string literals
void getCSVEntry(const char* filename, std::size_t row, std::size_t col, std::string& outBuf) {
  
  std::ifstream csv;
  openCSV(filename, csv);
  std::string lineBuf;

  int i;
  
  for (i = 0; i <= row; ++i) getline(csv, lineBuf);
  std::stringstream lineStream(lineBuf);
  for (i = 0; i <= col; ++i) getline(lineStream, outBuf, ',');
  csv.close();
}

double getCSVEntryD(const char* filename, std::size_t row, std::size_t col) {
  std::string buf;
  getCSVEntry(filename, row, col, buf);
  return atof(buf.c_str());
}

int getCSVEntryI(const char* filename, std::size_t row, std::size_t col) {
  std::string buf;
  getCSVEntry(filename, row, col, buf);
  return atoi(buf.c_str());
}

std::string getCSVEntryS(const char* filename, std::size_t row, std::size_t col) {
  std::string buf;
  getCSVEntry(filename, row, col, buf);
  buf.erase(buf.length()-1,1);
  buf.erase(0,1);
  return buf;
}

std::size_t getCSVNRows(const char* filename) {
  std::ifstream csv;
  openCSV(filename, csv);
  std::string buf;
  int i = 0;
  while (getline(csv, buf)) ++i;
  csv.close();
  return i;
}

RCP<MV> newCorrespondingVector(const Teuchos::RCP<const Tpetra::RowMatrix<Scalar, Ordinal, Ordinal, Node> >& A) {
  Teuchos::RCP<Tpetra::Map<Ordinal, Ordinal, Node> > mapVec(new Tpetra::Map<Ordinal, Ordinal, Node>(A->getGlobalNumRows(), 0, A->getComm(), Tpetra::GloballyDistributed,  Kokkos::DefaultNode::getDefaultNode()));
  //mapVec_ = rcp(new Tpetra::Map<Ordinal, Ordinal, Node>(A_->getGlobalNumRows(), 0, A_->getComm(), Tpetra::GloballyDistributed,  Kokkos::DefaultNode::getDefaultNode()));
  //Y_ = rcp(new MV(mapVec_, 1));
  return rcp(new MV(mapVec, 1));
}

void dump(const RCP<MV>& v, const std::string& name) {
  dump(*v, name);
}

void dump(const MV& v, const std::string& name) {
  std::cout << name << std::endl;
  Teuchos::ArrayRCP<const Scalar> view = v.get1dView();
  for (Teuchos::ArrayRCP<const Scalar>::iterator it = view.begin(); it != view.end(); ++it) {
    std::cout << *it << std::endl;
  }
  std::cout << std::endl;
}


void dump(Scalar s, const std::string& name) {
  return;
  std::cout << name << std::endl << s << std::endl << std::endl;
}

/*
void timeDiff(const struct timespec *start, const struct timespec *stop, struct timespec *diff) {
  time_t sec_adj = 0;
  if (stop->tv_nsec < start->tv_nsec) {
    sec_adj = 1;
    diff->tv_nsec = 1e9 - (start->tv_nsec - stop->tv_nsec);
  } else {
    diff->tv_nsec = stop->tv_nsec - start->tv_nsec;
  }
  diff->tv_sec = stop->tv_sec - start->tv_sec - sec_adj;
}
*/

}
