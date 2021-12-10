#include "tiger/codegen/codegen.h"

#include <cassert>
#include <sstream>

#define DEBUG

#ifdef DEBUG
  #define LOG(format, args...) do {   \
    FILE* debug_log = fopen("codegen.log", "a+");  \
    fprintf(debug_log, "%d, %s:", __LINE__, __func__); \
    fprintf(debug_log, format, ##args); \
    fclose(debug_log);\
  } while (0)
#else
  #define LOG(format, args...) do{} while (0)
#endif

extern frame::RegManager *reg_manager;

namespace {

constexpr int maxlen = 1024;

// static std::string frameName;

static temp::Temp* saved_rbx = NULL;
static temp::Temp* saved_rbp = NULL;
static temp::Temp* saved_r12 = NULL;
static temp::Temp* saved_r13 = NULL;
static temp::Temp* saved_r14 = NULL;
static temp::Temp* saved_r15 = NULL;

} // namespace

namespace cg {

void CodeGen::Codegen() {
  LOG("~~~~~~CodeGen: Start~~~~~~\n");
  /* TODO: Put your lab5 code here */
  fs_ = frame_->label->Name();

  assem::InstrList* instr_list = new assem::InstrList();
  saveCalleeRegs(*instr_list, fs_);
  
  std::list<tree::Stm *> trace_list = traces_->GetStmList()->GetList();
  for (auto trace : trace_list)
  {
    assert(trace != NULL);
    trace->Munch(*instr_list, fs_);
  }

  restoreCalleeRegs(*instr_list, fs_);
  LOG("~~~~~~CodeGen: Almost Finish~~~~~~\n");
  assem_instr_ = std::make_unique<cg::AssemInstr>(frame::ProcEntryExit2(instr_list));
  LOG("~~~~~~CodeGen: Finish~~~~~~\n");
  return;
};

void CodeGen::saveCalleeRegs(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start saveCalleeRegs\n");
  saved_rbx = temp::TempFactory::NewTemp(); assert(saved_rbx);
  saved_rbp = temp::TempFactory::NewTemp(); assert(saved_rbp);
  saved_r12 = temp::TempFactory::NewTemp(); assert(saved_r12);
  saved_r13 = temp::TempFactory::NewTemp(); assert(saved_r13);
  saved_r14 = temp::TempFactory::NewTemp(); assert(saved_r14);
  saved_r15 = temp::TempFactory::NewTemp(); assert(saved_r15);
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_rbx), new temp::TempList(reg_manager->RBX())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_rbp), new temp::TempList(reg_manager->RBP())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_r12), new temp::TempList(reg_manager->R12())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_r13), new temp::TempList(reg_manager->R13())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_r14), new temp::TempList(reg_manager->R14())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_r15), new temp::TempList(reg_manager->R15())));
  LOG("End saveCalleeRegs\n");
  return;
};

void CodeGen::restoreCalleeRegs(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start restoreCalleeRegs");
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RBX()), new temp::TempList(saved_rbx)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RBP()), new temp::TempList(saved_rbp)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->R12()), new temp::TempList(saved_r12)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->R13()), new temp::TempList(saved_r13)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->R14()), new temp::TempList(saved_r14)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->R15()), new temp::TempList(saved_r15)));
  LOG("End restoreCalleeRegs");
  return;
}

void AssemInstr::Print(FILE *out, temp::Map *map) const {
  for (auto instr : instr_list_->GetList())
    instr->Print(out, map);
  fprintf(out, "\n");
}
} // namespace cg

namespace tree {
/* TODO: Put your lab5 code here */

void SeqStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start SeqStm\n");
  /* TODO: Put your lab5 code here */
  left_->Munch(instr_list, fs);
  right_->Munch(instr_list, fs);
  LOG("End SeqStm\n");
  return;
}

void LabelStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start LabelStm : label: %s\n", this->label_->Name().c_str());
  /* TODO: Put your lab5 code here */
  instr_list.Append(new assem::LabelInstr(temp::LabelFactory::LabelString(label_), label_));
  LOG("End LabelStm: Label: %s\n", this->label_->Name().c_str());
  return;
}

void JumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start JumpStm : nameLabel: %s\n", this->exp_->name_->Name().c_str());
  /* TODO: Put your lab5 code here */
  instr_list.Append(new assem::OperInstr("jmp `j0", NULL, NULL, new assem::Targets(jumps_)));
  LOG("End JumpStm : nameLabel: %s\n", this->exp_->name_->Name().c_str());
  return;
}

void CjumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start CjumpStm\n");
  /* TODO: Put your lab5 code here */
  // temp::Temp* left = left_->Munch(instr_list, fs);
  // temp::Temp* right = right_->Munch(instr_list, fs);

  // temp::TempList* right_left_list = new temp::TempList(right);
  // right_left_list->Append(left);

  // instr_list.Append(new assem::OperInstr("cmpq `s0, `s1", NULL, right_left_list, NULL));

  // std::string str = "";
  // switch (op_)
  // {
  // case tree::RelOp::EQ_OP: str = std::string("je"); break;
  // case tree::RelOp::NE_OP: str = std::string("jne"); break;
  // case tree::RelOp::LT_OP: str = std::string("jl"); break;
  // case tree::RelOp::GT_OP: str = std::string("jg"); break;
  // case tree::RelOp::LE_OP: str = std::string("jle"); break;
  // case tree::RelOp::GE_OP: str = std::string("jge "); break;
  // default:
  //   break;
  // };
  // std::vector<temp::Label *> *true_labels = new std::vector<temp::Label*>();
  // true_labels->push_back(true_label_);
  // str = str.append(temp::LabelFactory::LabelString(true_label_));
  // instr_list.Append(new assem::OperInstr(str, NULL, NULL, new assem::Targets(true_labels)));
  temp::Temp* e1temp = left_->Munch(instr_list, fs);
  temp::Temp* e2temp = right_->Munch(instr_list, fs);
  std::string str = "";
  switch (op_)
  {
  case tree::RelOp::EQ_OP: str = std::string("je"); break;
  case tree::RelOp::NE_OP: str = std::string("jne"); break;
  case tree::RelOp::LT_OP: str = std::string("jl"); break;
  case tree::RelOp::GT_OP: str = std::string("jg"); break;
  case tree::RelOp::LE_OP: str = std::string("jle"); break;
  case tree::RelOp::GE_OP: str = std::string("jge "); break;
  default:
    break;
  };
  std::vector<temp::Label *> *labels = new std::vector<temp::Label *>();
  labels->push_back(true_label_);
  labels->push_back(false_label_);
  instr_list.Append(new assem::OperInstr("cmpq `s0, `s1", NULL, new temp::TempList({e2temp, e1temp}), NULL));
  instr_list.Append(new assem::OperInstr(str + " `j0", NULL, NULL, new assem::Targets(labels)));
  LOG("End CjumpStm\n");
  return;
}

void MoveStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start MoveStm\n");
  /* TODO: Put your lab5 code here */
  if (dst_->kind_ == tree::Exp::Kind::TEMP) {
    temp::Temp* left = src_->Munch(instr_list, fs); assert(left);
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(((tree::TempExp *)dst_)->temp_), new temp::TempList(left)));
  } else if (dst_->kind_ == tree::Exp::Kind::MEM) {
    temp::Temp* left = src_->Munch(instr_list, fs); assert(left);
    temp::Temp* right = ((tree::MemExp *) dst_)->exp_->Munch(instr_list, fs); assert(right);

    temp::TempList* left_right_list = new temp::TempList(left);
    left_right_list->Append(right);

    instr_list.Append(new assem::OperInstr("movq `s0, (`s1)", NULL, left_right_list, NULL));
  } else {
    assert(0);
  };
  LOG("End MoveStm\n");
  return;
  // if (dst_->kind_ == tree::Exp::Kind::MEM) {
  //   tree::MemExp* memDst = (tree::MemExp*) dst_;
  //   if (memDst->exp_->kind_ == tree::Exp::Kind::BINOP && ((tree::BinopExp*) memDst->exp_)->op_ == tree::BinOp::PLUS_OP && ((tree::BinopExp*) memDst->exp_)->right_->kind_ == tree::Exp::Kind::CONST) {
  //     tree::Exp* e1 = ((tree::BinopExp*) memDst->exp_)->left_, *e2 = src_;
  //     temp::Temp* e1temp = e1->Munch(instr_list, fs);
  //     temp::Temp* e2temp = e2->Munch(instr_list, fs);
  //     std::stringstream stream;
  //     if (e1temp == reg_manager->RBP()) {
  //       stream << "movq `s0, (" << fs << "_framesize" << ((tree::ConstExp*) ((tree::BinopExp*) memDst->exp_)->right_)->consti_ << ")(`s1)";
  //       std::string assem = stream.str();
  //       instr_list.Append(new assem::OperInstr(assem, NULL, new temp::TempList({e2temp, reg_manager->RSP()}), NULL));
  //     } else {
  //       stream << "movq `s0, " << ((tree::ConstExp*) ((tree::BinopExp*) memDst->exp_)->right_)->consti_ << "(`s1)";
  //       std::string assem = stream.str();
  //       instr_list.Append(new assem::OperInstr(assem, NULL, new temp::TempList({e2temp, e1temp}), NULL));
  //     };
  //   } else if (memDst->exp_->kind_ == tree::Exp::Kind::BINOP && ((tree::BinopExp*) memDst->exp_)->op_ == tree::BinOp::PLUS_OP && ((tree::BinopExp*) memDst->exp_)->left_->kind_ == tree::Exp::Kind::CONST) {
  //     tree::Exp* e1 = ((tree::BinopExp*) memDst->exp_)->right_, *e2 = src_;
  //     temp::Temp* e1temp = e1->Munch(instr_list, fs);
  //     temp::Temp* e2temp = e2->Munch(instr_list, fs);
  //     std::stringstream stream;
  //     assert(e1temp != reg_manager->RBP());
  //     assert(e2temp != reg_manager->RBP());
  //     stream << "movq `s0, " << ((tree::ConstExp*) ((tree::BinopExp*) memDst->exp_)->left_)->consti_ << "(`s1)";
  //     std::string assem = stream.str();
  //     instr_list.Append(new assem::OperInstr(assem, NULL, new temp::TempList({e2temp, e1temp}), NULL));
  //   } else if (src_->kind_ == tree::Exp::Kind::MEM) {
  //     tree::Exp* e1 = memDst->exp_, *e2 = ((tree::MemExp*) src_)->exp_;
  //     temp::Temp* t = temp::TempFactory::NewTemp();
  //     temp::Temp* e1temp = e1->Munch(instr_list, fs);
  //     temp::Temp* e2temp = e2->Munch(instr_list, fs);
  //     assert(e1temp != reg_manager->RBP());
  //     assert(e2temp != reg_manager->RBP());
  //     instr_list.Append(new assem::OperInstr("movq (`s0), `d0", new temp::TempList(t), new temp::TempList(e2temp), NULL));
  //     instr_list.Append(new assem::OperInstr("movq `s0, (`s1)", NULL, new temp::TempList({t, e1temp}), NULL));
  //   } else if (memDst->kind_ == tree::Exp::Kind::CONST) {
  //     tree::Exp* e2 = src_;
  //     temp::Temp* e2temp = e2->Munch(instr_list, fs);
  //     assert(e2temp != reg_manager->RBP());
  //     instr_list.Append(new assem::OperInstr("movq (some const), `s0", NULL, new temp::TempList(e2temp), NULL));
  //   } else {
  //     tree::Exp* e1 = memDst->exp_, *e2 = src_;
  //     temp::Temp* e1temp = e1->Munch(instr_list, fs);
  //     temp::Temp* e2temp = e2->Munch(instr_list, fs);
  //     assert(e1temp != reg_manager->RBP());
  //     assert(e2temp != reg_manager->RBP());
  //     instr_list.Append(new assem::OperInstr("movq `s0, (`s1)", NULL, new temp::TempList({e2temp, e1temp}), NULL));
  //   };
  // } else if (dst_->kind_ == tree::Exp::Kind::TEMP) {
  //   tree::Exp *e2 = src_;
  //   temp::Temp* e2temp = e2->Munch(instr_list, fs);
  //   if (e2temp == reg_manager->RBP()) {
  //     std::stringstream stream;
  //     stream << "movq " << fs << "_framesize(`s0), `d0";
  //     std::string assem = stream.str();
  //     instr_list.Append(new assem::OperInstr(assem, new temp::TempList(((tree::TempExp*) dst_)->temp_), new temp::TempList(reg_manager->RSP()), NULL));
  //   } else {
  //     instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(((tree::TempExp*) dst_)->temp_), new temp::TempList(e2temp)));
  //   }
  // }
  // LOG("End MoveStm\n");
  // return;
}

void ExpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start ExpStm\n");
  /* TODO: Put your lab5 code here */
  // exp_->Munch(instr_list, fs);
  // return;
  exp_->Munch(instr_list, fs);
  LOG("End ExpStm\n");
  return;
}

temp::Temp *BinopExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start BinopExp\n");
  /* TODO: Put your lab5 code here */
  // temp::Temp* reg = temp::TempFactory::NewTemp();
  // switch (op_)
  // {
  // case tree::PLUS_OP:
  // {
  //   temp::Temp* left = left_->Munch(instr_list, fs);
  //   temp::Temp* right = right_->Munch(instr_list, fs);
  //   instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg), new temp::TempList(left)));
  //   instr_list.Append(new assem::OperInstr("addq `s0, `d0", new temp::TempList(reg), new temp::TempList({right, reg}), NULL));
  //   return reg;
  // }
  // case tree::MINUS_OP:
  // {
  //   temp::Temp* left = left_->Munch(instr_list, fs);
  //   temp::Temp* right = right_->Munch(instr_list, fs);
  //   instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg), new temp::TempList(left)));
  //   instr_list.Append(new assem::OperInstr("subq `s0, `d0", new temp::TempList(reg), new temp::TempList({right, reg}), NULL));
  //   return reg;
  // }
  // case tree::MUL_OP:
  // {
  //   temp::Temp* left = left_->Munch(instr_list, fs);
  //   temp::Temp* right = right_->Munch(instr_list, fs);
  //   instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg), new temp::TempList(left)));
  //   instr_list.Append(new assem::OperInstr("imulq `s0, `d0", new temp::TempList(reg), new temp::TempList({right, reg}), NULL));
  //   return reg;
  // }
  // case tree::DIV_OP:
  // {
  //   temp::Temp* left = left_->Munch(instr_list, fs);
  //   temp::Temp* right = right_->Munch(instr_list, fs);
  //   instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RAX()), new temp::TempList(left)));
  //   instr_list.Append(new assem::OperInstr("cltd", new temp::TempList( { reg_manager->RAX(), reg_manager->RDX() } ), new temp::TempList( reg_manager->RAX() ), NULL));

  //   instr_list.Append(new assem::OperInstr("idivq `s0", new temp::TempList( { reg_manager->RAX(), reg_manager->RDX() } ), new temp::TempList( { right, reg_manager->RAX(), reg_manager->RDX() } ), NULL));
  //   instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg), new temp::TempList(reg_manager->RAX())));
  //   return reg;
  // }
  // default:
  //   break;
  // }
  temp::Temp* r = temp::TempFactory::NewTemp();
  switch (op_)
  {
  case tree::PLUS_OP: {
    tree::Exp* e1 = left_;
    tree::Exp* e2 = right_;
    temp::Temp* e1temp = e1->Munch(instr_list, fs);
    temp::Temp* e2temp = e2->Munch(instr_list, fs);
    if (e1temp == reg_manager->RBP()) {
      std::stringstream stream;
      stream << "leaq " << fs << "_framesize(`s0), `d0";
      std::string assem = stream.str();
      instr_list.Append(new assem::OperInstr(assem, new temp::TempList(r), new temp::TempList(reg_manager->RSP()), NULL));
    } else {
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(r), new temp::TempList(e1temp)));
    };
    instr_list.Append(new assem::OperInstr("addq `s0, `d0", new temp::TempList(r), new temp::TempList({e2temp, r}), NULL));
    break;
  }
  case tree::MINUS_OP: {
    tree::Exp* e1 = left_;
    tree::Exp* e2 = right_;
    temp::Temp* e1temp = e1->Munch(instr_list, fs);
    temp::Temp* e2temp = e2->Munch(instr_list, fs);
    if (e1temp == reg_manager->RBP()) {
      std::stringstream stream;
      stream << "leaq " << fs << "_framesize(`s0), `d0";
      std::string assem = stream.str();
      instr_list.Append(new assem::OperInstr(assem, new temp::TempList(r), new temp::TempList(reg_manager->RSP()), NULL));
    } else {
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(r), new temp::TempList(e1temp)));
    };
    instr_list.Append(new assem::OperInstr("subq `s0, `d0", new temp::TempList(r), new temp::TempList({e2temp, r}), NULL));
    break;
  }
  case tree::MUL_OP: {
    tree::Exp* e1 = left_;
    tree::Exp* e2 = right_;
    temp::Temp* e1temp = e1->Munch(instr_list, fs);
    temp::Temp* e2temp = e2->Munch(instr_list, fs);
    assert(e1temp != reg_manager->RBP());
    assert(e2temp != reg_manager->RBP());
    // instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(r), new temp::TempList(e1temp)));
    // instr_list.Append(new assem::OperInstr("imulq `s0, `d0", new temp::TempList(r), new temp::TempList({e2temp, r}), NULL));
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RAX()), new temp::TempList(e1temp)));
    instr_list.Append(new assem::OperInstr("imulq `s0", new temp::TempList(reg_manager->RAX()), new temp::TempList(e2temp), NULL));
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(r), new temp::TempList(reg_manager->RAX())));
    break;
  }
  case tree::DIV_OP: {
    tree::Exp* e1 = left_;
    tree::Exp* e2 = right_;
    temp::Temp* e1temp = e1->Munch(instr_list, fs);
    temp::Temp* e2temp = e2->Munch(instr_list, fs);
    assert(e1temp != reg_manager->RBP());
    assert(e2temp != reg_manager->RBP());
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RAX()), new temp::TempList(e1temp)));
    instr_list.Append(new assem::OperInstr("cqto", new temp::TempList({reg_manager->RDX(), reg_manager->RAX()}), new temp::TempList(reg_manager->RAX()), NULL));
    instr_list.Append(new assem::OperInstr("idivq `s0", new temp::TempList({reg_manager->RDX(), reg_manager->RAX()}), new temp::TempList(e2temp), NULL));
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(r), new temp::TempList(reg_manager->RAX())));
    break;
  }
  default:
    break;
  }
  LOG("End BinOpExp\n");
  return r;
}

