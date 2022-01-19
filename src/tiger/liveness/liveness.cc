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
  live::MoveList *res = new MoveList();
  for (auto move : move_list_) {
    res->move_list_.push_back(move);
  }
  if (list)
    for (auto move : list->GetList()) {
      if (!res->Contain(move.first, move.second))
        res->move_list_.push_back(move);
    }
  return res;
}

MoveList *MoveList::Intersect(MoveList *list) {
  live::MoveList* res = new MoveList();
  if (list)
    for (auto move : list->GetList()) {
      if (Contain(move.first, move.second))
        res->move_list_.push_back(move);
    };
  return res;
}

std::set<temp::Temp *> ToSet(const std::list<temp::Temp *>& origin) {
  std::set<temp::Temp *> res;
  for (auto &it : origin)
    res.insert(it);
  return res;
};

temp::TempList* ToTempList(const std::set<temp::Temp *>& origin) {
  temp::TempList* res = new temp::TempList();
  for (auto &it : origin) 
    res->Append(it);
  return res;
};

void LiveGraphFactory::LiveMap() {
  LOG("Begin LiveMap\n");
  /* TODO: Put your lab6 code here */

  /* Init in_ and out_ */
  auto in = in_.get();
  auto out = out_.get();
  for (auto &it : flowgraph_->Nodes()->GetList()) {
    in->Enter(it, new temp::TempList());
    out->Enter(it, new temp::TempList());
  };

  bool fixed_point = false;

  while (!fixed_point) {
    /* Maybe Errors When Compare */
    fixed_point = true;

    for (auto &it: flowgraph_->Nodes()->GetList()) {
      auto cur = it->NodeInfo();
      
      auto in_1 = ToSet(out->Look(it)->GetList());

      /* Add cur->Use() - cur->Def() into in_ */
      for (auto &iter : cur->Def()->GetList())
        in_1.erase(iter);
      for (auto &iter : cur->Use()->GetList())
        in_1.insert(iter);
      /* Judge whether change the original in_ TempList */
      if (in_1 != ToSet(in->Look(it)->GetList())) {
        fixed_point = false;
        in->Set(it, ToTempList(in_1));
      };

      std::set<temp::Temp *> out_1;
      for (auto &iter : it->Succ()->GetList())
        for (auto &iterator : in->Look(iter)->GetList())
          out_1.insert(iterator);
      /* Judge whether change the original out_ TempList */
      if (out_1 != ToSet(out->Look(it)->GetList())) {
        fixed_point = false;
        out->Set(it, ToTempList(out_1));
      };
    };
  };
  LOG("End LiveMap\n");
  return;
}

void LiveGraphFactory::InterfGraph() {
  LOG("Begin InterGraph\n");
  /* TODO: Put your lab6 code here */

  /* Add all used temp into templist */
  std::set<temp::Temp *> templist;
  for (auto &it : flowgraph_->Nodes()->GetList()) {
    for (auto &iter : it->NodeInfo()->Def()->GetList())
      templist.insert(iter);
    for (auto &iter : it->NodeInfo()->Use()->GetList()) 
      templist.insert(iter);
  };
  for (auto &it : reg_manager->Registers()->GetList())
    templist.insert(it);
  /* Create every node */
  for (auto &it : templist)
    temp_node_map_->Enter(it, live_graph_.interf_graph->NewNode(it));
  
  auto out = out_.get();

  for (auto &it : flowgraph_->Nodes()->GetList())
    for (auto &iter : it->NodeInfo()->Def()->GetList())
      for (auto &iterator : out->Look(it)->GetList())
        if (iter != iterator
          && !(it->NodeInfo()->kind_ == assem::Instr::MOVE && it->NodeInfo()->Use()->GetList().front() == iterator)) {
            live_graph_.interf_graph->AddEdge(temp_node_map_->Look(iter), temp_node_map_->Look(iterator));
            live_graph_.interf_graph->AddEdge(temp_node_map_->Look(iterator), temp_node_map_->Look(iter));
          };
  for (auto node : flowgraph_->Nodes()->GetList()) {
    temp::TempList* def = node->NodeInfo()->Def();
    temp::TempList* use = node->NodeInfo()->Use();
    if (node->NodeInfo()->kind_ == assem::Instr::MOVE && node->NodeInfo()->Def() && node->NodeInfo()->Use()) {
      for (auto d : def->GetList()) {
        for (auto temp : out->Look(node)->GetList()) {
          if (use->Contain(temp)) continue;
          // if (d == reg_manager->StackPointer() || temp == reg_manager->StackPointer()) continue;
          live_graph_.interf_graph->AddEdge(temp_node_map_->Look(d), temp_node_map_->Look(temp));
          live_graph_.interf_graph->AddEdge(temp_node_map_->Look(temp), temp_node_map_->Look(d));
        };

        for (auto u : use->GetList()) {
          // if (d == reg_manager->StackPointer() || u == reg_manager->StackPointer()) continue;
          // if (!live_graph_.moves->Contain(temp_node_map_->Look(u), temp_node_map_->Look(d))) {
            live_graph_.moves->Fusion(temp_node_map_->Look(u), temp_node_map_->Look(d));
          // }
        }
      }
    } else {
      for (auto d : def->GetList()) {
        for (auto temp : out->Look(node)->GetList()) {
          // if (d == reg_manager->StackPointer() || temp == reg_manager->StackPointer()) continue;
          auto dst = temp_node_map_->Look(d);
          auto out = temp_node_map_->Look(temp);
          if (dst != out) {
            live_graph_.interf_graph->AddEdge(dst, out);
            live_graph_.interf_graph->AddEdge(out, dst);
          };
        };
      };
    };
  };
  
  /* Add precolored conflict */
  for (auto iter : reg_manager->Registers()->GetList())
    for (auto iterator : reg_manager->Registers()->GetList())
      if (iter != iterator) {
        live_graph_.interf_graph->AddEdge(temp_node_map_->Look(iter), temp_node_map_->Look(iterator));
        live_graph_.interf_graph->AddEdge(temp_node_map_->Look(iterator), temp_node_map_->Look(iter));
      };
  
  /* Calculate priority */
  for (auto p : flowgraph_->Nodes()->GetList()) {
    temp::TempList* def = p->NodeInfo()->Def();
    temp::TempList* use = p->NodeInfo()->Use();
    for (auto q : def->GetList()) {
      if (live_graph_.priority.find(q) == live_graph_.priority.end()) live_graph_.priority[q] = 0;
      live_graph_.priority[q]++;
    };
    for (auto q : use->GetList()) {
      if (live_graph_.priority.find(q) == live_graph_.priority.end()) live_graph_.priority[q] = 0;
      live_graph_.priority[q]++;
    };
  };
  // for (auto p : live_graph_.interf_graph->Nodes()->GetList()) {
  //   if (live_graph_.priority.find(p->NodeInfo()) == live_graph_.priority.end()) live_graph_.priority[p->NodeInfo()] = 0;
  //   live_graph_.priority[p->NodeInfo()] /= p->Degree();
  // };

  LOG("End InterGraph\n");
  return;
}

void LiveGraphFactory::Liveness() {
  LOG("Begin Liveness\n");
  LiveMap();
  InterfGraph();
  LOG("End Liveness\n");
  return;
}

} // namespace live
