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
class Targets {
public:
  Targets(temp::LabelList* labels_) : labels_(labels_) {}; 

private:
  temp::LabelList* labels_;
};

class Instr {
public:
  enum Kind { OPER, LABEL, MOVE };

  Kind kind_;

  Instr(Kind kind_) : kind_(kind_) {};

  virtual void Print(FILE* out, temp::Map* m) const = 0;

  virtual ~Instr() {};
};

class OperInstr : public Instr {
public:
  std::string assem_;
  temp::TempList* dst_, *src_;
  Targets* jumps_;

  OperInstr(std::string assem_, temp::TempList* dst_, temp::TempList* src_, Targets* jumps_)
  : Instr(OPER), assem_(assem_), dst_(dst_), src_(src_), jumps_(jumps_) {};

  void Print(FILE* out, temp::Map* m) const override;
};

class LabelList : public Instr {
public:
  std::string assem_;
  temp::Label* label_;

  LabelList(std::string assem_, temp::Label* label_)
  : Instr(LABEL), assem_(assem_), label_(label_) {};

  void Print(FILE* out, temp::Map* m) const override;
};

class MoveInstr : public Instr {
public:
  std::string assem_;
  temp::TempList* dst_, *src_;

  MoveInstr(std::string assem_, temp::TempList* dst_, temp::TempList* src_)
  : Instr(MOVE), assem_(assem_), dst_(dst_), src_(src_) {};

  void Print(FILE* out, temp::Map* m) const override;
};

class InstrList {
public:
  explicit InstrList(Instr* i) : instr_list_({i}) {};
  InstrList(std::initializer_list<Instr*> list_) : instr_list_(list_) {};
  InstrList() = default;
  ~InstrList() {};
  void Append(Instr* i) { instr_list_.push_back(i); };
  [[nodiscard]] const std::list<Instr*> &GetList() const { return instr_list_; };

private:
  std::list<Instr*> instr_list_;
};

class Proc {
public:
  std::string prolog_;
  InstrList* body_;
  std::string epilog_;

  Proc(std::string prolog_, InstrList* body, std::string epilog_)
  : prolog_(prolog_), body_(body_), epilog_(epilog_) {};
};

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
};

class SeqStm : public Stm {
public:
  Stm *left_, *right_;

  SeqStm(Stm *left, Stm *right) : left_(left), right_(right) { assert(left); }
  ~SeqStm() override;

  void Print(FILE *out, int d) const override;
};

class LabelStm : public Stm {
public:
  temp::Label *label_;

  explicit LabelStm(temp::Label *label) : label_(label) {}
  ~LabelStm() override;

  void Print(FILE *out, int d) const override;
};

class JumpStm : public Stm {
public:
  NameExp *exp_;
  temp::LabelList *jumps_;

  JumpStm(NameExp *exp, temp::LabelList *jumps)
      : exp_(exp), jumps_(jumps) {}
  ~JumpStm() override;

  void Print(FILE *out, int d) const override;
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
};

class MoveStm : public Stm {
public:
  Exp *dst_, *src_;

  MoveStm(Exp *dst, Exp *src) : dst_(dst), src_(src) {}
  ~MoveStm() override;

  void Print(FILE *out, int d) const override;
};

class ExpStm : public Stm {
public:
  Exp *exp_;

  explicit ExpStm(Exp *exp) : exp_(exp) {}
  ~ExpStm() override;

  void Print(FILE *out, int d) const override;
};

/**
 *Expressions
 */

class Exp {
public:
  virtual ~Exp() = default;

  virtual void Print(FILE *out, int d) const = 0;
};

class BinopExp : public Exp {
public:
  BinOp op_;
  Exp *left_, *right_;

  BinopExp(BinOp op, Exp *left, Exp *right)
      : op_(op), left_(left), right_(right) {}
  ~BinopExp() override;

  void Print(FILE *out, int d) const override;
};

class MemExp : public Exp {
public:
  Exp *exp_;

  explicit MemExp(Exp *exp) : exp_(exp) {}
  ~MemExp() override;

  void Print(FILE *out, int d) const override;
};

class TempExp : public Exp {
public:
  temp::Temp *temp_;

  explicit TempExp(temp::Temp *temp) : temp_(temp) {}
  ~TempExp() override;

  void Print(FILE *out, int d) const override;
};

class EseqExp : public Exp {
public:
  Stm *stm_;
  Exp *exp_;

  EseqExp(Stm *stm, Exp *exp) : stm_(stm), exp_(exp) {}
  ~EseqExp() override;

  void Print(FILE *out, int d) const override;
};

class NameExp : public Exp {
public:
  temp::Label *name_;

  explicit NameExp(temp::Label *name) : name_(name) {}
  ~NameExp() override;

  void Print(FILE *out, int d) const override;
};

class ConstExp : public Exp {
public:
  int consti_;

  explicit ConstExp(int consti) : consti_(consti) {}
  ~ConstExp() override;

  void Print(FILE *out, int d) const override;
};

class CallExp : public Exp {
public:
  Exp *fun_;
  ExpList *args_;

  CallExp(Exp *fun, ExpList *args) : fun_(fun), args_(args) {}
  ~CallExp() override;

  void Print(FILE *out, int d) const override;
};

class ExpList {
public:
  ExpList() = default;
  ExpList(Exp* exp) : exp_list_({exp}) {} 
  ExpList(std::initializer_list<Exp *> list) : exp_list_(list) {}

  void Append(Exp *exp) { exp_list_.push_back(exp); }
  void Insert(Exp *exp) { exp_list_.push_front(exp); }
  std::list<Exp *> &GetNonConstList() { return exp_list_; }
  const std::list<Exp *> &GetList() { return exp_list_; }

private:
  std::list<Exp *> exp_list_;
};

MemExp* NewMemPlus_Const(Exp* left, int right);
} // namespace tree

#endif // TIGER_TRANSLATE_TREE_H_
