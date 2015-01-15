

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#define DBG_MSQL
#ifdef DBG_MSQL
#define debug printf
#else
#define debug(s...) NULL
#endif

#define fs() debug("\n[%s], start\n", __FUNCTION__)
#define fe() debug("[%s], done. \n\n", __FUNCTION__)
#define fe_return(s) \
{ \
  if (s) fe(); \
  return s; \
} \

#define Y 1

#endif /* __COMMON_H__ */


