#include "tiger/codegen/codegen.h"

#include <cassert>
#include <sstream>

extern frame::RegManager *reg_manager;

namespace {

constexpr int maxlen = 1024;


} // namespace

namespace cg {

static temp::Temp* saved_rbx = NULL;
static temp::Temp* saved_rbp = NULL;
static temp::Temp* saved_r12 = NULL;
static temp::Temp* saved_r13 = NULL;
static temp::Temp* saved_r14 = NULL;
static temp::Temp* saved_r15 = NULL;

void CodeGen::saveCalleeRegs(assem::InstrList &instr_list, std::string_view fs) {
  saved_rbx = temp::TempFactory::NewTemp();
  saved_rbp = temp::TempFactory::NewTemp();
  saved_r12 = temp::TempFactory::NewTemp();
  saved_r13 = temp::TempFactory::NewTemp();
  saved_r14 = temp::TempFactory::NewTemp();
  saved_r15 = temp::TempFactory::NewTemp();
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_rbx), new temp::TempList(reg_manager->RBX())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_rbp), new temp::TempList(reg_manager->RBP())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_r12), new temp::TempList(reg_manager->R12())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_r13), new temp::TempList(reg_manager->R13())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_r14), new temp::TempList(reg_manager->R14())));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(saved_r15), new temp::TempList(reg_manager->R15())));
};

void CodeGen::restoreCalleeRegs(assem::InstrList &instr_list, std::string_view fs) {
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RBX()), new temp::TempList(saved_rbx)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RBP()), new temp::TempList(saved_rbp)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->R12()), new temp::TempList(saved_r12)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->R13()), new temp::TempList(saved_r13)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->R14()), new temp::TempList(saved_r14)));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->R15()), new temp::TempList(saved_r15)));
}

void CodeGen::Codegen() {
  /* TODO: Put your lab5 code here */
  fs_ = frame_->label->Name();
  auto instr_list = new assem::InstrList();

  saveCalleeRegs(*instr_list, fs_);
  for (auto &it : traces_->GetStmList()->GetList()) {
    it->Munch(*instr_list, fs_);
  }
  restoreCalleeRegs(*instr_list, fs_);

  assem_instr_ = std::make_unique<AssemInstr> (instr_list);
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
  /* TODO: Put your lab5 code here */
  left_->Munch(instr_list, fs);
  right_->Munch(instr_list, fs);
}

void LabelStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  instr_list.Append(new assem::LabelInstr(temp::LabelFactory::LabelString(label_), label_));
}

void JumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  instr_list.Append(new assem::OperInstr(std::string("jmp `j0"), nullptr, nullptr, new assem::Targets(jumps_)));
}

void CjumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto right = right_->Munch(instr_list, fs);
  auto left = left_->Munch(instr_list, fs);
  auto tempList = new temp::TempList();
  tempList->Append(right);
  tempList->Append(left);

  std::string str = "";
  switch(op_){
    case EQ_OP: 
      str = std::string("je ");  
      break;
    case NE_OP: 
      str = std::string("jne "); 
      break;
    case LT_OP: 
      str = std::string("jl ");  
      break;
    case GT_OP: 
      str = std::string("jg ");  
      break;
    case LE_OP: 
      str = std::string("jle "); 
      break;
    case GE_OP: 
      str = std::string("jge "); 
      break;
  }

  auto labelList = new std::vector<temp::Label *>();
  labelList->push_back(true_label_);
  instr_list.Append(new assem::OperInstr("cmpq `s0, `s1", nullptr, new temp::TempList({right, left}), nullptr));
  instr_list.Append(new assem::OperInstr(str + "`j0", nullptr, nullptr, new assem::Targets(labelList)));
}

void MoveStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  if(dst_->kind_ == Exp::TEMP){
    auto left = src_->Munch(instr_list, fs);
    auto tempExp = (TempExp *)dst_;
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(tempExp->temp_), new temp::TempList(left)));
  }
  else if(dst_->kind_ == Exp::MEM){
    auto left = src_->Munch(instr_list, fs);
    auto right = ((MemExp *)dst_)->exp_->Munch(instr_list, fs);
    
    auto tempList = new temp::TempList();
    tempList->Append(left);
    tempList->Append(right);
    instr_list.Append(new assem::OperInstr("movq `s0, (`s1)", nullptr, tempList, nullptr));
  }
}

void ExpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  exp_->Munch(instr_list, fs);
}

temp::Temp *BinopExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *newReg = temp::TempFactory::NewTemp();
  temp::Temp *left;
  temp::Temp *right;
  switch (op_){
    case PLUS_OP:
      left = left_->Munch(instr_list, fs);
      right = right_->Munch(instr_list, fs);
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(newReg), new temp::TempList(left)));
      instr_list.Append(new assem::OperInstr("addq `s0, `d0", new temp::TempList(newReg), new temp::TempList(right), nullptr));
      break;

    case MINUS_OP:
      left = left_->Munch(instr_list, fs);
      right = right_->Munch(instr_list, fs);
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(newReg), new temp::TempList(left)));
      instr_list.Append(new assem::OperInstr("subq `s0, `d0", new temp::TempList(newReg), new temp::TempList(right), nullptr));
      break;

    case MUL_OP:
      left = left_->Munch(instr_list, fs);
      right = right_->Munch(instr_list, fs);

      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RAX()), new temp::TempList(left)));
      instr_list.Append(new assem::OperInstr("imulq `s0", nullptr, new temp::TempList(right), nullptr));
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(newReg), new temp::TempList(reg_manager->RAX())));
      break;

    case DIV_OP:
      left = left_->Munch(instr_list, fs);
      right = right_->Munch(instr_list, fs);
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->RAX()), new temp::TempList(left)));
      instr_list.Append(new assem::OperInstr("cqto", nullptr, nullptr, nullptr));
      instr_list.Append(new assem::OperInstr("idivq `s0", nullptr, new temp::TempList(right), nullptr));
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(newReg), new temp::TempList(reg_manager->RAX())));
      break;
  }

  return newReg;
}

