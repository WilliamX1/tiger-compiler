#ifndef TIGER_COMPILER_COLOR_H
#define TIGER_COMPILER_COLOR_H

#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/util/graph.h"

namespace col {
struct Result {
  Result() : coloring(nullptr), spills(nullptr) {}
  Result(temp::Map *coloring, live::INodeListPtr spills)
      : coloring(coloring), spills(spills) {}
  temp::Map *coloring;
  live::INodeListPtr spills;
};

template<typename T>
using Table = graph::Table<temp::Temp, T>;

class Color {
  /* TODO: Put your lab6 code here */
public:
  Color(fg::FGraph* flow_graph) 
  : flow_graph_(flow_graph), live_graph_(live::LiveGraphFactory::GetLiveGraph()) {};
  
  Result Color(frame::Frame* f, assem::InstrList* il) {};

  void Build();
  void MakeWorkList();
  void Simplify();
  void Coalesce();
  void Freeze();
  void SelectSpill();
  void AssignColors();

  void ShowStatus();

  static void StringBoolTableShow(std::string* s, bool* b);
  static void NodeStringTableShow(live::INode* n, std::string* s);
  static void NodeIntTableShow(live::INode* n, int* s);
  static void NodeNodeTableShow(live::INode* n, live::INode* s);
  static void NodeMoveTableShow(live::INode* n, live::MoveList* s);
  static void TempGraphShow(FILE* out, temp::Temp* t);
  
private:
  fg::FGraph* flow_graph_;

  live::INodeListPtr simplify_worklist_;
  live::INodeListPtr freeze_worklist_;
  live::INodeListPtr spill_worklist_;
  live::INodeListPtr spilled_nodes_;
  live::INodeListPtr coalesced_nodes_;
  live::INodeListPtr colored_nodes_;
  live::INodeListPtr select_stack_;

  live::MoveList* coalesced_moves_;
  live::MoveList* constrained_moves_;
  live::MoveList* frozen_moves_;
  live::MoveList* worklist_moves_;
  live::MoveList* active_moves_;

  Table<int> *degree_tab_;
  Table<live::MoveList> *movelist_tab_;
  Table<live::INode> *alias_tab_;
  Table<std::string> *color_tab_;
  temp::TempList* no_spill_temps_;

  live::LiveGraph live_graph_;

  int K = 15;

};
} // namespace col

#endif // TIGER_COMPILER_COLOR_H
