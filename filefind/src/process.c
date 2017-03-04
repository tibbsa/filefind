//#define NO_RECEIVED
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

char szFullFilename [75];
struct _minf                  miMsgInfo;
HAREA                         hInputArea;
HMSG                          mhInputMsg;
XMSG                          xmInputHdr;
XMSG                          xmOutputHdr;
char                          szControlBuf [4096U];
char                          szBody [16384U];
Search_Key                    Keys [36];
unsigned long                 lAreaFiles;
unsigned long                 lAreaBytes;
unsigned long                 lTotalFiles=0L;
unsigned long                 lTotalBytes=0L;
char szComment [8192U];
char szFinished_Comment [8192U];
char szFormatted_Comment [8192U];
int nCurrentMsgArea=0;

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
unsigned long lCurMsg=1L;
unsigned long lNumMsg;

   /* Repeat the code until we've done all of the message areas. */
   while (Config.Areas [nCurrentMsgArea] != NULL)
   {
   char *p;
      lCurMsg=1L;
      lNumMsg=0L;

      /*********************************************************************/
      /* INITIALIZE                                                        */
      /*********************************************************************/

      memset (&miMsgInfo, '\0', sizeof miMsgInfo);
      printf ("Area %s\n", Config.Areas [nCurrentMsgArea]->szTitle);

      /* Initialize the MsgAPI */
      miMsgInfo.req_version = MSGAPI_VERSION;
      miMsgInfo.def_zone = Config.Areas [nCurrentMsgArea]->AKA.zone;

      /* If we can't init the API, go on to the next area. */
      if (MsgOpenApi (&miMsgInfo) == -1)
      {
         Log_Write ("!MsgAPI initialization failed");
         printf ("Error: MsgAPI initialization error!\n");
         continue;
      }

      /*
      ** Open the message area.  First, check to see if it's a Squish-style
      ** base.  Then, if so, set the '$' to a <space>, and trim it off.
      */
      p = Config.Areas [nCurrentMsgArea]->szPath;
      if (*p != '$')
      {
         hInputArea = MsgOpenArea (p, MSGAREA_NORMAL, MSGTYPE_SDM|MSGTYPE_ECHO);
      }
      else
      {
         /* Open a Squish style message area. */
         p++;
         hInputArea = MsgOpenArea (p, MSGAREA_NORMAL, MSGTYPE_SQUISH);
      }

      if (!hInputArea)
      {
         Log_Write ("!Error opening message base \"%s\"!\n", p);
         printf ("Error opening message base \"%s\"!\n", p);
         goto Deinit;
      }

      /*********************************************************************/
      /* SEARCH                                                            */
      /*********************************************************************/
      /*
      ** Continue looping until we find something addressed to
      ** Config.szSearchName.
      */
      lNumMsg = MsgGetNumMsg (hInputArea);
      mhInputMsg = MsgOpenMsg (hInputArea, MOPEN_RW, lCurMsg);
      if (!mhInputMsg)
      {
         if (msgapierr != 5)
         {
            Log_Write ("!Error %u opening message #%ld", msgapierr, lCurMsg);
            printf ("Error opening msg #%ld!\n", lCurMsg);
         }
         else
         {
            printf ("Area empty!\n");
            goto CloseMsgArea;
         }
      }

      while (lCurMsg <= lNumMsg)
      {
         if (mhInputMsg)
         {
            /* Read the message. */
            nResult = MsgReadMsg (mhInputMsg, &xmInputHdr, 0L, 16384L, (unsigned char *)szBody, 4096L, (unsigned char *)szControlBuf);
            if (nResult == -1)
            {
               Log_Write ("!Error reading message #%ld!", lCurMsg);
               printf ("Internal error reading msg #%ld: ", lCurMsg);
               if (msgapierr == MERR_BADH)
                  printf ("Invalid handle @ %p?\n", mhInputMsg);
               else if (msgapierr == MERR_BADF)
                  printf ("Invalid function requested @ %p?\n", mhInputMsg);
               else if (msgapierr == MERR_NOMEM)
                  printf ("Not enough memory!\n");
   
               continue;
            }
            strtrim (xmInputHdr.to);
            strtrim (Config.szSearchName);
            if (!strcmpi (Config.szSearchName, xmInputHdr.to))
            {
               Process_Message();
               /* OK, update the header. */
               xmInputHdr.attr |= MSGREAD;
#ifndef NO_RECEIVED
               nResult = MsgWriteMsg (mhInputMsg, 0, &xmInputHdr, NULL, 0L, 0L, 0L, NULL);
#endif
            }
            /* Close the message. */
            MsgCloseMsg (mhInputMsg);
         }

         lCurMsg++;
         mhInputMsg = MsgOpenMsg (hInputArea, MOPEN_RW, lCurMsg);
         if (!mhInputMsg)
         {
            if (msgapierr != 5)
            {
               Log_Write ("!Error %u opening message #%ld", msgapierr, lCurMsg);
               printf ("Error opening msg #%ld!\n", lCurMsg);
            }
         }
      }

CloseMsgArea:
      /*********************************************************************/
      /* CLOSE MESSAGE AREA                                                */
      /*********************************************************************/
      /* Close the message area. */
      nResult = MsgCloseArea (hInputArea);
      if (nResult == -1)
      {
         Log_Write ("!Possible error %u closing message area", msgapierr);
         printf ("Internal error in MsgCloseArea(): ");
         if (msgapierr == MERR_BADH)
            printf ("Invalid area @ %p?\n", hInputArea);
         else if (msgapierr == MERR_EOPEN)
            printf ("Messages are still open @ %p?\n", hInputArea);
         else
            printf ("Unknown error %u @ %p\n", msgapierr, hInputArea);
      }

Deinit:
      /*********************************************************************/
      /* DEINITIALIZE                                                      */
      /*********************************************************************/
      /* Deinitialize the MsgAPI. */
      if (MsgCloseApi() == -1)
      {
         Log_Write ("?MsgAPI may not have deinitialized properly!");
         printf ("Warning: MsgAPI may not have deinitialized properly!\n");
         continue;
      }
      
      /* Go on to the next list. */
      nCurrentMsgArea++;
   }

   /* All is OK. */
   return 0;
}

