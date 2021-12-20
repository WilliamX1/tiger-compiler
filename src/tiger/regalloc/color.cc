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
bool Color::Precolored(temp::Temp* temp) {
    LOG("Begin Precolored\n");
    std::string* str = reg_manager->temp_map_->Look(temp);
    if (str) LOG("str: %s\n", (*str).c_str());
    
    bool res = reg_manager->temp_map_->Look(temp);
    LOG("res: %d\n", res);
    LOG("End Precolored\n");
    return res;
};

void Color::AddEdge(graph::Node<temp::Temp>* u, graph::Node<temp::Temp>* v) {
    LOG("Begin AddEdge\n");
    if (adj_set_.find(std::make_pair(u, v)) == adj_set_.end() && u != v) {
        LOG("AddEdge exe 0\n");
        adj_set_.insert(std::make_pair(u, v));
        adj_set_.insert(std::make_pair(v, u));
        if (!Precolored(u->NodeInfo())) {
            LOG("AddEdge execute\n");
            adj_list_[u].insert(v);
            degree_[u]++;
        };
        if (!Precolored(v->NodeInfo())) {
            LOG("AddEdge execute\n");
            adj_list_[v].insert(u);
            degree_[v]++;
        };
    };
    LOG("End AddEdge\n");
    return;
};

void Color::Build() {
    LOG("Begin Build\n");
    for (auto move : liveness_.moves->GetList()) {
        graph::Node<temp::Temp>* src_node = move.first;
        graph::Node<temp::Temp>* dst_node = move.second;
        if (!movelist_[src_node])
            movelist_[src_node] = new live::MoveList();
        if (!movelist_[dst_node])
            movelist_[dst_node] = new live::MoveList();
        movelist_[src_node]->Prepend(src_node, dst_node);
        movelist_[dst_node]->Prepend(src_node, dst_node);
    };
    LOG("1\n");
    worklist_moves_ = liveness_.moves;
    for (auto node : liveness_.interf_graph->Nodes()->GetList())
        for (auto adj : node->Adj()->GetList())
            AddEdge(node, adj);
    LOG("2\n");
    auto registers = reg_manager->Registers();
    for (auto reg : registers->GetList()) {
        std::string *color = reg_manager->temp_map_->Look(reg);
        coloring_->Enter(reg, color);
    };
    std::string* rsp_color = new std::string("%rsp");
    coloring_->Enter(reg_manager->RSP(), rsp_color);
    LOG("End Build\n");
    return;
};

live::MoveList* Color::NodeMoves(graph::Node<temp::Temp> *node) {
    LOG("Begin NodeMoves\n");
    live::MoveList* res = movelist_[node] ? movelist_[node]->Intersect(active_moves_->Union(worklist_moves_)) : NULL;
    LOG("End NodeMoves\n");
    return res;
};

bool Color::MoveRelated(graph::Node<temp::Temp>* node) {
    LOG("Begin MoveRelated\n");
    bool res = NodeMoves(node) != NULL && !NodeMoves(node)->GetList().empty();
    LOG("End MoveReplated\n");
    return res;
};

void Color::MakeWorkList() {
    LOG("Begin MakeWorkList\n");
    for (auto node : liveness_.interf_graph->Nodes()->GetList()) {
        if (Precolored(node->NodeInfo())) continue;
        if (degree_[node] >= reg_manager->K) spill_worklist_.insert(node);
        else if (MoveRelated(node)) freeze_worklist_.insert(node);
        else simplify_worklist_.insert(node);
    };
    LOG("End MakeWorkList\n");
    return;
};

void Color::EnableMoves(graph::NodeList<temp::Temp> *nodes) {
    LOG("Begin EnableMoves\n");
    if (nodes)
        for (auto node : nodes->GetList()) 
        {
            auto nodemoves = NodeMoves(node);
            if (nodemoves)
                for (auto move : nodemoves->GetList())
                    if (active_moves_ && active_moves_->Contain(move.first, move.second)) {
                        active_moves_ = active_moves_->Substract(new live::MoveList(move.first, move.second));
                        worklist_moves_ = worklist_moves_->Union(new live::MoveList(move.first, move.second));
                    };
        }
    LOG("End EnableMoves\n");
    return;
};

graph::NodeList<temp::Temp>* Color::Adjacent(graph::Node<temp::Temp>* node) {
    LOG("Begin Adjacent\n");
    graph::NodeList<temp::Temp>* result = new graph::NodeList<temp::Temp>();
    for (auto adj : adj_list_[node]) {
        if (std::find(select_stack_.begin(), select_stack_.end(), adj) == select_stack_.end()
        && coalesced_nodes_.find(adj) == coalesced_nodes_.end()) {
            result->Prepend(adj);
        };
    };
    LOG("End Adjacent\n");
    return result;
};

