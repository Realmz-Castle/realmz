#include "prototypes.h"
#include "variables.h"

#define VERS_RES_ID 1 /* the resource ID of the 'vers' to use (usually 1 as of Sys7.1) */
#define BYTE_OFFSET 6 /* the offset of the first byte containing the version  	*/

/*********************************** GetVersStr *************************/
void GetVersStr(short id, short refnum) {
  Handle resHandle; /* holds the 'vers' resource 		*/
  short versStrLength; /* the length of the version string */
  short currresfile;

  currresfile = CurResFile();

  if (refnum)
    UseResFile(refnum);

  resHandle = Get1Resource('vers', id); /* load in the resource */

  if (resHandle) // Myriad: Check if handle found
  {
    HLockHi(resHandle); /* Lock the handle because we're BlockMoving */

    versStrLength = (short)((unsigned char)(*(*resHandle + BYTE_OFFSET)));
    /* get the length of the string from the first byte */

    BlockMove((*resHandle + BYTE_OFFSET), theString, (versStrLength + 1));
    /* copy the appropriate bytes into theString 							*/
    /* note the addition of 1 to versStrLength -- this gets the length byte */

    HUnlock(resHandle); /* unlock the handle */
    ReleaseResource(resHandle); /* release the resource from memory */
    MyrStrPtoC((Ptr)theString); // Myriad
  } else
    strcpy(theString, (StringPtr) "\pUnknown");
  UseResFile(currresfile);
}
