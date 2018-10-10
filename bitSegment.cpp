#include "bitSegment.h"

/*
 * Implementation for bitSegment class
 * Look at header file for details
 */


bitSegment::bitSegment(int size, std::string filename) {
  // check size
  if(size < 1 || size > 8) {
    this->size = 8;
  } else {
    this->size = size;
  }

  // translate filename to C-string
  const char* filename_c = filename.c_str();

  // open file at end
  bs.open(filename_c, std::fstream::out | std::fstream::in
    | std::fstream::binary | std::fstream::ate);

  // check whether stream is open
  if(!bs.is_open()) {
    // stream fails to open
    // most likely because file doesn't exist
    // create empty file with filename
    bs.open(filename_c, std::fstream::out);
    bs.close();
    bs.open(filename_c, std::fstream::out | std::fstream::in
      | std::fstream::binary | std::fstream::ate);
  }

  // check again whether stream is open
  if(!bs.is_open()) {
    // some other error has occurred
    // handle error TODO
    return;
  }

  // check length of file to determine whether it's empty
  int fileLength = bs.tellp();

  // move cursor to beginning of file (position 0)
  bs.seekp(0);

  // startIndex = 0 regardless of file emptiness
  startIndex = 0;

  // initialize private members according to emptiness

  if(fileLength == 0) {
    // file is empty
    empty = true;
    firstByte = 0;
    bits = 0;
  }

  else {
    // file has content
    empty = false;
    // store first byte in firstByte
    bs.read(&firstByte, 1);
    // move cursor back to beginning
    bs.seekp(0);
    // extract bits from firstByte; first "size" bits, shifted rightmost
    bits = firstByte >> (8 - size);
    // just in case, mask out irrelevant bits on the left
    bits = bits & zeroMask(true, 8 - size);
  }
}


bitSegment::~bitSegment() {
  // close stream
  bs.close();
}


unsigned char bitSegment::getBits() {
  return bits;
}


std::string bitSegment::getBitsStr() {
  // translate bits to string of length size
  std::string output;

  // make disposable local copy of bits
  unsigned char lBits = bits;

  // iterate through lBits from left to right, starting from first
  // meaningful bit (disregard irrelevant leftmost bits)
  for(int i = 8 - size; i < 8; i++) {

    // compare lBits to the relevant power of 2 to determine whether
    // the i'th bit is set
    // append "1" to string if i'th bit is set
    // append "0" to string if i'th bit isn't set
    int poweroftwo = std::pow(2, (7 - i)) - 1;
    if(lBits > poweroftwo) {
      // i'th bit is set
      output.push_back("1");
    } else {
      // i'th bit isn't set
      output.push_back("0");
    }

    // set i'th bit to 0 so next iteration's comparison is valid
    if(lBits > poweroftwo) {
      lBits -= (poweroftwo + 1);
    }
  }

  return output;
}


