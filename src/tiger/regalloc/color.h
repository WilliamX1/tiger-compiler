#ifndef TIGER_COMPILER_COLOR_H
#define TIGER_COMPILER_COLOR_H

#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/util/graph.h"

#include <set>
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <sstream>
#include <iostream>

namespace col {
struct Result {
  Result() : coloring(nullptr), spills(nullptr) {}
  Result(temp::Map *coloring, live::INodeListPtr spills)
      : coloring(coloring), spills(spills) {}
  temp::Map *coloring;
  live::INodeListPtr spills;
};

template<typename T>
using Table = graph::Table<temp::Temp, T>;

class Color {
  /* TODO: Put your lab6 code here */
public:
  explicit Color(live::LiveGraph live_graph) :live_graph_(live_graph) {};
  Result GetResult();
private:
  live::LiveGraph live_graph_;
};
} // namespace col

#endif // TIGER_COMPILER_COLOR_H
