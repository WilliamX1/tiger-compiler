#include "straightline/slp.h"

#include <iostream>

namespace A {
int A::CompoundStm::MaxArgs() const {
  // TODO: put your code here (lab1).
}

Table *A::CompoundStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
}

int A::AssignStm::MaxArgs() const {
  // TODO: put your code here (lab1).
}

Table *A::AssignStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
}

int A::PrintStm::MaxArgs() const {
  // TODO: put your code here (lab1).
}

Table *A::PrintStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
}


int Table::Lookup(const std::string &key) const {
  if (id == key) {
    return value;
  } else if (tail != nullptr) {
    return tail->Lookup(key);
  } else {
    assert(false);
  }
}

Table *Table::Update(const std::string &key, int val) const {
  return new Table(key, val, this);
}
}  // namespace A
