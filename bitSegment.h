#ifndef BITSEGMENT_H
#define BITSEGMENT_H

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include "bitMasker.h"

/*
 * A string of bits of a specified size [1 ~ 8]
 * Enables traversal through bit stream by specified number of bits
 * instead of the standard 8-bit byte
 * Cannot be larger than a byte
 */
class bitSegment {

private:

  // number of bits in a bitSegment
  // must be in range [1 ~ 8], maximum being a byte
  int size;

  // the actual bits in the segment
  // if size < 8, rightmost bits will be used
  unsigned char bits;

  // the full byte (8 bits) in the stream that holds the first bit of
  // the current segment
  unsigned char firstByte;

  // index of segment's first bit within its original byte (firstByte)
  int startIndex;

  // stream of bytes to read from / write to
  fstream bs;

  // whether byte stream is empty
  bool empty;

public:

  // constructor
  // if size outside of [1 ~ 8] is given, default size of 8 is used
  // initializes bits to the first segment in the stream
  bitSegment(int size);

  // destructor
  ~bitSegment();

  // get the value the segment holds
  unsigned char getBits();

  // get the value the segment holds as a string
  std::string getBitsStr();

  // overwrite the bits in the segment
  // returns false on failure (# of specified bits > size of segment)
  bool setBits(unsigned char bits);
  bool setBits(std::string bits);

  // move to segment on the left
  // returns false on failure (segment is leftmost in stream)
  bool moveLeft();

  // move to segment on the right
  // returns false on failure (segment is at the end of the stream)
  // allowed to be one segment to right of rightmost written segment, for
  // the purpose of appending new bits at the end of the stream
  bool moveRight();

};

#endif
