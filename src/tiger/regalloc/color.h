#ifndef TIGER_COMPILER_COLOR_H
#define TIGER_COMPILER_COLOR_H

#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/util/graph.h"

#include <set>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

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
  Color() {};

  void Build();
  void MakeWorkList();
  void Simplify();
  void Coalesce();
  void Freeze();
  void SelectSpill();
  void AssignColors();

  temp::Map* coloring_ = temp::Map::Empty();

  std::set<graph::Node<temp::Temp> *> simplify_worklist_, freeze_worklist_, spill_worklist_;

  std::map<graph::Node<temp::Temp> *, live::MoveList *> movelist_;

  fg::FGraph* flow_graph_;

  live::LiveGraph liveness_;

  std::set<graph::Node<temp::Temp> *> coalesced_nodes_, spilled_nodes_, colored_nodes_;

  std::vector<graph::Node<temp::Temp> *> select_stack_;

  live::MoveList* worklist_moves_ = new live::MoveList(), *active_moves_ = new live::MoveList(), 
  *coalesced_moves_ = new live::MoveList(), *constrained_moves_ = new live::MoveList(), *frozen_moves_ = new live::MoveList();

  std::map<graph::Node<temp::Temp> *, int> degree_;

  std::set<std::pair<graph::Node<temp::Temp> *, graph::Node<temp::Temp> *> > adj_set_;
  
  std::map<graph::Node<temp::Temp> *, std::set<graph::Node<temp::Temp> *> > adj_list_;

  std::map<graph::Node<temp::Temp> *, graph::Node<temp::Temp> *> alias_;

  std::set<temp::Temp *> no_spill_temp_;

  bool Precolored(temp::Temp* temp);
  void AddEdge(graph::Node<temp::Temp> *u, graph::Node<temp::Temp> *v);
  live::MoveList* NodeMoves(graph::Node<temp::Temp> *node);
  bool MoveRelated(graph::Node<temp::Temp> *node);
  void EnableMoves(graph::NodeList<temp::Temp> *nodes);
  graph::NodeList<temp::Temp> *Adjacent(graph::Node<temp::Temp> *node);
  void DecrementDegree(graph::Node<temp::Temp> *node);
  graph::Node<temp::Temp>* GetAlias(graph::Node<temp::Temp> *node);
  void AddWorklist(graph::Node<temp::Temp>* node);
  bool OK(graph::Node<temp::Temp>* t, graph::Node<temp::Temp>* r);
  bool OKForAll(graph::NodeList<temp::Temp>* t, graph::Node<temp::Temp>* r);
  bool Conservative(graph::NodeList<temp::Temp>* nodes);
  void Combine(graph::Node<temp::Temp>* u, graph::Node<temp::Temp>* v);
  void FreezeMoves(graph::Node<temp::Temp>* u);
};
} // namespace col

#endif // TIGER_COMPILER_COLOR_H