/***************************************************************************
 Function : void Process_Message (void) 
 Arguments: none
 Return   : none

 Descript.:
   This is only called when a message addressed to "Config.szSearchName"
   is found.  It should search the file listings, etc.
 ***************************************************************************/
void Process_Message (void)
{
unsigned int n=0;
char *Reply_Body;
int nFirstThisArea=1;
int nDidSomething =0;

   if (xmInputHdr.attr & MSGREAD)
      return;

   Reply_Body = (char *)malloc (32000U);
   if (!Reply_Body)
   {
      printf ("\aRequest from %s ignored; not enough memory!\n", xmInputHdr.from);
      return;
   }
   *Reply_Body = '\0';                 /* Start with a NUL string. */

   memset (&Keys, '\0', sizeof Keys);

   printf ("Request from %-36.36s  (%d:%d/%d", xmInputHdr.from,
                                               xmInputHdr.orig.zone,
                                               xmInputHdr.orig.net,
                                               xmInputHdr.orig.node);
   Log_Write ("#Request from %s (%d:%d/%d.%d)", xmInputHdr.from,
                                                xmInputHdr.orig.zone,
                                                xmInputHdr.orig.net,
                                                xmInputHdr.orig.node,
                                                xmInputHdr.orig.point);

   if (xmInputHdr.orig.point)
      printf (".%d)\n", xmInputHdr.orig.point);
   else
      printf (")\n");
   printf ("\tRequest: %s\n", xmInputHdr.subj);
   Log_Write (":  Request: %s", xmInputHdr.subj);

   /* Add the 'header' file to the message body. */
   Import_Header (Reply_Body, nCurrentMsgArea);
   /* Continue looping for each file area. */
   while (Config.FAreas [n] != NULL)
   {
   FILE *hFileList;
   char szLine [2048];


      lAreaBytes=lAreaFiles=0L;
      nFirstThisArea=1;
      printf ("\t\tScanning %-50.50s\r", Config.FAreas [n]->szTitle);
      hFileList = fopen (Config.FAreas [n]->szFileList, "r");
      if (!hFileList)
      {
         Log_Write ("!  Area \"%s\" file list not found!", Config.FAreas[n]->szTitle, Config.FAreas [n]->szFileList);
         printf ("\t\t\"%s\" has no file list!\n", Config.FAreas[n]->szTitle, Config.FAreas [n]->szFileList);
         n++;
         continue;
      }

      /*********************************************************************/
      /* Process each line.                                                */
      /*********************************************************************/
      while (fgets (szLine, 2048, hFileList) != NULL)
      {
      char szFilename [MAXFILE];
      char szExtension [MAXEXT];
      char szFile [MAXFILE+MAXEXT+10];
      char szMisc [120];
      int flags=0;
      char *p;
      char *z;
Do:

         strtrim (szLine);
         if (*szLine == '\0')
            continue;

         if (*szLine == ' ')
            continue;

         /* First, let's extract the filename. */
         p = strtok (szLine, " \t");
         if (!p)
            continue;
         strcpy (szFullFilename, p);
         flags = fnsplit (szFullFilename, NULL, NULL, szFilename, szExtension);
         if (!(flags & FILENAME))
            continue;
         if (flags & EXTENSION)
            sprintf (szFile, "%s%s", szFilename, szExtension);
         else
            sprintf (szFile, "%s", szFilename);

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
         strcat (szComment, "\r");         

         /*
         ** Extract the keywords into a easy to parse structure...
         */
         Extract_Keywords (xmInputHdr.subj);

         /*
         ** Did it barf?
         */
         if (Keys[0].szKey[0] == '\0')
         {
            fclose (hFileList);
            free (Reply_Body);
            return;
         }

         /*
         ** No, so check the filename for keywords.
         */
         if (Matched (szFile) || Matched (szComment))
         {
            Log_Write ("-  File found in %s: %s", Config.FAreas [n]->szTitle, szFile);
            printf ("\t\t\tFile matched: %s\n", szFile);
            if (nFirstThisArea)
            {
               Area_Header (Reply_Body, n);
               nFirstThisArea=0;
            }
            Add_File (Reply_Body, szFile, szComment, n);
            nDidSomething = 1;
         }
         if (strlen (Reply_Body) > 25000U)
         {
         HMSG hOutputMsg;
         char szTemp [120];
   
            Log_Write ("!  Message getting too big; split");
   
            printf ("\n\t\t*** Split message\n");

            /* Don't do any statistics stuff. */

            strcat (Reply_Body, "\r\r... Continued on next message ...\r\r");
      
            Import_Footer (Reply_Body, nCurrentMsgArea);
            /* Add tear / origin lines. */
            sprintf (szTemp, "--- FileFind %s\r * Origin: %s (%d:%d/%d", VER, Config.Areas [nCurrentMsgArea]->szOrigin,
                                                                              Config.Areas [nCurrentMsgArea]->AKA.zone,
                                                                              Config.Areas [nCurrentMsgArea]->AKA.net,
                                                                              Config.Areas [nCurrentMsgArea]->AKA.node);
            strcat (Reply_Body, szTemp);
            if (Config.Areas [nCurrentMsgArea]->AKA.point)
               sprintf (szTemp, ".%d)\r\r", Config.Areas[nCurrentMsgArea]->AKA.point);
            else
               sprintf (szTemp, ")\r\r");
   
            strcat (Reply_Body, szTemp);
      
            /* Create the control information. */
            *szControlBuf = '\0';
            Build_Msg_Control_Data (szControlBuf, nCurrentMsgArea);
      
            hOutputMsg = MsgOpenMsg (hInputArea, MOPEN_CREATE, 0);
            if (!hOutputMsg)
            {
               Log_Write ("!Unable to write reply (err %d)\n", msgapierr);
               printf ("ERROR: Unable to write reply! (err=%d)\n", msgapierr);
            }
            else
            {
               MsgWriteMsg (hOutputMsg, 0, &xmOutputHdr, Reply_Body, strlen (Reply_Body), strlen (Reply_Body), strlen (szControlBuf), szControlBuf);
               MsgCloseMsg (hOutputMsg);
            }
   
            *Reply_Body = '\0';
            /* Add the 'header' file to the message body. */
            Import_Header (Reply_Body, nCurrentMsgArea);
            /* Add a small header. */
            strcat (Reply_Body, "\r... Continuing from previous message\r\r");
            Area_Header (Reply_Body, n);
         }
      }

/*
   /SOUND and /32Bit or /SOUNDBLASTER
   Has "sound" & "32bit", or just "soundblaster"
*/

      /* OK, add the stats. */
      Add_Stats (Reply_Body, 0);
      fclose (hFileList);
      n++;
   }
   /*
   ** If we did something (i.e. found a match), import the footer, and
   ** write a reply.
   */
   if (nDidSomething)
   {
   HMSG hOutputMsg;
   char szTemp [120];

      /* OK, add the stats. */
      Add_Stats (Reply_Body, 1);

      Import_Footer (Reply_Body, nCurrentMsgArea);
      /* Add tear / origin lines. */
      sprintf (szTemp, "--- FileFind %s\r * Origin: %s (%d:%d/%d", VER, Config.Areas [nCurrentMsgArea]->szOrigin,
                                                                        Config.Areas [nCurrentMsgArea]->AKA.zone,
                                                                        Config.Areas [nCurrentMsgArea]->AKA.net,
                                                                        Config.Areas [nCurrentMsgArea]->AKA.node);
      strcat (Reply_Body, szTemp);
      if (Config.Areas [nCurrentMsgArea]->AKA.point)
         sprintf (szTemp, ".%d)\r\r", Config.Areas[nCurrentMsgArea]->AKA.point);
      else
         sprintf (szTemp, ")\r\r");
      strcat (Reply_Body, szTemp);


      /* Create the control information. */
      *szControlBuf = '\0';
      Build_Msg_Control_Data (szControlBuf, nCurrentMsgArea);

      hOutputMsg = MsgOpenMsg (hInputArea, MOPEN_CREATE, 0);
      if (!hOutputMsg)
      {
         Log_Write ("!Unable to write reply (err %d)\n", msgapierr);
         printf ("ERROR: Unable to write reply! (err=%d)\n", msgapierr);
      }
      else
      {
         MsgWriteMsg (hOutputMsg, 0, &xmOutputHdr, Reply_Body, strlen (Reply_Body), strlen (Reply_Body), strlen (szControlBuf), szControlBuf);
         MsgCloseMsg (hOutputMsg);
      }
   }
}

