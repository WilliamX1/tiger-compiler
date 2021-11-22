#include "tiger/frame/x64frame.h"

extern frame::RegManager *reg_manager;

namespace frame {

/* TODO: Put your lab5 code here */

/* TODO: Put your lab5 code here */

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

tree::Stm* procEntryExit1(Frame* frame, tree::Stm* stm) {
  int num = 1;

  tree::Stm* viewshift = new tree::ExpStm(new tree::ConstExp(0));
  
  auto formal_list = frame->formals->GetList();

  for (auto ele : formal_list) {
    if (reg_manager->GetNthReg(num));
      viewshift = new tree::SeqStm(viewshift, new tree::MoveStm(ele->ToExp(new tree::TempExp(reg_manager->FramePointer())), new tree::TempExp(reg_manager->GetNthArg(num))));
    num++;
  };
  return new tree::SeqStm(viewshift, stm);
};

assem::InstrList* procEntryExit2(assem::InstrList* body) {
  static temp::TempList* retlist = NULL;
  if (!retlist)
    retlist = new temp::TempList(reg_manager->ReturnValue());
  // assem::OperInstr* ele = new assem::OperInstr("", NULL, retlist, new assem::Targets(NULL));
  // body->Append(ele);
  return body;
};

assem::Proc* procEntryExit3(frame::Frame* frame, assem::InstrList* body) {
  static char instr[256];

  std::string prolog;
  sprintf(instr, ".set %s_framesize, %d\n", frame->label->Name().c_str(), -frame->s_offset);
  prolog = std::string(instr);
  sprintf(instr, "%s:\n", frame->label->Name(). c_str());
  prolog.append(std::string(instr));
  sprintf(instr, "\tsubq $%s_framesize, %%rsp\n", frame->label->Name().c_str());
  prolog.append(std::string(instr));

  sprintf(instr, "\taddq $%s_framesize, %%rsp\n", frame->label->Name().c_str());
  std::string epilog = std::string(instr);
  epilog.append(std::string("\tret\n"));
  return new assem::Proc(prolog, body, epilog);
};

temp::TempList* X64RegManager::Registers() {
  static temp::TempList* templist = NULL;
  
  if (templist) return templist;
  
  templist = new temp::TempList();

  templist->Append(rax);
  templist->Append(rdi);
  templist->Append(rsi);
  templist->Append(rdx);
  templist->Append(rcx);
  templist->Append(r8);
  templist->Append(r9);
  templist->Append(r10);
  templist->Append(r11);
  templist->Append(rbx);
  templist->Append(rbp);
  templist->Append(r12);
  templist->Append(r13);
  templist->Append(r14);
  templist->Append(r15);
  return templist;
};

temp::TempList* X64RegManager::ArgRegs() {
  static temp::TempList* templist = NULL;

  if (templist) return templist;

  templist = new temp::TempList();
  templist->Append(rdi);
  templist->Append(rsi);
  templist->Append(rdx);
  templist->Append(rcx);
  templist->Append(r8);
  templist->Append(r9);
  return templist;
};

temp::TempList* X64RegManager::CallerSaves() {
  static temp::TempList* templist = NULL;

  if (templist) return templist;

  templist = new temp::TempList();
  templist->Append(rax);
  templist->Append(rdi);
  templist->Append(rsi);
  templist->Append(rdx);
  templist->Append(rcx);
  templist->Append(r8);
  templist->Append(r9);
  templist->Append(r10);
  templist->Append(r11);
  return templist;
};
temp::TempList* X64RegManager::CalleeSaves() {
  static temp::TempList* templist = NULL;

  if (templist) return templist;

  templist = new temp::TempList();
  templist->Append(rbx);
  templist->Append(rbp);
  templist->Append(r12);
  templist->Append(r13);
  templist->Append(r14);
  templist->Append(r15);
  return templist; 
};

temp::TempList* X64RegManager::ReturnSink() {
  return NULL;
};

int X64RegManager::WordSize() {
  return 8;
};

temp::Temp* X64RegManager::FramePointer() {
  return rbp;
};

temp::Temp* X64RegManager::StackPointer() {
  return rsp;
};

temp::Temp* X64RegManager::ReturnValue() {
  return rax;
};

temp::Temp* X64RegManager::GetNthReg(int i) {
  switch (i) {
    case 1: return rax;
    case 2: return rdi;
    case 3: return rsi;
    case 4: return rdx;
    case 5: return rcx;
    case 6: return r8;
    case 7: return r9;
    case 8: return r10;
    case 9: return r11;
    case 10: return rbx;
    case 11: return rbp;
    case 12: return r12;
    case 13: return r13;
    case 14: return r14;
    case 15: return r15;
    case 16: return rsp;
    default: return NULL;
  }
};

temp::Temp* X64RegManager::GetNthArg(int i) {
  switch(i) {
    case 1: return rdi;
    case 2: return rsi;
    case 3: return rdx;
    case 4: return rcx;
    case 5: return r8;
    case 6: return r9;
    default: return NULL;
  };
};
/* TODO: Put your lab5 code here */
} // namespace frame