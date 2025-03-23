#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "debug.h"

void print_stacktrace() {
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 10);
  strings = backtrace_symbols(array, size);

  fprintf(stderr, "Stack trace:\n");
  for (i = 0; i < size; i++) {
    fprintf(stderr, "%s\n", strings[i]);
  }

  free(strings);
} 