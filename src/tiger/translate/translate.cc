#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/x64frame.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/frame.h"

extern frame::Frags *frags;
extern frame::RegManager *reg_manager;

namespace tr {

/* Use log to debug .
 * Must be "a+" to append.
 */
#define LOG(format, args...) do {   \
  FILE* debug_log = fopen("translate.log", "a+");  \
  fprintf(debug_log, "%d, %s:", __LINE__, __func__); \
  fprintf(debug_log, format, ##args); \
  fclose(debug_log);\
} while (0)

Access *Access::AllocLocal(Level *level, bool escape) {
  /* TODO: Put your lab5 code here */
  return new Access(level, ((frame::X64Frame *)(level->frame_))->AllocLocal(escape));
}

/* Conditional expression 
 * Expressed as a statement that may transfer to one of the two labels
 * True Lable & False Label
 */
class Cx {
public:
  temp::Label **trues_;
  temp::Label **falses_;
  tree::Stm *stm_;

  Cx(temp::Label **trues, temp::Label **falses, tree::Stm *stm)
      : trues_(trues), falses_(falses), stm_(stm) {}
};

/* A union type to simulate three kinds of expressions */
class Exp {
public:
  enum Kind {EX, NX, CX};

  Kind kind_;

  Exp(Kind kind): kind_(kind) {}

  [[nodiscard]] virtual tree::Exp *UnEx() const = 0;
  [[nodiscard]] virtual tree::Stm *UnNx() const = 0;
  [[nodiscard]] virtual Cx UnCx(err::ErrorMsg *errormsg) const = 0;
};

class ExpAndTy {
public:
  tr::Exp *exp_;
  type::Ty *ty_;

  ExpAndTy(tr::Exp *exp, type::Ty *ty) : exp_(exp), ty_(ty) {}
};

/* Normal expression, with value returns */
class ExExp : public Exp {
public:
  tree::Exp *exp_;

  explicit ExExp(tree::Exp *exp) : exp_(exp), Exp(EX) {}

  [[nodiscard]] tree::Exp *UnEx() const override { 
    /* TODO: Put your lab5 code here */
    return this->exp_;
  }
  /* Wrap it from a exp to a stm, so there will be no return */
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::ExpStm(exp_);
  }
  /* Transfer it as 
   * "If exp_ != 0 then true_label_ else false_label_" 
   */
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    auto cjs = new tree::CjumpStm(tree::NE_OP, exp_, new tree::ConstExp(0), nullptr, nullptr);
    return Cx(&(cjs->true_label_), &(cjs->false_label_), cjs);
  }
};

/* No return expression, just like a simple statement */
class NxExp : public Exp {
public:
  tree::Stm *stm_;

  explicit NxExp(tree::Stm *stm) : stm_(stm), Exp(NX) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::EseqExp(stm_, new tree::ConstExp(0));
  }
  [[nodiscard]] tree::Stm *UnNx() const override { 
    /* TODO: Put your lab5 code here */
    return stm_;
  }
  /* This cannot happen in a normal program */
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    assert(false);
  }
};

/* Conditional expression */
class CxExp : public Exp {
public:
  Cx cx_;

  CxExp(temp::Label** trues, temp::Label** falses, tree::Stm *stm)
      : cx_(trues, falses, stm), Exp(CX) {}
  
  /* Default return is true(1)
   * If false, jump to label f and set return to false(0)
   * Else, directly return true(1)
   */
  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    auto r = temp::TempFactory::NewTemp();
    auto t = temp::LabelFactory::NewLabel();
    auto f = temp::LabelFactory::NewLabel();
    *cx_.trues_ = t;
    *cx_.falses_ = f;

