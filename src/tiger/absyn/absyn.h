#ifndef TIGER_ABSYN_ABSYN_H_
#define TIGER_ABSYN_ABSYN_H_

#include <cstdio>
#include <list>
#include <string>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/escape/escape.h"
#include "tiger/frame/frame.h"
#include "tiger/semant/types.h"
#include "tiger/symbol/symbol.h"

/**
 * Forward Declarations
 */
namespace tr {
class Exp;
class ExpAndTy;
} // namespace tr

namespace absyn {

class Var;
class Exp;
class Dec;
class Ty;

class ExpList;
class FieldList;
class FunDecList;
class NameAndTyList;
class DecList;
class EFieldList;

enum Oper {
  PLUS_OP,
  MINUS_OP,
  TIMES_OP,
  DIVIDE_OP,
  EQ_OP,
  NEQ_OP,
  LT_OP,
  LE_OP,
  GT_OP,
  GE_OP,
  ABSYN_OPER_COUNT,
};

/**
 * Abstract syntax tree root
 */
class AbsynTree {
public:
  AbsynTree() = delete;
  AbsynTree(nullptr_t) = delete;
  explicit AbsynTree(absyn::Exp *root);
  AbsynTree(const AbsynTree &absyn_tree) = delete;
  AbsynTree(AbsynTree &&absyn_tree) = delete;
  AbsynTree &operator=(const AbsynTree &absyn_tree) = delete;
  AbsynTree &operator=(AbsynTree &&absyn_tree) = delete;
  ~AbsynTree();

  void Print(FILE *out) const;
  void SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                  err::ErrorMsg *errormsg) const;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const;
  void Traverse(esc::EscEnvPtr env);

private:
  absyn::Exp *root_;
};

/**
 * Variables
 */

class Var {
public:
  int pos_;
  virtual ~Var() = default;
  virtual void Print(FILE *out, int d) const = 0;
  virtual type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount,
                               err::ErrorMsg *errormsg) const = 0;
  virtual tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const = 0;
  virtual void Traverse(esc::EscEnvPtr env, int depth) = 0;
  
  enum Kind {SIMPLE, FIELD, SUBSCRIPT};
  Kind kind_;

protected:
  explicit Var(Kind kind, int pos) : kind_(kind), pos_(pos) {}
};