bool bitSegment::setBits(unsigned char bits) {
  // check that only "size" rightmost bits are set
  // remaining leftmost bits should be all 0's
  if(bits >= std::pow(2, size)) {
    return false;
  }

  this->bits = bits;

  // if the file is empty, simply write to file the given segment
  if(empty) {
    // make a copy of bits
    unsigned char bitsCopy = bits;
    // shift left to beginning of byte
    bitsCopy = bitsCopy << (8 - size);
    // write to file its first bit
    bs.seekp(0);
    bs.write(&bitsCopy, 1);
    bs.seekp(0);
    // update empty
    empty = false;
    return true;
  }

  // 2 possibilities:
  // a) current segment holds bits from one single byte
  // b) current segment holds bits from two different bytes

  // case a: segment contained in a single byte
  if(startIndex + size <= 8) {
    // make a copy of bits
    unsigned char bitsCopy = bits;
    // shift left to correct position in byte
    bitsCopy = bitsCopy << (8 - startIndex - size);
    // irrelevant bits are already set to 0
    // set 1's in firstByte
    firstByte = firstByte | bitsCopy;
    // set irrelevant bits to 1
    bitsCopy = bitsCopy | zeroMask(false, 8 - startIndex);
    bitsCopy = bitsCopy | zeroMask(true, startIndex + size);
    // set 0's in firstByte
    firstByte = firstByte & bitsCopy;
    // write firstByte to stream
    bs.write(&firstByte, 1);
    // shift cursor back one unit to make up for write()'s movement
    bs.seekp(-1, std::ios_base::cur);
  }

  // case b: segment contained in two consecutive bytes
  else {
    // step 1: end of byte A
    // make a copy of bits
    unsigned char bitsCopy = bits;
    // shift right to correct position in byte A
    bitsCopy = bitsCopy >> ((startIndex + size) % 8);
    // set irrelevant bits to 0
    bitsCopy = bitsCopy & zeroMask(true, startIndex);
    // set 1's in firstByte
    firstByte = firstByte | bitsCopy;
    // set irrelevant bits to 1
    bitsCopy = bitsCopy | zeroMask(false, 8 - startIndex);
    // set 0's in firstByte
    firstByte = firstByte & bitsCopy;
    // write firstByte to stream
    bs.write(&firstByte, 1);
    // shift cursor back
    bs.seekp(-1, std::ios_base::cur);

    // step 2: beginning of byte B
    // re-copy bits
    bitsCopy = bits;
    // shift left to correct position in byte B
    bitsCopy = bitsCopy << (8 - ((startIndex + size) % 8));
    // pull the next byte from the stream
    // if the current segment has never been written (current position
    //   is one after the last written segment), byte B doesn't exist
    //   in the file. reading from here would cause an error. simply
    //   write to it, creating byte B in the file.

    // check that byte B exists
    // read byte B and if EOF is set, byte B does not exist
    bool atend = false;
    bs.seekp(1, std::ios_base::cur);
    unsigned char tempC = 0;
    bs.read(&tempC, 1);
    if(bs.eof()) {
      // byte B doesn't exist
      atend = true;
      // reset EOF bit
      bs.clear();
      // recover cursor to point to firstByte
      bs.seekp(-1, std::ios_base::end);
    } else {
      // byte B exists
      // recover cursor to point to firstByte
      seekp(-2, std::ios_base::cur);
    }

    if(!atend) {
      // byte B exists; pull it from the stream
      bs.seekp(1, std::ios_base::cur);
      unsigned char nextByte = 0;
      bs.read(&nextByte, 1);
      bs.seekp(-1, std::ios_base::cur);
      // irrelevant bits are already set to 0
      // set 1's in nextByte
      nextByte = nextByte | bitsCopy;
      // set irrelevant bits to 1
      bitsCopy = bitsCopy | zeroMask(true, (startIndex + size) % 8);
      // set 0's in nextByte
      nextByte = nextByte & bitsCopy;
      // write nextByte to stream and return cursor to firstByte
      bs.write(&nextByte, 1);
      bs.seekp(-2, std::ios_base::cur);
    }

    else {
      // byte B does not exist; writing this segment creates byte B
      //   in the file
      // no previous data to worry about; simply overwrite entire
      //   byte as is
      bs.seekp(1, std::ios_base::cur);
      bs.write(&bitsCopy, 1);
      bs.seekp(-2, std::ios_base::cur);
    }
  }

  return true;
}


bool bitSegment::setBits(std::string bits) {
  // check that length of string is equal to segment size
  if(bits.size() != size) {
    return false;
  }

  // local copy of segment to later push into instance variable
  // and into the stream
  unsigned char lBits = 0;

  // translate string to unsigned char
  for(int i = 0; i < size; i++) {
    if(bits[i] == '1') {
      // 2 ^ (7 - (8 - size + i))
      lBits += std::pow(2, (size - i - 1) );
    } else if(bits[i] != '0') {
      return false;
    }
  }

  // write it to stream
  return this->setBits(lBits);
}


bool bitSegment::moveLeft() {
  // if file is empty, return false
  if(empty) {
    return false;
  }

  // if startIndex - size >= 0, segment can be shifted without pulling
  // a byte from the stream
  if(startIndex - size >= 0) {
    // shift segment using firstByte

    // update startIndex
    startIndex -= size;

    // make local copy of firstByte
    unsigned char lBits = firstByte;

    // shift right so the segment is rightmost
    lBits = lBits >> (8 - (startIndex + size) );
    // mask out bits on left that aren't in the segment
    lBits = lBits & zeroMask(true, 8 - size);
    // set bits as lBits
    bits = lBits;
  }
  // otherwise, pull byte to the left from stream
  else {
    // if firstByte is position 0, cannot move left
    if(bs.tellp() == 0) {
      return false;
    }

    // copy previous byte into lBits
    unsigned char lBits = 0;
    unsigned char prevByte = 0;
    bs.seekp(-1, std::ios_base::cur);
    bs.read(&prevByte, 1);
    bs.seekp(1, std::ios_base::cur);
    lBits = prevByte;

    // shift left to make room on the right for bits from firstByte
    lBits = lBits << startIndex;
    // mask out irrelevant bits on left (into 0)
    lBits = lBits & zeroMask(true, 8 - size);
    // pull out the relevant bits from firstByte
    unsigned char rBits = firstByte >> (8 - startIndex);
    // mask out irrelevant bits on left (into 0)
    rBits = rBits & zeroMask(true, 8 - startIndex);
    // set rBits' 1's into lBits
    lBits = lBits | rBits;
    // set irrelevant bits in rBits to 1
    rBits = rBits | zeroMask(false, startIndex);
    // set rBits' 0's into lBits
    lBits = lBits & rBits;

    // set lBits as bits
    bits = lBits;
    // update other private members
    firstByte = prevByte;
    startIndex = startIndex - size + 8;
  }

  return true;
}

