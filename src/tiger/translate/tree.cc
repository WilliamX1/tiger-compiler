#include "tiger/translate/tree.h"

#include <cassert>
#include <cstdio>

namespace {

void Indent(FILE *out, int d) {
  for (int i = 0; i <= d; i++)
    fprintf(out, " ");
}

} // namespace

namespace tree {

SeqStm::~SeqStm() {
  delete left_;
  delete right_;
}

LabelStm::~LabelStm() = default;

JumpStm::~JumpStm() { delete exp_; }

CjumpStm::~CjumpStm() {
  delete left_;
  delete right_;
}

MoveStm::~MoveStm() {
  delete dst_;
  delete src_;
}

ExpStm::~ExpStm() { delete exp_; }

BinopExp::~BinopExp() {
  delete left_;
  delete right_;
}

MemExp::~MemExp() { delete exp_; }

TempExp::~TempExp() = default;

EseqExp::~EseqExp() {
  delete stm_;
  delete exp_;
}

NameExp::~NameExp() = default;

ConstExp::~ConstExp() = default;

CallExp::~CallExp() {
  delete fun_;
  delete args_;
}

void SeqStm::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "SEQ(\n");
  left_->Print(out, d + 1);
  fprintf(out, ",\n");
  right_->Print(out, d + 1);
  fprintf(out, ")");
}

void LabelStm::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "LABEL %s", label_->Name().data());
}

void JumpStm::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "JUMP(\n");
  exp_->Print(out, d + 1);
  fprintf(out, ")");
}

void CjumpStm::Print(FILE *out, int d) const {
  static std::array<std::string_view, tree::REL_OPER_COUNT> rel_oper = {
      "EQ", "NE", "LT", "GT", "LE", "GE", "ULT", "ULE", "UGT", "UGE"};
  Indent(out, d);
  fprintf(out, "CJUMP(%s,\n", rel_oper[op_].data());
  left_->Print(out, d + 1);
  fprintf(out, ",\n");
  right_->Print(out, d + 1);
  fprintf(out, ",\n");
  Indent(out, d + 1);
  fprintf(out, "%s,", true_label_->Name().data());
  fprintf(out, "%s", false_label_->Name().data());
  fprintf(out, ")");
}

void MoveStm::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "MOVE(\n");
  dst_->Print(out, d + 1);
  fprintf(out, ",\n");
  src_->Print(out, d + 1);
  fprintf(out, ")");
}

void ExpStm::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "EXP(\n");
  exp_->Print(out, d + 1);
  fprintf(out, ")");
}

void BinopExp::Print(FILE *out, int d) const {
  static std::array<std::string_view, tree::BIN_OPER_COUNT> bin_oper = {
      "PLUS", "MINUS",  "TIMES",  "DIVIDE",  "AND",
      "OR",   "LSHIFT", "RSHIFT", "ARSHIFT", "XOR"};
  Indent(out, d);
  fprintf(out, "BINOP(%s,\n", bin_oper[op_].data());
  left_->Print(out, d + 1);
  fprintf(out, ",\n");
  right_->Print(out, d + 1);
  fprintf(out, ")");
}

void MemExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "MEM");
  fprintf(out, "(\n");
  exp_->Print(out, d + 1);
  fprintf(out, ")");
}

void TempExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "temp %s", temp::Map::Name()->Look(temp_)->data());
}

void EseqExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "ESEQ(\n");
  stm_->Print(out, d + 1);
  fprintf(out, ",\n");
  exp_->Print(out, d + 1);
  fprintf(out, ")");
}

void NameExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "NAME %s", name_->Name().data());
}

void ConstExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "CONST %d", consti_);
}

void CallExp::Print(FILE *out, int d) const {
  Indent(out, d);
  fprintf(out, "CALL(\n");
  fun_->Print(out, d + 1);
  for (auto arg : args_->GetList()) {
    fprintf(out, ",\n");
    arg->Print(out, d + 2);
  }
  fprintf(out, ")");
}

void StmList::Print(FILE *out) const {
  for (auto stm : stm_list_) {
    stm->Print(out, 0);
    fprintf(out, "\n");
  }
}

RelOp NotRel(RelOp r) {
  switch (r) {
  case EQ_OP:
    return NE_OP;
  case NE_OP:
    return EQ_OP;
  case LT_OP:
    return GE_OP;
  case GE_OP:
    return LT_OP;
  case GT_OP:
    return LE_OP;
  case LE_OP:
    return GT_OP;
  case ULT_OP:
    return UGE_OP;
  case UGE_OP:
    return ULT_OP;
  case ULE_OP:
    return UGT_OP;
  case UGT_OP:
    return ULE_OP;
  default:
    assert(false);
  }
}

RelOp Commute(RelOp r) {
  switch (r) {
  case EQ_OP:
    return EQ_OP;
  case NE_OP:
    return NE_OP;
  case LT_OP:
    return GT_OP;
  case GE_OP:
    return LE_OP;
  case GT_OP:
    return LT_OP;
  case LE_OP:
    return GE_OP;
  case ULT_OP:
    return UGT_OP;
  case UGE_OP:
    return ULE_OP;
  case ULE_OP:
    return UGE_OP;
  case UGT_OP:
    return ULT_OP;
  default:
    assert(false);
  }
}

} // namespace tree
