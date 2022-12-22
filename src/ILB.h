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

class ILB : public LTR390 {
public:
  ILB() : LTR390();
private:
};

#endif // _ILB_H_
