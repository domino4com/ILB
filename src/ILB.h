/*!
 * @file ILB.h
 *
 * Written by @gotfredsen, with contributions from the open source community.
 *
 * MIT license.
 *
 */

#ifndef _ILB_H_
#define _ILB_H_

#include "Arduino.h"
#include <LTR390.h>
#include <Wire.h>

class ILB : public LTR390 {
public:
  ILB() : LTR390() {};
  ILB(int addr) : LTR390( addr) {};
  ILB(TwoWire *w, int addr) : LTR390( *w, addr) {};
  ILB(TwoWire *w) : LTR390(*w) {};
private:
};

#endif // _ILB_H_
