#ifndef TIGER_REGALLOC_REGALLOC_H_
#define TIGER_REGALLOC_REGALLOC_H_

#include "tiger/codegen/assem.h"
#include "tiger/codegen/codegen.h"
#include "tiger/frame/frame.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/regalloc/color.h"
#include "tiger/util/graph.h"

namespace ra {

class Result {
public:
  temp::Map *coloring_;
  assem::InstrList *il_;

  Result() : coloring_(nullptr), il_(nullptr) {}
  Result(temp::Map *coloring, assem::InstrList *il)
      : coloring_(coloring), il_(il) {}
  Result(const Result &result) = delete;
  Result(Result &&result) = delete;
  Result &operator=(const Result &result) = delete;
  Result &operator=(Result &&result) = delete;
  ~Result() {};
};

class RegAllocator {
  /* TODO: Put your lab6 code here */
public:
  RegAllocator(frame::Frame* frame, std::unique_ptr<cg::AssemInstr> assem_instr)
  : frame_(frame), assem_instr_(std::move(assem_instr)), color_(new col::Color()) {
    fprintf(stderr, "Start RegAllocator\n");
    flow_graph_factory_ = new fg::FlowGraphFactory(assem_instr_->GetInstrList());
    live_graph_factory_ = new live::LiveGraphFactory(flow_graph_factory_->GetFlowGraph());
    fprintf(stderr, "End RegAllocator\n");
  };
  
  std::unique_ptr<ra::Result> TransferResult() {
    return std::move(result_);
  };

  void RegAlloc();
  void RewriteProgram();
  void RemoveUnnecessary();

private:
  frame::Frame* frame_;
  std::unique_ptr<ra::Result> result_;
  std::unique_ptr<cg::AssemInstr> assem_instr_;

  fg::FlowGraphFactory* flow_graph_factory_;
  live::LiveGraphFactory* live_graph_factory_;

  col::Color* color_;
};

} // namespace ra

#endif