/***************************************************************************
 Function : void Extract_Keywords (char *pszString)
 Arguments: char *pszString      Extracts keywords from "pszString"
 Return   : none

 Descript.:
   Extracts all of the keywords from pszString, and places them into the
   SearchKeys structure:
      char     szKey [60];                /* Phrase to search for */
      char     Next_AND;                  /* "AND" with the next one? */
      char     Next_OR;                   /* "OR" with the next one? */
 ***************************************************************************/
void Extract_Keywords (char *pszString)
{
char *p;
int CurKey=0;
int GMatch=0;
int x;
char Buf[75];
   strcpy (Buf, pszString);

   memset (&Keys, '\0', sizeof Keys);

   p = Buf;
   if (*p != '/')
   {
      *Keys[0].szKey = '\0';
      Log_Write ("?  Invalid search request: Doesn't begin with keyword?");
      printf ("\t\t\tInvalid search request; ignored\n");
      return;
   }
   p = strtok (Buf, " ");

   while (p)
   {
      if (*p != '/' && !GMatch)
      {
         Log_Write ("?  Invalid search request: Keyword missing?");
         printf ("\t\t\tInvalid search request; ignored\n");
         memset (&Keys, '\0', sizeof Keys);
         return;
      }
      if (*p == '/')
      {
         p++;
         if (!*p)
         {
            Log_Write ("?  Invalid search request: Keyword terminated illegally");
            printf ("\t\t\tInvalid search request; ignored\n");
            memset (&Keys, '\0', sizeof Keys);
            return;
         }
         if (*p != '\"')
         {
            x=0;
            while (*p != ' ' && *p)
            {
               Keys[CurKey].szKey[x] = *p;
               x++;
               p++;
            }
            Keys[CurKey].szKey[x] = '\0';
         }
         else
         {
            p++;
            x=0;
            while (*p != '\"' && *p)
            {
               Keys[CurKey].szKey[x] = *p;
               x++;
               p++;
            }
            Keys[CurKey].szKey[x] = '\0';
         }
         GMatch+=1;
         CurKey++;
         Keys[CurKey].szKey[0] = '\0';
         p = strtok (NULL, " ");
         continue;
      }
      if ((strcmpi(p, "and") == 0) && GMatch)
      {
         if (CurKey == 0)
         {
            GMatch=0;
            continue;
         }

         Keys[CurKey-1].Next_AND=1;
         p = strtok (NULL, " ");
         GMatch=0;
         continue;
      }

      p = strtok (NULL, " ");
      GMatch=0;
   }
}

