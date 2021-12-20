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

bool IsMove(graph::Node<assem::Instr>* n) {
  return n->NodeInfo()->kind_ == assem::Instr::Kind::MOVE;
};

void FlowGraphFactory::AssemFlowGraph() {
  LOG("Begin AssemFlowGraph\n");
  /* TODO: Put your lab6 code here */
  assert(this->flowgraph_ != NULL);
  assert(this->label_map_ != NULL);

  graph::Node<assem::Instr>* prev = NULL, *cur = NULL;
  for (auto p : instr_list_->GetList()) {
    ShowAssem(p);

    cur = flowgraph_->NewNode(p);
    // if (instr_map_->Look(p) == NULL) 
      instr_map_->Enter(p, cur);
    // else 
      // instr_map_->Set(p, cur);

    if (prev) {
      LOG("AddEdge from %s --> %s\n", GetAssem(prev->NodeInfo()).c_str(), GetAssem(cur->NodeInfo()).c_str());
      flowgraph_->AddEdge(prev, cur);
    };

    if (p->kind_ == assem::Instr::Kind::LABEL)
    {
      // if (label_map_->Look(((assem::LabelInstr*) p)->label_) == NULL) 
        label_map_->Enter(((assem::LabelInstr*) p)->label_, cur);
      // else 
        // label_map_->Set(((assem::LabelInstr*) p)->label_, cur);
    } else if (p->kind_ == assem::Instr::Kind::OPER && ((assem::OperInstr*) p)->assem_.find("jmp") == 0) prev = NULL;
    else prev = cur;
  };

  for (auto p : instr_list_->GetList()) {
    if (p->kind_ == assem::Instr::Kind::OPER && ((assem::OperInstr*) p)->jumps_) {
      std::vector<temp::Label *>* labels = ((assem::OperInstr*) p)->jumps_->labels_;
      if (labels == NULL || labels->empty()) {
        LOG("Strange No Label\n");
        continue;
      };
      for (auto label : *labels) { LOG("8\n");
        if (label_map_->Look(label) == NULL || instr_map_->Look(p) == NULL) {
          fprintf(stderr, "Impossible omg\n");
          continue;
        };
        // assert(label_map_->Look(label));
        // assert(instr_map_->Look(p));
        graph::Node<assem::Instr>* jmp_node = instr_map_->Look(p);
        graph::Node<assem::Instr>* label_node = label_map_->Look(label);
        LOG("AddEdge from %s --> %s\n", GetAssem(jmp_node->NodeInfo()).c_str(), GetAssem(label_node->NodeInfo()).c_str());
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
