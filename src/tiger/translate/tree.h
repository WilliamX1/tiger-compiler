#ifndef TIGER_TRANSLATE_TREE_H_
#define TIGER_TRANSLATE_TREE_H_

#include <array>
#include <cassert>
#include <cstdio>
#include <list>
#include <string>

#include "tiger/frame/temp.h"

// Forward Declarations
namespace canon {
class StmAndExp;
class Canon;
} // namespace canon

namespace assem {
class InstrList;
} // namespace assem

namespace frame {
class RegManager;
} // namespace frame

namespace tree {

class Stm;
class Exp;
class NameExp;

class ExpList;
class StmList;

enum BinOp {
  PLUS_OP,
  MINUS_OP,
  MUL_OP,
  DIV_OP,
  AND_OP,
  OR_OP,
  LSHIFT_OP,
  RSHIFT_OP,
  ARSHIFT_OP,
  XOR_OP,
  BIN_OPER_COUNT,
};

enum RelOp {
  EQ_OP,
  NE_OP,
  LT_OP,
  GT_OP,
  LE_OP,
  GE_OP,
  ULT_OP,
  ULE_OP,
  UGT_OP,
  UGE_OP,
  REL_OPER_COUNT,
};

/**
 * Statements
 */

class Stm {
public:
  virtual ~Stm() = default;

  virtual void Print(FILE *out, int d) const = 0;
  virtual Stm *Canon() = 0;
  virtual void Munch(assem::InstrList &instr_list, std::string_view fs) = 0;
  // Used for Canon
  bool IsNop();
  static Stm *Seq(Stm *x, Stm *y);
  static bool Commute(tree::Stm *x, tree::Exp *y);
};

class SeqStm : public Stm {
public:
  Stm *left_, *right_;

  SeqStm(Stm *left, Stm *right) : left_(left), right_(right) { assert(left); }
  ~SeqStm() override;

  void Print(FILE *out, int d) const override;
  Stm *Canon() override;
  void Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class LabelStm : public Stm {
public:
  temp::Label *label_;

  explicit LabelStm(temp::Label *label) : label_(label) {}
  ~LabelStm() override;

  void Print(FILE *out, int d) const override;
  Stm *Canon() override;
  void Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class JumpStm : public Stm {
public:
  NameExp *exp_;
  std::vector<temp::Label *> *jumps_;

  JumpStm(NameExp *exp, std::vector<temp::Label *> *jumps)
      : exp_(exp), jumps_(jumps) {}
  ~JumpStm() override;

  void Print(FILE *out, int d) const override;
  Stm *Canon() override;
  void Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class CjumpStm : public Stm {
public:
  RelOp op_;
  Exp *left_, *right_;
  temp::Label *true_label_, *false_label_;

  CjumpStm(RelOp op, Exp *left, Exp *right, temp::Label *true_label,
           temp::Label *false_label)
      : op_(op), left_(left), right_(right), true_label_(true_label),
        false_label_(false_label) {}
  ~CjumpStm() override;

  void Print(FILE *out, int d) const override;
  Stm *Canon() override;
  void Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class MoveStm : public Stm {
public:
  Exp *dst_, *src_;

  MoveStm(Exp *dst, Exp *src) : dst_(dst), src_(src) {}
  ~MoveStm() override;

  void Print(FILE *out, int d) const override;
  Stm *Canon() override;
  void Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class ExpStm : public Stm {
public:
  Exp *exp_;

  explicit ExpStm(Exp *exp) : exp_(exp) {}
  ~ExpStm() override;

  void Print(FILE *out, int d) const override;
  Stm *Canon() override;
  void Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

/**
 *Expressions
 */

class Exp {
public:
  virtual ~Exp() = default;

  virtual void Print(FILE *out, int d) const = 0;
  virtual canon::StmAndExp Canon() = 0;
  virtual temp::Temp *Munch(assem::InstrList &instr_list, std::string_view fs) = 0;
};

class BinopExp : public Exp {
public:
  BinOp op_;
  Exp *left_, *right_;

  BinopExp(BinOp op, Exp *left, Exp *right)
      : op_(op), left_(left), right_(right) {}
  ~BinopExp() override;

  void Print(FILE *out, int d) const override;
  canon::StmAndExp Canon() override;
  temp::Temp *Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class MemExp : public Exp {
public:
  Exp *exp_;

  explicit MemExp(Exp *exp) : exp_(exp) {}
  ~MemExp() override;

  void Print(FILE *out, int d) const override;
  canon::StmAndExp Canon() override;
  temp::Temp *Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class TempExp : public Exp {
public:
  temp::Temp *temp_;

  explicit TempExp(temp::Temp *temp) : temp_(temp) {}
  ~TempExp() override;

  void Print(FILE *out, int d) const override;
  canon::StmAndExp Canon() override;
  temp::Temp *Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class EseqExp : public Exp {
public:
  Stm *stm_;
  Exp *exp_;

  EseqExp(Stm *stm, Exp *exp) : stm_(stm), exp_(exp) {}
  ~EseqExp() override;

  void Print(FILE *out, int d) const override;
  canon::StmAndExp Canon() override;
  temp::Temp *Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class NameExp : public Exp {
public:
  temp::Label *name_;

  explicit NameExp(temp::Label *name) : name_(name) {}
  ~NameExp() override;

  void Print(FILE *out, int d) const override;
  canon::StmAndExp Canon() override;
  temp::Temp *Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class ConstExp : public Exp {
public:
  int consti_;

  explicit ConstExp(int consti) : consti_(consti) {}
  ~ConstExp() override;

  void Print(FILE *out, int d) const override;
  canon::StmAndExp Canon() override;
  temp::Temp *Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class CallExp : public Exp {
public:
  Exp *fun_;
  ExpList *args_;

  CallExp(Exp *fun, ExpList *args) : fun_(fun), args_(args) {}
  ~CallExp() override;

  void Print(FILE *out, int d) const override;
  canon::StmAndExp Canon() override;
  temp::Temp *Munch(assem::InstrList &instr_list, std::string_view fs) override;
};

class ExpList {
public:
  ExpList() = default;
  ExpList(std::initializer_list<Exp *> list) : exp_list_(list) {}

  void Append(Exp *exp) { exp_list_.push_back(exp); }
  void Insert(Exp *exp) { exp_list_.push_front(exp); }
  std::list<Exp *> &GetNonConstList() { return exp_list_; }
  const std::list<Exp *> &GetList() { return exp_list_; }
  temp::TempList *MunchArgs(assem::InstrList &instr_list, std::string_view fs);

private:
  std::list<Exp *> exp_list_;
};

class StmList {
  friend class canon::Canon;

public:
  StmList() = default;

  const std::list<Stm *> &GetList() { return stm_list_; }
  void Linear(Stm *stm);
  void Print(FILE *out) const;

private:
  std::list<Stm *> stm_list_;
};

RelOp NotRel(RelOp);  // a op b == not(a NotRel(op) b)
RelOp Commute(RelOp); // a op b == b Commute(op) a

} // namespace tree

#endif // TIGER_TRANSLATE_TREE_H_
