#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "CuTest.h"
#include "wrap.h"

#define MAX_SIZE 1024

int Open(const char *pathname, int flags, mode_t mode) {
  int fd;
  if ((fd = open(pathname, flags, mode)) < 0) perr_exit("open error");
  return fd;
}

// test functions
void TestReadWrite(CuTest *tc) {
  int fd = Open("testfile.tmp", O_RDWR | O_CREAT | O_TRUNC, 0644);
  char buf[MAX_SIZE];
  int n;

  const char *s = "123456789";
  Write(fd, s, strlen(s));

  lseek(fd, 0, SEEK_SET);
  n = Read(fd, buf, strlen(s));
  buf[n] = 0;
  Close(fd);
  CuAssertStrEquals(tc, s, buf);
}

void TestReadnWriten(CuTest *tc) {
  int fd = Open("testfile.tmp", O_RDWR | O_CREAT | O_TRUNC, 0644);
  char out_buf[MAX_SIZE], in_buf[MAX_SIZE];
  int n, kTestTimes = 1024*1024; // 大约1GB

  srand((unsigned int)time(NULL));

  for (int i = 0; i < kTestTimes; i++) {
    for (n = 0; n < MAX_SIZE - 1; n++) {
      out_buf[n] = rand() % 26 + 'a';
    }
    out_buf[n] = 0;

    Writen(fd, out_buf, MAX_SIZE - 1);

    lseek(fd, i * (MAX_SIZE - 1), SEEK_SET);
    n = Readn(fd, in_buf, MAX_SIZE - 1);
    in_buf[n] = 0;

    CuAssertStrEquals(tc, out_buf, in_buf);
  }

  Close(fd);
}

// get test suite
CuSuite *wrapGetSuite() {
  CuSuite *suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestReadWrite);
  SUITE_ADD_TEST(suite, TestReadnWriten);
}
