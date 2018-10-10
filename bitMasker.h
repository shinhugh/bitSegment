#ifndef BITMASKER_H
#define BITMASKER_H

#include <stdio.h>
#include <cmath>

/*
 * Create string of 8 bits (byte) with specified number of 0's on the
 * specified side, with the rest being 1's
 * bool left: whether 0's go on the left
 * int maskNum: number of 0's
 * Example: zeroMask(true, 3) creates 00011111
 */
unsigned char zeroMask(bool left, int maskNum) {
  unsigned char output = 0;
  if(left) {
    for(int i = 0; i < (8 - maskNum); i++) {
      output += std::pow(2, i);
    }
  } else {
    for(int i = 7; i > (maskNum - 1); i--) {
      output += std::pow(2, i);
    }
  }

  return output;
}

#endif
