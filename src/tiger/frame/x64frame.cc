#include "tiger/frame/x64frame.h"

extern frame::RegManager *reg_manager;

namespace frame {

/* TODO: Put your lab5 code here */
X64Frame::X64Frame(temp::Label *name, std::list<bool> escapes) {
  label = name;
  formals = new AccessList();

  s_offset = -8;
  int i = 1;
  int arg_num = escapes.size();
  for (auto it : escapes) {
    Access *a = AllocLocal(it);
    formals->Append(a);

    // push args
    if (reg_manager->GetNthArg(i)) {
      save_args.push_back(new tree::MoveStm(a->ToExp(new tree::TempExp(reg_manager->FramePointer())), new tree::TempExp(reg_manager->GetNthArg(i))));
    }
    else {
      // 7th~ args
      save_args.push_back(new tree::MoveStm(a->ToExp(
        new tree::TempExp(reg_manager->FramePointer())), 
        new tree::MemExp(
          new tree::BinopExp(tree::BinOp::PLUS_OP, 
            new tree::TempExp(reg_manager->FramePointer()), 
              new tree::ConstExp((arg_num - i + 2) * frame::wordsize)))));
    }
    ++i;
  }
}

tree::Stm *X64Frame::ProcEntryExit1(tree::Stm *body) {
  // save args
  for (auto &it : save_args) {
    body = tree::Stm::Seq(it, body);
  }
  return body;
}

// useless now
assem::InstrList *X64Frame::ProcEntryExit2(assem::InstrList *body) {
  return body;
}

assem::Proc *X64Frame::ProcEntryExit3(assem::InstrList *body) {
  static char buf[256];
  std::string prolog;
  std::string epilog;

  // prolog part
  sprintf(buf, ".set %s_framesize, %d\n", label->Name().c_str(), -s_offset);
  prolog = std::string(buf);

  sprintf(buf, "%s:\n", label->Name(). c_str());
  prolog.append(std::string(buf));

  sprintf(buf, "subq $%d, %%rsp\n", -s_offset);
  prolog.append(std::string(buf));

  // epilog part
  sprintf(buf, "addq $%d, %%rsp\n", -s_offset);
  epilog.append(std::string(buf));
  
  epilog.append(std::string("retq\n"));

  return new assem::Proc(prolog, body, epilog);
}

tree::Exp *ExternalCall(std::string s, tree::ExpList *args) {
  return new tree::CallExp(new tree::NameExp(temp::LabelFactory::NamedLabel(s)), args);
}

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

std::vector<std::string> X64RegManager::Colors() {
  std::vector<std::string> res;
  res.push_back("%rax");
  res.push_back("%rdi");
  res.push_back("%rsi");
  res.push_back("%rdx");
  res.push_back("%rcx");
  res.push_back("%r8");
  res.push_back("%r9");
  res.push_back("%r10");
  res.push_back("%r11");
  res.push_back("%rbx");
  res.push_back("%rbp");
  res.push_back("%r12");
  res.push_back("%r13");
  res.push_back("%r14");
  res.push_back("%r15");
  return res;
}

/* TODO: Put your lab5 code here */
} // namespace frame