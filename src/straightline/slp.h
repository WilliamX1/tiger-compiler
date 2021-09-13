#ifndef STRAIGHTLINE_SLP_H_
#define STRAIGHTLINE_SLP_H_

#include <algorithm>
#include <cassert>
#include <string>
#include <list>

namespace A {

class Stm;
class Exp;
class ExpList;

enum BinOp { PLUS = 0, MINUS, TIMES, DIV };

// some data structures used by interp
class Table;
class IntAndTable;

class Stm {
 public:
  virtual int MaxArgs() const = 0;
  virtual Table *Interp(Table *) const = 0;
};

class CompoundStm : public Stm {
 public:
  CompoundStm(Stm *stm1, Stm *stm2) : stm1(stm1), stm2(stm2) {}
  int MaxArgs() const override;
  Table *Interp(Table *) const override;

 private:
  Stm *stm1, *stm2;
};

class AssignStm : public Stm {
 public:
  AssignStm(std::string id, Exp *exp) : id(std::move(id)), exp(exp) {}
  int MaxArgs() const override;
  Table *Interp(Table *) const override;

 private:
  std::string id;
  Exp *exp;
};

class PrintStm : public Stm {
 public:
  explicit PrintStm(ExpList *exps) : exps(exps) {}
  int MaxArgs() const override;
  Table *Interp(Table *) const override;

 private:
  ExpList *exps;
};

class Exp {
  // TODO: you'll have to add some definitions here (lab1).
  // Hints: You may add interfaces like `int MaxArgs()`,
  //        and ` IntAndTable *Interp(Table *)`
};

class IdExp : public Exp {
 public:
  explicit IdExp(std::string id) : id(std::move(id)) {}
  // TODO: you'll have to add some definitions here (lab1).

 private:
  std::string id;
};

class NumExp : public Exp {
 public:
  explicit NumExp(int num) : num(num) {}
  // TODO: you'll have to add some definitions here.

 private:
  int num;
};

class OpExp : public Exp {
 public:
  OpExp(Exp *left, BinOp oper, Exp *right)
      : left(left), oper(oper), right(right) {}

 private:
  Exp *left;
  BinOp oper;
  Exp *right;
};

class EseqExp : public Exp {
 public:
  EseqExp(Stm *stm, Exp *exp) : stm(stm), exp(exp) {}

 private:
  Stm *stm;
  Exp *exp;
};

class ExpList {
 public:
  // TODO: you'll have to add some definitions here (lab1).
  // Hints: You may add interfaces like `int MaxArgs()`, `int NumExps()`,
  //        and ` IntAndTable *Interp(Table *)`
};

class PairExpList : public ExpList {
 public:
  PairExpList(Exp *exp, ExpList *tail) : exp(exp), tail(tail) {}
  // TODO: you'll have to add some definitions here (lab1).
 private:
  Exp *exp;
  ExpList *tail;
};

class LastExpList : public ExpList {
 public:
  LastExpList(Exp *exp) : exp(exp) {}
  // TODO: you'll have to add some definitions here (lab1).
 private:
  Exp *exp;
};

class Table {
 public:
  Table(std::string id, int value, const Table *tail)
      : id(std::move(id)), value(value), tail(tail) {}
  int Lookup(const std::string &key) const;
  Table *Update(const std::string &key, int val) const;

 private:
  std::string id;
  int value;
  const Table *tail;
};

struct IntAndTable {
  int i;
  Table *t;

  IntAndTable(int i, Table *t) : i(i), t(t) {}
};

}  // namespace A

#endif  // STRAIGHTLINE_SLP_H_