/***************************************************************************
 Function : int Matched (char *pszTemplate)
 Arguments: char *pszTemplate    Body of text to search for      
 Return   : 1=found, 0=not found

 Descript.:
   Checks to see if any of the keywords match in pszTemplate.
 ***************************************************************************/
int Matched (char *pszTemplate)
{
int CurKey=0;
int HadMatch=0;

   if (Keys[0].szKey[0] == '\0')
   {
      Log_Write ("!Internal error: Keys[0]->szKeys[0] is blank?");
      printf ("\n\n\n\aInternal error: Keys[0].szKeys[0] is blank?\n");
      abort();
   }

   while (Keys[CurKey].szKey [0] != '\0')
   {
   int anded;
      anded=0;
      if (!stristr (pszTemplate, Keys[CurKey].szKey))
      {
         HadMatch=0;
         while (Keys[CurKey].Next_AND)
            CurKey++;

         CurKey++;
         continue;
      }

      HadMatch=3;

      while (Keys[CurKey].Next_AND && HadMatch)
      {
         anded=1;
         if (!*Keys[CurKey].szKey)
         {
            HadMatch = 0;
            break;
         }
         CurKey++;
         if (!stristr (pszTemplate, Keys[CurKey].szKey))
         {
         int xyz;
            /* See if the rest of the line is "and", or if there is an OR */
            xyz=CurKey;
            while (*Keys[xyz].szKey != '\0')
            {
               if (Keys[xyz].Next_AND == 0)
               {
                  if (*Keys[xyz+1].szKey != '\0')
                  {
                     xyz++;
                     CurKey=xyz;
                     anded=125;
                     break;
                  }
                  else
                  {
                     HadMatch = 0;
                     return 0;
                  }
               }
               xyz++;
            }
            if (anded == 125)
               break;

            HadMatch=0;
            return 0;
         }
      }
      if (anded == 125)
      {
         anded=0;
         continue;
      }

      if (anded)
      {
         return 1;
      }

      if (!HadMatch)
      {
         CurKey++;
         continue;
      }

      HadMatch=1;
      CurKey++;
   }
   return HadMatch;
}

