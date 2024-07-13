#include "prototypes.h"
#include "variables.h"

/***************** loaditem **************************************/
void loaditem(short id) {
  short temp;

  id = abs(id);

  temp = (id / 200) * 200;
  switch (temp) {
    case 0:
      item = allweapons[id - temp];
      break;

    case 200:
      item = allarmor[id - temp];
      break;

    case 400:
      item = allhelms[id - temp];
      break;

    case 600:
      item = allmagic[id - temp];
      break;

    case 800:
      item = allsupply[id - temp];
      break;
  }

  item.type = abs(item.type);
}

/***************** loaditemnotype **************************************/
void loaditemnotype(short id) {
  short temp;

  id = abs(id);

  temp = (id / 200) * 200;
  switch (temp) {
    case 0:
      item = allweapons[id - temp];
      break;

    case 200:
      item = allarmor[id - temp];
      break;

    case 400:
      item = allhelms[id - temp];
      break;

    case 600:
      item = allmagic[id - temp];
      break;

    case 800:
      item = allsupply[id - temp];
      break;
  }
}
