#include "tiger/output/output.h"

#include <cstdio>

#include "tiger/output/logger.h"

extern frame::RegManager *reg_manager;
extern frame::Frags *frags;

namespace output {
void AssemGen::GenAssem(bool need_ra) {
  frame::Frag::OutputPhase phase;

  // Output proc
  phase = frame::Frag::Proc;
  fprintf(out_, ".text\n");
  fprintf(stderr, "start output proc\n");
  for (auto &&frag : frags->GetList())
    frag->OutputAssem(out_, phase, need_ra);

  fprintf(stderr, "end output proc\n");
  fprintf(stderr, "start output string\n");
  // Output string
  phase = frame::Frag::String;
  fprintf(out_, ".section .rodata\n");
  for (auto &&frag : frags->GetList())
    frag->OutputAssem(out_, phase, need_ra);
  fprintf(stderr, "edn output proc\n");
}

} // namespace output

namespace frame {

void ProcFrag::OutputAssem(FILE *out, OutputPhase phase, bool need_ra) const {
  std::unique_ptr<canon::Traces> traces;
  std::unique_ptr<cg::AssemInstr> assem_instr;
  std::unique_ptr<ra::Result> allocation;

  // When generating proc fragment, do not output string assembly
  if (phase != Proc)
    return;

  TigerLog("-------====IR tree=====-----\n");
  TigerLog(body_);

  {
    // Canonicalize
    TigerLog("-------====Canonicalize=====-----\n");
    canon::Canon canon(body_);

    // Linearize to generate canonical trees
    TigerLog("-------====Linearlize=====-----\n");
    tree::StmList *stm_linearized = canon.Linearize();
    TigerLog(stm_linearized);

    // Group list into basic blocks
    TigerLog("------====Basic block_=====-------\n");
    canon::StmListList *stm_lists = canon.BasicBlocks();
    TigerLog(stm_lists);

    // Order basic blocks into traces_
    TigerLog("-------====Trace=====-----\n");
    tree::StmList *stm_traces = canon.TraceSchedule();
    TigerLog(stm_traces);

    traces = canon.TransferTraces();
  }
  temp::Map *color = temp::Map::LayerMap(reg_manager->temp_map_, temp::Map::Name());
  {
    // Lab 5: code generation
    TigerLog("-------====Code generate=====-----\n");
    cg::CodeGen code_gen(frame_, std::move(traces));
    fprintf(stderr, "start codegen\n");
    code_gen.Codegen();
    fprintf(stderr, "end codegen\n");
    assem_instr = code_gen.TransferAssemInstr();
    TigerLog(assem_instr.get(), color);
  }
  fprintf(stderr, "start il\n");
  assem::InstrList *il = assem_instr.get()->GetInstrList();
  fprintf(stderr, "end il\n");
  
  if (need_ra) {
    // Lab 6: register allocation
    TigerLog("----====Register allocate====-----\n");
    fprintf(stderr, "start init reg_allocator\n");
    ra::RegAllocator reg_allocator(frame_, std::move(assem_instr));
    fprintf(stderr, "end reg_allocator\n");
    fprintf(stderr, "start regalloc\n");
    reg_allocator.RegAlloc();
    fprintf(stderr, "end regalloc\n");
    allocation = reg_allocator.TransferResult();
    il = allocation->il_;
    color = temp::Map::LayerMap(reg_manager->temp_map_, allocation->coloring_);
  }

  TigerLog("-------====Output assembly for %s=====-----\n",
           frame_->label->Name().data());

  fprintf(stderr, "start procEntryExit3\n");
  assem::Proc *proc = frame_->ProcEntryExit3(il);
  fprintf(stderr, "end procEntryExit3\n");
  // std::string proc_name = frame_->GetLabel();
  assert(frame_);
  assert(frame_->label);

  std::string proc_name = frame_->label->Name();

  fprintf(stderr, "proc_name: %s\n", proc_name.c_str());

  fprintf(out, ".globl %s\n", proc_name.data());
  fprintf(out, ".type %s, @function\n", proc_name.data());
  // prologue  
  fprintf(stderr, "start prologue\n");
  assert(proc);
  assert(proc->prolog_);
  fprintf(out, "%s", proc->prolog_.data());
  // body
  fprintf(stderr, "start body\n");
  assert(proc->body_);
  proc->body_->Print(out, color);
  // epilog_
  fprintf(stderr, "start epilog\n");
  assert(proc->epilog_);
  fprintf(out, "%s", proc->epilog_.data());
  fprintf(out, ".size %s, .-%s\n", proc_name.data(), proc_name.data());
  fprintf(stderr, "end end\n");
}

void StringFrag::OutputAssem(FILE *out, OutputPhase phase, bool need_ra) const {
  // When generating string fragment, do not output proc assembly
  if (phase != String)
    return;

  fprintf(out, "%s:\n", label_->Name().data());
  int length = static_cast<int>(str_.size());
  // It may contain zeros in the middle of string. To keep this work, we need
  // to print all the charactors instead of using fprintf(str)
  fprintf(out, ".long %d\n", length);
  fprintf(out, ".string \"");
  for (int i = 0; i < length; i++) {
    if (str_[i] == '\n') {
      fprintf(out, "\\n");
    } else if (str_[i] == '\t') {
      fprintf(out, "\\t");
    } else if (str_[i] == '\"') {
      fprintf(out, "\\\"");
    } else {
      fprintf(out, "%c", str_[i]);
    }
  }
  fprintf(out, "\"\n");
}
} // namespace frame
