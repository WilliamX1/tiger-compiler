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

void ShowAssem(assem::Instr* p) {
  if (p == NULL) {
    fprintf(stderr, "Impossible Instr is NULL\n");
    return;
  }
  switch (p->kind_)
  {
  case assem::Instr::Kind::LABEL:
    LOG("LabelInstr assem: %s\n", ((assem::LabelInstr*) p)->assem_.c_str());
    break;
  case assem::Instr::Kind::MOVE:
    LOG("MoveInstr assem: %s\n", ((assem::MoveInstr*) p)->assem_.c_str());
    break;
  case assem::Instr::Kind::OPER:
    LOG("OperInstr assem: %s\n", ((assem::LabelInstr*) p)->assem_.c_str());
    break;
  default:
    break;
  };
  return;
}

std::string GetAssem(assem::Instr* p) {
  if (p == NULL) {
    fprintf(stderr, "Impossible Instr is NULL\n");
    return "";
  }
  std::string ret = "";
  switch (p->kind_)
  {
  case assem::Instr::Kind::LABEL:
    ret = ((assem::LabelInstr*) p)->assem_.c_str();
    break;
  case assem::Instr::Kind::MOVE:
    ret =((assem::MoveInstr*) p)->assem_.c_str();
    break;
  case assem::Instr::Kind::OPER:
    ret = ((assem::LabelInstr*) p)->assem_.c_str();
    break;
  default:
    break;
  };
  return ret;
}

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
  if (list)
    for (auto move : list->GetList()) {
      if (!res->Contain(move.first, move.second))
        res->move_list_.push_back(move);
    }
  return res;
}

