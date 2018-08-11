#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

char* encode(char* in);
char* decode(char* d);
uint8_t dec_char(char c);

const uint8_t enc_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const uint8_t dec_table[] = {62,0,0,0,63,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};

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

  printf("%s\n", decode(encode("123")));
  printf("%s\n", decode(encode("1234")));
  printf("%s\n", decode(encode("12345")));
  printf("%s\n", decode(encode("123456")));
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
  int rem = strlen(in);
  int outl = ((rem+2)/3)*4;
  char *out = malloc(outl+1);
  char *i = in, *o = out;

  #pragma clang loop unroll_count(8)
  while (rem >= 8) {
    // Grab 8 bytes of input (we will only use 6)
    uint64_t i8 = *(uint64_t *)i;
    // We want BE, but our arch is probably LE
    i8 = __builtin_bswap64(i8);
    // cut out 6-bit slices to use as table indices
    *o++ = enc_table[(i8 >> 58) & 0x3F];
    *o++ = enc_table[(i8 >> 52) & 0x3F];
    *o++ = enc_table[(i8 >> 46) & 0x3F];
    *o++ = enc_table[(i8 >> 40) & 0x3F];
    *o++ = enc_table[(i8 >> 34) & 0x3F];
    *o++ = enc_table[(i8 >> 28) & 0x3F];
    *o++ = enc_table[(i8 >> 22) & 0x3F];
    *o++ = enc_table[(i8 >> 16) & 0x3F];
    i += 6;
    rem -= 6;
  }
  // At this point, there are 0-7 bytes of input remaining
  if (rem >= 4) {
    encode_chunk_32: ;
    // Grab 4 bytes of input (we will only use 3)
    uint32_t b = *(uint32_t *)i;
    b = __builtin_bswap32(b);
    // cut out 6-bit slices to use a table indices
    *o++ = enc_table[(b >> 26) & 0x3F];
    *o++ = enc_table[(b >> 20) & 0x3F];
    *o++ = enc_table[(b >> 14) & 0x3F];
    *o++ = enc_table[(b >>  8) & 0x3F];
    i += 3;
    rem -= 3;
  }
  // At this point, there are 0-3 bytes of input remaining
  if (rem == 3) {
    // Because the input is null terminated, we can read one byte past the end of
    // our data without segfaulting. So even though we only have 3 bytes left, we
    // can use the previous routine that reads 4 bytes but only uses 3.
    goto encode_chunk_32;
  } else if (rem) {
    // At this point, there are either 1 or 2 bytes of input remaining.
    // again, because of the null terminator, we can safely read two bytes.
    uint16_t b = *(uint16_t *)i;
    b = __builtin_bswap16(b);
    *o++ = enc_table[(b >> 10) & 0x3F];
    *o++ = enc_table[(b >>  4) & 0x3F];
    *o++ = (rem == 2) ? enc_table[(b << 2) & 0x3F] : '=';
    *o++ = '=';
  }
  // Now there are 0 bytes of input remaining. Add a null terminator and we're done.
  *o = '\0';
  return out;
}

char * decode(char* d) {
  int rem = strlen(d);
  char *i = d, *o = d;
  #pragma clang loop unroll_count(8)
  while (rem) {
    uint32_t *o4 = (uint32_t *)o;
    *o4 = (dec_char(i[0]) << 26)
        | (dec_char(i[1]) << 20)
        | (dec_char(i[2]) << 14)
        | (dec_char(i[3]) <<  8);
    *o4 = __builtin_bswap32(*o4);
    i += 4;
    o += 3;
    rem -= 4;
  }
  // We get the null terminator for free because the last byte of every 4-byte
  // chunk we write to is null. No need to set it again.
  return d;
}

uint8_t inline dec_char(char c) {
  return dec_table[c-43];
}
