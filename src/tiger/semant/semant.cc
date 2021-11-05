#include "tiger/absyn/absyn.h"
#include "tiger/semant/semant.h"

namespace {
  static type::TyList* make_formal_tylist(sym::Table<type::Ty>* tenv, absyn::FieldList* params, err::ErrorMsg *errormsg) {
    if (params == nullptr) return nullptr;

    type::TyList* ans = new type::TyList();

    const std::list<absyn::Field *> * param_list = &params->GetList();
    for (auto iter = param_list->begin(); iter != param_list->end(); iter++) {
      type::Ty* ty = tenv->Look((*iter)->typ_);
      if (ty == nullptr)
        errormsg->Error((*iter)->pos_, "undefined type %s", (*iter)->typ_->Name().c_str());
      ans->Append(ty->ActualTy());
    };

    return ans;
  };

  static type::FieldList* make_fieldlist(sym::Table<type::Ty>* tenv, absyn::FieldList* fields, err::ErrorMsg* errormsg) {
    if (fields == nullptr) return nullptr;

    type::FieldList* ans = new type::FieldList();

    const std::list<absyn::Field *> * field_list = &fields->GetList();
    for (auto iter = field_list->begin(); iter != field_list->end(); iter++) {
      type::Ty* ty = tenv->Look((*iter)->typ_);
      
      if (ty == nullptr)
        errormsg->Error((*iter)->pos_, "undefined type %s", (*iter)->typ_->Name().c_str());
      
      ans->Append(new type::Field((*iter)->name_, ty));
    };
    return ans;
  };
};

namespace absyn {

void AbsynTree::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  this->root_->SemAnalyze(venv, tenv, 0, errormsg);
  return;
}

type::Ty *SimpleVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  env::EnvEntry *entry = venv->Look(sym_);
  if (entry && entry->kind_ == env::EnvEntry::VAR) {
    return ((env::VarEntry*) entry)->ty_->ActualTy();
  } else {
    errormsg->Error(pos_, "undefined variable %s", sym_->Name().c_str());
    return type::IntTy::Instance();
  };
}

type::Ty *FieldVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (ty->kind_ != type::Ty::Kind::RECORD) {
    errormsg->Error(pos_, "not a record type");
    return type::IntTy::Instance();
  };

  type::FieldList* fieldlist = ((type::RecordTy*)ty)->fields_;

  std::list<type::Field *>* contents = &fieldlist->GetList();
  for (auto iter = contents->begin(); iter != contents->end(); iter++)
    if ((*iter)->name_ == sym_) return (*iter)->ty_;

  errormsg->Error(pos_, "field %s doesn't exist", sym_->Name().c_str());
  return type::IntTy::Instance();
}

type::Ty *SubscriptVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   int labelcount,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *var_ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *exp_ty = subscript_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (var_ty->kind_ != type::Ty::Kind::ARRAY) {
    errormsg->Error(pos_, "array type required");
    return type::IntTy::Instance();
  };

  if (exp_ty->kind_ != type::Ty::Kind::INT) {
    errormsg->Error(pos_, "array index must be integer");
    return type::IntTy::Instance();
  };

  return ((type::ArrayTy*) var_ty)->ty_->ActualTy();
}

type::Ty *VarExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  switch (var_->kind_)
  {
  case Var::Kind::SIMPLE:
    return ((SimpleVar*) var_)->SemAnalyze(venv, tenv, labelcount, errormsg);
  case Var::Kind::FIELD:
    return ((FieldVar*) var_)->SemAnalyze(venv, tenv, labelcount, errormsg);
  case Var::Kind::SUBSCRIPT:
    return ((SubscriptVar*) var_)->SemAnalyze(venv, tenv, labelcount, errormsg);
  default:
    assert(false);
    break;
  };
}

type::Ty *NilExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::NilTy::Instance();
}

type::Ty *IntExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::IntTy::Instance();
}

type::Ty *StringExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::StringTy::Instance();
}

