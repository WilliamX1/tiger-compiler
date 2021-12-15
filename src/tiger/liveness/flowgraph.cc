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

bool IsMove(graph::Node<assem::Instr>* n) {
  return n->NodeInfo()->kind_ == assem::Instr::Kind::MOVE;
};

bool IsJmp(graph::Node<assem::Instr>* n) {
  assem::Instr* instr = n->NodeInfo();
  return instr->kind_ == assem::Instr::Kind::OPER && ((assem::OperInstr*) instr)->assem_.find("jmpq") != std::string::npos;
};

void FlowGraphFactory::AssemFlowGraph() {
  /* TODO: Put your lab6 code here */
  LOG("AssemFlowGraph\n");
  this->flowgraph_;
  this->label_map_;

  graph::Node<assem::Instr>* prev = NULL;
  for (auto instr : this->instr_list_->GetList()) {
    graph::Node<assem::Instr>* node = this->flowgraph_->NewNode(instr);

    if (prev && IsJmp(prev)) this->flowgraph_->AddEdge(prev, node);
    if (instr->kind_ == assem::Instr::Kind::LABEL) this->label_map_->Enter(((assem::LabelInstr*) instr)->label_, node);    
  };

  for (auto node : this->flowgraph_->Nodes()->GetList()) {
    if (node->NodeInfo()->kind_ != assem::Instr::Kind::OPER || !((assem::OperInstr*) node->NodeInfo())->jumps_) continue;
    for (auto label : *(((assem::OperInstr*) node->NodeInfo())->jumps_->labels_))
      this->flowgraph_->AddEdge(node, this->label_map_->Look(label));
  };
  return;
}
} // namespace fg

namespace assem {

temp::TempList *LabelInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return NULL;
}

temp::TempList *MoveInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return this->dst_;
}

temp::TempList *OperInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return this->dst_;
}

temp::TempList *LabelInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return NULL;
}

temp::TempList *MoveInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return this->src_;
}

temp::TempList *OperInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return this->src_;
}
} // namespace assem
