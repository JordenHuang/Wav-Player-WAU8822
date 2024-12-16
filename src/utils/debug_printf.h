#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

#include <stdio.h>

// 1 to enable debug print, 0 to disable
#define DEBUGGING 0

#if (DEBUGGING == 1)
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...)
#endif

#endif // _DEBUG_PRINT_H_
