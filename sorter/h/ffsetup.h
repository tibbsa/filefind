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
/* FileArea Configuration                                                  */
/***************************************************************************/
typedef struct
{
   char     szFileList [120];                         /* Header for area   */
} FF_FArea;

/***************************************************************************/
/* Configuration Structure                                                 */
/***************************************************************************/
typedef struct
{
   char    *FAreas [MAXAREAS];                     /* #/fileareas       */
} FF_Setup;

#endif      /* #ifndef __FFSETUP_H__ */

