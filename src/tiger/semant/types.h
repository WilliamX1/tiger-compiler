#ifndef TIGER_SEMANT_TYPES_H_
#define TIGER_SEMANT_TYPES_H_

#include "tiger/symbol/symbol.h"
#include <list>

namespace type {

class TyList;
class Field;
class FieldList;

class Ty {
public:
  enum Kind {NIL, INT, STRING, VOID, RECORD, ARRAY, NAME };

  Kind kind_;

  virtual Ty *ActualTy();
  virtual bool IsSameType(Ty *);

  virtual ~Ty() = default;

protected:
  Ty(Kind kind) : kind_(kind) {};
};

class NilTy : public Ty {
public:
  static NilTy *Instance() { nilty_.kind_ = NIL; return &nilty_; }

private:
  static NilTy nilty_;

  NilTy() : Ty(NIL) {};
};

class IntTy : public Ty {
public:
  static IntTy *Instance() { intty_.kind_ = INT; return &intty_; }

private:
  static IntTy intty_;

  IntTy() : Ty(INT) {};
};

class StringTy : public Ty {
public:
  static StringTy *Instance() { stringty_.kind_ = STRING; return &stringty_; }

private:
  static StringTy stringty_;

  StringTy() : Ty(STRING) {};
};

class VoidTy : public Ty {
public:
  static VoidTy *Instance() { voidty_.kind_ = VOID; return &voidty_; }

private:
  static VoidTy voidty_;

  VoidTy() : Ty(VOID) {};
};

class RecordTy : public Ty {
public:
  FieldList *fields_;

  explicit RecordTy(FieldList *fields) : fields_(fields) , Ty(RECORD) {};
};

class ArrayTy : public Ty {
public:
  Ty *ty_;

  explicit ArrayTy(Ty *ty) : ty_(ty) , Ty(ARRAY) {};
};

class NameTy : public Ty {
public:
  sym::Symbol *sym_;
  Ty *ty_;

  NameTy(sym::Symbol *sym, Ty *ty) : sym_(sym), ty_(ty) , Ty(NAME) {};

  Ty *ActualTy() override;
};

class TyList {
public:
  TyList() = default;
  explicit TyList(Ty *ty) : ty_list_({ty}) {}
  TyList(std::initializer_list<Ty *> list) : ty_list_(list) {}

  const std::list<Ty *> &GetList() { return ty_list_; }
  void Append(Ty *ty) { ty_list_.push_back(ty); }

private:
  std::list<Ty *> ty_list_;
};

class Field {
public:
  sym::Symbol *name_;
  Ty *ty_;

  Field(sym::Symbol *name, Ty *ty) : name_(name), ty_(ty) {}
};

class FieldList {
public:
  FieldList() = default;
  explicit FieldList(Field *field) : field_list_({field}) {}
  FieldList(std::initializer_list<Field *> list) : field_list_(list) {}
  
  std::list<Field *> &GetList() { return field_list_; }
  void Append(Field *field) { field_list_.push_back(field); }

private:
  std::list<Field *> field_list_;
};

} // namespace type

#endif // TIGER_SEMANT_TYPES_H_
