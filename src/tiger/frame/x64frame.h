//
// Created by wzl on 2021/10/12.
//

#ifndef TIGER_COMPILER_X64FRAME_H
#define TIGER_COMPILER_X64FRAME_H

#include "tiger/frame/frame.h"

namespace frame {

const int wordsize = 8;

class X64RegManager : public RegManager {
public:
  X64RegManager() : frame::RegManager() {
    rax = temp::TempFactory::NewTemp();
    temp_map_->Enter(rax, new std::string("%rax"));
    rdi = temp::TempFactory::NewTemp();
    temp_map_->Enter(rdi, new std::string("%rdi"));
    rsi = temp::TempFactory::NewTemp();
    temp_map_->Enter(rsi, new std::string("%rsi"));
    rdx = temp::TempFactory::NewTemp();
    temp_map_->Enter(rdx, new std::string("%rdx"));
    rcx = temp::TempFactory::NewTemp();
    temp_map_->Enter(rcx, new std::string("%rcx"));
    r8 = temp::TempFactory::NewTemp();
    temp_map_->Enter(r8, new std::string("%r8"));
    r9 = temp::TempFactory::NewTemp();
    temp_map_->Enter(r9, new std::string("%r9"));
    r10 = temp::TempFactory::NewTemp();
    temp_map_->Enter(r10, new std::string("%r10"));
    r11 = temp::TempFactory::NewTemp();
    temp_map_->Enter(r11, new std::string("%r11"));
    rbx = temp::TempFactory::NewTemp();
    temp_map_->Enter(rbx, new std::string("%rbx"));
    rbp = temp::TempFactory::NewTemp();
    temp_map_->Enter(rbp, new std::string("%rbp"));
    r12 = temp::TempFactory::NewTemp();
    temp_map_->Enter(r12, new std::string("%r12"));
    r13 = temp::TempFactory::NewTemp();
    temp_map_->Enter(r13, new std::string("%r13"));
    r14 = temp::TempFactory::NewTemp();
    temp_map_->Enter(r14, new std::string("%r14"));
    r15 = temp::TempFactory::NewTemp();
    temp_map_->Enter(r15, new std::string("%r15"));
    rsp = temp::TempFactory::NewTemp();
    temp_map_->Enter(rsp, new std::string("%rsp"));
  };
  /* TODO: Put your lab5 code here */
  temp::TempList* Registers();
  temp::TempList* ArgRegs();
  temp::TempList* CallerSaves();
  temp::TempList* CalleeSaves();
  temp::TempList* ReturnSink();
  int WordSize();
  temp::Temp* FramePointer();
  temp::Temp* StackPointer();
  temp::Temp* ReturnValue();
  temp::Temp* GetNthReg(int i);
  temp::Temp* GetNthArg(int i);

private:
  temp::Temp *rax, *rdi, *rsi, *rdx, *rcx, *r8, 
            *r9, *r10, *r11, *rbx, *rbp, *r12, 
            *r13, *r14, *r15, *rsp;
};

class InFrameAccess : public Access {
public:
  int offset;

  InFrameAccess(int offset) : Access(INFRAME), offset(offset) { assert(offset < 0); };
  tree::Exp* ToExp(tree::Exp* framPtr) const { return tree::NewMemPlus_Const(framPtr, offset); };
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
  X64Frame(temp::Label* name, std::list<bool> escapes) : Frame(name, escapes) {
    this->s_offset = -8;
    this->formals = new AccessList();

    for (auto ele : escapes) {
      this->formals->PushBack(allocLocal(ele));
    }
  };
  Access* allocLocal(bool escape) override;
};

} // namespace frame
#endif // TIGER_COMPILER_X64FRAME_H
