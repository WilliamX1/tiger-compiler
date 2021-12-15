#include "tiger/liveness/liveness.h"

extern frame::RegManager *reg_manager;

#define DEBUG

#ifdef DEBUG
  #define LOG(format, args...) do {   \
    FILE* debug_log = fopen("liveness.log", "a+");  \
    fprintf(debug_log, "%d, %s: ", __LINE__, __func__); \
    fprintf(debug_log, format, ##args); \
    fclose(debug_log);\
  } while (0)
#else
  #define LOG(format, args...) do{} while (0)
#endif

namespace live {

bool MoveList::Contain(INodePtr src, INodePtr dst) {
  return std::any_of(move_list_.cbegin(), move_list_.cend(),
                     [src, dst](std::pair<INodePtr, INodePtr> move) {
                       return move.first == src && move.second == dst;
                     });
}

void MoveList::Delete(INodePtr src, INodePtr dst) {
  assert(src && dst);
  auto move_it = move_list_.begin();
  for (; move_it != move_list_.end(); move_it++) {
    if (move_it->first == src && move_it->second == dst) {
      break;
    }
  }
  move_list_.erase(move_it);
}

MoveList *MoveList::Union(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : move_list_) {
    res->move_list_.push_back(move);
  }
  for (auto move : list->GetList()) {
    if (!res->Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

MoveList *MoveList::Intersect(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : list->GetList()) {
    if (Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

MoveList *MoveList::Difference(MoveList *left, MoveList *right) {
  auto *res = new MoveList();
  for (auto move : left->GetList()) {
    if (!right->Contain(move.first, move.second))
      res->move_list_.push_back(move);
  };
  return res;
}

temp::TempList* Union(temp::TempList* left, temp::TempList* right) {
  temp::TempList *res = new temp::TempList();
  for (auto ele : left->GetList())
    res->Append(ele);
  for (auto ele : right->GetList())
    if (!Contain(res, ele))
      res->Append(ele);
  return res;
}

temp::TempList* Difference(temp::TempList* left, temp::TempList* right) {
  temp::TempList* res = new temp::TempList();
  for (auto ele : left->GetList()) 
    if (!Contain(right, ele))
      res->Append(ele);
  return res;
}

bool Equal(temp::TempList* left, temp::TempList* right) {
  for (auto ele : left->GetList())
    if (!Contain(left, ele))
      return false;
  for (auto ele : right->GetList()) 
    if (!Contain(right, ele))
      return false;
  return true;
}

bool Contain(temp::TempList* left, temp::Temp* right) {
  std::list<temp::Temp *> list = left->GetList();
  return std::find(list.begin(), list.end(), right) != list.end();
}

void LiveGraphFactory::LiveMap() {
  /* TODO: Put your lab6 code here */
  for (auto node : this->flowgraph_->Nodes()->GetList()) {
    in_->Enter(node, NULL);
    out_->Enter(node, NULL);
  };
  /* repeat:
   *   for each node:
   *      in[node] <- use[node] U (out[node] - def[node])
   *      for n in succ[node]:
   *          out[node] <- out[node] U in[n]
   * until fixedpoint
   */
  LOG("Liveness: Find Fixed Point\n");
  bool fixedpoint = false;
  while (!fixedpoint) {
    LOG("\tRound\n");
    fixedpoint = true;
    auto reverse_list = this->flowgraph_->Nodes()->GetList();
    reverse_list.reverse();
    for (auto node : reverse_list) {
      temp::TempList* oldin_templist = in_->Look(node);
      temp::TempList* oldout_temolist = out_->Look(node);
      for (auto succ : node->Succ()->GetList())
        out_->Set(node, Union(out_->Look(node), in_->Look(succ)));
      in_->Set(node, Union(node->NodeInfo()->Use(), Difference(out_->Look(node), node->NodeInfo()->Def())));

      if (!Equal(oldin_templist, in_->Look(node)) || !Equal(oldout_temolist, out_->Look(node)))
        fixedpoint = false;
    };
  };
  return;
}

void LiveGraphFactory::InterfGraph() {
  /* TODO: Put your lab6 code here */
  /* Interference Graph 
   * 1. register interference
   * 2. non-move instr: addEdge(def, out)
   *    move instr    : addEdge(def, out-use)
   */
  LOG("Liveness: Build Interference Graph\n");
  this->live_graph_;

  /* step 1 */
  this->temp_node_map_;
  for (auto ele : reg_manager->Registers()->GetList()) {
    auto node = this->live_graph_.interf_graph->NewNode(ele);
    this->temp_node_map_->Enter(ele, node);
  };

  auto reg_list = reg_manager->Registers()->GetList();
  auto from_iter = reg_list.begin();
  auto to_iter = reg_list.begin();
  while (from_iter != reg_list.end()) {
    to_iter = from_iter;
    while (++to_iter != reg_list.end()) {
      this->live_graph_.interf_graph->AddEdge(this->temp_node_map_->Look(*from_iter), this->temp_node_map_->Look(*to_iter));
      this->live_graph_.interf_graph->AddEdge(this->temp_node_map_->Look(*to_iter), this->temp_node_map_->Look(*from_iter));
    };
  };

  /* step 2 */
  for (auto node : this->flowgraph_->Nodes()->GetList()) {
    for (auto temp : Union(out_->Look(node), node->NodeInfo()->Def())->GetList()) {
      if (temp == reg_manager->StackPointer()) continue;
      if (!this->temp_node_map_->Look(temp))
        this->temp_node_map_->Enter(temp, this->live_graph_.interf_graph->NewNode(temp));
    };
  };

  for (auto node : this->flowgraph_->Nodes()->GetList()) {
    /* non-move instr */
    if (!node->NodeInfo()->kind_ == assem::Instr::Kind::MOVE) {
      for (auto def : node->NodeInfo()->Def()->GetList()) {
        for (auto out : out_->Look(node)->GetList()) {
          if (def == reg_manager->StackPointer() || out == reg_manager->StackPointer()) continue;

          this->live_graph_.interf_graph->AddEdge(this->temp_node_map_->Look(def), this->temp_node_map_->Look(out));
          this->live_graph_.interf_graph->AddEdge(this->temp_node_map_->Look(out), this->temp_node_map_->Look(def));
        };

        for (auto use : node->NodeInfo()->Use()->GetList()) {
          if (def == reg_manager->StackPointer() || use == reg_manager->StackPointer()) continue;

          if (!this->live_graph_.moves->Contain(this->temp_node_map_->Look(def), this->temp_node_map_->Look(use)))
            this->live_graph_.moves->Append(this->temp_node_map_->Look(def), this->temp_node_map_->Look(use));
        };
      };
    };

    return;
  }

  return;
}

void LiveGraphFactory::Liveness() {
  LiveMap();
  InterfGraph();
}

} // namespace live
