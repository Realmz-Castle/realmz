#include "prototypes.h"
#include "variables.h"

/******************************************* plus **********************/
void plus(char charactername[30], short level) {
  short t;
  FILE* op;
  char name[30];
  short temp = 0;

  if ((op = MyrFopen(":Character Files:Data CD", "r+b")) == NULL) {
    if ((op = MyrFopen(":Character Files:Data CD", "w+b")) == NULL)
      scratch2(10);
  }

  for (t = 0; t < 500; t++) {
    strcpy(name, (StringPtr) "");
    fread(&name, 30, 1, op);
    fread(&temp, 2, 1, op); /***** level placeholder ****/
    CvtShortToPc(&temp);
    if (!strlen(name)) {
      fseek(op, t * 32, SEEK_SET);
      fwrite(charactername, 30, 1, op);
      CvtShortToPc(&level);
      fwrite(&level, sizeof level, 1, op); /***** level placeholder ****/
      CvtShortToPc(&level);
      fclose(op);
      return;
    }
  }
  fclose(op);
}

/******************************************* minus **********************/
void minus(char name[30], short mode) {
  short t, temp = 0;
  FILE* op;
  char name2[30];

  strcpy(filename, ":Character Files:");
  strcat((StringPtr)filename, name);
  if (!mode)
    MyrRemove(filename);

  if ((op = MyrFopen(":Character Files:Data CD", "r+b")) == NULL) {
    if ((op = MyrFopen(":Character Files:Data CD", "w+b")) == NULL)
      scratch2(10);
  }

  for (t = 0; t < 500; t++) {
    strcpy(name2, (StringPtr) "");
    fseek(op, t * 32, SEEK_SET);
    if (fread(&name2, 30, 1, op)) {
      fread(&temp, 2, 1, op); /***** level placeholder ****/
      CvtShortToPc(&temp);
      if (!strcmp(name2, name)) {
        strcpy(name2, (StringPtr) "");
        fseek(op, t * 32, SEEK_SET);
        fwrite(&name2, 30, 1, op);
        CvtShortToPc(&temp);
        fwrite(&temp, 2, 1, op); /***** level placeholder ****/
        CvtShortToPc(&temp);
        fclose(op);
        return;
      }
    } else {
      fclose(op);
      return;
    }
  }
  fclose(op);
}
