/********************************************************************************

    PROJECT:    clut_fade.�

    FILE:       fade.h

    PURPOSE:    constants, type definitions and prototypes

    ??/??/93    1.0 written by N. Jonas Englund
    07/26/94    Changes by Mark Womack to allow fading all monitors, only the
                            main monitor, or all monitors except the main monitor.
    10/21/94    Changes by Mark Womack to make pascal friendly, fix < 256 color
                            crasher, cleaned up to make more readable.
    10/24/94    Integrated fade_to_clut function written by Macneil Shonle.
                            Added copy_gdevice_clut function to make it easier to save
                            and restore device clut's.
    04/22/96    Fixed bug found by David Padilla. Fade routines changed the ctSeed
                            value of color tables, which slows subsequent calls to CopyBits.
                            Now restore or set the ctSeed value back to original value.

    =-=-= PLEASE SEE THE README THAT ACCOMPANIED THIS PROJECT FOR DETAILS =-=-=

    This software is considered Public Domain. You are free to use it in any
    manner you wish. You are free to upload it to your favorite service, but
    you must post it with the accompanying readme and description files.
    If you use or appreciate it, please let us know!! We all love to get email.
    See the addresses below.

    This software is offered 'as is'. The authors are not responsible for any
    damages caused by bugs or defects that might be lurking. But if it blows up
    your monitor, please let us know. If you find any bugs, problems, enhancements,
    please contact us.

    Email Addresses:  MarkWomack@aol.com, MacneilS@aol.com, KenLong@aol.com.

 ********************************************************************************/

//=================================== FLAGS =====================================

#define fadeMainOnly 1
#define fadeAll 2
#define fadeAllButMain 4

//================================= FUNCTIONS ===================================

extern pascal void fade_to_black(long numSteps, short fadeFlags, Boolean fadeOut);

extern pascal void fade_to_clut(long numSteps, CTabHandle destTab, GDHandle aGDevice);

extern pascal void fade_to_color(long numSteps, RGBColor* destColor, GDHandle aGDevice);

extern pascal void copy_gdevice_clut(GDHandle aGDevice, CTabHandle* copyOfClut);

//=================================== EOF =======================================
