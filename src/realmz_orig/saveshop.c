#include "prototypes.h"
#include "variables.h"

/***************************** saveencounter ***************************/
void saveencounter(short id, short type) {
  FILE* fp = NULL;
  if (type == 1) {
    if ((fp = MyrFopen(":Data Files:CE", "r+b")) == NULL)
      scratch(154);
    fseek(fp, id * (sizeof enc + 320), SEEK_SET);
    CvtEncountToPc(&enc);
    fwrite(&enc, sizeof enc, 1, fp);
    CvtEncountToPc(&enc);
    fclose(fp);
  } else {
    if ((fp = MyrFopen(":Data Files:CE2", "r+b")) == NULL)
      scratch(155);
    fseek(fp, id * (sizeof enc2 + 360), SEEK_SET);
    CvtEncount2ToPc(&enc2);
    fwrite(&enc2, sizeof enc2, 1, fp);
    CvtEncount2ToPc(&enc2);
    fclose(fp);
  }
}
