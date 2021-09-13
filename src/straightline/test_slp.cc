#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "straightline/prog1.h"

int main(int argc, char **argv) {
  int args;
  int test_num;

  assert(argc == 2);
  test_num = atoi(argv[1]);

  switch (test_num) {
    case 0:
      printf("Prog\n");
      args = Prog()->MaxArgs();
      printf("args: %d\n", args);
      Prog()->Interp(nullptr);

      printf("ProgProg\n");
      args = ProgProg()->MaxArgs();
      printf("args: %d\n", args);
      ProgProg()->Interp(nullptr);
      break;
    case 1:
      printf("ProgProg\n");
      args = ProgProg()->MaxArgs();
      printf("args: %d\n", args);
      ProgProg()->Interp(nullptr);

      printf("Prog\n");
      args = Prog()->MaxArgs();
      printf("args: %d\n", args);
      Prog()->Interp(nullptr);
      break;
    default:
      printf("unexpected case\n");
      exit(-1);
  }
  printf("RightProg\n");
  args = RightProg()->MaxArgs();
  printf("args: %d\n", args);
  RightProg()->Interp(nullptr);

  return 0;
}
