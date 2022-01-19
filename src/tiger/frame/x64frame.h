//
// Created by wzl on 2021/10/12.
//

#ifndef TIGER_COMPILER_X64FRAME_H
#define TIGER_COMPILER_X64FRAME_H

#include "tiger/frame/frame.h"

namespace frame {

const int wordsize = 8;

class X64RegManager : public RegManager {

private:
  temp::Temp *rax, *rdi, *rsi, *rdx, *rcx, *r8, 
            *r9, *r10, *r11, *rbx, *rbp, *r12, 
            *r13, *r14, *r15, *rsp;

public:
  int K = 15;

  X64RegManager() : frame::RegManager() {
    temp_map_->Enter(RAX(), new std::string("%rax"));
    temp_map_->Enter(RDI(), new std::string("%rdi"));
    temp_map_->Enter(RSI(), new std::string("%rsi"));
    temp_map_->Enter(RDX(), new std::string("%rdx"));
    temp_map_->Enter(RCX(), new std::string("%rcx"));
    temp_map_->Enter(R8(), new std::string("%r8"));
    temp_map_->Enter(R9(), new std::string("%r9"));
    temp_map_->Enter(R10(), new std::string("%r10"));
    temp_map_->Enter(R11(), new std::string("%r11"));
    temp_map_->Enter(RBX(), new std::string("%rbx"));
    temp_map_->Enter(RBP(), new std::string("%rbp"));
    temp_map_->Enter(R12(), new std::string("%r12"));
    temp_map_->Enter(R13(), new std::string("%r13"));
    temp_map_->Enter(R14(), new std::string("%r14"));
    temp_map_->Enter(R15(), new std::string("%r15"));
    temp_map_->Enter(RSP(), new std::string("%rsp"));
  };
  /* TODO: Put your lab5 code here */
  temp::TempList* Registers() override;
  temp::TempList* ArgRegs() override;
  temp::TempList* CallerSaves() override;
  temp::TempList* CalleeSaves() override;
  temp::TempList* ReturnSink() override;
  int WordSize() override;
  temp::Temp* FramePointer() override;
  temp::Temp* StackPointer() override;
  temp::Temp* ReturnValue() override;
  
  std::vector<std::string> Colors() override;

  /* get nth argument for function, range from 1 to 6 */
  temp::Temp* GetNthArg(int i) {
    assert(i != 0);
    switch (i)
    {
    case 1: return RDI();
    case 2: return RSI();
    case 3: return RDX();
    case 4: return RCX();
    case 5: return R8();
    case 6: return R9();
    };
    return NULL;
    // assert(0);
  };
  temp::Temp* RAX() { 
    if (rax == nullptr) rax = temp::TempFactory::NewTemp(); 
    return rax; 
  };
  temp::Temp* RDI() { 
    if (rdi == nullptr) rdi = temp::TempFactory::NewTemp();
    return rdi; 
  };
  temp::Temp* RSI() {
    if (rsi == nullptr) rsi = temp::TempFactory::NewTemp();
    return rsi; 
  };
  temp::Temp* RDX() {
    if (rdx == nullptr) rdx = temp::TempFactory::NewTemp();
    return rdx;
  };
  temp::Temp* RCX() {
    if (rcx == nullptr) rcx = temp::TempFactory::NewTemp();
    return rcx;
  };
  temp::Temp* R8() {
    if (r8 == nullptr) r8 = temp::TempFactory::NewTemp();
    return r8;
  };
  temp::Temp* R9() { 
    if (r9 == nullptr) r9 = temp::TempFactory::NewTemp();
    return r9; 
  };
  temp::Temp* R10() { 
    if (r10 == nullptr) r10 = temp::TempFactory::NewTemp();
    return r10; 
  };
  temp::Temp* R11() { 
    if (r11 == nullptr) r11 = temp::TempFactory::NewTemp();
    return r11;
  };
  temp::Temp* RBX() {
    if (rbx == nullptr) rbx = temp::TempFactory::NewTemp();
    return rbx;
  };
  temp::Temp* RBP() { 
    if (rbp == nullptr) rbp = temp::TempFactory::NewTemp();
    return rbp;
  };
  temp::Temp* R12() {
    if (r12 == nullptr) r12 = temp::TempFactory::NewTemp();
    return r12;
  };
  temp::Temp* R13() { 
    if (r13 == nullptr) r13 = temp::TempFactory::NewTemp();
    return r13;
  };
  temp::Temp* R14() { 
    if (r14 == nullptr) r14 = temp::TempFactory::NewTemp();
    return r14; 
  };
  temp::Temp* R15() { 
    if (r15 == nullptr) r15 = temp::TempFactory::NewTemp();
    return r15;
  };
  temp::Temp* RSP() {
    if (rsp == nullptr) rsp = temp::TempFactory::NewTemp();
    return rsp;
  };
};

class InFrameAccess : public Access {
public:
  int offset;

  InFrameAccess(int offset) : Access(INFRAME), offset(offset) { assert(offset < 0); };
  tree::Exp* ToExp(tree::Exp* framPtr) const { 
    return new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP, framPtr, new tree::ConstExp(offset)));
  };
};

class InRegAccess : public Access {
public:
  temp::Temp* reg;

  InRegAccess(temp::Temp* reg) : Access(INREG), reg(reg) {};
  tree::Exp* ToExp(tree::Exp* framePtr) const { return new tree::TempExp(reg); };
};

class X64Frame : public Frame {
  /* TODO: Put your lab5 code here */
public:
  std::list<tree::Stm *> save_args;

  X64Frame(temp::Label* name, std::list<bool> escapes);
  Access* AllocLocal(bool escape) {
    Access *tmp = nullptr;
    if (escape) {
      tmp = new InFrameAccess(s_offset);
      s_offset -= wordsize;
    } else {
      tmp = new InRegAccess(temp::TempFactory::NewTemp());
    }
    return tmp;
  };
  tree::Stm *ProcEntryExit1(tree::Stm *body) override;
  assem::InstrList *ProcEntryExit2(assem::InstrList *body) override;
  assem::Proc *ProcEntryExit3(assem::InstrList *body) override;
};

} // namespace frame
#endif // TIGER_COMPILER_X64FRAME_H
