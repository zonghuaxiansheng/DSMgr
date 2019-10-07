#ifndef _USTC_TYPES_H_
#define _USTC_TYPES_H_

#define FRAMESIZE 4096
#define DEFBUFSIZE  1024

struct bFrame {
  /*
    \! One frame contains FRAMESIZE bytes.
  */
  char field[FRAMESIZE];
};

#endif