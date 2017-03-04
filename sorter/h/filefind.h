/***************************************************************************/
/* AllFix-equivalent `FILEFIND' request responder                          */
/* Version 1                                                               */
/*                                                                         */
/* general include file                                                    */
/***************************************************************************/

#ifndef __FILEFIND_H__
#define __FILEFIND_H__

#include <conio.h>
#include <util.h>
#include <msgapi.h>

#include <dir.h>
#include <dos.h>
#include "w_wrap.h"
#include "ffsetup.h"
#include "snipsort.h"
#include "snipfile.h"

/***************************************************************************/
/* Program name/version #                                                  */
/***************************************************************************/
#define PROG         "Files.Bbs Sort Utility, Version 1.0\n"
#define COPY         "Copyright (c) 1998 by Anthony Tibbs\n"
#define VER          "1.0"

/***************************************************************************/
/* Prototypes                                                              */
/***************************************************************************/
void Init_Vars (void);
int Configure (void);
void Parse_Address (NETADDR *pAddr, char *pszStr);
unsigned char Do_It (void);
char *stristr(const char *String, const char *Pattern);
void Rewrite (char *pszFileListing);

#define LOADING      Status [0]
#define SORTING      Status [1]
#define WRITING      Status [2]
#define REMOVE_ERR   Status [3]
#define SHARING_ERR  Status [4]
#define RENAME_ERR   Status [5]
#define SUCCESSFUL   Status [6]
#define CLEAR        Status [7]
#define OPEN_ERR     Status [8]
#define TOOMANYFILES Status [9]

#endif               /* __FILEFIND_H__ */

