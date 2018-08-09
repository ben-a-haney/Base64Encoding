#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

char* encode(char* data);
char* decode(char* data);
int getIndexOf(char c);

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

  printf("%s\n", decode(encode("AAAAAAAAAAAA")));
  printf("%s\n", decode(encode("123")));
  printf("%s\n", decode(encode("1234")));
  printf("%s\n", decode(encode("12345")));
  printf("%s\n", decode(encode("123456")));
  printf("%s\n", decode("QUJDYWJjMTIzWFlaeHl6"));
  printf("%s\n", encode("This is a string that will be encoded and then decoded.\n\
If you can read this, my hand crafted algorithm is working swimingly...\n\
Now for some non-Base64 characters: ~~~```<<<()()()$$$$$^^^^^@@@@@()()()>>>```~~~"));
  printf("%s\n", decode(encode("This is a string that will be encoded and then decoded.\n\
If you can read this, my hand crafted algorithm is working swimingly...\n\
Now for some non-Base64 characters: ~~~```<<<()()()$$$$$^^^^^@@@@@()()()>>>```~~~")));

  gettimeofday(&timeStart, NULL);
  for(long i = 0; i < 1000000; i++){
    largeTest_enc = encode(largeData);
    largeTest_dec = decode(largeTest_enc);
    free(largeTest_enc);
    free(largeTest_dec);
  }

  gettimeofday(&timeStop, NULL);
  largeTest_enc = encode(largeData);
  largeTest_dec = decode(largeTest_enc);
  printf("%s\n", largeTest_dec);
  printf("Total time in seconds: %lf\n", (timeStop.tv_sec - timeStart.tv_sec) + (timeStop.tv_usec - timeStart.tv_usec)/1000000.0);
}

char* encode(char* data) {
  int datLen = strlen(data);
  int rem = datLen % 3;
  int encLen = (datLen / 3 + (rem ? 1 : 0)) * 4 + 1;
  char* buffer = malloc(encLen);
  for (int i = 0, j = 0; i < (datLen - rem); i += 3, j += 4) {
    buffer[j] = indexTable[data[i] >> 2];
    buffer[j+1] = indexTable[(data[i] & 0x03) << 4 | data[i+1] >> 4];
    buffer[j+2] = indexTable[(data[i+1] & 0x0F) << 2 | data[i+2] >> 6];
    buffer[j+3] = indexTable[data[i+2] & 0x3F];
  }

  if (rem > 0) {
    char* lastIn = data + datLen - rem;
    char* lastOut = buffer + encLen - 5;
    memcpy(lastOut, "====", 4);
    lastOut[0] = indexTable[lastIn[0] >> 2];
    if (rem == 1) {
      lastOut[1] = indexTable[(lastIn[0] & 0x03) << 4];
    }
    else if (rem == 2) {
      lastOut[1] = indexTable[(lastIn[0] & 0x03) << 4 | lastIn[1] >> 4];
      lastOut[2] = indexTable[(lastIn[1] & 0x0F) << 2];
    }
  }
  buffer[encLen-1] = '\0';
  return buffer;
}

char* decode(char* data) {
  int encLen = strlen(data);
  int rem = (data[encLen-1] == '=') + (data[encLen-2] == '=');
  int decLen = ((encLen * 3) / 4) - rem + 1;
  char* buffer = malloc(decLen);

  for (int i = 0, j = 0; i < (encLen - 4); i += 4, j += 3) {
    buffer[j] = (char)((getIndexOf(data[i]) << 2) | (getIndexOf(data[i+1]) >> 4));
    buffer[j+1] = (char)(0xF0 & (getIndexOf(data[i+1]) << 4) | 0x0F & (getIndexOf(data[i+2]) >> 2));
    buffer[j+2] = (char)(0xC0 & (getIndexOf(data[i+2]) << 6) | 0x3F & (getIndexOf(data[i+3])));
  }

  char* last = data + encLen - 4;
  if (rem == 0) {
    buffer[decLen-4] = (char)((getIndexOf(last[0]) << 2) | (getIndexOf(last[1]) >> 4));
    buffer[decLen-3] = (char)(0xF0 & (getIndexOf(last[1]) << 4) | 0x0F & (getIndexOf(last[2]) >> 2));
    buffer[decLen-2] = (char)(0xC0 & (getIndexOf(last[2]) << 6) | 0x3F & (getIndexOf(last[3])));
  } else if (rem == 1) {
    buffer[decLen-3] = (char)((getIndexOf(last[0]) << 2) | (getIndexOf(last[1]) >> 4));
    buffer[decLen-2] = (char)(0xF0 & (getIndexOf(last[1]) << 4) | 0x0F & (getIndexOf(last[2]) >> 2));
  } else if (rem == 2) {
    buffer[decLen-2] = (char)((getIndexOf(last[0]) << 2) | (getIndexOf(last[1]) >> 4));
  }
  buffer[decLen-1] = '\0';
  return buffer;
}

int inline getIndexOf(char c) {
  return revTable[c-43];
}
