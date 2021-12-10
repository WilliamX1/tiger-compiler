#include "tiger/frame/x64frame.h"

extern frame::RegManager *reg_manager;

namespace frame {

/* TODO: Put your lab5 code here */
X64Frame::X64Frame(temp::Label* name, std::list<bool> escapes) : Frame(name, escapes) {
  this->s_offset = -frame::wordsize;
  this->formals = new AccessList();

  for (auto escape : escapes) {
    this->formals->PushBack(allocLocal(escape));
  };

  int i = 1;
  viewShift = NULL;
  for (auto& formal : formals->GetList()) {
    tree::Exp* dstExp;
    if (formal->kind_ == Access::INFRAME) {
      dstExp = new tree::MemExp(new tree::BinopExp(tree::PLUS_OP, new tree::TempExp(reg_manager->RBP()), new tree::ConstExp(((frame::InFrameAccess*) formal)->offset)));
    } else dstExp = new tree::TempExp(((frame::InRegAccess*) formal)->reg);

    tree::Stm* stm;

    if (i <= 6) {
      stm = new tree::MoveStm(dstExp, new tree::TempExp(reg_manager->GetNthArg(i)));
    } else {
      stm = new tree::MoveStm(dstExp, new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP, new tree::TempExp(reg_manager->RBP()), new tree::ConstExp((6 - i) * frame::wordsize))));
    }

    if (viewShift == nullptr) {
      viewShift = new tree::StmList(stm);
    } else {
      viewShift->Append(stm);
    };
    i++;
  };
  return;
};


Access* X64Frame::allocLocal(bool escape) {
  Access* local = NULL;
  if (escape) {
    local = new InFrameAccess(s_offset);
    s_offset -= wordsize;
  } else {
    local = new InRegAccess(temp::TempFactory::NewTemp());
  }
  
  return local;
};

tree::Exp* externalCall(std::string s, tree::ExpList* args) {
  return new tree::CallExp(new tree::NameExp(temp::LabelFactory::NamedLabel(s)), args);
};

tree::Stm* ProcEntryExit1(Frame* frame, tree::Stm* stm) {
  // int num = 1;
  // 
  // tree::Stm* viewshift = new tree::ExpStm(new tree::ConstExp(0));
  // 
  // auto formal_list = frame->formals->GetList();
  // 
  // for (auto formal : formal_list) {
  //   if (reg_manager->GetNthArg(num));
  //     viewshift = new tree::SeqStm(viewshift, new tree::MoveStm(formal->ToExp(new tree::TempExp(reg_manager->FramePointer())), new tree::TempExp(reg_manager->GetNthArg(num))));
  //   num++;
  // };
  // return new tree::SeqStm(viewshift, stm);
  tree::Stm* result = new tree::ExpStm(new tree::ConstExp(0));
  for (auto& view : frame->viewShift->GetList()) {
    result = new tree::SeqStm(result, view);
  };
  return new tree::SeqStm(result, stm);;
};

assem::InstrList* ProcEntryExit2(assem::InstrList* body) {
  static temp::TempList* retlist = NULL;
  if (!retlist)
    retlist = new temp::TempList(reg_manager->ReturnValue());
  assem::OperInstr* ele = new assem::OperInstr("", NULL, retlist, new assem::Targets(NULL));
  body->Append(ele);
  return body;
};

assem::Proc* ProcEntryExit3(frame::Frame* frame, assem::InstrList* body) {
  static char instr[256];

  std::string prolog;
  sprintf(instr, ".set %s_framesize, %d\n", frame->label->Name().c_str(), -frame->s_offset);
  prolog = std::string(instr);
  sprintf(instr, "%s:\n", frame->label->Name(). c_str());
  prolog.append(std::string(instr));
  sprintf(instr, "subq $%d, %%rsp\n", -frame->s_offset);
  // sprintf(instr, "subq %s_framesize, %%rsp\n", frame->label->Name().c_str());
  prolog.append(std::string(instr));

  sprintf(instr, "addq $%d, %%rsp\n", -frame->s_offset);
  // sprintf(instr, "addq %s_framesize, %%rsp\n", frame->label->Name().c_str());
  std::string epilog = std::string(instr);
  epilog.append(std::string("retq\n"));
  return new assem::Proc(prolog, body, epilog);
};

temp::TempList* X64RegManager::Registers() {
  static temp::TempList* templist = NULL;
  
  if (templist) return templist;
  
  templist = new temp::TempList();

  templist->Append(RAX());
  templist->Append(RDI());
  templist->Append(RSI());
  templist->Append(RDX());
  templist->Append(RCX());
  templist->Append(R8());
  templist->Append(R9());
  templist->Append(R10());
  templist->Append(R11());
  templist->Append(RBX());
  templist->Append(RBP());
  templist->Append(R12());
  templist->Append(R13());
  templist->Append(R14());
  templist->Append(R15());
  return templist;
};

temp::TempList* X64RegManager::ArgRegs() {
  static temp::TempList* templist = NULL;

  if (templist) return templist;

  templist = new temp::TempList();
  templist->Append(RDI());
  templist->Append(RSI());
  templist->Append(RDX());
  templist->Append(RCX());
  templist->Append(R8());
  templist->Append(R9());
  return templist;
};

temp::TempList* X64RegManager::CallerSaves() {
  static temp::TempList* templist = NULL;

  if (templist) return templist;

  templist = new temp::TempList();
  templist->Append(RAX());
  templist->Append(RDI());
  templist->Append(RSI());
  templist->Append(RDX());
  templist->Append(RCX());
  templist->Append(R8());
  templist->Append(R9());
  templist->Append(R10());
  templist->Append(R11());
  return templist;
};
temp::TempList* X64RegManager::CalleeSaves() {
  static temp::TempList* templist = NULL;

  if (templist) return templist;

  templist = new temp::TempList();
  templist->Append(RBX());
  templist->Append(RBP());
  templist->Append(R12());
  templist->Append(R13());
  templist->Append(R14());
  templist->Append(R15());
  return templist; 
};

temp::TempList* X64RegManager::ReturnSink() {
  assert(0);
  return NULL;
};

int X64RegManager::WordSize() {
  return 8;
};

temp::Temp* X64RegManager::FramePointer() {
  return RBP();
};

temp::Temp* X64RegManager::StackPointer() {
  return RSP();
};

temp::Temp* X64RegManager::ReturnValue() {
  return RAX();
};

/* TODO: Put your lab5 code here */
} // namespace frame