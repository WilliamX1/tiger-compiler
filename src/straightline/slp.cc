#include "straightline/slp.h"

#include <iostream>

namespace A {
int A::CompoundStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  return stm1->MaxArgs() > stm2->MaxArgs() ? stm1->MaxArgs() : stm2->MaxArgs();
}

Table *A::CompoundStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  return stm2->Interp(stm1->Interp(t));
}

int A::AssignStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  return exp->MaxArgs();
}

Table *A::AssignStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  IntAndTable* int_and_table = exp->InterpExp(t);
  Table* table = int_and_table->t;
  table = table->Update(id, int_and_table->i);
  return table;
}

int A::PrintStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  return exps->MaxArgs();
}

Table *A::PrintStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  IntAndTable* int_and_table = exps->Interp(t);
  return int_and_table->t;
}

int IdExp::MaxArgs() const {
  return 1;
}

IntAndTable* IdExp::InterpExp(Table* t) const {
  return new IntAndTable(t->Lookup(id), t);
}

int NumExp::MaxArgs() const {
  return 1;
}

IntAndTable* NumExp::InterpExp(Table* t) const {
  return new IntAndTable(num, t);
}

int OpExp::MaxArgs() const {
  return 1;
}

IntAndTable* OpExp::InterpExp(Table* t) const {
  IntAndTable* left_int_and_table = left->InterpExp(t);
  IntAndTable* right_int_and_table = right->InterpExp(left_int_and_table->t);

  switch (oper)
  {
  case PLUS: return new IntAndTable(left_int_and_table->i + right_int_and_table->i, right_int_and_table->t);
    break;
  case MINUS: return new IntAndTable(left_int_and_table->i - right_int_and_table->i, right_int_and_table->t);
    break;
  case TIMES: return new IntAndTable(left_int_and_table->i * right_int_and_table->i, right_int_and_table->t);
    break;
  case DIV: return new IntAndTable(left_int_and_table->i / right_int_and_table->i, right_int_and_table->t);
    break;
  default:
    break;
  }
  return nullptr;
}

int EseqExp::MaxArgs() const {
  return stm->MaxArgs() > exp->MaxArgs() ? stm->MaxArgs() : exp->MaxArgs();
}

IntAndTable* EseqExp::InterpExp(Table* t) const {
  return exp->InterpExp(stm->Interp(t));
}

int PairExpList::MaxArgs() const {
  return exp->MaxArgs() + tail->MaxArgs();
}

int PairExpList::NumExps() const {
  return 1 + tail->NumExps();
}

IntAndTable* PairExpList::Interp(Table* t) const {
  IntAndTable* int_and_table = exp->InterpExp(t);
  printf("%d ", int_and_table->i);
  return tail->Interp(int_and_table->t);
}

int LastExpList::MaxArgs() const {
  return 1;
}

int LastExpList::NumExps() const {
  return 1;
}

IntAndTable* LastExpList::Interp(Table* t) const {
  IntAndTable* int_and_table = exp->InterpExp(t);
  printf("%d\n", int_and_table->i);
  return int_and_table;
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
