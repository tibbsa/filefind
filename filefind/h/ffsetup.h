/***************************************************************************/
/* AllFix-equivalent `FILEFIND' request responder                          */
/* Version 1                                                               */
/*                                                                         */
/* Configuration information header.                                       */
/***************************************************************************/

#ifndef __FFSETUP_H__
#define __FFSETUP_H__

#define MAXAREAS        500

/***************************************************************************/
/* MsgArea Configuration                                                   */
/***************************************************************************/
typedef struct
{
   char     szTitle [36];                             /* Area "title"      */
   char     szPath [120];                             /* Path to area      */
   char     szHeader [120];                           /* Header for area   */
   char     szFooter [120];                           /* Footer for area   */
   char     szOrigin [76];                            /* Origin line text  */
   NETADDR  AKA;                                      /* Aka to use        */
} FF_Area;

/***************************************************************************/
/* FileArea Configuration                                                  */
/***************************************************************************/
typedef struct
{
   char     szTitle [72];                             /* Area "title"      */
   char     szPath [120];                             /* Path to area      */
   char     szFileList [120];                         /* Header for area   */
} FF_FArea;

/***************************************************************************/
/* Configuration Structure                                                 */
/***************************************************************************/
typedef struct
{
   char     szFilename [120];                         /* Config filename   */
   char     szSearchName [36];                        /* Name to reply to  */
   char     szReplyFrom [36];                         /* Name to reply frm */
   char     szLog [120];                              /* Log file          */
   FF_Area *Areas [MAXAREAS];                         /* #/areas           */
   FF_FArea*FAreas [MAXAREAS];                     /* #/fileareas       */
} FF_Setup;

#endif      /* #ifndef __FFSETUP_H__ */

