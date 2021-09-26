#include "tiger/errormsg/errormsg.h"

#include <cstdarg>
#include <cstdio>

namespace err {

void ErrorMsg::Newline() {
  line_num_++;
  line_pos_.push_front(tok_pos_);
}

void ErrorMsg::Error(int pos, std::string_view message, ...) {
  va_list ap;
  int num = line_num_;
  int val = -1;

  // Backtrace to the error line number
  any_errors_ = true;
  for (auto i : line_pos_) {
    val = i;
    if (i < pos)
      break;
    num--;
  }

  // Output error message
  if (!file_name_.empty())
    fprintf(stderr, "%s:", file_name_.data());
  if (val != -1)
    fprintf(stderr, "%d.%d: ", num, pos - val);
  va_start(ap, message);
  vfprintf(stderr, message.data(), ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

} // namespace err
