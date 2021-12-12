#include "tiger/regalloc/regalloc.h"

#include "tiger/output/logger.h"

extern frame::RegManager *reg_manager;

namespace ra {
/* TODO: Put your lab6 code here */
void RegAllocator::RegAlloc() {
    do {
        /* do actual color assignment */
        // fg::FlowGraphFactory::AssemFlowGraph();
        // fg::FGraph* flow_graph = fg::FlowGraphFactory::GetFlowGraph();
        /* showInterference(stdout, live_result) */
        // col::Result col_result = col::Color(flow_graph);
        // if (col_result.spills != NULL) {
        //     this->assem_instr_ = std::make_unique<cg::AssemInstr>(rewriteProgram(col_result.spills));
        //     std::cout << "Rewritten program: " << std::endl;
        //     this->assem_instr_->Print(stdout, reg_manager->temp_map_);
        // } else {
        //     /* layer the colormap */
            // temp::Map* result_map = temp::Map::LayerMap(col_result.coloring, reg_manager->temp_map_);
        //     /* remove all unnecessary move instrs */
            // this->assem_instr_ = std::make_unique<cg::AssemInstr>(sweepMove());
            // this->result_ = std::make_unique<ra::Result>(result_map, this->assem_instr_.get());
            break;
        // };
    } while (1);
    return;
};

assem::InstrList* RegAllocator::sweepMove() {
    // assem::InstrList* new_instr_list = new assem::InstrList();
    // for (auto instr : this->assem_instr_->GetInstrList()->GetList()) {
    //     if (instr->kind_ == assem::Instr::MOVE) {
    //         assem::MoveInstr* ins = (assem::MoveInstr*) instr;
    //         const std::string *src = reg_manager->temp_map_->Look(ins->src_->GetList().front());
    //         const std::string *dst = reg_manager->temp_map_->Look(ins->dst_->GetList().front());
    //         if (src == dst)
    //             continue;
    //     };
    //     new_instr_list->Append(instr);
    // };
    // return new_instr_list;
    return new assem::InstrList();
};

assem::InstrList* RegAllocator::rewriteProgram(live::INodeListPtr spills) {
    return new assem::InstrList();
}
} // namespace ra