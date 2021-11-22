#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>
#include <vector>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/x64frame.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/frame.h"

#define DEBUG

#ifdef DEBUG
  #define LOG(format, args...) do {   \
    FILE* debug_log = fopen("register.log", "a+");  \
    fprintf(debug_log, "%d, %s: ", __LINE__, __func__); \
    fprintf(debug_log, format, ##args); \
    fclose(debug_log);\
  } while (0)
#else
  #define LOG(format, args...) do{} while (0)
#endif

#define DEBUGLAB5

#ifdef DEBUGLAB5
  #define LOG5(format, args...) do {   \
    FILE* debug_log = fopen("debug5.log", "a+");  \
    fprintf(debug_log, "%d, %s: ", __LINE__, __func__); \
    fprintf(debug_log, format, ##args); \
    fclose(debug_log);\
  } while (0)
#else
  #define LOG5(format, args...) do{} while (0)
#endif

extern frame::Frags *frags;
extern frame::RegManager *reg_manager;

namespace tr {

Access *Access::AllocLocal(Level *level, bool escape) {
  /* TODO: Put your lab5 code here */
  if (level == NULL) LOG5("level is NULL\n");
  if (level->frame_ == NULL) LOG5("level-frame is NULL\n");
  return new Access(level, level->frame_->allocLocal(escape)); 
}

class Cx {
public:
  // temp::Label **trues_;
  // temp::Label **falses_;
  temp::LabelList* trues_;
  temp::LabelList* falses_;
  tree::Stm *stm_;

  // Cx(temp::Label **trues, temp::Label **falses, tree::Stm *stm)
      // : trues_(trues), falses_(falses), stm_(stm) {}
  Cx(temp::LabelList* trues, temp::LabelList* falses, tree::Stm* stm)
  : trues_(trues), falses_(falses), stm_(stm) {};
};

class Exp {
public:
  enum Kind { EX, NX, CX };

  Kind kind_;

  Exp(Kind kind) : kind_(kind) {};

  [[nodiscard]] virtual tree::Exp *UnEx() const = 0;
  [[nodiscard]] virtual tree::Stm *UnNx() const = 0;
  [[nodiscard]] virtual Cx UnCx(err::ErrorMsg *errormsg) const = 0;

  virtual ~Exp() = default;
};

class ExpAndTy {
public:
  tr::Exp *exp_;
  type::Ty *ty_;

  ExpAndTy(tr::Exp *exp, type::Ty *ty) : exp_(exp), ty_(ty) {}
};

class ExExp : public Exp {
public:
  tree::Exp *exp_;

  explicit ExExp(tree::Exp *exp) : Exp(EX), exp_(exp) {}

  [[nodiscard]] tree::Exp *UnEx() const override { 
    /* TODO: Put your lab5 code here */
    return exp_;
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::ExpStm(exp_);
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    tree::CjumpStm* stm = new tree::CjumpStm(tree::RelOp::NE_OP, exp_, new tree::ConstExp(0), NULL, NULL);
    temp::LabelList* trues = new temp::LabelList(stm->true_label_);
    temp::LabelList* falses = new temp::LabelList(stm->false_label_);
    return Cx(trues, falses, stm);
  }
};

class NxExp : public Exp {
public:
  tree::Stm *stm_;

  explicit NxExp(tree::Stm *stm) : Exp(NX), stm_(stm) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::EseqExp(stm_, new tree::ConstExp(0));
  }
  [[nodiscard]] tree::Stm *UnNx() const override { 
    /* TODO: Put your lab5 code here */
    return stm_;
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    printf("Error: Nx cannot be a test exp.");
  }
};

class CxExp : public Exp {
public:
  Cx cx_;

  // CxExp(temp::Label** trues, temp::Label** falses, tree::Stm *stm)
  //     : Exp(CX), cx_(trues, falses, stm) {}
  
  CxExp(temp::LabelList* trues, temp::LabelList* falses, tree::Stm* stm)
  : Exp(CX), cx_(trues, falses, stm) {};
  
  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    temp::Temp* r = temp::TempFactory::NewTemp();
    temp::Label* t = temp::LabelFactory::NewLabel(), *f = temp::LabelFactory::NewLabel();
    auto true_list = cx_.trues_->GetList(), false_list = cx_.falses_->GetList();
    for (auto iter = true_list.begin(); iter != true_list.end(); iter++) *iter = t;
    for (auto iter = false_list.begin(); iter != false_list.end(); iter++) *iter = f;

    return new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r), new tree::ConstExp(1)),
      new tree::EseqExp(cx_.stm_, 
      new tree::EseqExp(new tree::LabelStm(f),
      new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r), new tree::ConstExp(0)),
      new tree::EseqExp(new tree::LabelStm(t), new tree::TempExp(r))))));
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    temp::Label* label = temp::LabelFactory::NewLabel();
    auto true_list = cx_.trues_->GetList(), false_list = cx_.falses_->GetList();
    for (auto iter = true_list.begin(); iter != true_list.end(); iter++) *iter = label;
    for (auto iter = false_list.begin(); iter != false_list.end(); iter++) *iter = label;
    return new tree::SeqStm(cx_.stm_, new tree::LabelStm(label));
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override { 
    /* TODO: Put your lab5 code here */
    return cx_;
  }
};