    return new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r), new tree::ConstExp(1)),
      new tree::EseqExp(cx_.stm_,
        new tree::EseqExp(new tree::LabelStm(f),
          new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r), new tree::ConstExp(0)),
            new tree::EseqExp(new tree::LabelStm(t), new tree::TempExp(r))))));
  }
  /* Set both true_label_ and false_label_ to one new label.
   * Directly execute the whole statement, with the new label append to it.
   */
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    auto l = temp::LabelFactory::NewLabel();
    *cx_.trues_ = l;
    *cx_.falses_ = l;
    return new tree::SeqStm(cx_.stm_, new tree::LabelStm(l));
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override { 
    /* TODO: Put your lab5 code here */
    return cx_;
  }
};

void ProgTr::Translate() {
  /* TODO: Put your lab5 code here */
  temp::Label* mainLabel = temp::LabelFactory::NamedLabel("tigermain"); /* The outermost proc frag name */

  /* Add pre-define types / funs */
  auto main_frame_ = new frame::X64Frame(mainLabel, {});
  main_level_.reset(new Level(main_frame_, nullptr));
  FillBaseTEnv();
  FillBaseVEnv();

  absyn_tree_->Translate(venv_.get(), 
                        tenv_.get(), 
                        main_level_.get(), 
                        mainLabel, 
                        errormsg_.get());
}

// get static link from current level to target level
tree::Exp* GetStaticLink(tr::Level* target, tr::Level* level) {
  /* current framepointer */
  tree::Exp* res = new tree::TempExp(reg_manager->FramePointer());
  while (level != target) {
    res = new tree::MemExp(new tree::BinopExp(tree::PLUS_OP, res, new tree::ConstExp(reg_manager->WordSize())));
    level = level->parent_;
  }
  return res;
}

} // namespace tr

namespace absyn {

/* Tool functions */
tree::ConstExp *GetConstExp(int i) {
  return new tree::ConstExp(i);
}

tree::BinopExp *GetPlusExp(tree::Exp *left, tree::Exp *right) {
  return new tree::BinopExp(tree::BinOp::PLUS_OP, left, right);
}

tree::BinopExp *GetMulExp(tree::Exp *left, tree::Exp *right) {
  return new tree::BinopExp(tree::BinOp::MUL_OP, left, right);
}

tree::MemExp *GetMemExp(tree::Exp *left, int word_count) {
  return new tree::MemExp(GetPlusExp(left, GetConstExp(word_count * reg_manager->WordSize())));
}

tr::ExpAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return root_->Translate(venv, tenv, level, label, errormsg);
}

tr::ExpAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto ent = (env::VarEntry *)venv->Look(sym_);

  /* Get it's definition from staticlink */
  tree::Exp *staticLink = tr::GetStaticLink(ent->access_->level_, level);
  tr::Exp *exp = new tr::ExExp(ent->access_->access_->ToExp(staticLink));

  return new tr::ExpAndTy(exp, ent->ty_->ActualTy());
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto res = var_->Translate(venv, tenv, level, label, errormsg);
  type::Ty *ty = res->ty_->ActualTy();

  /* Get actual type's fields */
  auto fields = ((type::RecordTy *)ty)->fields_;
  int i = 0;

  /* Match and create exp 
   * e.g.
   *  type t = {a:int, b:int}
   *  var x := t{a = 3, b = 4}
   */
  for (auto it : fields->GetList()) {
    if (it->name_ == sym_) {
      tr::Exp *exp = new tr::ExExp(GetMemExp(res->exp_->UnEx(), i));
      return new tr::ExpAndTy(exp, it->ty_->ActualTy());
    }
    ++i;
  }

  assert(false);
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto vres = var_->Translate(venv, tenv, level, label, errormsg);
  auto sres = subscript_->Translate(venv, tenv, level, label, errormsg);

  /* Get addr of a array 
   * e.g. arr[1]
   */
  tr::Exp *exp = new tr::ExExp(
    new tree::MemExp(
      GetPlusExp(vres->exp_->UnEx(), 
        GetMulExp(sres->exp_->UnEx(), GetConstExp(reg_manager->WordSize())))));

  // return var's type
  type::Ty *ty = ((type::ArrayTy *) vres->ty_)->ty_->ActualTy();;

  return new tr::ExpAndTy (exp, ty);
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  switch(var_->kind_) {
    case Var::SIMPLE:
      return ((SimpleVar *)var_)->Translate(venv, tenv, level, label, errormsg);
    case Var::FIELD:
      return ((FieldVar *)var_)->Translate(venv, tenv, level, label, errormsg);
    case Var::SUBSCRIPT:
      return ((SubscriptVar *)var_)->Translate(venv, tenv, level, label, errormsg);
    default:
      break;
  }
  assert(false);
}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(new tr::ExExp(new tree::ConstExp(0)), type::NilTy::Instance());
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(new tr::ExExp(new tree::ConstExp(val_)), type::IntTy::Instance());
}

