
#include "parser.h"
#include "se.h"

int
main(int argc, char* argv[])
{
  int rt;

  rt = init_se();
  if (!rt) return -1;

  rt = msql();
  if (!rt) return -1;

  return 0;
}

