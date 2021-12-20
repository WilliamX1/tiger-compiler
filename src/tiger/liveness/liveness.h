#ifndef TIGER_LIVENESS_LIVENESS_H_
#define TIGER_LIVENESS_LIVENESS_H_

#include "tiger/codegen/assem.h"
#include "tiger/frame/x64frame.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/flowgraph.h"
#include "tiger/util/graph.h"

#include <algorithm>
#include <map>

namespace live {

using INode = graph::Node<temp::Temp>;
using INodePtr = graph::Node<temp::Temp>*;
using INodeList = graph::NodeList<temp::Temp>;
using INodeListPtr = graph::NodeList<temp::Temp>*;
using IGraph = graph::Graph<temp::Temp>;
using IGraphPtr = graph::Graph<temp::Temp>*;

class MoveList {
public:
  MoveList() = default;

  MoveList(INodePtr src, INodePtr dst) {
    move_list_.emplace_back(src, dst);
  };

  [[nodiscard]] const std::list<std::pair<INodePtr, INodePtr>> &
  GetList() const {
    return move_list_;
  }
  void Append(INodePtr src, INodePtr dst) { move_list_.emplace_back(src, dst); }
  bool Contain(INodePtr src, INodePtr dst);
  void Delete(INodePtr src, INodePtr dst);
  void Prepend(INodePtr src, INodePtr dst) {
    move_list_.emplace_front(src, dst);
  }
  [[nodiscard]] MoveList *Union(MoveList *list);
  [[nodiscard]] MoveList *Intersect(MoveList *list);
  [[nodiscard]] MoveList *Substract(MoveList *list);

private:
  std::list<std::pair<INodePtr, INodePtr>> move_list_;
};

struct LiveGraph {
  IGraphPtr interf_graph;
  MoveList *moves;
  std::map<temp::Temp*, double> priority;

  LiveGraph(IGraphPtr interf_graph, MoveList *moves)
      : interf_graph(interf_graph), moves(moves) {}
  LiveGraph() {};
};

class LiveGraphFactory {
public:
  explicit LiveGraphFactory(fg::FGraphPtr flowgraph)
      : flowgraph_(flowgraph), live_graph_(new IGraph(), new MoveList()),
        in_(std::make_unique<graph::Table<assem::Instr, temp::TempList>>()),
        out_(std::make_unique<graph::Table<assem::Instr, temp::TempList>>()),
        temp_node_map_(new tab::Table<temp::Temp, INode>()) {}
  void Liveness();
  LiveGraph GetLiveGraph() { return live_graph_; }
  tab::Table<temp::Temp, INode> *GetTempNodeMap() { return temp_node_map_; }

private:
  fg::FGraphPtr flowgraph_;
  LiveGraph live_graph_;

  std::unique_ptr<graph::Table<assem::Instr, temp::TempList>> in_;
  std::unique_ptr<graph::Table<assem::Instr, temp::TempList>> out_;
  tab::Table<temp::Temp, INode> *temp_node_map_;

  void LiveMap();
  void InterfGraph();

  graph::Node<temp::Temp>* GetNode(graph::Graph<temp::Temp>* graph, temp::Temp* temp);
  void ShowTemp(temp::Temp* temp);
  void AddPrecoloredConflice(graph::Graph<temp::Temp>* graph);

};

temp::TempList* Union(temp::TempList* left, temp::TempList* right);
temp::TempList* Substract(temp::TempList* left, temp::TempList* right);
bool Equal(temp::TempList* left, temp::TempList* right);
bool Equal(std::map<graph::Node<assem::Instr>*, temp::TempList* > lhs, std::map<graph::Node<assem::Instr>*, temp::TempList* > rhs);
bool Contain(temp::TempList* container, temp::Temp* temp);

} // namespace live

#endif