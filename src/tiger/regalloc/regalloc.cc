#include "tiger/regalloc/regalloc.h"

#include "tiger/output/logger.h"

extern frame::RegManager *reg_manager;

#define DEBUG

#ifdef DEBUG
  #define LOG(format, args...) do {   \
    FILE* debug_log = fopen("regalloc.log", "a+");  \
    fprintf(debug_log, "%d, %s: ", __LINE__, __func__); \
    fprintf(debug_log, format, ##args); \
    fclose(debug_log);\
  } while (0)
#else
  #define LOG(format, args...) do{} while (0)
#endif

namespace ra {
/* TODO: Put your lab6 code here */

// void RegAllocator::RegAlloc() {
//   LOG("Begin RegAlloc\n");

//   bool fixed_point = false;
//   while (!fixed_point) {
//     /* Flow Graph */
//     LOG("Begin AssemFlowGraph\n");
//     fg::FlowGraphFactory flow_graph_factory(assem_instr_.get()->GetInstrList());
//     flow_graph_factory.AssemFlowGraph();
//     auto flow_graph = flow_graph_factory.GetFlowGraph();
//     LOG("End AssemFlowGraph\n");

//     /* Live Graph */
//     LOG("Begin Liveness\n");
//     live::LiveGraphFactory live_graph_factory(flow_graph);
//     live_graph_factory.Liveness();
//     auto live_graph = live_graph_factory.GetLiveGraph();
//     LOG("End Liveness\n");

//     /* Try Color */
//     LOG("Begin Color\n");
//     col::Color color(live_graph);
//     auto res = color.GetResult();
//     LOG("End Color\n");

//     /* If Successfully Colored */
//     if (res.spills->GetList().empty()) {
//       LOG("Success\n");
//       fixed_point = true;
//       auto result = result_.get();
//       result->coloring_ = res.coloring;
//       result->il_ = assem_instr_.get()->GetInstrList();
//     } else {
//       /* ReBegin */
//       LOG("ReBegin\n");
//       for (auto& it : res.spills->GetList()) {
//         auto instr_list = assem_instr_.get()->GetInstrList();
//         for (auto iter = instr_list->GetList().cbegin(); iter != instr_list->GetList().cend(); iter++) {
//           /* Use */
//           if ((*iter)->Use()->Contain(it->NodeInfo())) {
//             frame_->s_offset -= frame::wordsize;
//             auto new_temp = temp::TempFactory::NewTemp();

//             instr_list->Insert(iter, new assem::OperInstr("leaq " + frame_->label->Name() + "_framesize(%rsp), `d0", new temp::TempList(new_temp), NULL, NULL));
//             instr_list->Insert(iter, new assem::OperInstr("addq $" + std::to_string(frame_->s_offset) + ", `d0", new temp::TempList(new_temp), NULL, NULL));
//             instr_list->Insert(iter, new assem::OperInstr("movq (`s0), `d0", new temp::TempList(it->NodeInfo()), new temp::TempList(new_temp), NULL));
//           };
//           /* Def */
//           if ((*iter)->Def()->Contain(it->NodeInfo())) {
//             ++iter;
//             frame_->s_offset -= frame::wordsize;
//             auto new_temp = temp::TempFactory::NewTemp();
            
//             instr_list->Insert(iter, new assem::OperInstr("leaq " + frame_->label->Name() + "_framesize(%rsp), `d0", new temp::TempList(new_temp), NULL, NULL));
//             instr_list->Insert(iter, new assem::OperInstr("addq $" + std::to_string(frame_->s_offset) + ", `d0", new temp::TempList(new_temp), NULL, NULL));
//             instr_list->Insert(iter, new assem::OperInstr("movq `s0, (`d0)", new temp::TempList(new_temp), new temp::TempList(it->NodeInfo()), NULL));
//             --iter;
//           };
//         };
//       };
//     };
//   };
//   LOG("End RegAlloc\n");
// };

bool RegAllocator::Contain(live::INodePtr node, live::INodeListPtr list) {
  LOG("Begin Contain\n");
  bool ret = false;
  for (auto ele : list->GetList()) {
    if (ele->NodeInfo() == node->NodeInfo()) {
      ret = true;
      break;
    };
  };
  LOG("End Contain\n");
  return false;
};

live::INodeListPtr RegAllocator::Union(live::INodeListPtr left, live::INodeListPtr right) {
  LOG("Begin Contain\n");
  live::INodeListPtr result = new live::INodeList();
  for (auto ele : left->GetList())
    result->Fusion(ele);
  for (auto ele : right->GetList())
    result->Fusion(ele);
  LOG("End Contain\n");
  return result;
};

live::INodeListPtr RegAllocator::Sub(live::INodeListPtr left, live::INodeListPtr right) {
  LOG("Begin Sub\n");
  live::INodeListPtr result = new live::INodeList();
  for (auto ele : left->GetList()) {
    if (!Contain(ele, right)) {
      result->Append(ele);
    };
  };
  LOG("End Sub\n");
  return result;
};

live::INodeListPtr RegAllocator::Intersect(live::INodeListPtr left, live::INodeListPtr right) {
  LOG("Begin Intersect\n");
  live::INodeListPtr result = new live::INodeList();
  for (auto ele : left->GetList()) {
    if (Contain(ele, right)) {
      result->Append(ele);
    };
  };
  LOG("End Intersect\n");
  return result;
};

bool RegAllocator::Contain(std::pair<live::INodePtr, live::INodePtr> node, live::MoveList* list) {
  LOG("Begin Contain\n");
  bool ret = false;
  for (auto ele : list->GetList()) {
    if (ele == node) {
      ret = true;
      break;
    };
  };
  LOG("End Contain\n");
  return false;
};

live::MoveList* RegAllocator::Union(live::MoveList* left, live::MoveList* right) {
  LOG("Begin Union\n");
  live::MoveList* result = new live::MoveList();
  for (auto ele : left->GetList())
    result->Fusion(ele.first, ele.second);
  for (auto ele : right->GetList()) 
    result->Fusion(ele.first, ele.second);
  LOG("End Union\n");
  return result;
};

live::MoveList* RegAllocator::Intersect(live::MoveList* left, live::MoveList* right) {
  LOG("Begin Intersect\n");
  live::MoveList* result = new live::MoveList();
  for (auto ele : left->GetList()) {
    if (Contain(ele, right)) {
      result->Append(ele.first, ele.second);
    };
  };
  LOG("End Intersect\n");
  return result;
};

void RegAllocator::Init() {
  LOG("Begin Init\n");

  K = reg_manager->Registers()->GetList().size();

  if (precolored_) precolored_->Clear();
  else precolored_ = new live::INodeList();
  if (initial_) initial_->Clear();
  else initial_ = new live::INodeList();

  if (simplify_work_list_) simplify_work_list_->Clear();
  else simplify_work_list_ = new live::INodeList();
  if (freeze_work_list_) freeze_work_list_->Clear();
  else freeze_work_list_ = new live::INodeList();
  if (spill_work_list_) spill_work_list_->Clear();
  else spill_work_list_ = new live::INodeList();
  if (spilled_nodes_) spilled_nodes_->Clear();
  else spilled_nodes_ = new live::INodeList();
  if (coalesced_nodes_) coalesced_nodes_->Clear();
  else coalesced_nodes_ = new live::INodeList();
  if (colored_nodes_) colored_nodes_->Clear();
  else colored_nodes_ = new live::INodeList();
  if (select_stack_) select_stack_->Clear();
  else select_stack_ = new live::INodeList();
  if (coalesced_moves_) delete coalesced_moves_;
  coalesced_moves_ = new live::MoveList();
  if (constrained_moves_) delete constrained_moves_;
  constrained_moves_ = new live::MoveList();
  if (frozen_moves_) delete frozen_moves_;
  frozen_moves_ = new live::MoveList();
  if (worklist_moves_) delete worklist_moves_;
  worklist_moves_ = new live::MoveList();
  if (active_moves_) delete active_moves_;
  active_moves_ = new live::MoveList();

  adj_set_.clear();
  adj_list_.clear();
  degree_.clear();
  move_list_.clear();
  alias_.clear();
  color_.clear();

  LOG("End Init\n");

  return;
};

void RegAllocator::Build() {
  LOG("Begin Build\n");
  for (auto ele : live_graph_.moves->GetList()) {
    live::INodePtr src = ele.first;
    live::INodePtr dst = ele.second;
    if (move_list_.find(src) == move_list_.end()) move_list_[src] = new live::MoveList();
    move_list_[src]->Append(src, dst);
    if (move_list_.find(dst) == move_list_.end()) move_list_[dst] = new live::MoveList();
    move_list_[dst]->Append(src, dst);
  };

  worklist_moves_ = live_graph_.moves;
  for (auto node : live_graph_.interf_graph->Nodes()->GetList()) {
    for (auto adj : node->Adj()->GetList()) {
      AddEdge(node, adj);
      degree_[node] = 0;
    };
  };

  /* Init precolored */
  precolored_->Clear();
  std::list<temp::Temp *> regs = reg_manager->Registers()->GetList();
  for (auto node : live_graph_.interf_graph->Nodes()->GetList()) {
    std::string* name = reg_manager->temp_map_->Look(node->NodeInfo());
    if (name != NULL) precolored_->Fusion(node);
  };
  /* Init color and initial */
  for (auto node : live_graph_.interf_graph->Nodes()->GetList()) {
    std::string* name = reg_manager->temp_map_->Look(node->NodeInfo());
    if (name != NULL) color_[node->NodeInfo()] = *name;
    else initial_->Fusion(node);

    LOG("TEMP: %d\n", node->NodeInfo()->Int());
  };
  LOG("End Build\n");
  return;
};

void RegAllocator::AddEdge(live::INodePtr u, live::INodePtr v) {
  LOG("Begin AddEdge\n");
  assert(u != NULL);
  assert(v != NULL);
  if (adj_set_.find(std::make_pair(u, v)) == adj_set_.end() && u != v) {
    adj_set_.insert(std::make_pair(u, v));
    adj_set_.insert(std::make_pair(v, u));
    if (!precolored_->Contain(u)) {
      adj_list_[u]->Fusion(v);
      // if (degree_.find(u) == degree_.end()) degree_[u] = 0;
      degree_[u]++;
      LOG("Degree : %d\n", degree_[u]);
    };
    if (!precolored_->Contain(v)) {
      adj_list_[v]->Fusion(u);
      // if (degree_.find(v) == degree_.end()) degree_[v] = 0;
      degree_[v]++;
      LOG("Degree : %d\n", degree_[v]);
    };
  };
  LOG("End AddEdge\n");
  return;
};

void RegAllocator::MakeWorkList() {
  LOG("Begin MakeWorkList\n");
  for (auto node : initial_->GetList()) {
    if (degree_[node] >= K) spill_work_list_->Fusion(node);
    else if (MoveRelated(node)) freeze_work_list_->Fusion(node);
    else simplify_work_list_->Fusion(node);
  };
  initial_->Clear();
  LOG("End MakeWorkList\n");
  return;
};

live::INodeListPtr RegAllocator::Adjacent(live::INodePtr node) {
  LOG("Begin Adjacent\n");
  if (adj_list_.find(node) == adj_list_.end()) {
    LOG("Impossible \n");
    adj_list_[node] = new live::INodeList();
    LOG("End Adjacent\n");
    return new live::INodeList();
  }
  live::INodeListPtr ret = adj_list_[node]->Diff(Union(select_stack_, coalesced_nodes_));
  LOG("End Adjacent\n");
  return ret;
};

live::MoveList* RegAllocator::NodeMoves(live::INodePtr node) {
  LOG("Begin NodeMoves\n");
  if (move_list_.find(node) == move_list_.end()) {
    LOG("Impossible \n");
    move_list_[node] = new live::MoveList();
    LOG("End NodeMoves\n");
    return new live::MoveList();
  }
  live::MoveList* ret = Intersect(move_list_[node], Union(active_moves_, worklist_moves_));
  LOG("End NodeMoves\n");
  return ret;
};

bool RegAllocator::MoveRelated(live::INodePtr node) {
  LOG("Begin MoveRelated\n");
  bool ret = !(NodeMoves(node)->GetList().empty());
  LOG("End MoveRelated\n");
  return ret;
};

void RegAllocator::Simplify() {
  LOG("Begin Simplify\n");
  if (!(simplify_work_list_->GetList().empty())) {
    auto node = simplify_work_list_->GetList().front();
    simplify_work_list_->DeleteNode(node);
    select_stack_->Append(node);
    for (auto tmp : Adjacent(node)->GetList()) 
      DecrementDegree(tmp);
  };
  LOG("End Simplify\n");
  return;
};

void RegAllocator::DecrementDegree(live::INodePtr m) {
  LOG("Begin DecreementDegree\n");
  int d = degree_[m];
  degree_[m] = d - 1;
  if (d == K) {
    live::INodeListPtr list = Adjacent(m);
    list->Fusion(m);
    EnableMoves(list);
    spill_work_list_->DeleteNode(m);
    if (MoveRelated(m)) freeze_work_list_->Fusion(m);
    else simplify_work_list_->Fusion(m);
  };
  LOG("End DecreementDegree\n");
  return;
};

void RegAllocator::EnableMoves(live::INodeListPtr nodes) {
  LOG("Begin EnableMoves\n");
  for (auto n : nodes->GetList()) {
    for (auto m : NodeMoves(n)->GetList()) {
      if (active_moves_->Contain(m.first, m.second)) {
        active_moves_->Delete(m.first, m.second);
        worklist_moves_->Fusion(m.first, m.second);
      };
    };
  };
  LOG("End EnableMoves\n");
  return;
};

void RegAllocator::Coalesce() {
  LOG("Begin Coalesce\n");
  if (worklist_moves_->GetList().empty()) {
    LOG("End Coalesce\n");
    return;
  };

  auto m = worklist_moves_->GetList().front();
  live::INodePtr x = m.first;
  live::INodePtr y = m.second;
  x = GetAlias(x);
  y = GetAlias(y);
  live::INodePtr u = NULL;
  live::INodePtr v = NULL;
  if (precolored_->Contain(y)) {
    u = y;
    v = x;
  } else {
    u = x;
    v = y;
  };
  worklist_moves_->Delete(m.first, m.second);
  if (u == v) {
    coalesced_moves_->Fusion(m.first, m.second);
    AddWorkList(u);
  } else if (precolored_->Contain(v) || adj_set_.find(std::make_pair(u, v)) != adj_set_.end()) {
    constrained_moves_->Fusion(m.first, m.second);
    AddWorkList(u);
    AddWorkList(v);
  } else {
    bool flag = true;
    for (auto t : Adjacent(v)->GetList()) {
      if (!OK(t, u)) {
        flag = false;
        break;
      };
    };

    if (precolored_->Contain(u) && flag
      || !precolored_->Contain(u) && Conservertive(Union(Adjacent(u), Adjacent(v)))) {
        coalesced_moves_->Fusion(m.first, m.second);
        Combine(u, v);
        AddWorkList(u);
    } else active_moves_->Fusion(m.first, m.second);
  };
  LOG("End Coalesce\n");
  return;
};

void RegAllocator::AddWorkList(live::INodePtr u) {
  LOG("Begin AddWorkList\n");
  if (!precolored_->Contain(u) && !MoveRelated(u) && degree_[u] < K) {
    freeze_work_list_->DeleteNode(u);
    simplify_work_list_->Fusion(u);
  };
  LOG("End AddWorkList\n");
  return;
};

bool RegAllocator::OK(live::INodePtr t, live::INodePtr r) {
  LOG("Begin OK\n");
  bool ret = degree_[t] < K || precolored_->Contain(t) || adj_set_.find(std::make_pair(t, r)) != adj_set_.end();
  LOG("End OK\n");
  return ret;
};

bool RegAllocator::Conservertive(live::INodeListPtr nodes) {
  LOG("Begin Conservertive\n");
  int k = 0;
  for (auto n : nodes->GetList()) {
    if (degree_[n] >= K) {
      k++;
    };
  };
  LOG("End Conservertive\n");
  return k < K;
};

live::INodePtr RegAllocator::GetAlias(live::INodePtr n) {
  LOG("Begin GetAlias\n");
  live::INodePtr ret = NULL;
  if (coalesced_nodes_->Contain(n))
    ret = GetAlias(alias_[n]);
  else ret = n;
  LOG("End GetAlias\n");
  return ret;
};

void RegAllocator::Combine(live::INodePtr u, live::INodePtr v) {
  LOG("Begin Combine\n");
  if (freeze_work_list_->Contain(v)) {
    freeze_work_list_->DeleteNode(v);
  } else {
    spill_work_list_->DeleteNode(v);
  };

  coalesced_nodes_->Fusion(v);
  alias_[v] = u;
  move_list_[u] = Union(move_list_[u], move_list_[v]);
  
  live::INodeListPtr v_list = new live::INodeList();
  v_list->Append(v);
  EnableMoves(v_list);

  for (auto t : Adjacent(v)->GetList()) {
    AddEdge(t, u);
    DecrementDegree(t);
  };
  if (degree_[u] >= K && freeze_work_list_->Contain(u)) {
    freeze_work_list_->DeleteNode(u);
    spill_work_list_->Fusion(u);
  };
  LOG("End Combine\n");
  return;
};

void RegAllocator::Freeze() {
  LOG("Begin Freeze\n");
  if (freeze_work_list_->GetList().empty()) {
    LOG("End Freeze\n");
    return;
  };

  auto u = freeze_work_list_->GetList().front();
  freeze_work_list_->DeleteNode(u);
  simplify_work_list_->Fusion(u);
  FreezeMoves(u);
  LOG("End Freeze\n");
  return;
};

void RegAllocator::FreezeMoves(live::INodePtr u) {
  LOG("Begin FreezeMoves\n");
  for (auto m : NodeMoves(u)->GetList()) {
    live::INodePtr x = m.first;
    live::INodePtr y = m.second;
    live::INodePtr v = NULL;
    if (GetAlias(y) == GetAlias(u)) v = GetAlias(x);
    else v = GetAlias(y);
    active_moves_->Delete(m.first, m.second);
    frozen_moves_->Fusion(m.first, m.second);
    if (NodeMoves(v)->GetList().empty() && degree_[v] < K) {
      freeze_work_list_->DeleteNode(v);
      simplify_work_list_->Fusion(v);
    };
  };
  LOG("End FreezeMoves\n");
  return;
};

void RegAllocator::SelectSpill() {
  live::INodePtr m = NULL;
  double chosen_priority = 1e20;
  for (auto tmp : spill_work_list_->GetList()) {
    if (not_spill_.find(tmp->NodeInfo()) != not_spill_.end()) continue;
    if (!spilled_nodes_->Contain(tmp) && !precolored_->Contain(tmp)) {
      double cal = live_graph_.priority[tmp->NodeInfo()] / double (degree_[tmp]);
      if (cal < chosen_priority) {
        m = tmp;
        chosen_priority = cal;
      }
    }
  };
  if (!m) m = spill_work_list_->GetList().front();
  spill_work_list_->DeleteNode(m);
  simplify_work_list_->Fusion(m);
  FreezeMoves(m);
};

void RegAllocator::AssignColor() {
  LOG("Begin AssignColors\n");
  auto list = select_stack_->GetList();
  LOG("SelectStack size: %d\n", (int) list.size());
  while (!list.empty()) {
    auto n = list.back();
    list.pop_back();

    LOG("TEMP: %d\n", n->NodeInfo()->Int());

    std::vector<std::string> ok_colors;
    for (auto color : reg_manager->Colors()) {
      ok_colors.push_back(color);
    };

    for (auto w : adj_list_[n]->GetList()) {
      if (Union(colored_nodes_, precolored_)->Contain(GetAlias(w))) {
        for (auto iter = ok_colors.begin(); iter != ok_colors.end(); iter++) {
          if (*iter == color_[GetAlias(w)->NodeInfo()]) {
            ok_colors.erase(iter);
            break;
          };
        };
      };
    };

    if (ok_colors.empty()) {
      spilled_nodes_->Fusion(n);
    } else {
      colored_nodes_->Fusion(n);
      color_[n->NodeInfo()] = ok_colors.front();
    };
  };
  select_stack_->Clear();
  for (auto n : coalesced_nodes_->GetList()) {
    color_[n->NodeInfo()] = color_[GetAlias(n)->NodeInfo()];
  };
  return;
};

void RegAllocator::RegAlloc() {
  LOG("Begin RegAlloc\n");

  Init();

  bool fixed_point = false;
  while (!fixed_point) {
    /* Flow Graph */
    LOG("Begin AssemFlowGraph\n");
    fg::FlowGraphFactory flow_graph_factory(assem_instr_.get()->GetInstrList());
    flow_graph_factory.AssemFlowGraph();
    flow_graph_ = flow_graph_factory.GetFlowGraph();
    LOG("End AssemFlowGraph\n");

    /* Live Graph */
    LOG("Begin Liveness\n");
    live::LiveGraphFactory live_graph_factory(flow_graph_);
    live_graph_factory.Liveness();
    live_graph_ = live_graph_factory.GetLiveGraph();
    LOG("End Liveness\n");

    /* Try Color */
    LOG("Begin Color\n");
    Init();
    for (auto node : live_graph_.interf_graph->Nodes()->GetList()) {
      adj_list_[node] = new live::INodeList();
      degree_[node] = 0;
    };
    Build();
    MakeWorkList();
    
    // LOG("Before:\n simplify_work_list_ size: %u\n worklist_moves_ size: %u\n freeze_work_list_ size: %u\n spill_work_list_ size: %u\n",
    //     simplify_work_list_->GetList().size(), worklist_moves_->GetList().size(), freeze_work_list_->GetList().size(), spill_work_list_->GetList().size());
    
    do {
      if (!simplify_work_list_->GetList().empty()) Simplify();
      if (!worklist_moves_->GetList().empty()) Coalesce();
      if (!freeze_work_list_->GetList().empty()) Freeze();
      if (!spill_work_list_->GetList().empty()) SelectSpill();
    } while (!simplify_work_list_->GetList().empty() || !worklist_moves_->GetList().empty()
      || !freeze_work_list_->GetList().empty() || !spill_work_list_->GetList().empty());
    // LOG("After:\n simplify_work_list_ size: %u\n worklist_moves_ size: %u\n freeze_work_list_ size: %u\n spill_work_list_ size: %u\n",
    //     simplify_work_list_->GetList().size(), worklist_moves_->GetList().size(), freeze_work_list_->GetList().size(), spill_work_list_->GetList().size());
    AssignColor();
    LOG("End Color\n");

    /* If Successfully Colored */
    if (spilled_nodes_->GetList().empty()) {
      LOG("Success\n");
      fixed_point = true;

      temp::Map* coloring = temp::Map::Empty();
      for (auto tmp : color_) {
        temp::Temp* t = tmp.first;
        std::string nm = tmp.second;
        std::string* name = new std::string(nm);
        coloring->Enter(t, name);
        LOG("Add Color: %d --> %s\n", t->Int(), nm.c_str());
      };
      for (auto tmp : reg_manager->Registers()->GetList())
      {
        std::string* name = reg_manager->temp_map_->Look(tmp);
        coloring->Enter(tmp, name);
        LOG("Add Color: %d --> %s\n", tmp->Int(), name->c_str());
      };
      coloring->Enter(reg_manager->StackPointer(), new std::string("%rsp"));

      SimplifyInstrList();
      result_ = std::make_unique<ra::Result>(coloring, assem_instr_.get()->GetInstrList());
    } else {
      /* RewriteProgram */
      RewriteProgram();
    };
  };
  LOG("End RegAlloc\n");
};

void RegAllocator::SimplifyInstrList() {
  LOG("Begin SimplifyInstrList\n");
  assem::InstrList* ret = new assem::InstrList();
  for (auto il : assem_instr_.get()->GetInstrList()->GetList()) {
    if (il->kind_ == assem::Instr::MOVE && !il->Def()->GetList().empty() && !il->Use()->GetList().empty() && 
        color_[il->Def()->GetList().front()] == color_[il->Use()->GetList().front()]) continue;
    else ret->Append(il);
  };
  assem_instr_ = std::make_unique<cg::AssemInstr>(ret);
  LOG("End SimplifyInstrList\n");
};

void RegAllocator::RewriteProgram() {
  LOG("Begin RewriteProgram\n");
  LOG("spilled_nodes_ size: %d\n", (int) spilled_nodes_->GetList().size());
  for (auto& it : spilled_nodes_->GetList()) {
    auto instr_list = assem_instr_.get()->GetInstrList();
    for (auto iter = instr_list->GetList().cbegin(); iter != instr_list->GetList().cend(); iter++) { 
      /* Use */
      if ((*iter)->Use()->Contain(it->NodeInfo())) {
        // frame_->s_offset -= frame::wordsize;
        // frame_->AllocLocal(true);
        auto new_temp = temp::TempFactory::NewTemp();
        not_spill_.insert(new_temp);

        (*iter)->Use()->Replace(it->NodeInfo(), new_temp);
        instr_list->Insert(iter, new assem::OperInstr("movq (" + frame_->label->Name() + "_framesize-" + std::to_string(-frame_->s_offset) + ")(`s0), `d0", 
          new temp::TempList(new_temp), new temp::TempList(reg_manager->StackPointer()), NULL));

        // instr_list->Insert(iter, new assem::OperInstr("leaq " + frame_->label->Name() + "_framesize(`s0), `d0", new temp::TempList(new_temp), new temp::TempList(reg_manager->StackPointer()), NULL));
        // instr_list->Insert(iter, new assem::OperInstr("addq $" + std::to_string(frame_->s_offset) + ", `d0", new temp::TempList(new_temp), NULL, NULL));
        // instr_list->Insert(iter, new assem::OperInstr("movq (`s0), `d0", new temp::TempList(it->NodeInfo()), new temp::TempList(new_temp), NULL));
      };
      /* Def */
      if ((*iter)->Def()->Contain(it->NodeInfo())) {
        ++iter;
        // frame_->s_offset -= frame::wordsize;
        // frame_->AllocLocal(true);
        auto new_temp = temp::TempFactory::NewTemp();
        not_spill_.insert(new_temp);
        
        (*iter)->Def()->Replace(it->NodeInfo(), new_temp);
        instr_list->Insert(iter, new assem::OperInstr("movq `s0, (" + frame_->label->Name() + "_framesize-" + std::to_string(-frame_->s_offset) + ")(`d0)", 
          new temp::TempList(reg_manager->StackPointer()), new temp::TempList(new_temp), NULL));

        // instr_list->Insert(iter, new assem::OperInstr("leaq " + frame_->label->Name() + "_framesize(`s0), `d0", new temp::TempList(new_temp), new temp::TempList(reg_manager->StackPointer()), NULL));
        // instr_list->Insert(iter, new assem::OperInstr("addq $" + std::to_string(frame_->s_offset) + ", `d0", new temp::TempList(new_temp), NULL, NULL));
        // instr_list->Insert(iter, new assem::OperInstr("movq `s0, (`d0)", new temp::TempList(new_temp), new temp::TempList(it->NodeInfo()), NULL));
        --iter;
      };
    };
  };

  LOG("End RewriteProgram\n");
}

} // namespace ra