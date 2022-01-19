#ifndef TIGER_REGALLOC_REGALLOC_H_
#define TIGER_REGALLOC_REGALLOC_H_

#include "tiger/codegen/assem.h"
#include "tiger/codegen/codegen.h"
#include "tiger/frame/frame.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/regalloc/color.h"
#include "tiger/util/graph.h"
#include <unordered_map>

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
  : frame_(frame), assem_instr_(std::move(assem_instr)), result_(new Result()) {};
  
  std::unique_ptr<ra::Result> TransferResult() {
    return std::move(result_);
  };

  void RegAlloc();

  void Init();
  void Build();
  void AddEdge(live::INodePtr, live::INodePtr);
  void MakeWorkList();
  live::INodeListPtr Adjacent(live::INodePtr node);
  live::MoveList* NodeMoves(live::INodePtr node);
  bool MoveRelated(live::INodePtr node);
  void Simplify();
  void DecrementDegree(live::INodePtr node);
  void EnableMoves(live::INodeListPtr nodelist);
  void Coalesce();
  void AddWorkList(live::INodePtr u);
  bool OK(live::INodePtr t, live::INodePtr r);
  bool Conservertive(live::INodeListPtr nodes);
  live::INodePtr GetAlias(live::INodePtr n);
  void Combine(live::INodePtr u, live::INodePtr v);
  void Freeze();
  void FreezeMoves(live::INodePtr u);
  void SelectSpill();
  void AssignColor();
  void SimplifyInstrList();
  void RewriteProgram();

private:
  frame::Frame* frame_;
  std::unique_ptr<ra::Result> result_;
  std::unique_ptr<cg::AssemInstr> assem_instr_;
  live::LiveGraph live_graph_;
  fg::FGraphPtr flow_graph_;

  live::INodeListPtr precolored_ = NULL;
  live::INodeListPtr initial_ = NULL;

  live::INodeListPtr simplify_work_list_ = NULL;
  live::INodeListPtr freeze_work_list_ = NULL;
  live::INodeListPtr spill_work_list_ = NULL;
  live::INodeListPtr spilled_nodes_ = NULL;
  live::INodeListPtr coalesced_nodes_ = NULL;
  live::INodeListPtr colored_nodes_ = NULL;
  live::INodeListPtr select_stack_ = NULL;

  live::MoveList* coalesced_moves_ = NULL;
  live::MoveList* constrained_moves_ = NULL;
  live::MoveList* frozen_moves_ = NULL;
  live::MoveList* worklist_moves_ = NULL;
  live::MoveList* active_moves_ = NULL;

  std::set<std::pair<live::INodePtr, live::INodePtr>> adj_set_;
  std::map<live::INodePtr, live::INodeListPtr> adj_list_;
  std::map<live::INodePtr, int> degree_;
  std::map<live::INodePtr, live::MoveList*> move_list_;
  std::map<live::INodePtr, live::INodePtr> alias_;
  std::map<temp::Temp*, std::string> color_;
  std::set<temp::Temp*> not_spill_;

  int K;

private:
  bool Contain(live::INodePtr node, live::INodeListPtr list);
  live::INodeListPtr Union(live::INodeListPtr left, live::INodeListPtr right);
  live::INodeListPtr Sub(live::INodeListPtr left, live::INodeListPtr right);
  live::INodeListPtr Intersect(live::INodeListPtr left, live::INodeListPtr right);
  bool Contain(std::pair<live::INodePtr, live::INodePtr> node, live::MoveList* list);
  live::MoveList* Union(live::MoveList* left, live::MoveList* right);
  live::MoveList* Intersect(live::MoveList* left, live::MoveList* right);
};

} // namespace ra

#endif