void Color::DecrementDegree(graph::Node<temp::Temp>* node) {
    LOG("Begin DecrementDegree\n");
    if (Precolored(node->NodeInfo())) return;
    int d = degree_[node];
    degree_[node] = d - 1;
    if (d == reg_manager->K) {
        graph::NodeList<temp::Temp>* list = new graph::NodeList<temp::Temp>();
        list->Append(node);
        list->CatList(Adjacent(node));

        EnableMoves(list);

        spill_worklist_.erase(node);
        if (MoveRelated(node)) freeze_worklist_.insert(node);
        else simplify_worklist_.insert(node);
    };
    LOG("End Decrement\n");
    return;
};

void Color::Simplify() {
    LOG("Begin Simplify\n");
    assert(!simplify_worklist_.empty());
    graph::Node<temp::Temp>* temp_node = *(simplify_worklist_.begin());
    simplify_worklist_.erase(temp_node);
    select_stack_.push_back(temp_node);
    for (auto adj : temp_node->Adj()->GetList()) {
        DecrementDegree(adj);
    };
    LOG("End Simplify\n");
    return;
};

graph::Node<temp::Temp>* Color::GetAlias(graph::Node<temp::Temp>* node) {
    LOG("Begin GetAlias\n");
    graph::Node<temp::Temp>* res = NULL;
    if (coalesced_nodes_.find(node) != coalesced_nodes_.end()) res = GetAlias(alias_[node]);
    else res = node;
    LOG("End GetAlias\n");
    return res;
};

void Color::AddWorklist(graph::Node<temp::Temp>* node) {
    LOG("Begin AddWorklist\n");
    if (!Precolored(node->NodeInfo()) && !MoveRelated(node) && degree_[node] < reg_manager->K) {
        freeze_worklist_.erase(node);
        simplify_worklist_.insert(node);
    };
    LOG("End AddWorklist\n");
};

bool Color::OK(graph::Node<temp::Temp>* t, graph::Node<temp::Temp>* r) {
    LOG("Begin OK\n");
    bool res = degree_[t] < reg_manager->K
        || Precolored(t->NodeInfo())
        || adj_set_.find(std::make_pair(t, r)) != adj_set_.end();
    LOG("End OK\n");
    return res;
};

bool Color::OKForAll(graph::NodeList<temp::Temp>* nodes, graph::Node<temp::Temp>* r) {
    LOG("Begin OKForAll\n");
    bool ret = true;
    for (auto node : nodes->GetList())
        if (!OK(node, r)) {
            ret = false;
            break;
        };
    LOG("End OKForAll\n");
    return ret;
};

bool Color::Conservative(graph::NodeList<temp::Temp>* nodes) {
    LOG("Begin Conservative\n");
    int k = 0;
    for (auto node : nodes->GetList())
        if (!Precolored(node->NodeInfo()) || degree_[node] >= reg_manager->K) k++;
    bool ret = k < reg_manager->K;
    LOG("End Conservative\n");
    return ret;
};

void Color::Combine(graph::Node<temp::Temp>* u, graph::Node<temp::Temp>* v) {
    LOG("Begin Combine\n");
    if (freeze_worklist_.find(v) != freeze_worklist_.end()) freeze_worklist_.erase(v);
    else spill_worklist_.erase(v);

    if (u == NULL) {
        fprintf(stderr, "Impossible\n");
        return;
    };
    coalesced_nodes_.insert(v);
    alias_[v] = u;
    movelist_[u] = movelist_[u]->Union(movelist_[v]);

    auto moves = new graph::NodeList<temp::Temp>(); 
    moves->Append(v);
    EnableMoves(moves);

    for (auto adj : Adjacent(v)->GetList()) {
        AddEdge(adj, u);
        DecrementDegree(adj);
    };

    if (degree_[u] >= reg_manager->K && freeze_worklist_.find(u) != freeze_worklist_.end()) {
        freeze_worklist_.erase(u);
        spill_worklist_.insert(u);
    };
    LOG("End Combine\n");
    return;
};