type::Ty *CallExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  env::EnvEntry *entry = venv->Look(func_);
  if (!entry || entry->kind_ != env::EnvEntry::Kind::FUN) {
    errormsg->Error(pos_, "undefined function %s", func_->Name().c_str());
    return type::IntTy::Instance();
  };

  type::TyList *formals = ((env::FunEntry*) entry)->formals_;
  const std::list<type::Ty *>* formallist = &formals->GetList();
  const std::list<Exp *>* arglist = &args_->GetList();
  
  
  int formal_size = formallist->size();
  int arg_size = arglist->size();

  int size = std::min(arg_size, formal_size);
  auto formal_iter = formallist->begin();
  auto arg_iter = arglist->begin();
  while (size-- > 0) {
    type::Ty* ty = (*arg_iter)->SemAnalyze(venv, tenv, labelcount, errormsg);
    if (!ty->IsSameType(*formal_iter)) {
      errormsg->Error(pos_, "para type mismatch");
      return type::IntTy::Instance();
    };
    formal_iter++;
    arg_iter++;
  };

  if (arg_size < formal_size) {
    errormsg->Error(pos_, "too little params in function %s", func_->Name().c_str());
    return type::IntTy::Instance();
  };
  if (arg_size > formal_size) {
    errormsg->Error(pos_, "too many params in function %s", func_->Name().c_str());
    return type::IntTy::Instance();
  };

  return ((env::FunEntry*) entry)->result_->ActualTy();
}

type::Ty *OpExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *left_ty = left_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *right_ty = right_->SemAnalyze(venv, tenv, labelcount, errormsg);

  switch (oper_) {
    case Oper::PLUS_OP: case Oper::MINUS_OP: case Oper::TIMES_OP: case Oper::DIVIDE_OP:
    {
      if (left_ty->kind_ != type::Ty::Kind::INT) errormsg->Error(left_->pos_, "integer required");
      if (right_ty->kind_ != type::Ty::Kind::INT) errormsg->Error(right_->pos_, "integer required");
      break;
    }
    
    case Oper::LT_OP: case Oper::LE_OP: case Oper::GT_OP: case Oper::GE_OP:
    {
      if (left_ty->kind_ != type::Ty::Kind::INT && left_ty->kind_ != type::Ty::Kind::STRING) 
        errormsg->Error(left_->pos_, "integer or string param required");
      if (right_ty->kind_ != type::Ty::Kind::INT && right_ty->kind_ != type::Ty::Kind::STRING) 
        errormsg->Error(right_->pos_, "integer or string param required");
      if (!left_ty->IsSameType(right_ty))
        errormsg->Error(pos_, "same type required");
      break;
    }

    case Oper::EQ_OP: case Oper::NEQ_OP:
    {
      if (left_ty->kind_ != type::Ty::Kind::INT && left_ty->kind_ != type::Ty::Kind::STRING
      && left_ty->kind_ != type::Ty::Kind::RECORD && left_ty->kind_ != type::Ty::Kind::ARRAY)
        errormsg->Error(left_->pos_, "integer or string or record or array param required");
      if (right_ty->kind_ != type::Ty::Kind::INT && right_ty->kind_ != type::Ty::Kind::STRING
      && right_ty->kind_ != type::Ty::Kind::RECORD && right_ty->kind_ != type::Ty::Kind::ARRAY)
        errormsg->Error(right_->pos_, "integer or string or record or array param required");
      if (left_ty->kind_ == type::Ty::NIL && right_ty->kind_ == type::Ty::NIL)
        errormsg->Error(pos_, "at least one operand should not be NIL");
      if (!left_ty->IsSameType(right_ty)
      && !(left_ty->kind_ == type::Ty::Kind::RECORD && right_ty->kind_ == type::Ty::Kind::NIL))
        errormsg->Error(pos_, "same type required");
      break;
    }

    default:
      assert(0);
  };
  return type::IntTy::Instance();
}