class SimpleVar : public Var {
public:
  sym::Symbol *sym_;
  SimpleVar(int pos, sym::Symbol *sym) : Var(SIMPLE, pos), sym_(sym) {}
  ~SimpleVar() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class FieldVar : public Var {
public:
  Var *var_;
  sym::Symbol *sym_;

  FieldVar(int pos, Var *var, sym::Symbol *sym)
      : Var(FIELD, pos), var_(var), sym_(sym) {}
  ~FieldVar() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class SubscriptVar : public Var {
public:
  Var *var_;
  Exp *subscript_;

  SubscriptVar(int pos, Var *var, Exp *exp)
      : Var(SUBSCRIPT, pos), var_(var), subscript_(exp) {}
  ~SubscriptVar() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

/**
 * Expressions
 */

class Exp {
public:
  int pos_;
  virtual ~Exp() = default;
  virtual void Print(FILE *out, int d) const = 0;
  virtual type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount,
                               err::ErrorMsg *errormsg) const = 0;
  virtual tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const = 0;
  virtual void Traverse(esc::EscEnvPtr env, int depth) = 0;

  enum Kind {VAR, NIL, INT, STRING, CALL, OP, RECORD, SEQ, ASSIGN, IF, WHILE, FOR, BREAK, LET, ARRAY, VOID};
  Kind kind_;

protected:
  explicit Exp(Kind kind, int pos) : kind_(kind), pos_(pos) {}
};

class VarExp : public Exp {
public:
  Var *var_;

  VarExp(int pos, Var *var) : Exp(VAR, pos), var_(var) {}
  ~VarExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class NilExp : public Exp {
public:
  explicit NilExp(int pos) : Exp(NIL, pos) {}
  ~NilExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class IntExp : public Exp {
public:
  int val_;

  IntExp(int pos, int val) : Exp(INT, pos), val_(val) {}
  ~IntExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class StringExp : public Exp {
public:
  std::string str_;

  StringExp(int pos, std::string *str) : Exp(STRING, pos), str_(*str) {}
  ~StringExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class CallExp : public Exp {
public:
  sym::Symbol *func_;
  ExpList *args_;

  CallExp(int pos, sym::Symbol *func, ExpList *args)
      : Exp(CALL, pos), func_(func), args_(args) {
    assert(args);
  }
  ~CallExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class OpExp : public Exp {
public:
  Oper oper_;
  Exp *left_, *right_;

  OpExp(int pos, Oper oper, Exp *left, Exp *right)
      : Exp(OP, pos), oper_(oper), left_(left), right_(right) {}
  ~OpExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class RecordExp : public Exp {
public:
  sym::Symbol *typ_;
  EFieldList *fields_;

  RecordExp(int pos, sym::Symbol *typ, EFieldList *fields)
      : Exp(RECORD, pos), typ_(typ), fields_(fields) {}
  ~RecordExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class SeqExp : public Exp {
public:
  ExpList *seq_;

  SeqExp(int pos, ExpList *seq) : Exp(SEQ, pos), seq_(seq) {}
  ~SeqExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class AssignExp : public Exp {
public:
  Var *var_;
  Exp *exp_;

  AssignExp(int pos, Var *var, Exp *exp) : Exp(ASSIGN, pos), var_(var), exp_(exp) {}
  ~AssignExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class IfExp : public Exp {
public:
  Exp *test_, *then_, *elsee_;

  IfExp(int pos, Exp *test, Exp *then, Exp *elsee)
      : Exp(IF, pos), test_(test), then_(then), elsee_(elsee) {}
  ~IfExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class WhileExp : public Exp {
public:
  Exp *test_, *body_;

  WhileExp(int pos, Exp *test, Exp *body)
      : Exp(WHILE, pos), test_(test), body_(body) {}
  ~WhileExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class ForExp : public Exp {
public:
  sym::Symbol *var_;
  Exp *lo_, *hi_, *body_;
  bool escape_;

  ForExp(int pos, sym::Symbol *var, Exp *lo, Exp *hi, Exp *body)
      : Exp(FOR, pos), var_(var), lo_(lo), hi_(hi), body_(body), escape_(true) {}
  ~ForExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class BreakExp : public Exp {
public:
  // bool in_loop_ = false;
  explicit BreakExp(int pos) : Exp(BREAK, pos) {}
  ~BreakExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class LetExp : public Exp {
public:
  DecList *decs_;
  Exp *body_;

  LetExp(int pos, DecList *decs, Exp *body)
      : Exp(LET, pos), decs_(decs), body_(body) {}
  ~LetExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class ArrayExp : public Exp {
public:
  sym::Symbol *typ_;
  Exp *size_, *init_;

  ArrayExp(int pos, sym::Symbol *typ, Exp *size, Exp *init)
      : Exp(ARRAY, pos), typ_(typ), size_(size), init_(init) {}
  ~ArrayExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class VoidExp : public Exp {
public:
  explicit VoidExp(int pos) : Exp(VOID, pos) {}
  ~VoidExp() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                       err::ErrorMsg *errormsg) const override;
  tr::ExpAndTy *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                          tr::Level *level, temp::Label *label,
                          err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

/**
 * Declarations
 */

class Dec {
public:
  int pos_;
  virtual ~Dec() = default;
  virtual void Print(FILE *out, int d) const = 0;
  virtual void SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                          err::ErrorMsg *errormsg) const = 0;
  virtual tr::Exp *Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                             tr::Level *level, temp::Label *label,
                             err::ErrorMsg *errormsg) const = 0;
  virtual void Traverse(esc::EscEnvPtr env, int depth) = 0;

  enum Kind {FUNCTION = 0, VAR, TYPE};
  Kind kind_;

protected:
  explicit Dec(Kind kind, int pos) : kind_(kind), pos_(pos) {}
};

class FunctionDec : public Dec {
public:
  FunDecList *functions_;

  FunctionDec(int pos, FunDecList *functions)
      : Dec(FUNCTION, pos), functions_(functions) {}
  ~FunctionDec() override;

  void Print(FILE *out, int d) const override;
  void SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                  err::ErrorMsg *errormsg) const override;
  tr::Exp *Translate(env::VEnvPtr venv, env::TEnvPtr tenv, tr::Level *level,
                     temp::Label *label, 
                     err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class VarDec : public Dec {
public:
  sym::Symbol *var_;
  sym::Symbol *typ_;
  Exp *init_;
  bool escape_;

  VarDec(int pos, sym::Symbol *var, sym::Symbol *typ, Exp *init)
      : Dec(VAR, pos), var_(var), typ_(typ), init_(init), escape_(true) {}
  ~VarDec() override;

  void Print(FILE *out, int d) const override;
  void SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                  err::ErrorMsg *errormsg) const override;
  tr::Exp *Translate(env::VEnvPtr venv, env::TEnvPtr tenv, tr::Level *level,
                     temp::Label *label, 
                     err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

class TypeDec : public Dec {
public:
  NameAndTyList *types_;

  TypeDec(int pos, NameAndTyList *types) : Dec(TYPE, pos), types_(types) {}
  ~TypeDec() override;

  void Print(FILE *out, int d) const override;
  void SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                  err::ErrorMsg *errormsg) const override;
  tr::Exp *Translate(env::VEnvPtr venv, env::TEnvPtr tenv, tr::Level *level,
                     temp::Label *label, 
                     err::ErrorMsg *errormsg) const override;
  void Traverse(esc::EscEnvPtr env, int depth) override;
};

/**
 * Types
 */

class Ty {
public:
  int pos_;
  virtual ~Ty() = default;
  virtual void Print(FILE *out, int d) const = 0;
  virtual type::Ty *SemAnalyze(env::TEnvPtr tenv,
                               err::ErrorMsg *errormsg) const = 0;
  virtual type::Ty *Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const = 0;

  enum Kind {NAME, RECORD, ARRAY};
  Kind kind_;

protected:
  explicit Ty(Kind kind, int pos) : kind_(kind), pos_(pos) {}
};

class NameTy : public Ty {
public:
  sym::Symbol *name_;

  NameTy(int pos, sym::Symbol *name) : Ty(NAME, pos), name_(name) {}
  ~NameTy() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::TEnvPtr tenv,
                       err::ErrorMsg *errormsg) const override;
  type::Ty *Translate(env::TEnvPtr tenv,
                      err::ErrorMsg *errormsg) const override;
};

class RecordTy : public Ty {
public:
  FieldList *record_;

  RecordTy(int pos, FieldList *record) : Ty(RECORD, pos), record_(record) {}
  ~RecordTy() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::TEnvPtr tenv,
                       err::ErrorMsg *errormsg) const override;
  type::Ty *Translate(env::TEnvPtr tenv,
                      err::ErrorMsg *errormsg) const override;
};

class ArrayTy : public Ty {
public:
  sym::Symbol *array_;

  ArrayTy(int pos, sym::Symbol *array) : Ty(ARRAY, pos), array_(array) {}
  ~ArrayTy() override;

  void Print(FILE *out, int d) const override;
  type::Ty *SemAnalyze(env::TEnvPtr tenv,
                       err::ErrorMsg *errormsg) const override;
  type::Ty *Translate(env::TEnvPtr tenv,
                      err::ErrorMsg *errormsg) const override;
};

/**
 * Linked lists and nodes of lists
 */

class Field {
public:
  int pos_;
  sym::Symbol *name_, *typ_;
  bool escape_;

  Field(int pos, sym::Symbol *name, sym::Symbol *typ)
      : pos_(pos), name_(name), typ_(typ), escape_(true) {}

  void Print(FILE *out, int d) const;
};

class FieldList {
public:
  FieldList() = default;
  explicit FieldList(Field *field) : field_list_({field}) { assert(field); }

  FieldList *Prepend(Field *field) {
    field_list_.push_front(field);
    return this;
  }
  [[nodiscard]] const std::list<Field *> &GetList() const {
    return field_list_;
  }
  void Print(FILE *out, int d) const;
  type::TyList *MakeFormalTyList(env::TEnvPtr tenv,
                                 err::ErrorMsg *errormsg) const;
  type::FieldList *MakeFieldList(env::TEnvPtr tenv,
                                 err::ErrorMsg *errormsg) const;

private:
  std::list<Field *> field_list_;
};

class ExpList {
public:
  ExpList() = default;
  explicit ExpList(Exp *exp) : exp_list_({exp}) { assert(exp); }

  ExpList *Prepend(Exp *exp) {
    exp_list_.push_front(exp);
    return this;
  }
  [[nodiscard]] const std::list<Exp *> &GetList() const { return exp_list_; }
  void Print(FILE *out, int d) const;

private:
  std::list<Exp *> exp_list_;
};

class FunDec {
public:
  int pos_;
  sym::Symbol *name_;
  FieldList *params_;
  sym::Symbol *result_;
  Exp *body_;

  FunDec(int pos, sym::Symbol *name, FieldList *params, sym::Symbol *result,
         Exp *body)
      : pos_(pos), name_(name), params_(params), result_(result), body_(body) {
    assert(params);
  }

  void Print(FILE *out, int d) const;
};

class FunDecList {
public:
  explicit FunDecList(FunDec *fun_dec) : fun_dec_list_({fun_dec}) {
    assert(fun_dec);
  }

  FunDecList *Prepend(FunDec *fun_dec) {
    fun_dec_list_.push_front(fun_dec);
    return this;
  }
  [[nodiscard]] const std::list<FunDec *> &GetList() const {
    return fun_dec_list_;
  }
  void Print(FILE *out, int d) const;

private:
  std::list<FunDec *> fun_dec_list_;
};

class DecList {
public:
  DecList() = default;
  explicit DecList(Dec *dec) : dec_list_({dec}) { assert(dec); }

  DecList *Prepend(Dec *dec) {
    dec_list_.push_front(dec);
    return this;
  }
  [[nodiscard]] const std::list<Dec *> &GetList() const { return dec_list_; }
  void Print(FILE *out, int d) const;

private:
  std::list<Dec *> dec_list_;
};

class NameAndTy {
public:
  sym::Symbol *name_;
  Ty *ty_;

  NameAndTy(sym::Symbol *name, Ty *ty) : name_(name), ty_(ty) {}

  void Print(FILE *out, int d) const;
};

class NameAndTyList {
public:
  explicit NameAndTyList(NameAndTy *name_and_ty)
      : name_and_ty_list_({name_and_ty}) {}

  NameAndTyList *Prepend(NameAndTy *name_and_ty) {
    name_and_ty_list_.push_front(name_and_ty);
    return this;
  }
  [[nodiscard]] const std::list<NameAndTy *> &GetList() const {
    return name_and_ty_list_;
  }
  void Print(FILE *out, int d) const;

private:
  std::list<NameAndTy *> name_and_ty_list_;
};

class EField {
public:
  sym::Symbol *name_;
  Exp *exp_;

  EField(sym::Symbol *name, Exp *exp) : name_(name), exp_(exp) {}
  EField(const EField &efield) = delete;
  EField(EField &&efield) = delete;
  EField &operator=(const EField &efield) = delete;
  EField &operator=(EField &&efield) = delete;
  ~EField();

  void Print(FILE *out, int d) const;
};

class EFieldList {
public:
  EFieldList() = default;
  explicit EFieldList(EField *efield) : efield_list_({efield}) {}

  EFieldList *Prepend(EField *efield) {
    efield_list_.push_front(efield);
    return this;
  }
  [[nodiscard]] const std::list<EField *> &GetList() const {
    return efield_list_;
  }
  void Print(FILE *out, int d) const;

private:
  std::list<EField *> efield_list_;
};

}; // namespace absyn

#endif // TIGER_ABSYN_ABSYN_H_
