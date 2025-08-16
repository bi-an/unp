#include <stdio.h>

#include "CuTest.h"
#include "wrap.h"

CuSuite *wrapGetSuite();

void RunAllTests() {
  CuString *output = CuStringNew();
  CuSuite *suite = CuSuiteNew();

  CuSuiteAddSuite(suite, wrapGetSuite());

  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
}

int main() { RunAllTests(); }