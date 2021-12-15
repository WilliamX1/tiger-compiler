#include "tiger/regalloc/color.h"

extern frame::RegManager *reg_manager;

#define DEBUG

#ifdef DEBUG
  #define LOG(format, args...) do {   \
    FILE* debug_log = fopen("color.log", "a+");  \
    fprintf(debug_log, "%d, %s: ", __LINE__, __func__); \
    fprintf(debug_log, format, ##args); \
    fclose(debug_log);\
  } while (0)
#else
  #define LOG(format, args...) do{} while (0)
#endif

namespace col {
/* TODO: Put your lab6 code here */

Result Color::Color(frame::Frame* f, assem::InstrList* il) {
    LOG("RegAlloc\n");
    bool done = true;
    while (done) {
      done = false;

      Build();
      MakeWorkList();

      while (simplify_worklist_ || worklist_moves_ || freeze_worklist_ || spill_worklist_) {
        if (simplify_worklist_) Simplify();
        else if (worklist_moves_) Coalesce();
        else if (freeze_worklist_) Freeze();
        else if (spill_worklist_) SelectSpill();
      };

      AssignColors();

      if (spilled_nodes_) {
        il = RewriteProgram(f, il);
        done = true;
      }
    }
  };

void StringBoolTableShow(std::string* s, bool* b) {
    LOG("%s true? %d\n", s->c_str(), *b);
};

void NodeStringTableShow(live::INode* n, std::string* s) {
    if (s) LOG("t%d -> %s\n", n->NodeInfo()->Int(), s->c_str());
    else LOG("t%d -> NULL\n", n->NodeInfo()->Int());
};

void NodeIntTableShow(live::INode* n, int* s) {
    if (s) LOG("t%d -> Num: %d\n", n->NodeInfo()->Int(), *s);
    else LOG("t%d -> NULL\n", n->NodeInfo()->Int());
};

void NodeNodeTableShow(live::INode* n, live::INode* s) {
    if (s) LOG("t%d -> t%d\n", n->NodeInfo()->Int(), s->NodeInfo()->Int());
    else LOG("t%d -> NULL\n", n->NodeInfo()->Int());
};

void NodeMoveTableShow(live::INode* n, live::MoveList* s) {
    if (s) {
        LOG("t%d: ---v\n", n->NodeInfo()->Int());
        for (auto move : s->GetList()) {
            LOG("Move:  t%d ---> t%d\n", move.first->NodeInfo()->Int(), move.second->NodeInfo()->Int());
        };
    }
    else LOG("t%d NULL\n", n->NodeInfo()->Int());
};

void TempGraphShow(FILE* out, temp::Temp* t) {
    fprintf(out, "Livegraph: t%d -> ", t->Int());
};

void Color::ShowStatus() {
// #ifdef DEBUG
//     FILE* debug_log = fopen("color.log", "a+");
//     fprintf(debug_log, "simplify_worklist_: \n");
//     livegraph.interf_graph->Show(debug_log, simplify_worklist_, TempGraphShow);
//     fprintf(debug_log, "freeze_worklist_: \n");
//     livegraph.interf_graph->Show(debug_log, freeze_worklist_, TempGraphShow);
//     fprintf(debug_log, "spill_worklist_: \n");
//     livegraph.interf_graph->Show(debug_log, spill_worklist_, TempGraphShow);
//     fprintf(debug_log, "spilled_nodes_: \n");
//     livegraph.interf_graph->Show(debug_log, spilled_nodes_, TempGraphShow);
//     fprintf(debug_log, "coalesced_nodes_: \n");
//     livegraph.interf_graph->Show(debug_log, coalesced_nodes_, TempGraphShow);
//     fprintf(debug_log, "colored_nodes_: \n");
//     livegraph.interf_graph->Show(debug_log, colored_nodes_, TempGraphShow);
//     fprintf(debug_log, "selectStack: \n");
//     livegraph.interf_graph->Show(debug_log, select_stack_, TempGraphShow);

//     fprintf(debug_log, "coalesced_moves: \n");
//     if (coalesced_moves_)
//         for (auto ele : coalesced_moves_->GetList())
//             fprintf(debug_log, "Move:   t%d ---> t%d\n", ele.first->NodeInfo()->Int(), ele.second->NodeInfo()->Int());
//     fprintf(debug_log, "constrained_moves: \n");
//     if (constrained_moves_)
//         for (auto ele : constrained_moves_->GetList())
//             fprintf(debug_log, "Move:   t%d ---> t%d\n", ele.first->NodeInfo()->Int(), ele.second->NodeInfo()->Int());
//     fprintf(debug_log, "frozen_moves: \n");
//     if (frozen_moves_)
//         for (auto ele : frozen_moves_->GetList())
//             fprintf(debug_log, "Move:   t%d ---> t%d\n", ele.first->NodeInfo()->Int(), ele.second->NodeInfo()->Int());
//     fprintf(debug_log, "worklist_moves_: \n");
//     if (worklist_moves_)
//         for (auto ele : worklist_moves_->GetList())
//             fprintf(debug_log, "Move:   t%d ---> t%d\n", ele.first->NodeInfo()->Int(), ele.second->NodeInfo()->Int());
//     fprintf(debug_log, "active_moves_: \n");
//     if (active_moves_)
//         for (auto ele : active_moves_->GetList())
//             fprintf(debug_log, "Move:   t%d ---> t%d\n", ele.first->NodeInfo()->Int(), ele.second->NodeInfo()->Int());
    
//     LOG("degree_tab_: \n");
//     if (degree_tab_) degree_tab_->Dump(NodeIntTableShow);
//     LOG("alias_tab_: \n");
//     if (alias_tab_) alias_tab_->Dump(NodeNodeTableShow);
//     LOG("movelist_tab_: \n");
//     if (movelist_tab_) movelist_tab_->Dump(NodeMoveTableShow);
//     LOG("color_tab_: \n");
//     if (color_tab_) color_tab_->Dump(NodeStringTableShow);
// #endif
    return;
};

void Color::Build() {
    LOG("Build\n");
    simplify_worklist_ = NULL;
    freeze_worklist_ = NULL;
    spill_worklist_ = NULL;

    spilled_nodes_ = NULL;
    coalesced_nodes_ = NULL;
    colored_nodes_ = NULL;

    select_stack_ = NULL;

    coalesced_moves_ = NULL;
    constrained_moves_ = NULL;
    frozen_moves_ = NULL;
    worklist_moves_ = live_graph_.moves;
    active_moves_ = NULL;

    degree_tab_ = new tab::Table<int>();
    movelist_tab_ = new tab::Table<live::MoveList>();
    alias_tab_ = new tab::Table<live::INode>();
    color_tab_ = new tab::Table<std::string>();

    for (auto node : live_graph_.interf_graph->Nodes()->GetList()) {
        degree_tab_->Enter(node, new int(node->OutDegree()));

        color_tab_->Enter(node, reg_manager->temp_map_->Look(node->NodeInfo()));

        alias_tab_->Enter(node, node);

        live::MoveList* movelist = new live::MoveList();
        for (auto list : worklist_moves_->GetList()) {
            if (list.first == node || list.second == node)
                movelist->Prepend(list.first, list.second);
        };
        movelist_tab_->Enter(node, movelist);
    }
};

bool MoveRelated(Node* n) {
    return NodeMoves(n) != NULL;
}


void Color::MakeWorkList() {
    LOG("MakeWorklist\n");
    for (auto node : live_graph_.interf_graph->Nodes()->GetList()) {
        if (color_tab_->Look(node)) continue;

        if (*degree_tab_->Look(node) >= K) this->spill_worklist_->Prepend(node);

        else {
            live::MoveList* left = this->movelist_tab_->Look(node);
            if (left->Intersect(active_moves_->Union(worklist_moves_)) != NULL) this->freeze_worklist_->Prepend(node);
            else this->simplify_worklist_->Prepend(node);
        };
    };
};

void Color::Simplify() {
    LOG("~~~~~~~~~~~~Before Simplify~~~~~~~~~~~\n");
    ShowStatus();

    
}

} // namespace col
