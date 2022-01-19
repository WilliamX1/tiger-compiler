#include "tiger/absyn/absyn.h"
#include "tiger/errormsg/errormsg.h"

namespace {

inline void Indent(FILE *out, int d) {
  for (int i = 0; i <= d; i++)
    fprintf(out, " ");
}

inline void PrintOper(FILE *out, absyn::Oper d) {
  static std::array<std::string_view, absyn::ABSYN_OPER_COUNT> str_oper = {
      "PLUS",     "MINUS",    "TIMES",  "DIVIDE", "EQUAL",
      "NOTEQUAL", "LESSTHAN", "LESSEQ", "GREAT",  "GREATEQ"};
  fprintf(out, "%s", str_oper[d].data());
}

} // namespace

namespace absyn {

AbsynTree::AbsynTree(absyn::Exp *root) : root_(root) {
  if (root == nullptr)
    throw std::invalid_argument("NULL pointer is not allowed in AbsynTree");
}

AbsynTree::~AbsynTree() { delete root_; }

void AbsynTree::Print(FILE *out) const { root_->Print(out, 0); }

SimpleVar::~SimpleVar() { delete sym_; }

FieldVar::~FieldVar() {
  delete var_;
  delete sym_;
}

SubscriptVar::~SubscriptVar() {
  delete var_;
  delete subscript_;
}

VarExp::~VarExp() { delete var_; }

NilExp::~NilExp() = default;

IntExp::~IntExp() = default;

StringExp::~StringExp() = default;

CallExp::~CallExp() {
  delete func_;
  delete args_;
}

OpExp::~OpExp() {
  delete left_;
  delete right_;
}

RecordExp::~RecordExp() {
  delete typ_;
  delete fields_;
}

SeqExp::~SeqExp() { delete seq_; }

AssignExp::~AssignExp() {
  delete var_;
  delete exp_;
}

IfExp::~IfExp() {
  delete test_;
  delete then_;
  delete elsee_;
}

WhileExp::~WhileExp() {
  delete test_;
  delete body_;
}

ForExp::~ForExp() = default;

BreakExp::~BreakExp() = default;

LetExp::~LetExp() {
  delete decs_;
  delete body_;
}

ArrayExp::~ArrayExp() {
  delete typ_;
  delete size_;
  delete init_;
}

VoidExp::~VoidExp() = default;

EField::~EField() {
  delete name_;
  delete exp_;
}

FunctionDec::~FunctionDec() { delete functions_; }

VarDec::~VarDec() {
  delete var_;
  delete typ_;
  delete init_;
}

TypeDec::~TypeDec() { delete types_; }

NameTy::~NameTy() { delete name_; }

RecordTy::~RecordTy() { delete record_; }

ArrayTy::~ArrayTy() { delete array_; }

void SimpleVar::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "simpleVar(%s)", sym_->Name().data());
}

void FieldVar::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "%s\n", "fieldVar(");
  var_->Print(out, d + 1);
  fprintf(out, "%s\n", ",");
  Indent(out, d + 1);
  fprintf(out, "%s)", sym_->Name().data());
}

void SubscriptVar::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "%s\n", "subscriptVar(");
  var_->Print(out, d + 1);
  fprintf(out, "%s\n", ",");
  subscript_->Print(out, d + 1);
  fprintf(out, "%s", ")");
}

void VarExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "varExp(\n");
  var_->Print(out, d + 1);
  fprintf(out, "%s", ")");
}

void NilExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "nilExp()");
}

void IntExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "intExp(%d)", val_);
}

void StringExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "stringExp(%s)", str_.data());
}

void CallExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "callExp(%s,\n", func_->Name().data());
  assert(args_);
  if (args_) {
    args_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void OpExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "opExp(\n");
  Indent(out, d + 1);
  PrintOper(out, oper_);
  fprintf(out, ",\n");
  left_->Print(out, d + 1);
  fprintf(out, ",\n");
  right_->Print(out, d + 1);
  fprintf(out, ")");
}

void RecordExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "recordExp(%s,\n", typ_->Name().data());
  if (fields_) {
    fields_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void SeqExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "seqExp(\n");
  seq_->Print(out, d + 1);
  fprintf(out, ")");
}

void AssignExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "assignExp(\n");
  var_->Print(out, d + 1);
  fprintf(out, ",\n");
  exp_->Print(out, d + 1);
  fprintf(out, ")");
}

void IfExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "iffExp(\n");
  test_->Print(out, d + 1);
  fprintf(out, ",\n");
  then_->Print(out, d + 1);
  if (elsee_) { /* else is optional */
    fprintf(out, ",\n");
    elsee_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void WhileExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "whileExp(\n");
  if (test_) {
    test_->Print(out, d + 1);
  }
  fprintf(out, ",\n");
  if (body_) {
    body_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void ForExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "forExp(%s,\n", var_->Name().data());
  if (lo_) {
    lo_->Print(out, d + 1);
  }
  fprintf(out, ",\n");
  if (hi_) {
    hi_->Print(out, d + 1);
  }
  fprintf(out, ",\n");
  if (body_) {
    body_->Print(out, d + 1);
  }
  fprintf(out, ",\n");
  Indent(out, d + 1);
  fprintf(out, "%s", escape_ ? "TRUE)" : "FALSE)");
}

void BreakExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "breakExp()");
}

void LetExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "letExp(\n");
  if (decs_) {
    decs_->Print(out, d + 1);
  }
  fprintf(out, ",\n");
  if (body_) {
    body_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void ArrayExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "arrayExp(%s,\n", typ_->Name().data());
  if (size_) {
    size_->Print(out, d + 1);
  }
  fprintf(out, ",\n");
  if (init_) {
    init_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void VoidExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "voidExp()");
}

void FunctionDec::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "functionDec(\n");
  if (functions_) {
    functions_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void VarDec::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "varDec(%s,\n", var_->Name().data());
  if (typ_) {
    Indent(out, d + 1);
    fprintf(out, "%s,\n", typ_->Name().data());
  }
  if (init_) {
    init_->Print(out, d + 1);
  }
  fprintf(out, ",\n");
  Indent(out, d + 1);
  fprintf(out, "%s", escape_ ? "TRUE)" : "FALSE)");
}

void TypeDec::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "typeDec(\n");
  if (types_) {
    types_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void NameTy::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "nameTy(%s)", name_->Name().data());
}

void RecordTy::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "recordTy(\n");
  if (record_) {
    record_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void ArrayTy::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "arrayTy(%s)", array_->Name().data());
}

void Field::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "field(%s,\n", name_->Name().data());
  Indent(out, d + 1);
  fprintf(out, "%s,\n", typ_->Name().data());
  Indent(out, d + 1);
  fprintf(out, "%s", escape_ ? "TRUE)" : "FALSE)");
}

void FieldList::Print(FILE *out, int d) const {
  Indent(out, d);
  if (!field_list_.empty()) {
    fprintf(out, "fieldList(");
    for (auto field : field_list_) {
      fprintf(out, "\n");
      field->Print(out, ++d);
      fprintf(out, ",\n");
      Indent(out, d);
      fprintf(out, "fieldList(");
    }
    for (int i = 0; i <= field_list_.size(); i++)
      fprintf(out, ")");
  } else
    fprintf(out, "fieldList()");
}


void ExpList::Print(FILE *out, int d) const {
  Indent(out, d);
  if (!exp_list_.empty()) {
    fprintf(out, "expList(");
    for (auto exp : exp_list_) {
      fprintf(out, "\n");
      exp->Print(out, ++d);
      fprintf(out, ",\n");
      Indent(out, d);
      fprintf(out, "expList(");
    }
    for (int i = 0; i <= exp_list_.size(); i++)
      fprintf(out, ")");
  } else
    fprintf(out, "expList()");
}

void FunDec::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "fundec(%s,\n", name_->Name().data());
  if (params_) {
    params_->Print(out, d + 1);
  }
  fprintf(out, ",\n");
  if (result_) {
    Indent(out, d + 1);
    fprintf(out, "%s,\n", result_->Name().data());
  }
  if (body_) {
    body_->Print(out, d + 1);
  }
  fprintf(out, ")");
}

void FunDecList::Print(FILE *out, int d) const {
  Indent(out, d);
  if (!fun_dec_list_.empty()) {
    fprintf(out, "fundecList(");
    for (auto fun_dec : fun_dec_list_) {
      fprintf(out, "\n");
      fun_dec->Print(out, ++d);
      fprintf(out, ",\n");
      Indent(out, d);
      fprintf(out, "fundecList(");
    }
    for (int i = 0; i <= fun_dec_list_.size(); i++)
      fprintf(out, ")");
  } else
    fprintf(out, "fundecList()");
}

void DecList::Print(FILE *out, int d) const {
  Indent(out, d);
  if (!dec_list_.empty()) {
    fprintf(out, "decList(");
    for (auto dec : dec_list_) {
      fprintf(out, "\n");
      dec->Print(out, ++d);
      fprintf(out, ",\n");
      Indent(out, d);
      fprintf(out, "decList(");
    }
    for (int i = 0; i <= dec_list_.size(); i++)
      fprintf(out, ")");
  } else
    fprintf(out, "decList()");
}

void NameAndTy::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "nameAndTy(%s,\n", name_->Name().data());
  ty_->Print(out, d + 1);
  fprintf(out, ")");
}

void NameAndTyList::Print(FILE *out, int d) const {
  Indent(out, d);
  if (!name_and_ty_list_.empty()) {
    fprintf(out, "nameAndTyList(");
    for (auto name_and_ty : name_and_ty_list_) {
      fprintf(out, "\n");
      name_and_ty->Print(out, ++d);
      fprintf(out, ",\n");
      Indent(out, d);
      fprintf(out, "nameAndTyList(");
    }
    for (int i = 0; i <= name_and_ty_list_.size(); i++)
      fprintf(out, ")");
  } else
    fprintf(out, "nameAndTyList()");
}

void EField::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "efield(%s,\n", name_->Name().data());
  exp_->Print(out, d + 1);
  fprintf(out, ")");
}

void EFieldList::Print(FILE *out, int d) const {
  Indent(out, d);
  if (!efield_list_.empty()) {
    fprintf(out, "efieldList(");
    for (auto efield : efield_list_) {
      fprintf(out, "\n");
      d++;
      if (efield) {
        efield->Print(out, d);
      } else {
        fprintf(out, "efield()");
      }
      fprintf(out, ",\n");
      Indent(out, d);
      fprintf(out, "efieldList(");
    }
    for (int i = 0; i <= efield_list_.size(); i++)
      fprintf(out, ")");
  } else
    fprintf(out, "efieldList()");
}

} // namespace absyn