/* Add the string to stringfrag, so it can be reached by it's label. 
 * frags is a global var
 */
tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto newLabel = temp::LabelFactory::NewLabel();
  frags->PushBack(new frame::StringFrag(newLabel, str_));
  return new tr::ExpAndTy(new tr::ExExp(new tree::NameExp(newLabel)), type::StringTy::Instance());
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto *exp_list = new tree::ExpList();
  auto fent = (env::FunEntry *)venv->Look(func_);

  /* The staticlink as the first argument 
   * Add other arguments into the expression list
   */
  auto staticLink = tr::GetStaticLink(fent->level_->parent_, level);
  exp_list->Append(staticLink);
  for (auto it : args_->GetList()) {
    tr::ExpAndTy *res = it->Translate(venv, tenv, level, label, errormsg);
    exp_list->Append(res->exp_->UnEx());
  }

  /* Judge return type */
  type::Ty *ty;
  if (fent->result_ != nullptr) {
    ty = fent->result_->ActualTy();
  }
  else {
    ty = type::VoidTy::Instance();
  }

  tr::Exp *exp;
  if (fent->level_->parent_ == nullptr) {
    /* If current level is the outermost level or current function is a libraries function.
     * Use 'ExternalCall' with the current function name
     */
    exp = new tr::ExExp(frame::ExternalCall(temp::LabelFactory::LabelString(func_), exp_list));
  }
  else {
    exp = new tr::ExExp(new tree::CallExp(new tree::NameExp(func_), exp_list));
  }
  
  return new tr::ExpAndTy(exp, ty);
}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *lres = left_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *rres = right_->Translate(venv, tenv, level, label, errormsg);

  tree::CjumpStm *stm = nullptr;
  tr::Exp *exp = nullptr;

  switch (oper_) {
    case Oper::PLUS_OP:
      exp = new tr::ExExp(new tree::BinopExp(tree::BinOp::PLUS_OP, lres->exp_->UnEx(), rres->exp_->UnEx()));
      break;

    case Oper::MINUS_OP:
      exp = new tr::ExExp(new tree::BinopExp(tree::BinOp::MINUS_OP, lres->exp_->UnEx(), rres->exp_->UnEx()));
      break;

    case Oper::TIMES_OP:
      exp = new tr::ExExp(new tree::BinopExp(tree::BinOp::MUL_OP, lres->exp_->UnEx(), rres->exp_->UnEx()));
      break;

    case Oper::DIVIDE_OP:
      exp = new tr::ExExp(new tree::BinopExp(tree::BinOp::DIV_OP, lres->exp_->UnEx(), rres->exp_->UnEx()));
      break;

    case Oper::LT_OP:
      stm = new tree::CjumpStm(tree::RelOp::LT_OP, lres->exp_->UnEx(), rres->exp_->UnEx(), nullptr, nullptr);
      exp = new tr::CxExp(&stm->true_label_, &stm->false_label_, stm);
      break;

    case Oper::LE_OP:
      stm = new tree::CjumpStm(tree::RelOp::LE_OP, lres->exp_->UnEx(), rres->exp_->UnEx(), nullptr, nullptr);
      exp = new tr::CxExp(&stm->true_label_, &stm->false_label_, stm);
      break;

    case Oper::GT_OP:
      stm = new tree::CjumpStm(tree::RelOp::GT_OP, lres->exp_->UnEx(), rres->exp_->UnEx(), nullptr, nullptr);
      exp = new tr::CxExp(&stm->true_label_, &stm->false_label_, stm);
      break;

    case Oper::GE_OP:
      stm = new tree::CjumpStm(tree::RelOp::GE_OP, lres->exp_->UnEx(), rres->exp_->UnEx(), nullptr, nullptr);
      exp = new tr::CxExp(&stm->true_label_, &stm->false_label_, stm);
      break;

    case Oper::EQ_OP:
      /* Support string compare */
      if (lres->ty_->kind_ == type::Ty::STRING && rres->ty_->kind_ == type::Ty::STRING) {
        auto expList = new tree::ExpList();
        expList->Append(new tree::TempExp(reg_manager->FramePointer()));
        expList->Append(lres->exp_->UnEx());
        expList->Append(rres->exp_->UnEx());
        stm = new tree::CjumpStm(tree::EQ_OP, frame::ExternalCall("string_equal", expList), GetConstExp(1), nullptr, nullptr);
      }
      else {
        stm = new tree::CjumpStm(tree::EQ_OP, lres->exp_->UnEx(), rres->exp_->UnEx(), nullptr, nullptr);
      }
      exp = new tr::CxExp(&stm->true_label_, &stm->false_label_, stm);
      break;

    case Oper::NEQ_OP:
      stm = new tree::CjumpStm(tree::NE_OP, lres->exp_->UnEx(), rres->exp_->UnEx(), nullptr, nullptr);
      exp = new tr::CxExp(&stm->true_label_, &stm->false_label_, stm);
      break;

    default:
      assert(0);
  }

  /* Int type result */
  return new tr::ExpAndTy(exp, type::IntTy::Instance());
}