// NOTE: when at the end of the stream, moving right can still be valid.
// if segment includes an unwritten bit, cannot move right until the
// unwritten bits are written to.
// if segment doesn't include an unwritten bit, can move right, even if
// the very next bit to the right is unwritten.
bool bitSegment::moveRight() {
  // if file is empty, return false
  if(empty) {
    return false;
  }

  // 4 possibilities: current segment is:
  // a) entirely inside firstByte & firstByte is written
  //    = can move
  // b) entirely inside firstByte & firstByte is not written
  //    = cannot move
  // c) bleeding into nextByte & nextByte is written
  //    = can move
  // d) bleeding into nextByte & nextByte is not written
  //    = cannot move

  // if the move puts the segment in unwritten territory, the move itself
  //   is valid but pulling to update the bits will set the EOF bit.
  //   reset EOF with clear() and set bits to 0. firstBit may or may not
  //   be affected.

  if(startIndex + size <= 8) {
    // segment is entirely inside firstByte
    // check whether firstByte is written
    unsigned char fb = 0;
    bs.read(&fb, 1);
    if(bs.eof()) {
      // firstByte is not written
      bs.clear();
      bs.seekp(-1, std::ios_base::end);
      // cannot move right
      return false;
    } else {
      // can move right
      bs.seekp(-1, std::ios_base::cur);

      // 3 possibilities

      if((startIndex + (2 * size) - 1) <= 7) {
        // next segment is still entirely within firstByte

        // update startIndex
        startIndex += size;

        // make local copy of firstByte
        unsigned char lBits = firstByte;

        // shift right so the segment is rightmost
        lBits = lBits >> (8 - (startIndex + size) );
        // mask out bits on left that aren't in the segment
        lBits = lBits & zeroMask(true, 8 - size);
        // set bits as lBits
        bits = lBits;
      }

      else if(startIndex + size < 8) {
        // firstByte remains unchanged
        // update startIndex
        startIndex += size;

        // new bits will bleed into next byte
        unsigned char rBits = 0;
        bs.seekp(1, std::ios_base::cur);
        bs.read(&rBits, 1);
        bs.seekp(-2, std::ios_base::cur);

        unsigned char lBits = firstByte;

        // combine relevant parts of lBits and rBits
        //TODO
      }

      else {
        // firstByte updates to next byte
        bs.seekp(1, std::ios_base::cur);
        bs.read(&firstByte, 1);
        bs.seekp(-1, std::ios_base::cur);

        // update startIndex
        startIndex = 0;

        // update bits
        unsigned char lBits = firstByte;
        //TODO
      }
    }
  }

  else {
    // segment bleeds into the next byte
    // check whether next byte is written
    unsigned char nb = 0;
    bs.seekp(1, std::ios_base::cur);
    bs.read(&nb, 1);
    if(bs.eof()) {
      // next byte is not written
      bs.clear();
      bs.seekp(-1, std::ios_base::end);
      // cannot move right
      return false;
    } else {
      // can move right
      bs.seekp(-1, std::ios_base::cur);

      // update firstByte to next byte
      firstByte = nb;
      // update startIndex
      startIndex = (startIndex + size) % 8;

      // 2 possibilities

      if(startIndex + size <= 8) {
        // next segment doesn't bleed into next-next byte
        // update bits
        //TODO
      }

      else {
        // next segment bleeds into next-next byte
        // update bits; need to pull from next-next byte
        //TODO
      }
    }
  }

  return true;

/*

  // if startIndex + (2 * size) - 1 <= 7, segment can be shifted without
  // pulling a byte from the stream
  if((startIndex + (2 * size) - 1) <= 7) {
    // shift segment using firstByte

    // update startIndex
    startIndex += size;

    // make local copy of firstByte
    unsigned char lBits = firstByte;

    // shift right so the segment is rightmost
    lBits = lBits >> (8 - (startIndex + size) );
    // mask out bits on left that aren't in the segment
    lBits = lBits & zeroMask(true, 8 - size);
    // set bits as lBits
    bits = lBits;
  }

  // otherwise, pull byte to the right from stream
  else {
    //TODO

    // if current segment (before move) includes even one
    //   unwritten bit, the segment hasn't been written yet
    // cannot move right

    bs.seekp(1, std::ios_base::cur);
    unsigned char nextByte = 0;
    bs.read(&nextByte, 1);
    if(bs.eof()) {
      // next byte doesn't exist
      // 
      // reset EOF bit
      bs.clear();
      // recover cursor to point to firstByte
      bs.seekp(-1, std::ios_base::end);
      // TODO
    }
    // next byte exists
    // recover cursor to point to firstByte
    bs.seekp(-2, std::ios_base::cur);

    // update startIndex
    startIndex = startIndex + size;
    if(startIndex >= 8) {
      // startIndex is in nextByte
      // update startIndex
      startIndex = startIndex % 8;
      // update firstByte
      firstByte = nextByte;
      bs.seekp(1, std::ios_base::cur);

      // determine whether segment bleeds into next next byte
      if(startIndex + size >= 8) {
        // check if the next byte exists
        bs.seekp(1, std::ios_base::cur);
        bs.read(&nextByte, 1);
        if(bs.eof()) {
          // next byte doesn't exist
          bs.clear();
          bs.seekp(-1, std::ios_base::end);
          //TODO
        }
      }
    }
  }
*/
}
