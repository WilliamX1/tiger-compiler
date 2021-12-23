#include "tiger/liveness/flowgraph.h"
#include <iostream>

#define DEBUG

#ifdef DEBUG
  #define LOG(format, args...) do {   \
    FILE* debug_log = fopen("flowgraph.log", "a+");  \
    fprintf(debug_log, "%d, %s: ", __LINE__, __func__); \
    fprintf(debug_log, format, ##args); \
    fclose(debug_log);\
  } while (0)
#else
  #define LOG(format, args...) do{} while (0)
#endif

namespace fg {
  
void FlowGraphFactory::AssemFlowGraph() {
  LOG("Begin AssemFlowGraph\n");
  /* TODO: Put your lab6 code here */
  auto label2node = label_map_.get();
  tab::Table<assem::Instr, FNode> instr2node;

  /* Record the previous pointer */
  FNodePtr prev = NULL;
  /* Add new nodes and add next-step edge */
  for (auto &it : instr_list_->GetList()) {
    /* Create a new node for this instr */
    auto cur = flowgraph_->NewNode(it);
    instr2node.Enter(it, cur);

    /* If prev, add edge for it */
    if (prev) flowgraph_->AddEdge(prev, cur);

    /* If is label, mark it */
    if (it->kind_ == assem::Instr::LABEL) label2node->Enter(((assem::LabelInstr *)it)->label_, cur);

    /* Update prev */
    if (it->kind_ == assem::Instr::OPER && ((assem::OperInstr *)it)->assem_.find("jmp") == 0) prev = NULL;
    else prev = cur;
  };

  /* Handle the jump list */
  for (auto &it : instr_list_->GetList()) {
    if (it->kind_ == assem::Instr::OPER && ((assem::OperInstr*) it)->jumps_) {
      std::vector<temp::Label *>* labels = ((assem::OperInstr*) it)->jumps_->labels_;

      for (auto &iter : *((assem::OperInstr *)it)->jumps_->labels_) {
        auto jmp_node = instr2node.Look(it);
        auto label_node = label2node->Look(iter);
        flowgraph_->AddEdge(jmp_node, label_node);
      };
    };
  };

  LOG("End AssemFlowGraph\n");
  return;
}
} // namespace fg

namespace assem {

temp::TempList *LabelInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return new temp::TempList();
}

temp::TempList *MoveInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return this->dst_ ? this->dst_ : new temp::TempList();
}

temp::TempList *OperInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return this->dst_ ? this->dst_ : new temp::TempList();
}

temp::TempList *LabelInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return new temp::TempList();
}

temp::TempList *MoveInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return this->src_ ? this->src_ : new temp::TempList();
}

temp::TempList *OperInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return this->src_ ? this->src_ : new temp::TempList();
}
} // namespace assem