tr::ExpAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,      
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty *ty = tenv->Look(typ_);
  tr::ExExp *exp = nullptr;

  auto expList = new tree::ExpList();
  auto elist = fields_->GetList();

  /* get ex of each item in field */
  for (auto &it : elist) {
    tr::ExpAndTy *res = it->exp_->Translate(venv, tenv, level, label, errormsg);
    expList->Append(res->exp_->UnEx());
  }

  auto reg = temp::TempFactory::NewTemp();

  /* Call malloc and store pointer */
  auto arg = new tree::ExpList();
  arg->Append(new tree::TempExp(reg_manager->FramePointer()));
  arg->Append(GetConstExp(elist.size() * reg_manager->WordSize()));
  tree::Stm *stm = new tree::MoveStm(new tree::TempExp(reg), frame::ExternalCall("alloc_record", arg));
  
  /* Create expression tree for record */
  int i = 0;
  for (auto &it : expList->GetList()){
    stm = new tree::SeqStm(stm, new tree::MoveStm(GetMemExp(new tree::TempExp(reg), i), it));
    ++i;
  }

  exp = new tr::ExExp(new tree::EseqExp(stm, new tree::TempExp(reg)));
  return new tr::ExpAndTy(exp, ty);
}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::Exp *exp = new tr::ExExp(new tree::ConstExp(0));

  /* Build tree from list.
   * Transfer SeqExp to EseqExp in order to execute stm and return stm.exp as well.
   */
  tr::ExpAndTy *res = nullptr;
  for (auto &it : seq_->GetList()) {
    res = it->Translate(venv, tenv, level, label, errormsg);
    exp = new tr::ExExp(new tree::EseqExp(exp->UnNx(), res->exp_->UnEx()));
  }
  
  return new tr::ExpAndTy(exp, res->ty_);
}

tr::ExpAndTy *AssignExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,                       
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto vres = var_->Translate(venv, tenv, level, label, errormsg);
  auto eres = exp_->Translate(venv, tenv, level, label, errormsg);

  tr::Exp *exp = new tr::NxExp(new tree::MoveStm(vres->exp_->UnEx(), eres->exp_->UnEx()));
  return new tr::ExpAndTy(exp, type::VoidTy::Instance());
}