/***************************************************************************
 Function : void Area_Header (char *pszBody, int nArea)
 Arguments: char *pszBody        Text of message buffer to fill
            int nArea            Area # to get information from.
 Return   : none

 Descript.:
   Adds an area header to the end of the message.  (e.g.

=========================================================================
Area: (TCN) Nodelists
=========================================================================

 ***************************************************************************/
void Area_Header (char *pszBody, int nArea)
{
   /* Add the first divider line. */
   strcat (pszBody, "\r==============================================================================\r");

   /* Add the area title. */
   strcat (pszBody, "Area: ");
   if (Config.FAreas[nArea]->szTitle == '\0')
      strcat (pszBody, "(no description available)");
   else
      strcat (pszBody, Config.FAreas[nArea]->szTitle);

   strcat (pszBody, "\r");

   /* Add the second divider line. */
   strcat (pszBody, "==============================================================================\r");
}

/***************************************************************************
 Function : int Import_Header (char *pszBody, int nArea)
 Arguments: char *pszBody        Text of message buffer to fill
            int nArea            MsgArea # to get information from.
 Return   : 1=OK, 0=Error [file not there]

 Descript.:
   Imports the message area's "header" file (Config.Areas[nArea]->szHeader)
   into the top of the message.
 ***************************************************************************/
