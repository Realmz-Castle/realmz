#include "prototypes.h"
#include "variables.h"

/*************** getselection **************************/
short getselection(short tempid) {
  tempselection = (tempid / 200) * 200;
  return (tempselection);
}
