#include "tiger/canon/canon.h"

namespace tree {
/**
 * Gets rid of the top-level SEQ's, producing a list
 * @param stm current statement
 */
void StmList::Linear(tree::Stm *stm) {
  if (typeid(*stm) == typeid(tree::SeqStm)) {
    auto seqstm = static_cast<tree::SeqStm *>(stm);
    Linear(seqstm->left_);
    Linear(seqstm->right_);
  } else {
    stm_list_.push_back(stm);
  }
}

bool Stm::IsNop() {
  if (typeid(*this) == typeid(tree::ExpStm)) {
    auto exp = static_cast<tree::ExpStm *>(this)->exp_;
    return typeid(*exp) == typeid(tree::ConstExp);
  } else
    return false;
}

Stm *Stm::Seq(tree::Stm *x, tree::Stm *y) {
  if (x->IsNop())
    return y;
  if (y->IsNop())
    return x;
  return new tree::SeqStm(x, y);
}

bool Stm::Commute(tree::Stm *x, tree::Exp *y) {
  if (x->IsNop())
    return true;
  if (typeid(*y) == typeid(tree::NameExp) ||
      typeid(*y) == typeid(tree::ConstExp))
    return true;
  return false;
}

} // namespace tree
namespace {

struct ExpRefList {
  std::list<std::reference_wrapper<tree::Exp *>> refs;

  ExpRefList() = delete;

  explicit ExpRefList(tree::Exp *&exp) { refs.push_front(exp); }

  ExpRefList(tree::Exp *&exp1, tree::Exp *&exp2) {
    refs.push_back(exp1);
    refs.push_back(exp2);
  }
  ExpRefList(tree::Exp *&head, std::list<tree::Exp *>::iterator begin,
             std::list<tree::Exp *>::iterator end)
      : refs(begin, end) {
    refs.push_front(head);
  }

  tree::Stm *Reorder() {
    if (refs.empty()) {
      return new tree::ExpStm(new tree::ConstExp(0)); // nop
    } else {
      tree::Exp *&ref = refs.front().get();
      if (typeid(*ref) == typeid(tree::CallExp)) {
        temp::Temp *t = temp::TempFactory::NewTemp();
        ref = new tree::EseqExp(new tree::MoveStm(new tree::TempExp(t), ref),
                                new tree::TempExp(t));
        return Reorder();
      } else {
        canon::StmAndExp hd = ref->Canon();
        refs.pop_front();
        tree::Stm *s = Reorder();
        if (tree::Stm::Commute(s, hd.e_)) {
          ref = hd.e_;
          return tree::Stm::Seq(hd.s_, s);
        } else {
          temp::Temp *t = temp::TempFactory::NewTemp();
          ref = new tree::TempExp(t);
          return tree::Stm::Seq(
              hd.s_, tree::Stm::Seq(
                         new tree::MoveStm(new tree::TempExp(t), hd.e_), s));
        }
      }
    }
  }
};

ExpRefList *GetCallRlist(tree::Exp *exp) {
  auto callexp = dynamic_cast<tree::CallExp *>(exp);
  assert(callexp);
  tree::ExpList *args = callexp->args_;
  auto *rlist = new ExpRefList(callexp->fun_, args->GetNonConstList().begin(),
                               args->GetNonConstList().end());
  return rlist;
}

} // namespace

