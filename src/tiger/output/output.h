#ifndef TIGER_COMPILER_OUTPUT_H
#define TIGER_COMPILER_OUTPUT_H

#include <list>
#include <memory>
#include <string>

#include "tiger/canon/canon.h"
#include "tiger/codegen/codegen.h"
#include "tiger/frame/frame.h"

namespace output {

class AssemGen {
public:
  AssemGen() = delete;
  explicit AssemGen(std::string_view infile) {
    std::string outfile = static_cast<std::string>(infile) + ".s";
    out_ = fopen(outfile.data(), "w");
  }
  AssemGen(const AssemGen &assem_generator) = delete;
  AssemGen(AssemGen &&assem_generator) = delete;
  AssemGen &operator=(const AssemGen &assem_generator) = delete;
  AssemGen &operator=(AssemGen &&assem_generator) = delete;
  ~AssemGen() { fclose(out_); }

  /**
   * Generate assembly
   */
  void GenAssem(bool need_ra);

private:
  FILE *out_; // Instream of source file
};

} // namespace output

#endif // TIGER_COMPILER_OUTPUT_H