/* Use True_Flag and False_Flag to locate */
tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto test_res = test_->Translate(venv, tenv, level, label, errormsg);
  auto then_res = then_->Translate(venv, tenv, level, label, errormsg);

  tr::Exp *exp;
  if(elsee_ != nullptr){
    auto else_res = elsee_->Translate(venv, tenv, level, label, errormsg);
    
    tr::Cx testc = test_res->exp_->UnCx(errormsg);
    temp::Temp *r = temp::TempFactory::NewTemp();
    temp::Label *trues = temp::LabelFactory::NewLabel();
    temp::Label *falses = temp::LabelFactory::NewLabel();
    temp::Label *flag = temp::LabelFactory::NewLabel();
    *testc.trues_ = trues;
    *testc.falses_ = falses;

    auto arr = new std::vector<temp::Label *>();
    arr->push_back(flag);
    
    exp = new tr::ExExp(new tree::EseqExp(testc.stm_,
      new tree::EseqExp(new tree::LabelStm(trues), 
        new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r), then_res->exp_->UnEx()), 
          new tree::EseqExp(new tree::JumpStm(new tree::NameExp(flag), arr), 
            new tree::EseqExp(new tree::LabelStm(falses), 
              new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r), else_res->exp_->UnEx()), 
                new tree::EseqExp(new tree::JumpStm(new tree::NameExp(flag), arr), 
                  new tree::EseqExp(new tree::LabelStm(flag), new tree::TempExp(r))))))))));
  }
  else {
    tr::Cx testc = test_res->exp_->UnCx(errormsg);
    temp::Temp *r = temp::TempFactory::NewTemp();
    temp::Label *trues = temp::LabelFactory::NewLabel();
    temp::Label *falses = temp::LabelFactory::NewLabel();
    temp::Label *flag = temp::LabelFactory::NewLabel();
    *testc.trues_ = trues;
    *testc.falses_ = falses;

    /* Because there is no elsee_, so no return value */
    exp = new tr::NxExp(
      new tree::SeqStm(testc.stm_, 
        new tree::SeqStm(new tree::LabelStm(trues), 
          new tree::SeqStm(then_res->exp_->UnNx(), new tree::LabelStm(falses)))));
  }

  return new tr::ExpAndTy(exp, then_res->ty_);
}

/* test label:
 *            if not(condition) goto done
 *            body label
 *            goto test
 * done label:
 */
tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,            
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  temp::Label *dlabel = temp::LabelFactory::NewLabel();
  tr::ExpAndTy *tres = test_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *bres = body_->Translate(venv, tenv, level, dlabel, errormsg);

  temp::Label *tlabel = temp::LabelFactory::NewLabel();
  temp::Label *blabel = temp::LabelFactory::NewLabel();
  tr::Cx cx = tres->exp_->UnCx(errormsg);
  *cx.trues_ = blabel;
  *cx.falses_ = dlabel;

  auto arr = new std::vector<temp::Label *>();
    arr->push_back(tlabel);
  
  tr::Exp *exp = new tr::NxExp(
    new tree::SeqStm(new tree::LabelStm(tlabel), 
      new tree::SeqStm(cx.stm_, 
        new tree::SeqStm(new tree::LabelStm(blabel), 
          new tree::SeqStm(bres->exp_->UnNx(), 
            new tree::SeqStm(new tree::JumpStm(new tree::NameExp(tlabel), arr), 
              new tree::LabelStm(dlabel)))))));

  return new tr::ExpAndTy(exp, type::VoidTy::Instance());
}

/* let var loop_var := lres
 *     var limit_var := hres
 * in while loop_var <= limit_var
 *    do (body; i++ )
 * end
 */
