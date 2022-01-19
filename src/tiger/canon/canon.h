#ifndef TIGER_CANON_CANON_H_
#define TIGER_CANON_CANON_H_

#include <cstdio>
#include <list>
#include <memory>
#include <vector>

#include "tiger/frame/temp.h"
#include "tiger/translate/tree.h"

// Forward Declarations
namespace tree {
class StmList;
class Exp;
class Stm;
} // namespace tree

namespace frame {
class ProcFrag;
}

namespace canon {

class StmListList {
  friend class Canon;

public:
  StmListList() = default;

  void Append(tree::StmList *stmlist) { stmlist_list_.push_back(stmlist); }
  [[nodiscard]] const std::list<tree::StmList *> &GetList() const {
    return stmlist_list_;
  }

private:
  std::list<tree::StmList *> stmlist_list_;
};

class Block {
public:
  temp::Label *label_;
  StmListList *stm_lists_;

  Block() : stm_lists_(nullptr), label_(nullptr) {}
  Block(temp::Label *label, StmListList *stm_lists)
      : label_(label), stm_lists_(stm_lists) {}
};

struct StmAndExp {
  tree::Stm *s_;
  tree::Exp *e_;

  StmAndExp(const StmAndExp &) = delete;
  StmAndExp &operator=(const StmAndExp &) = delete;
};

class Traces {
public:
  Traces() = delete;
  Traces(nullptr_t) = delete;
  explicit Traces(tree::StmList *stm_list) : stm_list_(stm_list) {
    if (stm_list == nullptr)
      throw std::invalid_argument("NULL pointer is not allowed in Traces");
  }
  Traces(const Traces &traces) = delete;
  Traces(Traces &&traces) = delete;
  Traces &operator=(const Traces &traces) = delete;
  Traces &operator=(Traces &&traces) = delete;
  ~Traces();

  [[nodiscard]] tree::StmList *GetStmList() const { return stm_list_; }

private:
  tree::StmList *stm_list_;
};

class Canon {
  friend class frame::ProcFrag;

public:
  Canon() = delete;
  explicit Canon(tree::Stm *stm_ir)
      : stm_ir_(stm_ir), stm_canon_(nullptr), block_(),
        block_env_(new sym::Table<tree::StmList>()) {}

  /**
   * From an arbitrary Tree statement, produce a list of cleaned trees
   * satisfying the following properties:
   * 1.  No SEQ's or ESEQ's
   * 2.  The parent_ of every CALL is an EXP(..) or a MOVE(temp t,..)
   * @return linearized statements
   */
  tree::StmList *Linearize();

  /**
   * Tree.stm_ list -> (Tree.stm_ list list * Tree.label_)
   * From a list of cleaned trees, produce a list of
   * basic blocks satisfying the following properties:
   * 1. and 2. as above;
   * 3.  Every block_ begins with a LABEL;
   * 4.  A LABEL appears only at the beginning of a block_;
   * 5.  Any JUMP or CJUMP is the last stm_ in a block_;
   * 6.  Every block_ ends with a JUMP or CJUMP;
   * Also produce the "label_" to which control will be passed
   * upon exit.
   * @return basic blocks
   */
  canon::StmListList *BasicBlocks();

  /**
   * Tree.stm_ list list * Tree.label_ -> Tree.stm_ list
   * From a list of basic blocks satisfying properties 1-6,
   * along with an "exit" label_,
   * produce a list of stms such that:
   * 1. and 2. as above;
   * 7. Every CJUMP(_,t,f) is immediately followed by LABEL f.
   * The blocks are reordered to satisfy property 7; also
   * in this reordering as many JUMP(tree.NAME(lab)) statements
   * as possible are eliminated by falling through into tree.LABEL(lab).
   * @return ordered traces_
   */
  tree::StmList *TraceSchedule();

  /**
   * Transfer the ownership of traces_ to outer scope
   * @return unique pointer of traces_
   */
  std::unique_ptr<Traces> TransferTraces() { return std::move(traces_); }

private:
  // IR tree in a process fragment
  tree::Stm *stm_ir_;
  // Canonical tree generated
  tree::StmList *stm_canon_;
  // Basic blocks
  Block block_;
  sym::Table<tree::StmList> *block_env_;
  // Traces from ordered basic blocks
  // The `stm_ir_` and `stm_canon_` are temprorary refs to the intermediate
  // state in the transformation of IR tree to block_ traces_, so we only care
  // about the lifetimes of stm_traces
  std::unique_ptr<Traces> traces_;

  /**
   * Get the Next block_ from the list of stm_lists_, using only those that have
   * not been traced yet
   * @return Next block_
   */
  tree::StmList *GetNext();

  void Trace(std::list<tree::Stm *> &stms);
};

} // namespace canon
#endif