void Color::Coalesce() {
    LOG("Begin Coalesce\n");
    graph::Node<temp::Temp> *x, *y, *u, *v;
    assert(!worklist_moves_->GetList().empty());

    auto first = worklist_moves_->GetList().begin();

    x = first->first;
    y = first->second;
    if (x == NULL || y == NULL) {
        fprintf(stderr, "Impossible_1\n");
        return;
    };
    x = GetAlias(x);
    y = GetAlias(y);
    LOG("1\n");
    if (x == NULL || y == NULL) {
        fprintf(stderr, "Impossible_2\n");
        return;
    };
    if (Precolored(y->NodeInfo())) {
        u = y;
        v = x;
    } else {
        u = x;
        v = y;
    };
    LOG("2\n");
    worklist_moves_->Delete(first->first, first->second);
    LOG("3\n");
    if (u == v) {
        LOG("4\n");
        coalesced_moves_ = coalesced_moves_->Union(new live::MoveList(x, y));
        AddWorklist(u);
    } else if (Precolored(v->NodeInfo()) || adj_set_.find(std::make_pair(u, v)) != adj_set_.end()) {
        LOG("5\n");
        constrained_moves_ = constrained_moves_->Union(new live::MoveList(x, y));
        AddWorklist(u);
        AddWorklist(v);
    } else {
        LOG("6\n");
        live::INodeListPtr ptr = Adjacent(u); ptr->CatList(Adjacent(v));

        if ((Precolored(u->NodeInfo()) && OKForAll(Adjacent(v), u))
        || (!Precolored(u->NodeInfo()) && Conservative(ptr))) {
            coalesced_moves_ = coalesced_moves_->Union(new live::MoveList(x, y));
            Combine(u, v);
            AddWorklist(u);
        } else active_moves_ = active_moves_->Union(new live::MoveList(x, y));
    };
    LOG("End Coalesce\n");
    return;
};

void Color::FreezeMoves(graph::Node<temp::Temp>* u) {
    LOG("Begin FreezeMoves\n");
    for (auto node : NodeMoves(u)->GetList()) {
        graph::Node<temp::Temp>* v;
        auto x = node.first;
        auto y = node.second;
        if (GetAlias(y) == GetAlias(u)) v = GetAlias(x);
        else v = GetAlias(y);

        active_moves_ = active_moves_->Substract(new live::MoveList(x, y));
        frozen_moves_ = frozen_moves_->Union(new live::MoveList(x, y)); 

        if (!Precolored(v->NodeInfo()) && !NodeMoves(v) && degree_[v] < reg_manager->K) {
            freeze_worklist_.erase(v);
            simplify_worklist_.insert(v);
        };
    };
    LOG("End FreezeMoves\n");
    return;
};

void Color::Freeze() {
    LOG("Begin Freeze\n");
    assert(!freeze_worklist_.empty());
    auto node = *(freeze_worklist_.begin());
    freeze_worklist_.erase(node);
    simplify_worklist_.insert(node);
    FreezeMoves(node);
    LOG("End Freeze\n");
    return;
};

void Color::SelectSpill() {
    LOG("Begin SelectSpill\n");
    graph::Node<temp::Temp>* chosen = NULL;
    double chosen_priority = 1e20;
    for (auto node : spill_worklist_) {
        if (no_spill_temp_.find(node->NodeInfo()) != no_spill_temp_.end()) continue;

        if (liveness_.priority[node->NodeInfo()] < chosen_priority) {
            chosen = node;
            chosen_priority = liveness_.priority[node->NodeInfo()];
        };
    };
    if (!chosen) {
        assert(!spilled_nodes_.empty());
        chosen = *(spilled_nodes_.begin());
    };
    spill_worklist_.erase(chosen);
    simplify_worklist_.insert(chosen);
    FreezeMoves(chosen);
    LOG("End SelectSpill\n");
    return;
};

void Color::AssignColors() {
    LOG("Begin AssignColors\n");
    while (!select_stack_.empty()) {
        auto n = select_stack_.back();
        select_stack_.pop_back();

        std::set<std::string> ok_colors;
        for (auto reg : reg_manager->Colors())
            ok_colors.insert(reg);

        LOG("~~~~~~~~Select_stack_round~~~~~~~~~~\n");
        for (auto adj : adj_list_[n]) {
            LOG("AssignColors execute? 0\n");
            if (colored_nodes_.find(GetAlias(adj)) != colored_nodes_.end() || Precolored(GetAlias(adj)->NodeInfo())) {
                std::string* c = coloring_->Look(GetAlias(adj)->NodeInfo());
                ok_colors.erase(*c);
                LOG("AssignColors execute? 1 ---- erase: %s\n", c->c_str());
            };
        };
        if (ok_colors.empty()) {
            spilled_nodes_.insert(n);
            LOG("spilled_nodes: %d\n", n->NodeInfo()->Int());
        } else {
            colored_nodes_.insert(n);
            std::string* color = new std::string;
            *color = *(ok_colors.begin());
            coloring_->Enter(n->NodeInfo(), color);
        };
    };
    for (auto n : coalesced_nodes_) {
        coloring_->Enter(n->NodeInfo(), coloring_->Look(GetAlias(n)->NodeInfo()));
    };
    LOG("End AssignColors\n");
    return;
};

} // namespace col
