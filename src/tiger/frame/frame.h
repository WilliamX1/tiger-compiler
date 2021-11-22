#ifndef TIGER_FRAME_FRAME_H_
#define TIGER_FRAME_FRAME_H_

#include <list>
#include <memory>
#include <string>

#include "tiger/frame/temp.h"
#include "tiger/translate/tree.h"

namespace frame {

class RegManager {
public:
  RegManager() : temp_map_(temp::Map::Empty()) {}

  virtual ~RegManager() = default;

  temp::Temp *GetRegister(int regno) { return regs_[regno]; }

  /**
   * Get general-purpose registers except RSI
   * NOTE: returned temp list should be in the order of calling convention
   * @return general-purpose registers
   */
  [[nodiscard]] virtual temp::TempList *Registers() = 0;

  /**
   * Get registers which can be used to hold arguments
   * NOTE: returned temp list must be in the order of calling convention
   * @return argument registers
   */
  [[nodiscard]] virtual temp::TempList *ArgRegs() = 0;

  /**
   * Get caller-saved registers
   * NOTE: returned registers must be in the order of calling convention
   * @return caller-saved registers
   */
  [[nodiscard]] virtual temp::TempList *CallerSaves() = 0;

  /**
   * Get callee-saved registers
   * NOTE: returned registers must be in the order of calling convention
   * @return callee-saved registers
   */
  [[nodiscard]] virtual temp::TempList *CalleeSaves() = 0;

  /**
   * Get return-sink registers
   * @return return-sink registers
   */
  [[nodiscard]] virtual temp::TempList *ReturnSink() = 0;

  /**
   * Get word size
   */
  [[nodiscard]] virtual int WordSize() = 0;

  [[nodiscard]] virtual temp::Temp *FramePointer() = 0;

  [[nodiscard]] virtual temp::Temp *StackPointer() = 0;

  [[nodiscard]] virtual temp::Temp *ReturnValue() = 0;

  [[nodiscard]] virtual temp::Temp* GetNthReg(int i) = 0;
  [[nodiscard]] virtual temp::Temp* GetNthArg(int i) = 0;
  
  temp::Map *temp_map_;
protected:
  std::vector<temp::Temp *> regs_;
};

class Access {
public:
  /* TODO: Put your lab5 code here */
  
  virtual ~Access() = default;
  
public:
  enum Kind {INFRAME, INREG};

  Kind kind_;

  Access(Kind kind_) : kind_(kind_) {};

  virtual tree::Exp* ToExp(tree::Exp* framePtr) const = 0;
};

class AccessList {
public:
  AccessList() = default;
  void PushBack(Access* access) { access_list_.push_back(access); };
  const std::list<Access *> &GetList() const { return access_list_; };

private:
  std::list<Access *> access_list_;
};

class Frame {
  /* TODO: Put your lab5 code here */
public:
  temp::Label* label;
  AccessList* formals;
  int s_offset;

  Frame(temp::Label* name, std::list<bool> escapes) : label(name) {};
  virtual Access *allocLocal(bool escape) = 0;

  virtual ~Frame() = default;
};

/**
 * Fragments
 */

class Frag {
public:
  virtual ~Frag() = default;

public:
  enum Kind { STRING, PROC };
  
  Kind kind_;

  Frag(Kind kind_) : kind_(kind_) {};
};

class StringFrag : public Frag {
public:
  temp::Label *label_;
  std::string str_;

  StringFrag(temp::Label *label, std::string str)
      : Frag(STRING), label_(label), str_(std::move(str)) {}
};

class ProcFrag : public Frag {
public:
  tree::Stm *body_;
  Frame *frame_;

  ProcFrag(tree::Stm *body, Frame *frame) 
  : Frag(PROC), body_(body), frame_(frame) {}
};

class Frags {
public:
  Frags() = default;
  void PushBack(Frag *frag) { frags_.push_back(frag); }
  const std::list<Frag*> &GetList() { return frags_; }

private:
  std::list<Frag*> frags_;
};

/* TODO: Put your lab5 code here */
tree::Exp* externalCall(std::string s, tree::ExpList* args);
tree::Stm* procEntryExit1(Frame* frame, tree::Stm* stm);
assem::InstrList* procEntryExit2(assem::InstrList* ilist);
assem::Proc* procEntryExit3(Frame* frame, assem::InstrList* ilist);

} // namespace frame

#endif