type::Ty *RecordExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty* ty = tenv->Look(typ_);

  if (!ty) {
    errormsg->Error(pos_, "undefined type %s", typ_->Name().c_str());
    return type::IntTy::Instance();
  };

  ty = ty->ActualTy();
  if (!ty) {
    errormsg->Error(pos_, "undefined type %s", typ_->Name().c_str());
    return type::IntTy::Instance();
  };
  if (ty->kind_ != type::Ty::Kind::RECORD) {
    errormsg->Error(pos_, "not record type %s", typ_->Name().c_str());
    return type::IntTy::Instance();
  };

  const std::list<absyn::EField *>* fieldlist = &fields_->GetList();
  std::list<type::Field *>* recordlist = &((type::RecordTy*) ty)->fields_->GetList();

  int field_size = fieldlist->size();
  int record_size = recordlist->size();
  if (field_size != record_size) {
    errormsg->Error(pos_, "field amount mismatch");
    return type::IntTy::Instance();
  };

  int size = field_size;
  auto field_iter = fieldlist->begin();
  auto record_iter = recordlist->begin();
  while (size-- > 0) {
    type::Ty *field_ty = (*field_iter)->exp_->SemAnalyze(venv, tenv, labelcount, errormsg);
    if ((*field_iter)->name_ != (*record_iter)->name_) {
      errormsg->Error(pos_, "field not defined");
      return type::IntTy::Instance();
    };
    if (field_ty->kind_ != (*record_iter)->ty_->kind_) {
      errormsg->Error(pos_, "field type mismatch");
      return type::IntTy::Instance();
    }
    field_iter++;
    record_iter++;
  };
  return ty;
}

type::Ty *SeqExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  const std::list<Exp *> * seqlist = &seq_->GetList();
  if (!seqlist) return type::VoidTy::Instance();
  type::Ty* ty;
  
  for (auto iter = seqlist->begin(); iter != seqlist->end(); iter++)
  {
    ty = (*iter)->SemAnalyze(venv, tenv, labelcount, errormsg);
    if ((*iter)->kind_ == BREAK && !venv->inLoop()) errormsg->Error((*iter)->pos_, "break is not inside any loop");
  };
  return ty;
}

type::Ty *AssignExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  if (var_->kind_ == Var::Kind::SIMPLE) {
    env::EnvEntry* entry = venv->Look(((SimpleVar*) var_)->sym_);
    if (entry->readonly_) {
      errormsg->Error(pos_, "loop variable can't be assigned");
      return type::IntTy::Instance();
    };
  };

  type::Ty* var_ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty* exp_ty = exp_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (!var_ty->IsSameType(exp_ty))
    errormsg->Error(pos_, "unmatched assign exp");

  return var_ty;
}

type::Ty *IfExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  if (this->test_->SemAnalyze(venv, tenv, labelcount, errormsg)->kind_ != type::Ty::INT) {
    errormsg->Error(pos_, "integer required");
    return type::IntTy::Instance();
  };
  type::Ty* ty = then_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (elsee_ == nullptr) {
    if (ty->kind_ != type::Ty::VOID) {
      errormsg->Error(pos_, "if-then exp's body must produce no value");
      return type::IntTy::Instance();
    };
    return type::VoidTy::Instance();
  } else {
    if (ty->IsSameType(elsee_->SemAnalyze(venv, tenv, labelcount, errormsg))) return ty;
    else {
      errormsg->Error(pos_, "then exp and else exp type mismatch");
      return type::VoidTy::Instance();
    };
  };
}

type::Ty *WhileExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty* test_ty = test_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (test_ty->kind_ != type::Ty::Kind::INT) 
  errormsg->Error(test_->pos_, "integer required");

  /* tolerate 'break' sentence */
  venv->BeginLoop();
  tenv->BeginLoop();

  type::Ty* body_ty = body_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if (body_ty->kind_ != type::Ty::Kind::VOID)
    errormsg->Error(body_->pos_, "while body must produce no value");
  
  venv->EndLoop();
  tenv->EndLoop();

  return type::VoidTy::Instance();
}