tr::ExpAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto lres = this->lo_->Translate(venv, tenv, level, label, errormsg);
  auto hres = this->hi_->Translate(venv, tenv, level, label, errormsg);

  venv->BeginScope();
  /* Alloc loopvar */
  env::VarEntry* loop_var_ent = new env::VarEntry(tr::Access::AllocLocal(level, false), type::IntTy::Instance(), true);
  venv->Enter(var_, loop_var_ent);

  /* Get limvar and loopvar */
  tr::Exp* limit_var = new tr::ExExp(new tree::TempExp(temp::TempFactory::NewTemp()));
  tr::Exp* loop_var =  new tr::ExExp(new tree::TempExp(((frame::InRegAccess*)loop_var_ent->access_->access_)->reg));
  
  /* Create new labels for loop */
  auto body_label = temp::LabelFactory::NewLabel();
  auto inc_label = temp::LabelFactory::NewLabel();
  auto done_label = temp::LabelFactory::NewLabel();

  auto bres = body_->Translate(venv, tenv, level, done_label, errormsg);

  /* Assign loop and limit var */
  tree::Stm* init_loop_var_stm = new tree::MoveStm(loop_var->UnEx(), lres->exp_->UnEx());
  tree::Stm* init_limit_stm = new tree::MoveStm(limit_var->UnEx(), hres->exp_->UnEx());
  
  /* First test: loop <= limit */
  tree::Stm* first_test_stm = new tree::CjumpStm(tree::LT_OP, loop_var->UnEx(), limit_var->UnEx(), body_label, done_label);
  /* ++ loop_var */
  tree::Stm* inc_loop_var_stm = new tree::MoveStm(loop_var->UnEx(), GetPlusExp(loop_var->UnEx(), GetConstExp(1)));
  
  /* Body label list */
  auto labelList = new std::vector<temp::Label* >(); 
  labelList->push_back(body_label);

  /* Test stm */
  tree::Stm* test_stm = new tree::SeqStm(
    new tree::CjumpStm(tree::LT_OP, loop_var->UnEx(), limit_var->UnEx(), inc_label, done_label),
      new tree::SeqStm(new tree::LabelStm(inc_label),
        new tree::SeqStm(inc_loop_var_stm,
          new tree::JumpStm(new tree::NameExp(body_label), labelList))));

  /* Combine all */
  tree::Stm* res = new tree::SeqStm(init_loop_var_stm,
    new tree::SeqStm(init_limit_stm,
      new tree::SeqStm(first_test_stm,
        new tree::SeqStm(new tree::LabelStm(body_label),
          new tree::SeqStm(bres->exp_->UnNx(),
            new tree::SeqStm(test_stm,
              new tree::LabelStm(done_label)))))));
  venv->EndScope();

  return new tr::ExpAndTy(new tr::NxExp(res), type::VoidTy::Instance());
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto expList = new std::vector<temp::Label *>();
  expList->push_back(label);

  tree::Stm *stm = new tree::JumpStm(new tree::NameExp(label), expList); 
  tr::Exp *exp = new tr::NxExp(stm);
  return new tr::ExpAndTy(exp, type::VoidTy::Instance());
}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  /* MAKE SURE JUST ONCE */
  static bool mainFunction = true;
  bool passBool = false;
  if (mainFunction) {
    passBool = true;
    mainFunction = false;
  }

  venv->BeginScope();
  tenv->BeginScope();
	tree::Stm *stm = nullptr;
  for (auto it : decs_->GetList()) {
    if (stm == nullptr) {
      stm = it->Translate(venv, tenv, level, label, errormsg)->UnNx();
    }
    else {
      stm = new tree::SeqStm(stm, it->Translate(venv, tenv, level, label, errormsg)->UnNx());
    }
  }
  auto bres = body_->Translate(venv, tenv, level, label, errormsg);
  venv->EndScope();
  tenv->EndScope();

  tree::Exp *res;

  if(stm) {
    res = new tree::EseqExp(stm, bres->exp_->UnEx());
  }
  else {
    res = bres->exp_->UnEx();
  }
  stm = new tree::ExpStm(res);

  if (passBool) {
    frags->PushBack(new frame::ProcFrag(stm, level->frame_));
  }

  return new tr::ExpAndTy(new tr::ExExp(res), bres->ty_->ActualTy());
}