namespace canon {

void Canon::Trace(std::list<tree::Stm *> &stms) {
  tree::Stm *last = stms.back();

  auto lab = dynamic_cast<tree::LabelStm *>(stms.front());
  assert(lab);
  block_env_->Enter(lab->label_, nullptr);

  if (typeid(*last) == typeid(tree::JumpStm)) {
    auto jumpstm = static_cast<tree::JumpStm *>(last);
    auto target = block_env_->Look(jumpstm->jumps_->front());
    if (target) {
      Trace(target->stm_list_);
      stms.pop_back();
      stms.insert(
          stms.end(), target->stm_list_.begin(),
          target->stm_list_.end()); // merge the 2 lists removing JUMP stm_
    } else {
      auto insert = GetNext()->stm_list_;
      stms.insert(stms.end(), insert.begin(),
                  insert.end()); // merge and keep JUMP stm_
    }
  } else if (typeid(*last) == typeid(tree::CjumpStm)) {
    // We want false label_ to follow CJUMP
    auto cjumpstm = static_cast<tree::CjumpStm *>(last);
    auto truelist = block_env_->Look(cjumpstm->true_label_);
    auto falselist = block_env_->Look(cjumpstm->false_label_);
    if (falselist) {
      Trace(falselist->stm_list_);
      stms.insert(stms.end(), falselist->stm_list_.begin(),
                  falselist->stm_list_.end());
    } else if (truelist) { // convert so that existing label_ is a false label_
      stms.pop_back();
      stms.push_back(new tree::CjumpStm(
          tree::NotRel(cjumpstm->op_), cjumpstm->left_, cjumpstm->right_,
          cjumpstm->false_label_, cjumpstm->true_label_));

      Trace(truelist->stm_list_);
      stms.insert(stms.end(), truelist->stm_list_.begin(),
                  truelist->stm_list_.end());
    } else {
      temp::Label *falselabel = temp::LabelFactory::NewLabel();
      stms.pop_back();
      std::list<tree::Stm *> tmp_stm_list = {
          new tree::CjumpStm(cjumpstm->op_, cjumpstm->left_, cjumpstm->right_,
                             cjumpstm->true_label_, falselabel),
          new tree::LabelStm(falselabel),
          new tree::JumpStm(
              new tree::NameExp(cjumpstm->false_label_),
              new std::vector<temp::Label *>({cjumpstm->false_label_}))};
      stms.insert(stms.end(), tmp_stm_list.begin(), tmp_stm_list.end());
      auto insert = GetNext()->stm_list_;
      stms.insert(stms.end(), insert.begin(), insert.end());
    }
  } else
    assert(0);
}

tree::StmList *Canon::GetNext() {
  if (block_.stm_lists_->stmlist_list_.empty()) {
    auto *last_stm_list = new tree::StmList();
    last_stm_list->stm_list_.push_back(new tree::LabelStm(block_.label_));
    return last_stm_list;
  } else {
    tree::StmList *s = block_.stm_lists_->stmlist_list_.front();
    auto lab = dynamic_cast<tree::LabelStm *>(s->stm_list_.front());
    assert(lab);
    if (block_env_->Look(lab->label_)) { // label_ exists in the table
      Trace(s->stm_list_);
      return s;
    } else {
      block_.stm_lists_->stmlist_list_.pop_front();
      return GetNext();
    }
  }
}

tree::StmList *Canon::Linearize() {
  stm_canon_ = new tree::StmList();
  stm_canon_->Linear(stm_ir_->Canon());
  return stm_canon_;
}

canon::StmListList *Canon::BasicBlocks() {
  temp::Label *done = temp::LabelFactory::NewLabel();
  auto *stm_lists = new canon::StmListList();
  auto *cur_list = new tree::StmList();
  bool start = true;
  auto left = stm_canon_->GetList().begin();
  auto right = stm_canon_->GetList().begin();
  for (auto stm : stm_canon_->GetList()) {
    // at the beginning of bb, supposed to create a label_
    if (start) {
      if (typeid(*stm) != typeid(tree::LabelStm)) {
        cur_list->stm_list_.push_front(
            new tree::LabelStm(temp::LabelFactory::NewLabel()));
      }
    }
    if (typeid(*stm) == typeid(tree::JumpStm) ||
        typeid(*stm) == typeid(tree::CjumpStm)) {
      // meet jump stm_, should terminate this bb.
      right++;
      cur_list->stm_list_.insert(cur_list->stm_list_.end(), left, right);
      left = right;
      start = true;
      stm_lists->Append(cur_list);
      cur_list = new tree::StmList();
      continue;
    } else if (typeid(*stm) == typeid(tree::LabelStm) && !start) {
      // meet label_ stm_, should terminate this bb and jump to current label_
      cur_list->stm_list_.insert(cur_list->stm_list_.end(), left, right);
      left = right;
      auto label = static_cast<tree::LabelStm *>(stm)->label_;
      cur_list->stm_list_.push_back(new tree::JumpStm(
          new tree::NameExp(label), new std::vector<temp::Label *>({label})));
      stm_lists->Append(cur_list);
      cur_list = new tree::StmList();
    }
    ++right;
    start = false;
  }
  cur_list->stm_list_.insert(cur_list->stm_list_.end(), left, right);
  cur_list->stm_list_.push_back(new tree::JumpStm(
      new tree::NameExp(done), new std::vector<temp::Label *>({done})));
  stm_lists->Append(cur_list);
  block_ = Block(done, stm_lists);
  return block_.stm_lists_;
}

tree::StmList *Canon::TraceSchedule() {
  tree::StmList *stm_traces;
  for (auto stm_list : block_.stm_lists_->stmlist_list_) {
    auto lab = dynamic_cast<tree::LabelStm *>(stm_list->stm_list_.front());
    assert(lab);
    block_env_->Enter(lab->label_, stm_list);
  }

  stm_traces = GetNext();
  traces_ = std::make_unique<Traces>(stm_traces);
  return stm_traces;
}

Traces::~Traces() { delete stm_list_; }
} // namespace canon