temp::Temp *MemExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start MemExp\n");
  /* TODO: Put your lab5 code here */
  temp::Temp* reg = temp::TempFactory::NewTemp();
  instr_list.Append(new assem::OperInstr("movq (`s0), `d0", new temp::TempList(reg), new temp::TempList(exp_->Munch(instr_list, fs)), NULL));
  LOG("End MemExp\n");  
  return reg;
  // temp::Temp* r = temp::TempFactory::NewTemp();
  // if (exp_->kind_ == tree::Exp::Kind::BINOP && ((tree::BinopExp*) exp_)->op_ == tree::PLUS_OP && ((tree::BinopExp*) exp_)->right_->kind_ == tree::Exp::Kind::CONST) {
  //   tree::Exp* e1 = ((tree::BinopExp*) exp_)->left_;
  //   temp::Temp* e1temp = e1->Munch(instr_list, fs);
  //   std::stringstream stream;
  //   if (e1temp == reg_manager->RBP()) {
  //     stream << "movq (" << fs << "_framesize" << ((tree::ConstExp*)(((tree::BinopExp*) exp_)->right_))->consti_ << ")(`s0), `d0";
  //     std::string assem = stream.str();
  //     instr_list.Append(new assem::OperInstr(assem, new temp::TempList(r), new temp::TempList(reg_manager->RSP()), NULL));
  //   } else {
  //     stream << "movq " << ((tree::ConstExp*)(((tree::BinopExp*) exp_)->right_))->consti_ << "(`s0), `d0";
  //     std::string assem = stream.str();
  //     instr_list.Append(new assem::OperInstr(assem, new temp::TempList(r), new temp::TempList(e1temp), NULL));
  //   };
  // } else if (exp_->kind_ == tree::Exp::Kind::BINOP && ((tree::BinopExp*) exp_)->op_ == tree::PLUS_OP && ((tree::BinopExp*) exp_)->left_->kind_ == tree::Exp::Kind::CONST) {
  //   tree::Exp* e1 = ((tree::BinopExp*) exp_)->right_;
  //   temp::Temp* e1temp = e1->Munch(instr_list, fs);
  //   std::stringstream stream;
  //   if (e1temp == reg_manager->RBP()) {
  //     stream << "movq (" << fs << "_framesize" << ((tree::ConstExp*)(((tree::BinopExp*) exp_)->left_))->consti_ << ")(`s0), `d0";
  //     std::string assem = stream.str();
  //     instr_list.Append(new assem::OperInstr(assem, new temp::TempList(r), new temp::TempList(reg_manager->RSP()), NULL));
  //   } else {
  //     stream << "movq " << ((tree::ConstExp*)(((tree::BinopExp*) exp_)->left_))->consti_ << "(`s0), `d0";
  //     std::string assem = stream.str();
  //     instr_list.Append(new assem::OperInstr(assem, new temp::TempList(r), new temp::TempList(e1temp), NULL));
  //   };
  // } else if (exp_->kind_ == tree::Exp::Kind::CONST) {
  //   instr_list.Append(new assem::OperInstr("movq111", new temp::TempList(r), NULL, NULL));
  // } else {
  //   tree::Exp* e1 = exp_;
  //   temp::Temp* e1temp = e1->Munch(instr_list, fs);
  //   assert(e1temp != reg_manager->RBP());
  //   instr_list.Append(new assem::OperInstr("movq (`s0), `d0", new temp::TempList(r), new temp::TempList(e1temp), NULL));
  // };
  // LOG("End MemExp\n");
  // return r;
}

