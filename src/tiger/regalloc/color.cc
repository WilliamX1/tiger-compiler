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
Result Color::GetResult() {
    Result result;

    LOG("Free Colors\n");
    /* Free colors */
    auto colors = reg_manager->Colors();
    int K = colors.size();

    LOG("Simplify and Spill\n");
    /* Simplify and Spill */
    /* Calculate degree */
    std::map<live::INodePtr, int> temp_degree;
    std::map<live::INodePtr, bool> temp_is_in_stack;
    auto node_list = live_graph_.interf_graph->Nodes()->GetList();
    for (auto &it : node_list) {
        /* Original graph has direction, so reduce it to non-direction graph */
        temp_degree[it] = it->Degree() / 2;
        temp_is_in_stack[it] = false;
    };

    LOG("Stack\n");
    /* Stack */
    std::stack<live::INodePtr> stack;
    while (stack.size() != node_list.size()) {
        /* Find min degree temp until degree < N */
        live::INodePtr chosen = NULL;
        for (auto &it : node_list) {
            if (temp_is_in_stack[it]) continue;
            if (chosen == NULL || temp_degree[chosen] > temp_degree[it]) chosen = it;
            if (temp_degree[chosen] < K) break; 
        };

        /* Push it into stack */
        for (auto &it : chosen->Pred()->GetList())
            temp_degree[it]--;

        temp_degree[chosen] = 0;
        stack.push(chosen);
        temp_is_in_stack[chosen] = true;
    };

    LOG("Select\n");
    /* Select */
    result.spills = new live::INodeList();
    while (true) {
        auto stack_copy = stack;

        /* Try select */
        bool can_select = true;
        /* Color map */
        auto map = temp::Map::Empty();
        while (!stack_copy.empty()) {
            auto top = stack_copy.top();
            stack_copy.pop();

            /* If this node is precolored */
            auto test_is_precolored = reg_manager->temp_map_->Look(top->NodeInfo());
            if (test_is_precolored)
                map->Enter(top->NodeInfo(), new std::string(*test_is_precolored));
            else {
                /* Find used */
                std::set<std::string> used;
                for (auto& it: top->Succ()->GetList()) {
                    auto pre_color = reg_manager->temp_map_->Look(it->NodeInfo());
                    if (pre_color) used.insert(*pre_color);
                    else {
                        auto has_color = map->Look(it->NodeInfo());
                        if (has_color) used.insert(*has_color);
                    };
                };

                /* If no color to use */
                if (used.size() == colors.size()) {
                    can_select = false;
                    break;
                };

                /* Find usable color */
                std::string usable;
                for (auto &it : colors)
                    if (!used.count(it)) {
                        usable = it;
                        break;
                    };

                /* Color */
                map->Enter(top->NodeInfo(), new std::string(usable));
            };        
        };

        if (can_select) {
            result.coloring = map;
            break;
        } else {
            /* Spill and rerun */
            result.spills->Append(stack.top());
            stack.pop();
            delete map;
        };
    };

    return result;
};

} // namespace col
