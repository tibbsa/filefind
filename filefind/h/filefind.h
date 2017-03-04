#define NDEBUG
/***************************************************************************/
/* AllFix-equivalent `FILEFIND' request responder                          */
/* Version 1                                                               */
/*                                                                         */
/* general include file                                                    */
/***************************************************************************/

#ifndef __FILEFIND_H__
#define __FILEFIND_H__

#include <util.h>
#include <msgapi.h>

#include <dir.h>
#include <dos.h>
#include "w_wrap.h"
#include "ffsetup.h"

/***************************************************************************/
/* Program name/version #                                                  */
/***************************************************************************/
#define PROG         "AllFix-Style FILEFIND Utility, Version 1.6\n"
#define COPY         "Copyright (c) 1998 by Anthony Tibbs\n"
#define VER          "1.6"

/***************************************************************************/
/* "Search Keys" Structure                                                 */
/***************************************************************************/
typedef struct
{
   char  szKey [60];
   char  Next_AND;
} Search_Key;

/***************************************************************************/
/* Prototypes                                                              */
/***************************************************************************/
void Init_Vars (void);
int Configure (void);
void Parse_Address (NETADDR *pAddr, char *pszStr);
unsigned char Do_It (void);
char *stristr(const char *String, const char *Pattern);
void Process_Message (void);
void Extract_Keywords (char *pszString);
int Matched (char *pszTemplate);
void Area_Header (char *pszBody, int nArea);
int Import_Footer (char *pszBody, int nArea);
int Import_Header (char *pszBody, int nArea);
void Add_File (char *pszBody, char *pszFilename, char *pszComment, int nArea);
size_t commafmt(char   *buf,            /* Buffer for formatted string  */
                int     bufsize,        /* Size of buffer               */
                long    N);             /* Number to convert            */
void Build_Msg_Control_Data (char *pszBuffer, int nArea);
static struct _stamp *GetTimeStamp (time_t tt);
void Add_Stats (char *pszBuffer, int nArea);
unsigned long crc32buf(char *buf, size_t len);
unsigned long Make_MSGID (void);

#endif               /* __FILEFIND_H__ */