int Import_Header (char *pszBody, int nArea)
{
FILE *hFile;
char szLine [256];
char szTemp [256];

   *pszBody = '\0';

   if (*Config.Areas[nArea]->szHeader == '\0')
      return 1;

   hFile = fopen (Config.Areas[nArea]->szHeader, "r");
   if (!hFile)
   {
      Log_Write ("!WARNING: Header \"%s\" missing!", Config.Areas[nArea]->szHeader);
      printf ("WARNING: Header \"%s\" missing!\n", Config.Areas[nArea]->szHeader);
      return 1;
   }

   while (!feof (hFile))
   {
   int i;

      *szLine = '\0';
      fgets (szLine, 256, hFile);
      for (i = strlen (szLine) -1; (szLine[i] == '\r' || szLine[i] == '\n') && i >= 0; i--);
      szLine[i+1] = '\0';
      strcat (szLine, "\r");
      strcat (pszBody, szLine);
   }

   fclose (hFile);
   return 1;
}

/***************************************************************************
 Function : int Import_Footer (char *pszBody, int nArea)
 Arguments: char *pszBody        Text of message buffer to fill
            int nArea            MsgArea # to get information from.
 Return   : 1=OK, 0=Error [file not there]

 Descript.:
   Imports the message area's "footer" file (Config.Areas[nArea]->szFooter)
   into the bottom of the message.
 ***************************************************************************/
int Import_Footer (char *pszBody, int nArea)
{
FILE *hFile;
char szLine [256];
char szTemp [256];

   if (*Config.Areas[nArea]->szFooter == '\0')
      return 1;

   hFile = fopen (Config.Areas[nArea]->szFooter, "r");
   if (!hFile)
   {
      Log_Write ("!WARNING: Footer \"%s\" missing!", Config.Areas[nArea]->szHeader);
      printf ("WARNING: Footer \"%s\" missing!\n", Config.Areas[nArea]->szHeader);
      return 1;
   }

   while (!feof (hFile))
   {
   int i;

      *szLine = '\0';
      fgets (szLine, 256, hFile);
      for (i = strlen (szLine) -1; (szLine[i] == '\r' || szLine[i] == '\n') && i >= 0; i--);
      szLine[i+1] = '\0';
      strcat (szLine, "\r");
      strcat (pszBody, szLine);
   }

   fclose (hFile);
   return 1;
}

/***************************************************************************
 Function : void Add_File (char *pszBody, char *pszFilename, char *pszComment,
                           int nArea)
 Arguments: char *pszBody        Text of message buffer to fill
            char *pszFilename    Filename to use
            char *pszComment     Comment/description to use
            int nArea            Current filearea
 Return   : none

 Descript.:
   Adds a file to the response message, including the file size (kilobytes)
   filename, and description. e.g.

OPENDOS2.ZIP 1548k  This is Caldera's OpenDOS package, a free operating
                    system for use ...
 ***************************************************************************/