tr::ExpAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,                    
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty *ty = tenv->Look(typ_)->ActualTy();

  auto sres = size_->Translate(venv, tenv, level, label, errormsg);
  auto ires = init_->Translate(venv, tenv, level, label, errormsg);

  auto expList = new tree::ExpList();
  expList->Append(new tree::TempExp(reg_manager->FramePointer()));
  expList->Append(sres->exp_->UnEx());
  expList->Append(ires->exp_->UnEx());

  tr::Exp *exp = new tr::ExExp(frame::ExternalCall("init_array", expList));
  return new tr::ExpAndTy(exp, ty);
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(nullptr, type::VoidTy::Instance());
}

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  for (auto &it : functions_->GetList()) {
    std::list<bool> escapes;
    type::TyList *tyList = new type::TyList();
    for (auto &iter : it->params_->GetList()) {
      escapes.push_back(iter->escape_);
      type::Ty *ty = tenv->Look(iter->typ_);
      tyList->Append(ty->ActualTy());
    }

    tr::Level *new_level = tr::Level::NewLevel(level, it->name_, escapes);

    type::Ty *result;
    if (it->result_) {
      result = tenv->Look(it->result_);
    }
    else {
      result = type::VoidTy::Instance();
    }
    venv->Enter(it->name_, new env::FunEntry(new_level, it->name_, tyList, result));
  }

  for (auto &it : functions_->GetList()) {
    venv->BeginScope();

    auto ent = (env::FunEntry *)venv->Look(it->name_);
    std::list<type::Ty *> formaltys = ent->formals_->GetList();
    auto ty_it = formaltys.begin();

    std::list<frame::Access *> formalaccs = ent->level_->frame_->formals->GetList();
    auto acc_it = formalaccs.begin();

    for (auto& field : it->params_->GetList()) {
      venv->Enter(field->name_, new env::VarEntry(new tr::Access(ent->level_, *acc_it), *ty_it));
      ++ty_it;
      ++acc_it;
    };

    auto res = it->body_->Translate(venv, tenv, ent->level_, ent->label_, errormsg);
    venv->EndScope();

    tree::MoveStm *stm = new tree::MoveStm(new tree::TempExp(reg_manager->ReturnValue()), res->exp_->UnEx());
    tree::Stm* proc = ent->level_->frame_->ProcEntryExit1(stm);
    frags->PushBack(new frame::ProcFrag(proc ,ent->level_->frame_));
  }

  return new tr::ExExp(new tree::ConstExp(0));
}

tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto ires = init_->Translate(venv, tenv, level, label, errormsg);

  tr::Access *access = tr::Access::AllocLocal(level, escape_);
  venv->Enter(var_, new env::VarEntry(access, ires->ty_));

  return new tr::NxExp(new tree::MoveStm(access->access_->ToExp(new tree::TempExp(reg_manager->FramePointer())), ires->exp_->UnEx()));
}

tr::Exp *TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, temp::Label *label,
                            err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  for (auto &it : types_->GetList()) {
    tenv->Enter(it->name_, new type::NameTy(it->name_, nullptr));
  }

  for (auto &it : types_->GetList()) {
    type::NameTy *nameTy = (type::NameTy *)tenv->Look(it->name_);
    nameTy->ty_ = it->ty_->Translate(tenv, errormsg);
  }
  
  return new tr::ExExp(new tree::ConstExp(0));
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty *ty = tenv->Look(name_);
  return new type::NameTy(name_, ty);
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto fieldList = new type::FieldList();

  for (auto &it : record_->GetList()) {
    type::Ty *ty = tenv->Look(it->typ_);
    fieldList->Append(new type::Field(it->name_, ty));
  }

  return new type::RecordTy(fieldList);
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty *ty = tenv->Look(array_);
  return new type::ArrayTy(ty);
}

} // namespace absyn
