#include "tiger/codegen/assem.h"

#include <cassert>

namespace temp {

Temp *TempList::NthTemp(int i) const {
  for (auto it : temp_list_)
    if (i-- == 0)
      return it;
  assert(0);
}
} // namespace temp

namespace assem {
/**
 * First param is string created by this function by reading 'assem' string
 * and replacing `d `s and `j stuff.
 * Last param is function to use to determine what to do with each temp.
 * @param assem assembly string
 * @param dst dst_ temp
 * @param src src temp
 * @param jumps jump labels_
 * @param m temp map
 * @return formatted assembly string
 */
static std::string Format(std::string_view assem, temp::TempList *dst,
                          temp::TempList *src, Targets *jumps, temp::Map *m) {
  std::string result;
  for (std::string::size_type i = 0; i < assem.size(); i++) {
    char ch = assem.at(i);
    if (ch == '`') {
      i++;
      switch (assem.at(i)) {
      case 's': {
        i++;
        int n = assem.at(i) - '0';
        std::string *s = m->Look(src->NthTemp(n));
        result += *s;
      } break;
      case 'd': {
        i++;
        int n = assem.at(i) - '0';
        std::string *s = m->Look(dst->NthTemp(n));
        result += *s;
      } break;
      case 'j': {
        i++;
        assert(jumps);
        std::string::size_type n = assem.at(i) - '0';
        std::string s = temp::LabelFactory::LabelString(jumps->labels_->at(n));
        result += s;
      } break;
      case '`': {
        result += '`';
      } break;
      default:
        assert(0);
      }
    } else {
      result += ch;
    }
  }
  return result;
}

void OperInstr::Print(FILE *out, temp::Map *m) const {
  std::string result = Format(assem_, dst_, src_, jumps_, m);
  fprintf(out, "%s\n", result.data());
}

void LabelInstr::Print(FILE *out, temp::Map *m) const {
  std::string result = Format(assem_, nullptr, nullptr, nullptr, m);
  fprintf(out, "%s:\n", result.data());
}

void MoveInstr::Print(FILE *out, temp::Map *m) const {
  if (!dst_ && !src_) {
    std::size_t srcpos = assem_.find_first_of('%');
    if (srcpos != std::string::npos) {
      std::size_t dstpos = assem_.find_first_of('%', srcpos + 1);
      if (dstpos != std::string::npos) {
        if ((assem_[srcpos + 1] == assem_[dstpos + 1]) &&
            (assem_[srcpos + 2] == assem_[dstpos + 2]) &&
            (assem_[srcpos + 3] == assem_[dstpos + 3]))
          return;
      }
    }
  }
  std::string result = Format(assem_, dst_, src_, nullptr, m);
  fprintf(out, "%s\n", result.data());
}

void InstrList::Print(FILE *out, temp::Map *m) const {
  for (auto instr : instr_list_)
    instr->Print(out, m);
  fprintf(out, "\n");
}

} // namespace assem
