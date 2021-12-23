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

void RegAllocator::RegAlloc() {
  LOG("Start RegAlloc\n");

  bool fixed_point = false;
  while (!fixed_point) {
    /* Flow Graph */
    LOG("Start AssemFlowGraph\n");
    fg::FlowGraphFactory flow_graph_factory(assem_instr_.get()->GetInstrList());
    flow_graph_factory.AssemFlowGraph();
    auto flow_graph = flow_graph_factory.GetFlowGraph();
    LOG("End AssemFlowGraph\n");

    /* Live Graph */
    LOG("Start Liveness\n");
    live::LiveGraphFactory live_graph_factory(flow_graph);
    live_graph_factory.Liveness();
    auto live_graph = live_graph_factory.GetLiveGraph();
    LOG("End Liveness\n");

    /* Try Color */
    LOG("Start Color\n");
    col::Color color(live_graph);
    auto res = color.GetResult();
    LOG("End Color\n");

    /* If Successfully Colored */
    if (res.spills->GetList().empty()) {
      LOG("Success\n");
      fixed_point = true;
      auto result = result_.get();
      result->coloring_ = res.coloring;
      result->il_ = assem_instr_.get()->GetInstrList();
    } else {
      /* Restart */
      LOG("Restart\n");
      for (auto& it : res.spills->GetList()) {
        auto instr_list = assem_instr_.get()->GetInstrList();
        for (auto iter = instr_list->GetList().cbegin(); iter != instr_list->GetList().cend(); iter++) {
          /* Use */
          if ((*iter)->Use()->Contain(it->NodeInfo())) {
            frame_->s_offset -= frame::wordsize;
            auto new_temp = temp::TempFactory::NewTemp();

            instr_list->Insert(iter, new assem::OperInstr("leaq " + frame_->label->Name() + "_framesize(%rsp), `d0", new temp::TempList(new_temp), NULL, NULL));
            instr_list->Insert(iter, new assem::OperInstr("addq $" + std::to_string(frame_->s_offset) + ", `d0", new temp::TempList(new_temp), NULL, NULL));
            instr_list->Insert(iter, new assem::OperInstr("movq (`s0), `d0", new temp::TempList(it->NodeInfo()), new temp::TempList(new_temp), NULL));
          };
          /* Def */
          if ((*iter)->Def()->Contain(it->NodeInfo())) {
            ++iter;
            frame_->s_offset -= frame::wordsize;
            auto new_temp = temp::TempFactory::NewTemp();
            
            instr_list->Insert(iter, new assem::OperInstr("leaq " + frame_->label->Name() + "_framesize(%rsp), `d0", new temp::TempList(new_temp), NULL, NULL));
            instr_list->Insert(iter, new assem::OperInstr("addq $" + std::to_string(frame_->s_offset) + ", `d0", new temp::TempList(new_temp), NULL, NULL));
            instr_list->Insert(iter, new assem::OperInstr("movq `s0, (`d0)", new temp::TempList(new_temp), new temp::TempList(it->NodeInfo()), NULL));
            --iter;
          };
        };
      };
    };
  };
  LOG("End RegAlloc\n");
};

} // namespace ra