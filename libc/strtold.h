#ifndef _STRTOLD_H
#define _STRTOLD_H

#ifdef __cplusplus
extern "C" {
#endif

long double __strtold(int max_width, int (*ReadProc)(void*, int, int), void* ReadProcArg, int* chars_scanned,
                      int* overflow);

#ifdef __cplusplus
}
#endif

#endif