temp::Temp *MemExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *newReg = temp::TempFactory::NewTemp();
  auto res = exp_->Munch(instr_list, fs);
  instr_list.Append(new assem::OperInstr("movq (`s0), `d0", new temp::TempList(newReg), new temp::TempList(res), nullptr));
  return newReg;
}

temp::Temp *TempExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  if (temp_ != reg_manager->FramePointer()) {
    return temp_;
  }

  // else replace it by rsp and framesize
  temp::Temp* reg = temp::TempFactory::NewTemp();
  std::stringstream stream;
  stream << "leaq " << fs << "_framesize(`s0), `d0";
  std::string assem = stream.str();
  instr_list.Append(new assem::OperInstr(assem, new temp::TempList(reg), new temp::TempList(reg_manager->RSP()), nullptr));
  return reg;
}

temp::Temp *EseqExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  stm_->Munch(instr_list, fs);
  return exp_->Munch(instr_list, fs);
}

temp::Temp *NameExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *newReg = temp::TempFactory::NewTemp();
  std::stringstream stream;
  stream << "leaq " << temp::LabelFactory::LabelString(name_) << "(%rip), `d0";
  std::string assem = stream.str();
  instr_list.Append(new assem::OperInstr(assem, new temp::TempList(newReg), nullptr, nullptr));
  return newReg;
}

temp::Temp *ConstExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *newReg = temp::TempFactory::NewTemp();
  std::stringstream stream;
  stream << "movq $" << consti_ << ", `d0";
  std::string assem = stream.str();
  instr_list.Append(new assem::OperInstr(assem, new temp::TempList(newReg), nullptr, nullptr));
  return newReg;
}

temp::Temp *CallExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::stringstream stream;
  std::string assem;

  char temp[maxlen];

  // store static link
  tree::Exp *staticlink = args_->GetList().front();
  args_->PopStaticLink();

  auto res = staticlink->Munch(instr_list, fs);
  temp::Temp *oldSP = temp::TempFactory::NewTemp();
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(oldSP), new temp::TempList(res)));
  // end

  // handle args
  std::string label = temp::LabelFactory::LabelString(((tree::NameExp*)fun_)->name_);
  args_->MunchArgs(instr_list, fs);

  // move rbp
  stream.str("");
  stream << "subq $" << frame::wordsize << ", `d0";
  assem = stream.str();
  instr_list.Append(new assem::OperInstr(assem, new temp::TempList(reg_manager->RSP()), nullptr, nullptr));
  // instr_list.Append(new assem::MoveInstr("movq `s0, (`d0)", new temp::TempList(reg_manager->RSP()), new temp::TempList(oldSP)));
  instr_list.Append(new assem::OperInstr("movq `s0, (`d0)", new temp::TempList(reg_manager->RSP()), new temp::TempList(oldSP), NULL));

  // call
  assem = std::string("callq ") + std::string(label);
  // instr_list.Append(new assem::OperInstr(assem, nullptr, nullptr, nullptr));
  instr_list.Append(new assem::OperInstr(assem, reg_manager->CallerSaves(), reg_manager->ArgRegs(), nullptr));
  
  temp::Temp *newReg = temp::TempFactory::NewTemp();
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(newReg), new temp::TempList(reg_manager->RAX())));
  
  // reset sp
  stream.str("");
  stream << "addq $" << frame::wordsize << ", `d0";
  assem = stream.str();
  instr_list.Append(new assem::OperInstr(assem, new temp::TempList(reg_manager->RSP()), nullptr, nullptr));

  args_->ResetSP(instr_list, fs);
  return newReg;
}

temp::TempList *ExpList::MunchArgs(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto res = new temp::TempList();

  int i = 1;
  for (auto &it : exp_list_) {
    temp::Temp *arg = it->Munch(instr_list, fs);
    // move vars
    if (i <= 6) {
      instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList(reg_manager->GetNthArg(i)), new temp::TempList(arg)));
    }
    else {
      std::stringstream stream;
      stream << "subq $" << frame::wordsize << ", `d0";
      std::string assem = stream.str();
      instr_list.Append(new assem::OperInstr(assem, new temp::TempList(reg_manager->RSP()), nullptr, nullptr));
      instr_list.Append(new assem::OperInstr("movq `s0, (`s1)", nullptr, new temp::TempList({arg, reg_manager->RSP()}), nullptr));
    }
    res->Append(arg);
    ++i;
  }

  return res;
}

void ExpList::ResetSP(assem::InstrList &instr_list, std::string_view fs) {
  int i = 1;
  for (auto &it : exp_list_) {
    if (i <= 6) {
      
    }
    else {
      std::stringstream stream;
      stream << "addq $" << frame::wordsize << ", `d0";
      std::string assem = stream.str();
      instr_list.Append(new assem::OperInstr(assem, new temp::TempList(reg_manager->RSP()), nullptr, nullptr));
    }
    ++i;
  }
}

void ExpList::PopStaticLink() {
  exp_list_.pop_front();
}

} // namespace tree
