/***************************************************************************/
/* AllFix-equivalent `FILEFIND' request responder                          */
/* Version 1                                                               */
/*                                                                         */
/* This module actually searches & responds to requests.                   */
/***************************************************************************/

#include <filefind.h>

/***************************************************************************/
/* Global Variables                                                        */
/***************************************************************************/
extern   FF_Setup             Config;
extern unsigned _stklen;
struct Files_Data
{
   char              szName [16];
   unsigned long     lOfs;
};

const char *Status[] = {"<  Loading   >",
                        "<  Sorting   >",
                        "<  Writing   >",
                        "< Remove Err >",
                        "<  Sharing   >",
                        "< Rename Err >",
                        "< Successful >",
                        "\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b",
                        "< Can't Open >",
                        "< TooManyFls >"};

                       
struct Files_Data    Files [2400];
char *QSortFiles [2400];

int qsCompare (const void *a, const void *b)
{
   return (strcmp ((char *)a, (char *)b));
}

/***************************************************************************
 Function : unsigned char Do_It (void)    
 Arguments: none
 Return   : 0 = OK, 1 = Error

 Descript.:
   This actually does it.  It calls MsgBase_Init(), Find(), Reply(), etc.
 ***************************************************************************/
unsigned char Do_It (void)
{
long nResult;
FILE *hArea;
int nDone=0;
char szLine [1024];
int nCurFile=0;
   memset (&Files, '\0', sizeof Files);

   while (Config.FAreas [nDone])
   {
   unsigned long lLastOffset=0L;
   int nx;

      if (nCurFile == 2399)
      {
         printf (CLEAR);
         printf (TOOMANYFILES);
         goto NextArea;
      }

      hArea = _fsopen (Config.FAreas [nDone], "r", SH_DENYWR);
      if (!hArea)
      {
         printf ("Area: %-45.45s  ", Config.FAreas [nDone]);
         printf (OPEN_ERR);
         goto NextArea;
      }

      printf ("Area: %-45.45s  ", Config.FAreas [nDone]);
      /*
      ** Logic:
      **    - Read a line
      **    - If we can get a filename, assume this is a file, and fill out
      **      Files [x] accordingly.
      **    - Continue to next line.
      */
      lLastOffset = ftell (hArea);
      printf (LOADING);
      while (fgets (szLine, 1024, hArea) != NULL)
      {
      int Flags;
      char szFilename [MAXFILE];
      char szExtension [MAXEXT];
      char *p;
      
         strtrim (szLine);
         if (*szLine == '\r' || *szLine == '\n' || *szLine == '\0')
            continue;
         p = strtok (szLine, " \t\r\n");
         if (!p)
            continue;

         Flags = fnsplit (p, NULL, NULL, szFilename, szExtension);
         if (!(Flags & FILENAME))
            continue;

         if ((Flags & EXTENSION))
            sprintf (Files [nCurFile].szName, "%s%s", szFilename, szExtension);
         else
            sprintf (Files [nCurFile].szName, "%s", szFilename);

         Files [nCurFile].lOfs = lLastOffset;
         lLastOffset = ftell (hArea);
         nCurFile++;
      }

      printf (CLEAR); printf (SORTING);

      if (nCurFile)
         qsort ((void *)&Files, nCurFile, sizeof (Files[0]), qsCompare);

      printf (CLEAR); printf (WRITING);


      fclose (hArea);
      Rewrite (Config.FAreas [nDone]);

NextArea:
      printf ("\n");
      memset (&Files, '\0', sizeof Files);
      nCurFile=0;
      nDone++;
   }


   /* All is OK. */
   return 0;
}

/***************************************************************************
 Function : void Rewrite (char *pszFileListing)
 Arguments: char *pszFileList    Filename of original FILES.BBS file.
 Return   : none

 Descript.:
   Rewrites a file listing in alphabetical order.
 ***************************************************************************/
void Rewrite (char *pszFileListing)
{
int nCurFile;
FILE *hInput;
FILE *hOutput;
char szComment [1024];
char szLine [1024];

   hInput = _fsopen (pszFileListing, "r", SH_DENYWR);
   if (!hInput)
   {
      printf (CLEAR); printf (OPEN_ERR);
      return;
   }

   hOutput = _fsopen ("TEMP.$$-", "w", SH_DENYWR);
   if (!hOutput)
   {
      fclose (hInput);
      printf (CLEAR); printf (OPEN_ERR);
      return;
   }

   nCurFile=0;
   while (*Files [nCurFile].szName)
   {
   char *p;

      /* Get to the location of the original file in FILES.BBS. */
      fseek (hInput, Files[nCurFile].lOfs, SEEK_SET);
      /* Read the line. */
      if (fgets (szLine, 1024, hInput) == NULL)
      {
         printf (CLEAR); printf (SHARING_ERR);
         fclose (hInput);
         fclose (hOutput);
         remove ("TEMP.$$-");
         return;
      }

      p = strtok (szLine, " \t");
      if (!p)
      {
         printf (CLEAR); printf (SHARING_ERR);
         fclose (hInput);
         fclose (hOutput);
         remove ("TEMP.$$-");
         return;
      }

      if (!stristr (p, Files[nCurFile].szName))
      {
         printf (CLEAR); printf (SHARING_ERR);
         fclose (hInput);
         fclose (hOutput);
         remove ("TEMP.$$-");
         return;
      }

      /* Write out the filename. */
      strtrim (p);
      fprintf (hOutput, "%s  ", p);

      /*
      ** Now, let's get the comment.  For the first line, just get
      ** the rest of the filename line.  Then, while the first 5 chars
      ** of the line are spaces, and there is text on it, add it to
      ** the comment.
      */

      p = strtok (NULL, "\r\n");
      if (!p)
         strcpy (szComment, "(Description not available)");
      else
         strcpy (szComment, p);

      /* Write out the comment. */
      fprintf (hOutput, "%s\n", szComment);

      nCurFile++;
   }
   fclose (hInput);
   fclose (hOutput);
   if (remove (pszFileListing) == -1)
   {
      printf (CLEAR); printf (REMOVE_ERR);
      return;
   }
   if (file_copy ("TEMP.$$-", pszFileListing) == Error_)
   {
      printf (CLEAR); printf (RENAME_ERR);
      return;
   }
   remove ("TEMP.$$-");
   printf (CLEAR); printf (SUCCESSFUL);
}
