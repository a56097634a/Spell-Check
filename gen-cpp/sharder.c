/* gcc -Wall sharder3.c -o sharder3 -lcrypto */
/* Also add -Wno-deprecated if compiling on OS X. */
/* Read a Unix words file on stdin and shard it based on MD5 hash. */
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <openssl/md5.h>

#define BUFBYTES 256
#define NUMSHARDS 2

int main() {
  char buf[BUFBYTES], fname[256], *p;
  FILE *wordfiles[NUMSHARDS];
  FILE *word = fopen("linuxwords", "r");
  int i, r;
  for (i=0; i<NUMSHARDS; i++) {
    sprintf(fname, "shard_%d_words", i);
    wordfiles[i] = fopen(fname, "w"); assert(wordfiles[i] != NULL);
  }
  while (fgets(buf, BUFBYTES, word)) {
    unsigned char shard, hash[16];
    assert(strlen(buf) > 0);
    p = buf + strlen(buf) - 1;
    if (*p == '\n') *p = 0;
    MD5((unsigned char*) buf, strlen(buf), hash);
    shard = hash[15] % NUMSHARDS;
    r = fprintf(wordfiles[shard], "%s\n", buf); assert(r == strlen(buf) + 1);
  }
  for (i=0; i<NUMSHARDS; i++) fclose(wordfiles[i]);
  return(0);
}