void Add_File (char *pszBody, char *pszFilename, char *pszComment, int nArea)
{
char *p;
char szTemp [128];
int n;
struct ffblk ffblk;
int i;
unsigned long lSize;
char *q;

   memset (&szTemp, '\0', sizeof szTemp);
   n=0;
   lSize=0L;
   i=0;
   memset (&szFormatted_Comment, '\0', sizeof szFormatted_Comment);
   memset (&szFinished_Comment, '\0', sizeof szFinished_Comment);

   if (!(fnsplit (szFullFilename, NULL, NULL, NULL, NULL) & DIRECTORY))
   {
      sprintf (szTemp, "%s\\%s", Config.FAreas[nArea]->szPath, pszFilename);
      strcpy (szFullFilename, szTemp);
   }

   /* Firstly...  Let's make sure the file exists. */
   i = findfirst (szFullFilename, &ffblk, 0);
   if (i != 0)                      /* File doesn't exist? */
   {
      Log_Write ("!     File %s listed in FILES.BBS but not on disk!", szFullFilename);
      lSize=0L;
   }
   else
      lSize = ffblk.ff_fsize;

   /*
   ** Now, add the filename / file size to the body. Get the commas, and then
   ** format it so that it comes out as "10k", with a width of 10.
   */
   if (lSize)
   {
      commafmt (szTemp, 128, lSize);
      sprintf (szFullFilename, "%s", szTemp);
      sprintf (szTemp, "%-11.11s", szFullFilename);
   }
   else
      sprintf (szTemp, "%-11.11s", "(missing?)");

   lAreaFiles++;
   lAreaBytes += lSize;
   lTotalFiles++;
   lTotalBytes += lSize;

   sprintf (szFullFilename, "%-12.12s  %-11.11s  ", pszFilename, szTemp);

   /* Now, the fun part.  We have to make the comment. */
   strcpy (szFormatted_Comment, word_wrap (pszComment, 46));
   /*
   ** It's word wrapped.  Now we have to add the spaces to it.
   */

   n=255;
   p=szFormatted_Comment;
   q=szFinished_Comment;
   while (*q)
      q++;

   /*
   ** OK, so, for the first one, we just want to copy up until '\0' or
   ** '\r', and the rest just until '\0' or until 47 chars reached.
   */
   while (*p)
   {
      if (n == 255)
      {
         while (*p)
         {
             if (*p == '\r' || *p == '\n')
             {
               n=1;
               *q = '\r';
               q++;
               p++;
               *q='\0';
               break;
            }
            *q = *p;
            p++;
            q++;
            *q='\0';
         }
      }
      else
      {
         /* Insert 27 spaces. */
         for (n=1; n <= 27; n++)
         {
            *q = ' ';
            q++;
         }
         *q = '\0';

         n=1;
         while (*p && n <= 47)
         {
            if (*p == '\r' || *p == '\n')
            {
               *q = '\r';
               q++;
               *q='\0';
               p++;
               break;
            }
            *q = *p;
            p++;
            q++;
            *q = '\0';
         }
      }
   }
   *q = '\0';

   strcat (pszBody, szFullFilename);
   strcat (pszBody, szFinished_Comment);
}

size_t commafmt(char   *buf,            /* Buffer for formatted string  */
                int     bufsize,        /* Size of buffer               */
                long    N)              /* Number to convert            */
{
      int len = 1, posn = 1, sign = 1;
      char *ptr = buf + bufsize - 1;

      if (2 > bufsize)
      {
ABORT:      *buf = '\0';
            return 0;
      }

      *ptr-- = '\0';
      --bufsize;
      if (0L > N)
      {
            sign = -1;
            N = -N;
      }

      for ( ; len <= bufsize; ++len, ++posn)
      {
            *ptr-- = (char)((N % 10L) + '0');
            if (0L == (N /= 10L))
                  break;
            if (0 == (posn % 3))
            {
                  *ptr-- = ',';
                  ++len;
            }
            if (len >= bufsize)
                  goto ABORT;
      }

      if (0 > sign)
      {
            if (len >= bufsize)
                  goto ABORT;
            *ptr-- = '-';
            ++len;
      }

      memmove(buf, ++ptr, len + 1);
      return (size_t)len;
}

