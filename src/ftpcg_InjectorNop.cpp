#include "ftpcg_InjectorNop.hpp"

namespace ftpcg {

InjectorNop::InjectorNop() {
}

void InjectorNop::setA(const RCP<const OP>& A, int matrixNum) {
}

const bool InjectorNop::isNop() {
  return true;
}

bool InjectorNop::isInjectIter(std::size_t iter) {
  return false;
}

void InjectorNop::inject(const RCP<CriticalState>& critState) {
}

void InjectorNop::describe(std::ostream& out) const {
  out << "Nop";
}

}