temp::Temp *TempExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start TempExp\n");
  temp::Temp* res = nullptr;
  if (temp_ != reg_manager->FramePointer()) res = temp_;
  else {
    temp::Temp* reg = temp::TempFactory::NewTemp();
    std::stringstream stream;
    stream << "leaq " << fs << "_framesize(`s0), `d0";
    std::string assem = stream.str();
    instr_list.Append(new assem::OperInstr(assem, new temp::TempList(reg), new temp::TempList(reg_manager->StackPointer()), NULL));
    res = reg;
  }
  assert(res != NULL);
  LOG("End TempExp\n");
  return res;
}

temp::Temp *EseqExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start EseqExp\n");
  /* TODO: Put your lab5 code here */
  assert(stm_ && exp_);
  stm_->Munch(instr_list, fs);
  temp::Temp* res = exp_->Munch(instr_list, fs);
  LOG("End EseqExp\n");
  return res;
};

temp::Temp *NameExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start NameExp\n");
  /* TODO: Put your lab5 code here */
  temp::Temp* r = temp::TempFactory::NewTemp();
  std::stringstream stream;
  stream << "leaq " << temp::LabelFactory::LabelString(name_) << "(%rip), `d0";
  std::string assem = stream.str();
  instr_list.Append(new assem::OperInstr(assem, new temp::TempList(r), NULL, NULL));
  LOG("End NameExp\n");
  return r;
}

temp::Temp *ConstExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start ConstExp : %d\n", this->consti_);
  /* TODO: Put your lab5 code here */
  temp::Temp* r = temp::TempFactory::NewTemp();
  std::stringstream stream;
  stream << "movq $" << consti_ << ", `d0";
  std::string assem = stream.str();
  instr_list.Append(new assem::OperInstr(assem, new temp::TempList(r), NULL, NULL));
  LOG("End ConstExp : %d\n", this->consti_);
  return r;
}

temp::Temp *CallExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start CallExp\n");
  /* TODO: Put your lab5 code here */
  temp::Temp* r = temp::TempFactory::NewTemp();
  std::string label = temp::LabelFactory::LabelString(((tree::NameExp*) fun_)->name_);
  args_->MunchArgs(instr_list, fs);
  std::string assem = std::string("callq ") + std::string(label);
  instr_list.Append(new assem::OperInstr(assem, reg_manager->CallerSaves(), reg_manager->ArgRegs(), NULL));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(r), new temp::TempList(reg_manager->RAX())));
  LOG("End CallExp : \n");
  return r;
}

temp::TempList *ExpList::MunchArgs(assem::InstrList &instr_list, std::string_view fs) {
  LOG("Start TempList\n");
  /* TODO: Put your lab5 code here */
  int i = 0;
  temp::TempList* reg_list = new temp::TempList();
  for (auto exp : exp_list_) {
    temp::Temp* arg = exp->Munch(instr_list, fs);
    i++;
    if (i <= 6) { /* using register as possible */
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->GetNthArg(i)), new temp::TempList(arg)));
    } else {
      assert(0); /* no thinking more than 6 parameters */
    }
    reg_list->Append(arg);
  };
  LOG("End TempList\n");
  return reg_list;
}

} // namespace tree