void ProgTr::Translate() {
  /* TODO: Put your lab5 code here */
  LOG("Start Translate IR tree (1/2)...\n");
  absyn_tree_->Translate(venv_.get(), tenv_.get(), new tr::Level(new frame::X64Frame(temp::LabelFactory::NamedLabel("__tigermain__"), {}), nullptr), temp::LabelFactory::NamedLabel("__tigermain__"), errormsg_.get());
}

tree::Exp* StaticLink(tr::Level* target, tr::Level* level) {
  tree::Exp* staticklink = new tree::TempExp(reg_manager->FramePointer());
  while (level != target) {
    staticklink = level->frame_->formals->GetList().front()->ToExp(staticklink);
    level = level->parent_;
  }
  return staticklink;
}

} // namespace tr

namespace absyn {

tr::ExpAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  LOG("Start Translate IR tree(2/2)...\n");
  LOG("root type : %d\n", root_->kind_);
  return root_->Translate(venv, tenv, level, label, errormsg);
}

tr::Exp* TranslateSimpleVar(tr::Access* access, tr::Level* level) {
  tree::Exp* staticlink = tr::StaticLink(access->level_, level);
  staticlink = access->access_->ToExp(staticlink);

  return new tr::ExExp(staticlink);
}

tr::ExpAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  LOG("Translate SimpleVar level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());
  tr::Exp* exp = NULL;
  type::Ty* ty = type::IntTy::Instance();

  env::EnvEntry* entry = venv->Look(sym_);
  if (!entry || entry->kind_ != env::EnvEntry::VAR) {}
    //errormsg->Error(pos_, "undefined variable %s", sym_->Name().c_str());
  
  env::VarEntry* var_entry = (env::VarEntry*) entry;
  exp = TranslateSimpleVar(var_entry->access_, level);
  ty = var_entry->ty_->ActualTy();

  return new tr::ExpAndTy(exp, ty);
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  LOG("Translate FieldVar level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());
  tr::ExpAndTy* check_var = var_->Translate(venv, tenv, level, label, errormsg);
  type::Ty* actual_ty = check_var->ty_->ActualTy();

  if (actual_ty->kind_ != type::Ty::Kind::RECORD) {
    //errormsg->Error(pos_, "not a record type");
    return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  } else {
    type::FieldList* fields = ((type::RecordTy*) actual_ty)->fields_;
    int order = 0;

    auto list = fields->GetList();
    for (auto ele : list) {
      if (ele->name_ == sym_) {
        if (check_var->exp_->kind_ != tr::Exp::Kind::EX)
          printf("Error: fieldVar's loc must be an expression");
        
        tr::Exp* exp = new tr::ExExp(tree::NewMemPlus_Const(check_var->exp_->UnEx(), order * frame::wordsize));
        type::Ty* ty = ele->ty_->ActualTy();
        return new tr::ExpAndTy(exp, ty);
      }
      order++;
    };

    //errormsg->Error(pos_, "field %s doesn't exist", sym_->Name().c_str());
    return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  };
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  LOG("Translate SubscriptVar level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());
  tr::ExpAndTy* check_var = var_->Translate(venv, tenv, level, label, errormsg);

  if (check_var->ty_->ActualTy()->kind_ != type::Ty::Kind::ARRAY) {
    //errormsg->Error(pos_, "array type required");
    return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  };

  tr::ExpAndTy* check_subscript = subscript_->Translate(venv, tenv, level, label, errormsg);

  if (check_subscript->ty_->ActualTy()->kind_ != type::Ty::Kind::INT) {
    //errormsg->Error(pos_, "array index must be integer");
    return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  };

  if (check_var->exp_->kind_ != tr::Exp::Kind::EX || check_subscript->exp_->kind_ != tr::Exp::Kind::EX)
    printf("Error: subscriptVar's loc or subscript must be an expression");
  
  tr::Exp* exp = new tr::ExExp(new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP, check_var->exp_->UnEx(), new tree::BinopExp(tree::BinOp::MUL_OP, check_subscript->exp_->UnEx(), new tree::ConstExp(frame::wordsize)))));
  type::Ty* ty = ((type::ArrayTy *) check_var->ty_)->ty_->ActualTy();
  return new tr::ExpAndTy(exp, ty);
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  LOG("Translate VarExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                  
  /* TODO: Put your lab5 code here */
  switch (var_->kind_) {
    case Var::Kind::SIMPLE:
      return ((SimpleVar *)var_)->Translate(venv, tenv, level, label, errormsg);
    case Var::Kind::FIELD:
      return ((FieldVar *)var_)->Translate(venv, tenv, level, label, errormsg);
    case Var::Kind::SUBSCRIPT:
      return ((SubscriptVar *)var_)->Translate(venv, tenv, level, label, errormsg);
    default:
      assert(0);
  };
}

tr::Exp* TranslateNilExp() {
  return new tr::ExExp(new tree::ConstExp(0));
}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  LOG("Translate NilExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                    
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(TranslateNilExp(), type::NilTy::Instance());
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  LOG("Translate IntExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                    
  return new tr::ExpAndTy(new tr::ExExp(new tree::ConstExp(1)), type::IntTy::Instance());
}

tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  LOG("Translate StringExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                    
  temp::Label* string_label = temp::LabelFactory::NewLabel();
  frags->PushBack(new frame::StringFrag(string_label, str_));
  return new tr::ExpAndTy(new tr::ExExp(new tree::NameExp(string_label)), type::StringTy::Instance());
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  LOG("Translate CallExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                    
  tr::Exp* exp = NULL;
  type::Ty* ty = type::VoidTy::Instance();
  env::EnvEntry* entry = venv->Look(func_);
  if (!entry || entry->kind_ != env::EnvEntry::Kind::FUN) {
    //errormsg->Error(pos_, "undefined function %s", func_->Name().c_str());
    return new tr::ExpAndTy(exp, ty);
  };
  env::FunEntry* fun_entry = (env::FunEntry*) entry;
  if (fun_entry->result_) ty = fun_entry->result_->ActualTy();
  else ty = type::VoidTy::Instance();

  auto formal_list = fun_entry->formals_->GetList();
  auto arg_list = args_->GetList();

  int formal_size = formal_list.size(), arg_size = arg_list.size();
  int size = std::min(formal_size, arg_size);
  auto formal_iter = formal_list.begin();
  auto arg_iter = arg_list.begin();

  tree::ExpList* list = new tree::ExpList();
  while (size-- > 0) {
    tr::ExpAndTy* check_arg = (*arg_iter)->Translate(venv, tenv, level, label, errormsg);
    if (!check_arg->ty_->IsSameType(*formal_iter)) {
      //errormsg->Error(pos_, "para type mismatch");
      return new tr::ExpAndTy(exp, ty);
    };
    formal_iter++;
    arg_iter++;
    list->Append(check_arg->exp_->UnEx());
  };

  if (arg_size < formal_size) {
    //errormsg->Error(pos_, "too little params in function %s", func_->Name().c_str());
    return new tr::ExpAndTy(exp, ty);
  };
  if (arg_size > formal_size) {
    //errormsg->Error(pos_, "too many params in function %s", func_->Name().c_str());
    return new tr::ExpAndTy(exp, ty);
  };

  if (!fun_entry->level_->parent_)
    exp = new tr::ExExp(frame::externalCall(func_->Name(), list));
  else {
    list->Append(tr::StaticLink(fun_entry->level_->parent_, level));

    exp = new tr::ExExp(new tree::CallExp(new tree::NameExp(func_), list));
  };
  return new tr::ExpAndTy(exp, ty);
}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  LOG("Translate OpExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                              
  /* TODO: Put your lab5 code here */

  tr::ExpAndTy* check_left = left_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy* check_right = right_->Translate(venv, tenv, level, label, errormsg);
  tree::CjumpStm *stm = NULL;
  tr::Exp* exp = NULL;
  switch (oper_) {
    case Oper::PLUS_OP: case Oper::MINUS_OP: case Oper::TIMES_OP: case Oper::DIVIDE_OP:
    {
      if (check_left->ty_->kind_ != type::Ty::Kind::INT) {}//errormsg->Error(left_->pos_, "integer required");
      if (check_right->ty_->kind_ != type::Ty::Kind::INT) {}//errormsg->Error(right_->pos_, "integer required");
      switch (oper_) {
        case Oper::PLUS_OP:
          exp = new tr::ExExp(new tree::BinopExp(tree::BinOp::PLUS_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx()));
          break;
        case Oper::MINUS_OP:
          exp = new tr::ExExp(new tree::BinopExp(tree::BinOp::MINUS_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx()));
          break;
        case Oper::TIMES_OP:
          exp = new tr::ExExp(new tree::BinopExp(tree::BinOp::MUL_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx()));
          break;
        case Oper::DIVIDE_OP:
          exp = new tr::ExExp(new tree::BinopExp(tree::BinOp::DIV_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx()));
          break;
      }
      break;
    }
    case Oper::LT_OP: case Oper::LE_OP: case Oper::GT_OP: case Oper::GE_OP:
    {
      if (check_left->ty_->kind_ != type::Ty::Kind::INT && check_left->ty_->kind_ != type::Ty::Kind::STRING) //errormsg->Error(left_->pos_, "integer or string required");
      if (check_right->ty_->kind_ != type::Ty::Kind::INT && check_right->ty_->kind_ != type::Ty::Kind::STRING) //errormsg->Error(right_->pos_, "integer or string required");
      if (!check_left->ty_->IsSameType(check_right->ty_)) //errormsg->Error(pos_, "same type required");
      
      tree::CjumpStm* stm;
      switch (oper_) {
        case Oper::LT_OP:
          stm = new tree::CjumpStm(tree::RelOp::LT_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx(), NULL, NULL);
          break;
        case Oper::LE_OP:
          stm = new tree::CjumpStm(tree::RelOp::LE_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx(), NULL, NULL);
          break;
        case Oper::GT_OP:
          stm = new tree::CjumpStm(tree::RelOp::GT_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx(), NULL, NULL);
          break;
        case Oper::GE_OP:
          stm = new tree::CjumpStm(tree::RelOp::GE_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx(), NULL, NULL);
          break;                
      }
      temp::LabelList* trues = new temp::LabelList();
      trues->Append(stm->true_label_);
      temp::LabelList* falses = new temp::LabelList();
      falses->Append(stm->false_label_);
      exp = new tr::CxExp(trues, falses, stm);
      break;
    }
    case Oper::EQ_OP: case Oper::NEQ_OP:
    {      
      tree::CjumpStm* stm;
      switch (oper_) {
        case Oper::EQ_OP:
          if (check_left->ty_->kind_ == type::Ty::STRING)
            stm = new tree::CjumpStm(tree::EQ_OP, frame::externalCall("stringEqual", new tree::ExpList({check_left->exp_->UnEx(), check_right->exp_->UnEx()})), new tree::ConstExp(1), NULL, NULL);
          else stm = new tree::CjumpStm(tree::RelOp::EQ_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx(), NULL, NULL);
          break;
        case Oper::NEQ_OP:
          stm = new tree::CjumpStm(tree::RelOp::NE_OP, check_left->exp_->UnEx(), check_right->exp_->UnEx(), NULL, NULL);
          break;
      }
      temp::LabelList* trues = new temp::LabelList();
      trues->Append(stm->true_label_);
      temp::LabelList* falses = new temp::LabelList();
      trues->Append(stm->false_label_);
      exp = new tr::CxExp(trues, falses, stm);
      break;
    }
    default:
      assert(0);
  }

  return new tr::ExpAndTy(exp, type::IntTy::Instance());
}

tr::ExpAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,      
                                   err::ErrorMsg *errormsg) const {
  LOG("Translate RecordExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                
  /* TODO: Put your lab5 code here */
  type::Ty* ty = tenv->Look(typ_);
  tr::ExExp* exp = NULL;
  tree::ExpList* list = new tree::ExpList();
  tree::ExpList* tail = list;
  
  if (!ty) {
    //errormsg->Error(pos_, "undefined type %s", typ_->Name().c_str());
    return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  };

  ty = ty->ActualTy();
  if (ty->kind_ != type::Ty::Kind::RECORD) {
    //errormsg->Error(pos_, "not record type %s", typ_->Name().c_str());
    return new tr::ExpAndTy(NULL, ty);
  };

  auto records = ((type::RecordTy*) ty)->fields_->GetList();
  auto efields = fields_->GetList();
  
  int records_size = records.size(), efields_size = efields.size();
  int size = std::min(records_size, efields_size);
  auto records_iter = records.begin();
  auto efields_iter = efields.begin();

  int count = 0;
  while (size-- > 0) {
    count++;
    tr::ExpAndTy* check_exp = (*efields_iter)->exp_->Translate(venv, tenv, level, label, errormsg);

    if (!check_exp->ty_->IsSameType((*records_iter)->ty_)) {
      //errormsg->Error(pos_, "record type unmatched");
    };

    records_iter++;
    efields_iter++;

    tail->Append(check_exp->exp_->UnEx());
  };

  temp::Temp* reg = temp::TempFactory::NewTemp();

  tree::Stm* stm = new tree::MoveStm(new tree::TempExp(reg), frame::externalCall("allocRecord", new tree::ExpList(new tree::ConstExp(count * frame::wordsize))));

  count = 0;
  
  for (auto ele : list->GetList()) {
    stm = new tree::SeqStm(stm, new tree::MoveStm(tree::NewMemPlus_Const(new tree::TempExp(reg), count * frame::wordsize), NULL));
    count++;
  }
  exp = new tr::ExExp(new tree::EseqExp(stm, new tree::TempExp(reg)));

  return new tr::ExpAndTy(exp, ty);
}

tr::Exp *TranslateSeqExp(tr::Exp* left, tr::Exp* right) {
  if (right) return new tr::ExExp(new tree::EseqExp(left->UnNx(), right->UnEx()));
  else return  new tr::ExExp(new tree::EseqExp(left->UnNx(), new tree::ConstExp(0)));
};

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  LOG("Translate SeqExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                
  /* TODO: Put your lab5 code here */
  tr::Exp* exp = TranslateNilExp();
  if (!seq_ && !seq_->GetList().size()) return new tr::ExpAndTy(NULL, type::VoidTy::Instance());

  tr::ExpAndTy* check_exp = NULL;
  for (auto ele : seq_->GetList()) {
    check_exp = (*ele).Translate(venv, tenv, level, label, errormsg);
    exp = TranslateSeqExp(exp, check_exp->exp_);
  };
  return new tr::ExpAndTy(exp, check_exp->ty_);
}

tr::Exp* TranslateAssignExp(tr::Exp* var, tr::Exp* exp) {
  return new tr::NxExp(new tree::MoveStm(var->UnEx(), exp->UnEx()));
};

tr::ExpAndTy *AssignExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,                       
                                   err::ErrorMsg *errormsg) const {
  LOG("Translate AssignExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                
  /* TODO: Put your lab5 code here */
  if (var_->kind_ == Var::Kind::SIMPLE) {
    env::EnvEntry* entry = venv->Look(((SimpleVar *) var_)->sym_);
    if (entry->readonly_) {
      //errormsg->Error(pos_, "loop variable can't be assigned");
    };
  };

  tr::ExpAndTy* check_var = var_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy* check_exp = exp_->Translate(venv, tenv, level, label, errormsg);
  if (!check_var->ty_->IsSameType(check_exp->ty_)) {} //errormsg->Error(pos_, "unmatched assign exp");
  tr::Exp* exp = TranslateAssignExp(check_var->exp_, check_exp->exp_);
  return new tr::ExpAndTy(exp, type::VoidTy::Instance());
}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  LOG("Translate IfExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                                         
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy* check_test = test_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy* check_then = then_->Translate(venv, tenv, level, label, errormsg);

  tr::Exp* exp;
  if (elsee_) {
    tr::ExpAndTy* check_elsee = elsee_->Translate(venv, tenv, level, label, errormsg);
    if (check_then->ty_->IsSameType(check_elsee->ty_)) {
      //errormsg->Error(pos_, "then exp and else exp type mismatch");
      return new tr::ExpAndTy(NULL, type::VoidTy::Instance());
    };

    tr::Cx testc = check_test->exp_->UnCx(errormsg);
    temp::Temp* r = temp::TempFactory::NewTemp();
    temp::Label* true_label = temp::LabelFactory::NewLabel();
    temp::Label* false_label = temp::LabelFactory::NewLabel();
    temp::Label* meeting = temp::LabelFactory::NewLabel();
    
    // do_patch  
    exp = new tr::ExExp(new tree::EseqExp(testc.stm_,
    new tree::EseqExp(new tree::LabelStm(true_label),
    new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r), check_then->exp_->UnEx()),
    new tree::EseqExp(new tree::JumpStm(new tree::NameExp(meeting), new temp::LabelList(meeting)),
    new tree::EseqExp(new tree::LabelStm(false_label),
    new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r), check_elsee->exp_->UnEx()),
    new tree::EseqExp(new tree::LabelStm(meeting), new tree::TempExp(r)))))))));
  } else {
    if (check_then->ty_->kind_ != type::Ty::VOID) {
      //errormsg->Error(pos_, "if-then exp's body must produce no value");
      return new tr::ExpAndTy(NULL, type::VoidTy::Instance());
    };

    tr::Cx testc = check_test->exp_->UnCx(errormsg);
    temp::Temp* r = temp::TempFactory::NewTemp();
    temp::Label* true_label = temp::LabelFactory::NewLabel();
    temp::Label* false_label = temp::LabelFactory::NewLabel();
    temp::Label* meeting = temp::LabelFactory::NewLabel();

    // do_patch

    exp = new tr::NxExp(new tree::SeqStm(testc.stm_, new tree::SeqStm(new tree::LabelStm(true_label), new tree::SeqStm(check_then->exp_->UnNx(), new tree::LabelStm(false_label)))));
  }
  return new tr::ExpAndTy(exp, check_then->ty_);
}

tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,            
                                  err::ErrorMsg *errormsg) const {
  LOG("Translate WhileExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                                         
  /* TODO: Put your lab5 code here */
  tr::Exp* exp = NULL;
  type::Ty* ty = type::VoidTy::Instance();

  temp::Label* done_label = temp::LabelFactory::NewLabel();
  tr::ExpAndTy* check_test = test_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy* check_body = body_->Translate(venv, tenv, level, done_label, errormsg);
  if (check_body->ty_->kind_ != type::Ty::Kind::INT) {
    //errormsg->Error(test_->pos_, "integer required");
    return new tr::ExpAndTy(exp, ty);
  };
  if (check_body->ty_->kind_ != type::Ty::Kind::VOID) {
    //errormsg->Error(body_->pos_, "while body must produce no value");
    return new tr::ExpAndTy(exp, ty);
  };

  temp::Label* test_label = temp::LabelFactory::NewLabel();
  temp::Label* body_label = temp::LabelFactory::NewLabel();
  tr::Cx condition = check_test->exp_->UnCx(errormsg);
  // do_patch(condition->tr)

  exp = new tr::NxExp(
    new tree::SeqStm(new tree::LabelStm(test_label),
    new tree::SeqStm(condition.stm_,
    new tree::SeqStm(new tree::LabelStm(body_label),
    new tree::SeqStm(check_body->exp_->UnNx(),
    new tree::SeqStm(new tree::JumpStm(new tree::NameExp(test_label), new temp::LabelList(test_label)),
    new tree::LabelStm(done_label)))))));

  return new tr::ExpAndTy(exp, ty);
}

tr::ExpAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  LOG("Translate ForExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                                         
  /* TODO: Put your lab5 code here */
  tr::Exp* exp = NULL;
  type::Ty* ty = type::VoidTy::Instance();

  tr::ExpAndTy* check_lo = lo_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy* check_hi = hi_->Translate(venv, tenv, level, label, errormsg);
  if (check_lo->ty_->kind_ != type::Ty::Kind::INT) //errormsg->Error(lo_->pos_, "for exp's range type is not integer");
  if (check_hi->ty_->kind_ != type::Ty::Kind::INT) //errormsg->Error(hi_->pos_, "for exp's range type is not integer");

  venv->BeginScope();
  venv->Enter(var_, new env::VarEntry(tr::Access::AllocLocal(level, escape_), check_lo->ty_));
  
  tr::ExpAndTy* check_body = body_->Translate(venv, tenv, level, label, errormsg);
  if (check_body->ty_->kind_ != type::Ty::Kind::VOID) {
    //errormsg->Error(body_->pos_, "for body must produce no value");
    return new tr::ExpAndTy(exp, ty);
  };
  venv->EndScope();

  absyn::DecList* declist = new absyn::DecList();
  declist->Append(new absyn::VarDec(0, var_, sym::Symbol::UniqueSymbol("int"), lo_));
  declist->Append(new absyn::VarDec(0, sym::Symbol::UniqueSymbol("__limit_var__"), sym::Symbol::UniqueSymbol("int"), hi_));
  
  absyn::ExpList* bodylist = new absyn::ExpList();
  bodylist->Append(body_);
  bodylist->Append(new absyn::IfExp(0, new absyn::OpExp(0, absyn::Oper::EQ_OP, new absyn::VarExp(0, new absyn::SimpleVar(0, var_)), new absyn::VarExp(0, new absyn::SimpleVar(0, sym::Symbol::UniqueSymbol("__limit_var__")))), new absyn::BreakExp(0), NULL));
  
  absyn::WhileExp* body = new absyn::WhileExp(0, new absyn::OpExp(0, absyn::Oper::LE_OP, new absyn::VarExp(0, new absyn::SimpleVar(0, var_)), new absyn::VarExp(0, new absyn::SimpleVar(0, sym::Symbol::UniqueSymbol("__limit_var__")))),
                      new absyn::SeqExp(0, bodylist));

  absyn::Exp* forexp_to_letexp = new absyn::LetExp(0, declist, body);
  return forexp_to_letexp->Translate(venv, tenv, level, label, errormsg);
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  LOG("Translate BreakExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                                       
  /* TODO: Put your lab5 code here */
  tree::Stm* stm = new tree::JumpStm(new tree::NameExp(label), new temp::LabelList(label));
  tr::Exp* nxexp = new tr::NxExp(stm);
  return new tr::ExpAndTy(nxexp, type::VoidTy::Instance());
}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  LOG("Translate LetExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                                     
  // /* TODO: Put your lab5 code here */
  // tr::Exp* exp = NULL;
  // type::Ty* ty = type::VoidTy::Instance();

  // static bool first = true;
  // bool isMain = false;
  // if (first) {
  //   isMain = true;
  //   first = false;
  // };

  // tree::Exp* res = NULL;

  // venv->BeginScope();
  // tenv->BeginScope();
  // tree::Stm* stm = NULL;
  
  // if (decs_) {
  //   auto list = decs_->GetList();
  //   auto list_iter = list.begin();
  //   if (list_iter != list.end()) {
  //     stm = (*list_iter)->Translate(venv, tenv, level, label, errormsg)->UnNx();
  //     list_iter++;
  //   };
  //   while (list_iter != list.end()) {
  //     stm = new tree::SeqStm(stm, (*list_iter)->Translate(venv, tenv, level, label, errormsg)->UnNx());
  //     list_iter++;
  //   };
  // };

  // tr::ExpAndTy* check_body = body_->Translate(venv, tenv, level, label, errormsg);
  // venv->EndScope();
  // tenv->EndScope();
  // if (stm) res = new tree::EseqExp(stm, check_body->exp_->UnEx());
  // else res = check_body->exp_->UnEx();
  // stm = new tree::ExpStm(res);

  // if (isMain) {
  //   frags->PushBack(new frame::ProcFrag(stm, level->frame_));
  //   isMain = false;
  // };

  return NULL;
  // return new tr::ExpAndTy(new tr::ExExp(res), check_body->ty_->ActualTy());
}

tr::ExpAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,                    
                                  err::ErrorMsg *errormsg) const {
  LOG("Translate ArrayExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                                     
  /* TODO: Put your lab5 code here */
  type::Ty* ty = tenv->Look(typ_)->ActualTy();
  if (!ty) {
    //errormsg->Error(pos_, "undefined type %s", typ_->Name().c_str());
    return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  };
  if (ty->kind_ != type::Ty::ARRAY) {
    //errormsg->Error(pos_, "not array type %d %d", ty->kind_, type::Ty::Kind::ARRAY);
    return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  };

  tr::ExpAndTy* check_size = size_->Translate(venv, tenv, level, label, errormsg);
  if (check_size->ty_->kind_ != type::Ty::INT) {
    //errormsg->Error(pos_, "type of size expression should be int");
    LOG("type of size expression should be int");
    return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  };

  tr::ExpAndTy* check_init = init_->Translate(venv, tenv, level, label, errormsg);
  // if (!check_init->ty_->IsSameType(((type::ArrayTy *)ty)->ty_)) {
  //   //errormsg->Error(pos_, "type mismatch");
  //   LOG("type mismatch");
  //   return new tr::ExpAndTy(NULL, type::IntTy::Instance());
  // };

  tr::Exp* exp = new tr::ExExp(frame::externalCall("initArray", new tree::ExpList({check_size->exp_->UnEx(), check_size->exp_->UnEx()})));
  return new tr::ExpAndTy(exp, ty);
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  LOG("Translate VoidExp level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                  
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(NULL, type::VoidTy::Instance());
};

static type::TyList *make_formal_tylist(sym::Table<type::Ty> *tenv, absyn::FieldList* params, err::ErrorMsg* errormsg) {
  if (params == NULL) return NULL;

  type::TyList* tylist = new type::TyList();
  for (auto param : params->GetList()) {
    type::Ty* ty = tenv->Look(param->typ_);
    if (ty == NULL) {
      // //errormsg->Error(param->pos_, "undefined type %s", param->typ_->Name().c_str());
    } else tylist->Append(ty->ActualTy());
  };
  return tylist;
};

static type::FieldList* make_fieldlist(sym::Table<type::Ty>* tenv, absyn::FieldList* fields, err::ErrorMsg* errormsg) {
  if (fields == NULL) return NULL;
  type::FieldList* fieldlist = new type::FieldList();
  for (auto field : fields->GetList()) {
    type::Ty* ty = tenv->Look(field->typ_);
    if (ty == NULL) //errormsg->Error(field->pos_, "undefined type %s", field->typ_->Name().c_str());
    fieldlist->Append(new type::Field(field->name_, ty));
  };
  return fieldlist;
}

std::list<bool> make_formal_ecslist(absyn::FieldList* params, err::ErrorMsg* errormsg) {
  std::list<bool> ecslist = std::list<bool>();
  for (auto param : params->GetList()) 
    ecslist.push_back(param->escape_);
  return ecslist;
};

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  LOG("Translate FunctionDec level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                  
  /* TODO: Put your lab5 code here */
  sym::Table<int> *check_table = new sym::Table<int>();

  auto list = functions_->GetList();
  for (auto ele : list) {
    if (check_table->Look(ele->name_)) {
      //errormsg->Error(ele->pos_, "two functions have the same name");
      continue;
    };
    check_table->Enter(ele->name_, (int *) 1);
    type::TyList* formaltys = make_formal_tylist(tenv, ele->params_, errormsg);
    tr::Level* new_level = tr::Level::NewLevel(level, ele->name_, make_formal_ecslist(ele->params_, errormsg));
    if (!ele->result_) {
      venv->Enter(ele->name_, new env::FunEntry(new_level, ele->name_, formaltys, type::VoidTy::Instance()));
      continue;
    };
    type::Ty* result = tenv->Look(ele->result_);
    if (!result) {
      //errormsg->Error(pos_, "FunctionsDec undefined result");
      continue;
    };
    venv->Enter(ele->name_, new env::FunEntry(new_level, ele->name_, formaltys, result));
  };
  for (auto fundec : list) {
    venv->BeginScope();
    env::FunEntry* funentry = (env::FunEntry *)venv->Look(fundec->name_);
    auto formaltys = funentry->formals_->GetList();
    auto formaltys_iter = formaltys.begin();
    auto formalaccs = funentry->level_->frame_->formals->GetList();
    auto formalaccs_iter = formalaccs.begin();

    for (auto field : fundec->params_->GetList()) {
      venv->Enter(field->name_, new env::VarEntry(new tr::Access(funentry->level_, *formalaccs_iter), *formaltys_iter));
      formaltys_iter++;
      formalaccs_iter++;
    };
    
    tr::ExpAndTy* entry = fundec->body_->Translate(venv, tenv, funentry->level_, funentry->label_, errormsg);
    if (!entry->ty_->IsSameType(type::VoidTy::Instance()) && fundec->result_ == NULL) //errormsg->Error(pos_, "procedure returns value");
    if (fundec->result_ && entry->ty_->IsSameType(tenv->Look(fundec->result_)->ActualTy())) //errormsg->Error(pos_, "function return value type incorrect");
    venv->EndScope();
    
    frags->PushBack(new frame::ProcFrag(frame::procEntryExit1(funentry->level_->frame_, new tree::MoveStm(new tree::TempExp(reg_manager->ReturnValue()), entry->exp_->UnEx())), funentry->level_->frame_));
  };

  return TranslateNilExp();
}

tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           err::ErrorMsg *errormsg) const {
  LOG("Translate VarDec level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy* check_init = init_->Translate(venv, tenv, level, label, errormsg);
  tr::Access* access;
  if (!typ_) {
    if (check_init->ty_->ActualTy()->kind_ == type::Ty::NIL) {} //errormsg->Error(pos_, "init should not be nil without type specified");
  } else {
    type::Ty* ty = tenv->Look(typ_);
    if (check_init->ty_->ActualTy()->kind_ == type::Ty::Kind::NIL && ty->ActualTy()->kind_ != type::Ty::Kind::RECORD) //errormsg->Error(pos_, "init should not be nil without type specified");
    if (ty && ty->kind_ != check_init->ty_->ActualTy()->kind_) {} //errormsg->Error(pos_, "type mismatch");
  };
  access = tr::Access::AllocLocal(level, this->escape_);
  venv->Enter(var_, new env::VarEntry(access, check_init->ty_->ActualTy()));

  return TranslateSimpleVar(access, level);
  return TranslateAssignExp(TranslateSimpleVar(access, level), check_init->exp_);
}

tr::Exp *TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, temp::Label *label,
                            err::ErrorMsg *errormsg) const {
  LOG("Translate TypeDec level %s label %s\n", temp::LabelFactory::LabelString(level->frame_->label).c_str(), temp::LabelFactory::LabelString(label).c_str());                                                                
  /* TODO: Put your lab5 code here */
  auto tylist = types_->GetList();
  auto tylist_iter = tylist.begin();
  while (tylist_iter != tylist.end()) {
    auto tylist_next_iter = tylist_iter;
    tylist_next_iter++;
    while (tylist_next_iter != tylist.end()) {
      if ((*tylist_iter)->name_ == (*tylist_next_iter)->name_)
        //errormsg->Error(pos_, "two types have the same name");
      tylist_next_iter++;
    };
    tenv->Enter((*tylist_iter)->name_, new type::NameTy((*tylist_iter)->name_, NULL));
    tylist_iter++;
  };

  for (auto ele : tylist) {
    type::NameTy* nameTy = (type::NameTy *)tenv->Look(ele->name_);
    nameTy->ty_ = ele->ty_->Translate(tenv, errormsg);
  };

  bool hasCycle = false;
  for (auto ele : tylist) {
    type::Ty* ty = tenv->Look(ele->name_);
    if (ty->kind_ == type::Ty::NAME) {
      type::Ty* tyTy = ((type::NameTy *)ty)->ty_;
      while (tyTy->kind_ == type::Ty::NAME) {
        type::NameTy* nameTy = (type::NameTy *)tyTy;
        if (nameTy->sym_->Name() == ele->name_->Name()) {
          //errormsg->Error(pos_, "illegal type cycle");
          hasCycle = true;
          break;
        };
        tyTy = nameTy->ty_;
      }
    };
    if (hasCycle) break;
  }
  return TranslateNilExp();
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  LOG("Translate NameTy\n");
  /* TODO: Put your lab5 code here */
  type::Ty* ty = tenv->Look(name_);
  if (!ty) {
    //errormsg->Error(pos_, "undefined type %s", name_->Name().c_str());
    return type::VoidTy::Instance();
  };
  return new type::NameTy(name_, ty);
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  LOG("Translate RecordTy\n");
  /* TODO: Put your lab5 code here */
  type::FieldList* fields = make_fieldlist(tenv, record_, errormsg);
  return new type::RecordTy(fields);
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  LOG("Translate ArrayTy\n");
  /* TODO: Put your lab5 code here */
  type::Ty* ty = tenv->Look(array_);
  if (!ty) {
    //errormsg->Error(pos_, "undefined type %s", array_->Name().c_str());
    return new type::ArrayTy(NULL);
  };
  return new type::ArrayTy(ty);
}

} // namespace absyn