/***************************************************************************
 Function : void Build_Msg_Control_Data (char *pszBuffer)
 Arguments: char *pszBuffer      Text of message buffer to fill
            int nArea            Current message area.
 Return   : none

 Descript.:
   Adds the necassary Fido control fields to a message. (MSGID)
 ***************************************************************************/
void Build_Msg_Control_Data (char *pszBuffer, int nArea)
{
unsigned long t;

   t=Make_MSGID();
   t |= crc32buf (szBody, strlen (szBody));

   sprintf (pszBuffer, "\01MSGID: %d:%d/%d.%d %-8lx\r", Config.Areas[nArea]->AKA.zone,
                                                      Config.Areas[nArea]->AKA.net,
                                                      Config.Areas[nArea]->AKA.node,
                                                      Config.Areas[nArea]->AKA.point,
                                                      t);

   strtrim (Config.szReplyFrom);

   strcpy (xmOutputHdr.to, xmInputHdr.from);
   strcpy (xmOutputHdr.from, Config.szReplyFrom);
   strcpy (xmOutputHdr.subj, xmInputHdr.subj);  

   xmOutputHdr.attr = MSGLOCAL;
   xmOutputHdr.attr = 0L;
   xmOutputHdr.date_written = *GetTimeStamp (time (NULL));
   xmOutputHdr.date_arrived = *GetTimeStamp (time (NULL));

   xmOutputHdr.orig.zone = Config.Areas[nArea]->AKA.zone;
   xmOutputHdr.orig.net = Config.Areas[nArea]->AKA.net;
   xmOutputHdr.orig.node = Config.Areas[nArea]->AKA.node;
   xmOutputHdr.orig.point= Config.Areas[nArea]->AKA.point;

   xmOutputHdr.dest.zone = Config.Areas[nArea]->AKA.zone;
   xmOutputHdr.dest.net = Config.Areas[nArea]->AKA.net;
   xmOutputHdr.dest.node = Config.Areas[nArea]->AKA.node;
   xmOutputHdr.dest.point= Config.Areas[nArea]->AKA.point;
}

static struct _stamp *GetTimeStamp (time_t tt)
{
    static struct _stamp st;
    struct tm *tms;

    tms = localtime(&tt);
    st.time.ss = tms->tm_sec >> 1;
    st.time.mm = tms->tm_min;
    st.time.hh = tms->tm_hour;
    st.date.da = tms->tm_mday;
    st.date.mo = tms->tm_mon + 1;
    st.date.yr = tms->tm_year - 80;
    return (&st);
}

/***************************************************************************
 Function : void Add_Stats (char *pszBuffer, int nType)
 Arguments: char *pszBuffer      Text of message buffer to fill
            int nType            Stats to add.  (0=area, 1=totals)
 Return   : none

 Descript.:
   Adds the "total files" statistics.
 ***************************************************************************/
void Add_Stats (char *pszBuffer, int nType)
{
char szOutput [128];
char szBytes [24];
char szFiles [24];

   if (!nType)             /* Area-by-area basis */
   {
      if (!lAreaFiles)
         return;

      commafmt (szBytes, 24, lAreaBytes);
      commafmt (szFiles, 24, lAreaFiles);
      
      strcat (pszBuffer, "==============================================================================\r");
      sprintf (szOutput, "Area Totals: %s byte%s in %s file%s\r", szBytes, lAreaBytes != 1L ? "s" : "", szFiles, lAreaFiles != 1L ? "s" : "");
      strcat (pszBuffer, szOutput);
   }
   else
   {
      if (!lTotalFiles)
         return;

      commafmt (szBytes, 24, lTotalBytes);
      commafmt (szFiles, 24, lTotalFiles);

      sprintf (szOutput, "\rTotal of %s byte%s in %s file%s listed.\r", szBytes, lTotalBytes != 1L ? "s" : "", szFiles, lTotalFiles != 1L ? "s" : "");
      strcat (pszBuffer, szOutput);
      lTotalBytes=lTotalFiles=0L;
   }
}