MoveList *MoveList::Intersect(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : this->move_list_) {
    if (list && list->Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

MoveList *MoveList::Substract(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : this->move_list_) {
    if (list && !list->Contain(move.first, move.second))
      res->move_list_.push_back(move);
  };
  return res;
}

temp::TempList* Union(temp::TempList* left, temp::TempList* right) {
  LOG("Begin Union\n");
  temp::TempList *res = new temp::TempList();
  
  if (left)
    for (auto ele : left->GetList())
      res->Append(ele);
  if (right)
    for (auto ele : right->GetList())
      if (!Contain(res, ele))
        res->Append(ele);
  LOG("End Union\n");
  return res;
}

temp::TempList* Substract(temp::TempList* left, temp::TempList* right) {
  LOG("Begin Substract\n");
  temp::TempList* res = new temp::TempList();
  if (left != NULL)
    for (auto ele : left->GetList()) 
      if (!Contain(right, ele))
        res->Append(ele);
  LOG("End Substract\n");
  return res;
}

bool Equal(temp::TempList* left, temp::TempList* right) {
  LOG("Begin Equal\n");
  bool ret;
  if (left == NULL && right == NULL) ret = true;
  else if (left == NULL || right == NULL) ret = false;
  else {
    ret = true;
    for (auto ele : left->GetList())
      if (!Contain(right, ele))
      {
        ret = false;
        break;
      };
    for (auto ele : right->GetList()) 
      if (!Contain(left, ele))
      {
        ret = false;
        break;
      };
  };
  LOG("End Equal\n");
  return ret;
}

bool Contain(temp::TempList* left, temp::Temp* right) {
  LOG("Begin Contain\n");
  bool ret = false;
  if (left != NULL) {
    std::list<temp::Temp *> list = left->GetList();
    ret = std::find(list.begin(), list.end(), right) != list.end();
  };
  LOG("End Contain\n");
  return ret;
}

graph::Node<temp::Temp>* LiveGraphFactory::GetNode(graph::Graph<temp::Temp>* graph, temp::Temp* temp) {
  LOG("Begin GetNode\n");
  if (temp_node_map_->Look(temp) == NULL) temp_node_map_->Enter(temp, graph->NewNode(temp));
  graph::Node<temp::Temp>* ret = temp_node_map_->Look(temp);
  assert(ret != NULL);
  LOG("End GetNode\n");
  return ret;
}

void LiveGraphFactory::ShowTemp(temp::Temp* temp) {
  assert(temp);
  LOG("reg t%d\n", temp->Int());
}

void LiveGraphFactory::AddPrecoloredConflice(graph::Graph<temp::Temp>* graph) {
  LOG("Begin AddPrecoloredConflice\n");
  for (auto temp1 : reg_manager->Registers()->GetList())
    for (auto temp2 : reg_manager->Registers()->GetList()) {
      graph::Node<temp::Temp>* temp1_node = GetNode(graph, temp1);
      graph::Node<temp::Temp>* temp2_node = GetNode(graph, temp2);
      if (temp1_node != temp2_node) {
        this->live_graph_.interf_graph->AddEdge(temp1_node, temp2_node);
        this->live_graph_.interf_graph->AddEdge(temp2_node, temp1_node);
      };
    };
  LOG("End AddPrecoloredConflice\n");
  return;
};

void LiveGraphFactory::LiveMap() {
  LOG("Begin LiveMap\n");
  /* TODO: Put your lab6 code here */
  this->live_graph_.interf_graph = new graph::Graph<temp::Temp>();
  this->live_graph_.moves = new live::MoveList();
  this->temp_node_map_ = new tab::Table<temp::Temp, live::INode>();

  bool fixed_point = false;
  int round = 0;
  
  // auto flowgraph_reverse_list = this->flowgraph_->Nodes()->GetList();
  // reverse(flowgraph_reverse_list.begin(), flowgraph_reverse_list.end());

  while (!fixed_point) {
    /* Maybe Errors When Compare */
    LOG("Round %d\n", round++);
    fixed_point = true;

    auto flowgraph_reverse_list = this->flowgraph_->Nodes()->GetList();
    reverse(flowgraph_reverse_list.begin(), flowgraph_reverse_list.end());

    for (auto p : flowgraph_reverse_list) {
      live::ShowAssem(p->NodeInfo());

      temp::TempList* oldin_templist = in_->Look(p);
      temp::TempList* oldout_templist = out_->Look(p);

      temp::TempList* defs = p->NodeInfo()->Def();
      temp::TempList* uses = p->NodeInfo()->Use();

      int defs_size = defs ? defs->GetList().size() : 0;
      int uses_size = uses ? uses->GetList().size() : 0;
      LOG("Defs size: %d, Uses size: %d\n", defs_size, uses_size);
      
      // if (in_->Look(p) == NULL) 
      //   in_->Enter(p, Union(uses, Substract(out_->Look(p), defs)));
      // // else 
      //   // in_->Set(p, Union(uses, Substract(out_->Look(p), defs)));
      
      // // if (out_->Look(p) == NULL) 
      //   out_->Enter(p, NULL);
      // // else 
      //   // out_->Set(p, NULL);
      // for (auto q : p->Succ()->GetList()) {
      //   // if (out_->Look(p) == NULL) 
      //     out_->Enter(p, Union(out_->Look(p), in_->Look(q)));
      //   // else 
      //     // out_->Set(p, Union(out_->Look(p), in_->Look(q)));
      // };


      // if (out_->Look(p) == NULL) 
        out_->Enter(p, NULL);
      // else 
        // out_->Set(p, NULL);
      for (auto q : p->Succ()->GetList()) {
        // if (out_->Look(p) == NULL) 
          out_->Enter(p, Union(out_->Look(p), in_->Look(q)));
        // else
          // out_->Set(p, Union(out_->Look(p), in_->Look(q)));
      };
      
      // if (in_->Look(p) == NULL) 
        in_->Enter(p, Union(uses, Substract(out_->Look(p), defs)));
      // else 
        // in_->Set(p, Union(uses, Substract(out_->Look(p), defs)));

      int in_size = in_->Look(p) ? in_->Look(p)->GetList().size() : 0;
      int out_size = out_->Look(p) ? out_->Look(p)->GetList().size() : 0;
      LOG("In size: %d, Out size: %d\n", in_size, out_size);

      if (!Equal(oldin_templist, in_->Look(p)) || !Equal(oldout_templist, out_->Look(p)))
        fixed_point = false;
    };

    reverse(flowgraph_reverse_list.begin(), flowgraph_reverse_list.end());
  };
  LOG("End LiveMap\n");
  return;
}

void LiveGraphFactory::InterfGraph() {
  LOG("Begin InterGraph\n");
  /* TODO: Put your lab6 code here */
  AddPrecoloredConflice(live_graph_.interf_graph);
  LOG("Inter 1\n");
  for (auto p : flowgraph_->Nodes()->GetList()) {
    temp::TempList* defs = p->NodeInfo()->Def();
    temp::TempList* uses = p->NodeInfo()->Use();
    LOG("kind: %d", p->NodeInfo()->kind_);
    if (p->NodeInfo()->kind_ == assem::Instr::Kind::MOVE) {
      if (!uses || !defs || uses->GetList().empty() || defs->GetList().empty()) {
        fprintf(stderr, "Impossible MOVE instr NULL\n");
        continue;
      };
      graph::Node<temp::Temp>* src_node = GetNode(live_graph_.interf_graph, uses->GetList().front());
      graph::Node<temp::Temp>* dst_node = GetNode(live_graph_.interf_graph, defs->GetList().front());
      if (src_node == NULL || dst_node == NULL) {
        fprintf(stderr, "Impossible 0.1\n");
        return;
      }
      live_graph_.moves->Prepend(src_node, dst_node);
      
      auto out_iter = out_->Look(p);
      if (out_iter)
        for (auto q : out_iter->GetList()) {
          if (q == uses->GetList().front()) continue;
          graph::Node<temp::Temp>* out_node = GetNode(live_graph_.interf_graph, q);
          if (dst_node != out_node) {
            live_graph_.interf_graph->AddEdge(dst_node, out_node);
            live_graph_.interf_graph->AddEdge(out_node, dst_node);
          };
        };
    } else {
      if (defs)
        for (auto def : defs->GetList()) {
          auto out_iter = out_->Look(p);
          if (out_iter != NULL)
            for (auto q : out_iter->GetList()) {
              graph::Node<temp::Temp>* dst_node = GetNode(live_graph_.interf_graph, def);
              graph::Node<temp::Temp>* out_node = GetNode(live_graph_.interf_graph, q);
              if (dst_node != out_node) {
                live_graph_.interf_graph->AddEdge(dst_node, out_node);
                live_graph_.interf_graph->AddEdge(out_node, dst_node);
              };
            };
        };
    };
  };

  LOG("Inter 2\n");

  for (auto p : flowgraph_->Nodes()->GetList()) {
    temp::TempList* defs = p->NodeInfo()->Def();
    temp::TempList* uses = p->NodeInfo()->Use();
    if (defs) for (auto q : defs->GetList()) live_graph_.priority[q]++;
    if (uses) for (auto q : uses->GetList()) live_graph_.priority[q]++;
  };
  LOG("Inter 3\n");
  for (auto p : live_graph_.interf_graph->Nodes()->GetList())
    live_graph_.priority[p->NodeInfo()] /= p->Degree();

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