namespace tree {

#define NOP (new tree::ExpStm(new tree::ConstExp(0)))

Stm *SeqStm::Canon() { return tree::Stm::Seq(left_->Canon(), right_->Canon()); }

Stm *LabelStm::Canon() { return this; }

Stm *JumpStm::Canon() {
  return tree::Stm::Seq(ExpRefList((tree::Exp *&)exp_).Reorder(), this);
}

Stm *CjumpStm::Canon() {
  return tree::Stm::Seq(ExpRefList(left_, right_).Reorder(), this);
}

Stm *MoveStm::Canon() {
  // RefList rlist;
  if (typeid(*dst_) == typeid(TempExp) && typeid(*src_) == typeid(CallExp)) {
    return tree::Stm::Seq(GetCallRlist(src_)->Reorder(), this);
  } else if (typeid(*(dst_)) == typeid(TempExp)) {
    return tree::Stm::Seq((new ExpRefList(src_))->Reorder(), this);
  } else if (typeid(*(dst_)) == typeid(MemExp)) {
    auto memexp = static_cast<MemExp *>(dst_);
    return tree::Stm::Seq((ExpRefList(memexp->exp_, src_).Reorder()), this);
  } else if (typeid(*(dst_)) == typeid(EseqExp)) {
    auto eseqexp = static_cast<EseqExp *>(dst_);
    Stm *s = eseqexp->stm_;
    dst_ = eseqexp->exp_;
    return (new SeqStm(s, this))->Canon();
  }
  assert(0); // dst_ should be temp or mem only
}

Stm *ExpStm::Canon() {
  if (typeid(*(exp_)) == typeid(CallExp))
    return tree::Stm::Seq((GetCallRlist(exp_)->Reorder()), this);
  else
    return tree::Stm::Seq(ExpRefList(exp_).Reorder(), this);
}

canon::StmAndExp BinopExp::Canon() {
  return {ExpRefList(left_, right_).Reorder(), this};
}

canon::StmAndExp MemExp::Canon() { return {ExpRefList(exp_).Reorder(), this}; }

canon::StmAndExp TempExp::Canon() { return {NOP, this}; }

canon::StmAndExp EseqExp::Canon() {
  canon::StmAndExp x = exp_->Canon();
  return {tree::Stm::Seq(stm_->Canon(), x.s_), x.e_};
}

canon::StmAndExp NameExp::Canon() { return {NOP, this}; }

canon::StmAndExp ConstExp::Canon() { return {NOP, this}; }

canon::StmAndExp CallExp::Canon() {
  return {GetCallRlist(this)->Reorder(), this};
}

} // namespace tree
