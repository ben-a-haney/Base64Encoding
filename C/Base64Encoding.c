#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

char* encode(char* in);
char* decode(char* d);
char getIndexOf(char c);

char* indexTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char revTable[] = {62,0,0,0,63,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};

char* largeData = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum mi purus, \
mollis et pulvinar quis, elementum non felis. Ut bibendum dolor ut mauris tempus, euismod \
ultricies odio vulputate. Nunc finibus elit non venenatis maximus. Maecenas in mollis ipsum, \
mattis laoreet purus. Sed lacus purus, tempus vel elementum sed, rutrum nec massa. Mauris \
mattis libero vitae nunc tempor, eget posuere ipsum molestie. Curabitur semper tempus diam. \
Morbi rutrum sollicitudin augue, rhoncus viverra velit volutpat vitae.\n";

int main(void) {
  struct timeval timeStart, timeStop;
  char* largeTest_enc;
  char* largeTest_dec;

  printf("%s\n", encode("123"));
  printf("%s\n", encode("1234"));
  printf("%s\n", encode("12345"));
  printf("%s\n", encode("123456"));
  printf("%s\n", encode("This is a string that will be encoded and then decoded.\n\
If you can read this, my hand crafted algorithm is working swimingly...\n\
Now for some non-Base64 characters: ~~~```<<<()()()$$$$$^^^^^@@@@@()()()>>>```~~~"));
  printf("%s\n", decode(encode("This is a string that will be encoded and then decoded.\n\
If you can read this, my hand crafted algorithm is working swimingly...\n\
Now for some non-Base64 characters: ~~~```<<<()()()$$$$$^^^^^@@@@@()()()>>>```~~~")));

  gettimeofday(&timeStart, NULL);
  for (long i = 0; i < 1000000; i++){
    free(decode(encode(largeData)));
  }

  gettimeofday(&timeStop, NULL);
  largeTest_enc = encode(largeData);
  largeTest_dec = decode(largeTest_enc);
  printf("%s\n", largeTest_dec);
  printf("Total time in seconds: %lf\n", (timeStop.tv_sec - timeStart.tv_sec) + (timeStop.tv_usec - timeStart.tv_usec)/1000000.0);
}

char * encode(char* in) {
  int inl = strlen(in);
  int rem = inl % 3;
  char *last = in+inl-rem;
  int outl = ((inl+2)/3)*4;
  char *out = malloc(outl+1);
  char *i, *j;
  #pragma clang loop unroll_count(8)
  for (i = in, j = out; i < last; i += 3, j += 4) {
    j[0] = indexTable[i[0] >> 2];
    j[1] = indexTable[(i[0] & 0x03) << 4 | i[1] >> 4];
    j[2] = indexTable[(i[1] & 0x0F) << 2 | i[2] >> 6];
    j[3] = indexTable[i[2] & 0x3F];
  }
  if (rem == 1) {
    j[2] = j[3] = '=';
    j[0] = indexTable[i[0] >> 2];
    j[1] = indexTable[(i[0] & 0x03) << 4];
  } else if (rem == 2) {
    j[2] = j[3] = '=';
    j[0] = indexTable[i[0] >> 2];
    j[1] = indexTable[(i[0] & 0x03) << 4 | i[1] >> 4];
    j[2] = indexTable[(i[1] & 0x0F) << 2];
  }
  out[outl] = '\0';
  return out;
}

char * decode(char* d) {
  int len = strlen(d);
  char *i, *j;
  #pragma clang loop unroll_count(8)
  for (i = j = d; i < d+len; i += 4, j += 3) {
    j[0] = (getIndexOf(i[0]) << 2) | (getIndexOf(i[1]) >> 4);
    j[1] = (0xF0 & (getIndexOf(i[1]) << 4)) | (0x0F & (getIndexOf(i[2]) >> 2));
    j[2] = (0xC0 & (getIndexOf(i[2]) << 6)) | (0x3F & (getIndexOf(i[3])));
  }
  j[0] = '\0';
  return d;
}

char inline getIndexOf(char c) {
  return revTable[c-43];
}