type::Ty *ForExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty* lo_ty = lo_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty* hi_ty = hi_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (lo_ty->kind_ != type::Ty::Kind::INT)
    errormsg->Error(lo_->pos_, "for exp's range type is not integer");
  if (hi_ty->kind_ != type::Ty::Kind::INT)
    errormsg->Error(hi_->pos_, "for exp's range type is not integer");

  venv->BeginScope();
  tenv->BeginScope();
  /* tolerate 'break' sentence */
  venv->BeginLoop();
  tenv->BeginLoop();

  venv->Enter(var_, new env::VarEntry(type::IntTy::Instance(), true));
  type::Ty* body_ty = body_->SemAnalyze(venv, tenv, labelcount, errormsg);
  venv->EndScope();
  tenv->EndScope();

  venv->EndLoop();
  tenv->EndLoop();

  return type::VoidTy::Instance();
}

type::Ty *BreakExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::VoidTy::Instance();
}

type::Ty *LetExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  venv->BeginScope();
  tenv->BeginScope();
  const std::list<Dec *>* decslist = &decs_->GetList();
  for (auto iter = decslist->begin(); iter != decslist->end(); iter++) 
    (*iter)->SemAnalyze(venv, tenv, labelcount, errormsg);

  type::Ty* ty = body_->SemAnalyze(venv, tenv, labelcount, errormsg);
  
  venv->EndScope();
  tenv->EndScope();
  return ty;
}

type::Ty *ArrayExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty* ty = tenv->Look(typ_)->ActualTy();
  if (!ty) {
    errormsg->Error(pos_, "undefined type %s", typ_->Name().c_str());
    return type::VoidTy::Instance();
  };
  if (size_->SemAnalyze(venv, tenv, labelcount, errormsg)->kind_ != type::Ty::INT) {
    errormsg->Error(pos_, "undefined type %s", typ_->Name().c_str());
    return type::VoidTy::Instance();
  };
  if (ty->kind_ != type::Ty::ARRAY) {
    errormsg->Error(pos_, "not array type %d %d", ty->kind_, type::Ty::Kind::ARRAY);
    return type::VoidTy::Instance();
  };
  type::ArrayTy* array_ty = (type::ArrayTy*) ty;
  if (!init_->SemAnalyze(venv, tenv, labelcount, errormsg)->IsSameType(array_ty->ty_)) {
    errormsg->Error(pos_, "type mismatch");
    return type::VoidTy::Instance();
  };
  return array_ty;
}

type::Ty *VoidExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::VoidTy::Instance();
}

void FunctionDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  const std::list<FunDec *> * function_list = &functions_->GetList();
  for (auto iter = function_list->begin(); iter != function_list->end(); iter++) {
    auto next_iter = iter;
    if (next_iter != function_list->end()) next_iter++;
    for (; next_iter != function_list->end(); next_iter++)
      if ((*next_iter)->name_->Name() ==(*iter)->name_->Name())
        errormsg->Error((*iter)->pos_, "two functions have the same name");

    type::Ty* result;
    if ((*iter)->result_ != nullptr) {
      result = tenv->Look((*iter)->result_);
      if (result == nullptr)
      errormsg->Error(pos_, "FunctionDec undefined result");
    } else result = type::VoidTy::Instance();
  
    venv->Enter((*iter)->name_, new env::FunEntry(make_formal_tylist(tenv, (*iter)->params_, errormsg), result));
  };

  for (auto iter = function_list->begin(); iter != function_list->end(); iter++) {
    venv->BeginScope();
    const std::list<Field *>* field_list = &(*iter)->params_->GetList();
    for (auto field_iter = field_list->begin(); field_iter != field_list->end(); field_iter++) {
      type::Ty* ty = tenv->Look((*field_iter)->typ_);
      if (!ty) errormsg->Error((*field_iter)->pos_, "undefined type %s", (*field_iter)->typ_->Name().c_str());
      venv->Enter((*field_iter)->name_, new env::VarEntry(ty));
    };
    type::Ty* body_ty = (*iter)->body_->SemAnalyze(venv, tenv, labelcount, errormsg);
    type::Ty* dec_ty = ((env::FunEntry*) venv->Look((*iter)->name_))->result_;
    if (!body_ty->IsSameType(dec_ty)) {
      if (dec_ty->kind_ == type::Ty::VOID) errormsg->Error((*iter)->body_->pos_, "procedure returns value");
      else errormsg->Error((*iter)->body_->pos_, "return type mismatch");
    };
    venv->EndScope();
  };
}

void VarDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                        err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  if (typ_ == nullptr) {
    type::Ty* ty = init_->SemAnalyze(venv, tenv, labelcount, errormsg);
    if (ty->kind_ == type::Ty::NIL) errormsg->Error(pos_, "init should not be nil without type specified");
    venv->Enter(var_, new env::VarEntry(ty->ActualTy()));
  } 
  else {
    type::Ty* ty = tenv->Look(typ_);
    if (ty == nullptr) {
      errormsg->Error(pos_, "undefined type %s", typ_->Name().c_str());
      return;
    };
    if (ty->IsSameType(init_->SemAnalyze(venv, tenv, labelcount, errormsg)))
      venv->Enter(var_, new env::VarEntry(tenv->Look(typ_)));
    else errormsg->Error(pos_, "type mismatch");
  };
}

void TypeDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                         err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  const std::list<NameAndTy *> * ty_list = &types_->GetList();
  for (auto iter = ty_list->begin(); iter != ty_list->end(); iter++) {
    auto next_iter = iter;
    if (next_iter != ty_list->end()) next_iter++;
    for (; next_iter != ty_list->end(); next_iter++)
      if ((*next_iter)->name_ == (*iter)->name_)
        errormsg->Error(pos_, "two types have the same name");
    tenv->Enter((*iter)->name_, new type::NameTy((*iter)->name_, nullptr));
  };

  for (auto iter = ty_list->begin(); iter != ty_list->end(); iter++) {
    type::NameTy* name_ty = (type::NameTy*) tenv->Look((*iter)->name_);
    name_ty->ty_ = (*iter)->ty_->SemAnalyze(tenv, errormsg);
  };

  bool cycle = false;
  for (auto iter = ty_list->begin(); iter != ty_list->end(); iter++) {
    type::Ty* ty = tenv->Look((*iter)->name_);
    if (ty->kind_ == type::Ty::NAME) {
      type::Ty* ty_ty = ((type::NameTy*) ty)->ty_;
      while (ty_ty->kind_ == type::Ty::NAME) {
        type::NameTy* name_ty = (type::NameTy*) ty_ty;
        if (name_ty->sym_->Name() == (*iter)->name_->Name()) {
          errormsg->Error(pos_, "illegal type cycle");
          cycle = true;
          break;
        };
        ty_ty = name_ty->ty_;
      };
    };
    if (cycle) break;
  };
  return;
}

type::Ty *NameTy::SemAnalyze(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty* ty = tenv->Look(name_);
  if (!ty) {
    errormsg->Error(pos_, "undefined type %s", name_->Name().c_str());
    return type::VoidTy::Instance();
  };
  return new type::NameTy(name_, ty);
}

type::Ty *RecordTy::SemAnalyze(env::TEnvPtr tenv,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::FieldList* field_list = make_fieldlist(tenv, record_, errormsg);
  return new type::RecordTy(field_list);
}

type::Ty *ArrayTy::SemAnalyze(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty* ty = tenv->Look(array_);
  if (!ty) {
    errormsg->Error(pos_, "undefined type %s", array_->Name().c_str());
    return type::VoidTy::Instance();
  };
  return new type::ArrayTy(ty);
}

} // namespace absyn

namespace sem {

void ProgSem::SemAnalyze() {
  FillBaseVEnv();
  FillBaseTEnv();
  absyn_tree_->SemAnalyze(venv_.get(), tenv_.get(), errormsg_.get());
}
} // namespace sem
