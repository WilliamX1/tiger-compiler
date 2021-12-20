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
    LOG("Begin RegAlloc\n");
    bool done = false;
    while (!done) {
        done = true;
        LOG("Begin AssemFlowGraph\n");
        this->flow_graph_factory_->AssemFlowGraph();
        LOG("End AssemFlowGraph\n");
        LOG("Begin GetFlowGraph\n");
        this->color_->flow_graph_ = this->flow_graph_factory_->GetFlowGraph();
        LOG("End GetFlowGraph\n");

        LOG("Begin Liveness\n");
        this->live_graph_factory_->Liveness();
        LOG("End Liveness\n");
        LOG("Begin GetLiveGraph\n");
        this->color_->liveness_ = this->live_graph_factory_->GetLiveGraph();
        LOG("End GetLiveGraph\n");
        
        LOG("Begin Build\n");
        color_->Build();
        LOG("End Build\n");
        LOG("Begin MakeWorkList\n");
        color_->MakeWorkList();
        LOG("End MakeWorkList\n");
        do {
            if (!color_->simplify_worklist_.empty()) color_->Simplify();
            else if (!color_->worklist_moves_->GetList().empty()) color_->Coalesce();
            else if (!color_->freeze_worklist_.empty()) color_->Freeze();
            else if (!color_->spill_worklist_.empty()) color_->SelectSpill();
        } while (!color_->simplify_worklist_.empty() || !color_->worklist_moves_->GetList().empty() || !color_->freeze_worklist_.empty() || !color_->spill_worklist_.empty());

        LOG("Begin AssignColors\n");
        color_->AssignColors();
        LOG("End AssignColors\n");
        if (!color_->spilled_nodes_.empty()) {
            LOG("Begin RewriteProgram\n");
            RewriteProgram();
            LOG("End RewriteProgram\n");
            done = false;
        };
    };
    // LOG("Begin RemoveUnecessary\n");
    // RemoveUnnecessary();
    // LOG("End RemoveUnnecessary\n");

    this->result_ = std::make_unique<ra::Result>(color_->coloring_, this->assem_instr_->GetInstrList());

    LOG("End RegAlloc\n");
    return;
};

void RegAllocator::RewriteProgram() {
    LOG("Begin RewriteProgram\n");
    assem::InstrList* il = new assem::InstrList();;
    for (auto node : color_->spilled_nodes_) {
        temp::Temp* spilled_temp = node->NodeInfo();
        frame_->s_offset -= frame::wordsize;
        assem::InstrList* new_il = new assem::InstrList();
        for (auto p : il->GetList()) {
            temp::TempList* src, *dst;
            switch (p->kind_)
            {
            case assem::Instr::LABEL: {
                src = NULL;
                dst = NULL;
                break;
            }
            case assem::Instr::MOVE: {
                auto move_instr = (assem::MoveInstr*) p;
                src = move_instr->src_;
                dst = move_instr->dst_;
                break;
            }
            case assem::Instr::OPER: {
                auto move_instr = (assem::OperInstr*) p;
                src = move_instr->src_;
                dst = move_instr->dst_;
                break;
            }
            default:
                break;
            }
            if (src && src->Contain(spilled_temp) && dst->Contain(spilled_temp)) {
                temp::Temp* new_temp = temp::TempFactory::NewTemp();
                color_->no_spill_temp_.insert(new_temp);
                src->Replace(spilled_temp, new_temp);
                dst->Replace(spilled_temp, new_temp);
                
                std::stringstream stream;
                stream << "movq (" << frame_->label->Name() << "_framesize" << frame_->s_offset << ")(`s0), `d0";
                std::string assem = stream.str();

                new_il->Append(new assem::OperInstr(assem, new temp::TempList(new_temp), new temp::TempList(reg_manager->RSP()), NULL));
                new_il->Append(p);

                stream.clear();
                stream << "movq `s0, (" << frame_->label->Name() << "_framesize" << frame_->s_offset << ")(`s1)";
                assem = stream.str();
                new_il->Append(new assem::OperInstr(assem, NULL, new temp::TempList(reg_manager->RSP()), NULL));
            } else if (src && src->Contain(spilled_temp)) {
                temp::Temp* new_temp = temp::TempFactory::NewTemp();
                color_->no_spill_temp_.insert(new_temp);
                src->Replace(spilled_temp, new_temp);
                
                std::stringstream stream;
                stream << "movq (" << frame_->label->Name() << "_framesize" << frame_->s_offset << ")(`s0), `d0";
                std::string assem = stream.str();

                new_il->Append(new assem::OperInstr(assem, new temp::TempList(new_temp), new temp::TempList(reg_manager->RSP()), NULL));
                new_il->Append(p);
            } else if (dst && dst->Contain(spilled_temp)) {
                temp::Temp* new_temp = temp::TempFactory::NewTemp();
                color_->no_spill_temp_.insert(new_temp);
                dst->Replace(spilled_temp, new_temp);

                std::stringstream stream;
                stream << "movq `s0, (" << frame_->label->Name() << "_framesize" << frame_->s_offset << ")(`s1)";
                std::string assem = stream.str();

                new_il->Append(p);
                new_il->Append(new assem::OperInstr(assem, NULL, new temp::TempList({new_temp, reg_manager->RSP()}), NULL));
            } else {
                new_il->Append(p);
            };
        };

        il = new_il;
    };

    this->assem_instr_.reset(new cg::AssemInstr(il));

    color_->spilled_nodes_.clear();
    color_->colored_nodes_.clear();
    color_->coalesced_nodes_.clear();

    LOG("End RewriteProgram\n");
    return;
}

void RegAllocator::RemoveUnnecessary() {
    LOG("Begin RemoveUnnecessary\n");
    assem::InstrList* new_instr_list = new assem::InstrList();
    for (auto instr : this->assem_instr_->GetInstrList()->GetList()) {
        if (instr->kind_ == assem::Instr::MOVE) {
            assem::MoveInstr* ins = (assem::MoveInstr*) instr;
            const std::string *src = reg_manager->temp_map_->Look(ins->src_->GetList().front());
            const std::string *dst = reg_manager->temp_map_->Look(ins->dst_->GetList().front());
            if (src == dst)
                continue;
        };
        new_instr_list->Append(instr);
    };

    this->assem_instr_.reset(new cg::AssemInstr(new_instr_list));
    LOG("End RemoveUnnecessary\n");
    return;
};


} // namespace ra