/***************************************************************************/
/* AllFix-equivalent `FILEFIND' request responder                          */
/* Version 1                                                               */
/*                                                                         */
/* This is the main module for 'FILEFIND', and it basically controls the   */
/* execution of the program, and supplies a few commonly used functions.   */
/***************************************************************************/

#include <filefind.h>

/***************************************************************************/
/* Global Variables                                                        */
/***************************************************************************/
FF_Setup             Config;
char                 Include_Nesting_Level=0;
extern unsigned _stklen;
int nCurArea=0;
int nCurFArea=0;

void remove_backslash (char *pszString)
{
char *p;

   p=pszString;
   strtrim (p);
   while (*p)
      p++;
   p--;
   if (*p == '\\')
      *p='\0';
}

/***************************************************************************
 Function : int main (int argc, char **argv)
 Arguments: int argc          Number of command line arguments
            char **argv       Command line arguments data
 Return   : 0 = OK

 Descript.:
   This is the main part of the program.  Calls init/deinit/load config/
   write routines.
 ***************************************************************************/
int main (int argc, char **argv)
{
unsigned char Errorlevel=0;
int i;
   _stklen = 32000U;

   /* Display the program name/version/copy. */
   printf (PROG);
   printf (COPY);
   if (CanShare())
      printf ("SHARE detected!\n");
   
   /* Initialize variables. */
   Init_Vars();

   /* Parse the command line arguments. */
   if (argc == 2)
   {
      strcpy (Config.szFilename, argv[1]);
      printf ("Using alternate configuration file \"%s\"\n", Config.szFilename);
   }

   /* Read the configuration file. */
   if (!Configure())
   {
      Errorlevel = 1;
      goto Exit;
   }

   /* Initialize the logging system. */
   if (!Log_Open (Config.szLog, NULL, "FFND", 1))
      printf ("Warning: Log file (%s) could not be opened!\n", Config.szLog);
   else
   {
      Log_Write ("-");
      Log_Write ("=======================================================");
      Log_Write ("+FileFind %s starting up", VER);
   }

   Errorlevel = Do_It();

Exit:
   /* Deallocate configuration memory. */
   Log_Write ("+FileFind %s ending", VER);
   Log_Close();
   i=0;
   while (Config.Areas [i])
   {
      free (Config.Areas [i]);
      i++;
   }
   i=0;
   while (Config.FAreas [i])
   {
      free (Config.FAreas [i]);
      i++;
   }

   /* Return the correct code to the operating system. */
   return (Errorlevel);
}

/***************************************************************************
 Function : void Init_Vars (void)
 Arguments: none
 Return   : none  

 Descript.:
   Initializes all of the variables used in the program.                
 ***************************************************************************/
void Init_Vars (void)
{
   strcpy (Config.szFilename, "FILEFIND.CFG");
   strcpy (Config.szSearchName, "Allfix");
   strcpy (Config.szReplyFrom, "Allfix FileFind");
   *Config.szLog = '\0';
   memset (&Config.Areas, '\0', sizeof Config.Areas);
   memset (&Config.FAreas, '\0', sizeof Config.FAreas);
}

/***************************************************************************
 Function : int Configure (void)
 Arguments: none
 Return   : 1 = OK, 0 = Error

 Descript.:
   Reads the configuration file.
 ***************************************************************************/
int Configure (void)
{
FILE *hConfigure;
unsigned long lCurLine;
char *p;
char szInputLine [512];
unsigned char Err=0;
unsigned char InFAreaSetup=0;
unsigned char InAreaSetup=0;

   hConfigure = fopen (Config.szFilename, "r");
   if (!hConfigure)
   {
      printf ("Configuration file, %s, not found / cannot be opened!\n", Config.szFilename);
      return 0;
   }

   lCurLine = 0L;
   while (fgets (szInputLine, 512, hConfigure) != NULL && !Err)
   {
      lCurLine++;

      if (*szInputLine == '\r' || *szInputLine == '\n' ||
          *szInputLine == '\0')
         continue;

      p = strtok (szInputLine, " \t");
      if (!p)
         continue;

      /* Trim it off. */
      strtrim (p);

      if (!strcmpi(p, "Include"))
      {
      char s[150];
         if (Include_Nesting_Level == 5)
         {
            printf ("Include nesting level too high on line %ld of %s!\n", lCurLine, Config.szFilename);
            Err=1;
            continue;
         }
         Include_Nesting_Level++;

         p=strtok (NULL, " \r\n\t;");
         if (!p)
         {
            printf ("Include filename not specified on line %ld of %s!\n", lCurLine, Config.szFilename);
            Err=1;
            continue;
         }
         strtrim (p);
         strcpy (s, Config.szFilename);
         strcpy (Config.szFilename, p);

         if (!Configure())
         {
            strcpy (Config.szFilename, s);
            Err=1;
            continue;
         }
         strcpy (Config.szFilename, s);
         Include_Nesting_Level--;
         continue;
      }

      /*
      ** SearchName
      */
      if (!strcmpi(p, "Search_For"))
      {
         p = strtok (NULL, "\r\n");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }

         strcpy (Config.szSearchName, p);
         strtrim (Config.szSearchName);
         continue;
      }

      /*
      ** ReplyFrom
      */
      if (!strcmpi(p, "Reply_Name"))
      {
         p = strtok (NULL, "\r\n");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }

         strcpy (Config.szReplyFrom, p);
         continue;
      }

      if (!strcmpi(p, "Log"))
      {
         p = strtok (NULL, "\r\n");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }
         strtrim (p);
         strcpy (Config.szLog, p);
         continue;
      }

      /********************************************************************/
      /* MsgArea Configuration                                            */
      /********************************************************************/
      if (!strcmpi(p, "MsgArea"))
      {
         Config.Areas [nCurArea] = (FF_Area *)malloc (sizeof (FF_Area));
         if (!Config.Areas [nCurArea])
         {
            printf ("Out of memory!\n");
            Err = 1;
            continue;
         }
         memset (Config.Areas [nCurArea], '\0', sizeof (FF_Area));
         p = strtok (NULL, "\r\n");
         if (!p)
            *Config.Areas [nCurArea]->szTitle = '\0';
         else
            strcpy (Config.Areas [nCurArea]->szTitle, p);

         InAreaSetup=1;
         continue;
      }

      if (!strcmpi(p, "End") && InAreaSetup)
      {
         InAreaSetup=0;
         nCurArea++;
         continue;
      }

      if (InAreaSetup && !strcmpi(p, "Path"))
      {
         p = strtok (NULL, " \r\n\t;");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }
         
         if (*Config.Areas [nCurArea]->szPath != '\0')
         {
            printf ("Error in definition for area \"%s\" on line %ld of %s: Duplicate setting!\n", Config.Areas [nCurArea]->szTitle, lCurLine, Config.szFilename);
            Err=1;
            continue;
         }

         remove_backslash (p);

         strcpy (Config.Areas [nCurArea]->szPath, p);
         continue;
      }
      
      if (InAreaSetup && !strcmpi(p, "Header"))
      {
         p = strtok (NULL, " \r\n\t;");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }

         if (*Config.Areas [nCurArea]->szHeader != '\0')
         {
            printf ("Error in definition for area \"%s\" on line %ld of %s: Duplicate setting!\n", Config.Areas [nCurArea]->szTitle, lCurLine, Config.szFilename);
            Err=1;
            continue;
         }

         strcpy (Config.Areas [nCurArea]->szHeader, p);
         continue;
      }

      if (InAreaSetup && !strcmpi(p, "Footer"))
      {
         p = strtok (NULL, " \r\n\t;");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }

         if (*Config.Areas [nCurArea]->szFooter != '\0')
         {
            printf ("Error in definition for area \"%s\" on line %ld of %s: Duplicate setting!\n", Config.Areas [nCurArea]->szTitle, lCurLine, Config.szFilename);
            Err=1;
            continue;
         }

         strcpy (Config.Areas [nCurArea]->szFooter, p);
         continue;
      }

      if (InAreaSetup && !strcmpi(p, "Address"))
      {
         p = strtok (NULL, " \r\n\t;");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }


         if (Config.Areas [nCurArea]->AKA.zone != 0)
         {
            printf ("Error in definition for area \"%s\" on line %ld of %s: Duplicate setting!\n", Config.Areas [nCurArea]->szTitle, lCurLine, Config.szFilename);
            Err=1;
            continue;
         }

         Parse_Address (&Config.Areas [nCurArea]->AKA, p);
         if (Config.Areas [nCurArea]->AKA.zone == 0)
         {
            printf ("Error in definition for area \"%s\" on line %ld of %s: Invalid address!\n", Config.Areas [nCurArea]->szTitle, lCurLine, Config.szFilename);
            Err=1;
            continue;
         }

         continue;
      }

      if (InAreaSetup && !strcmpi(p, "Origin"))
      {
         p = strtok (NULL, "\r\n");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }

         strcpy (Config.Areas [nCurArea]->szOrigin, p);
         strtrim (Config.Areas [nCurArea]->szOrigin);
         continue;
      }

      /********************************************************************/
      /* FileArea Configuration                                           */
      /********************************************************************/
      if (!strcmpi(p, "FileArea"))
      {
         Config.FAreas [nCurFArea] = (FF_FArea *)malloc (sizeof (FF_FArea));
         if (!Config.FAreas [nCurFArea])
         {
            printf ("Out of memory!\n");
            Err = 1;
            continue;
         }
         memset (Config.FAreas [nCurFArea], '\0', sizeof (FF_FArea));
         p = strtok (NULL, "\r\n");
         if (!p)
            strcpy (Config.FAreas [nCurFArea]->szTitle, "(untitled)");
         else
            strcpy (Config.FAreas [nCurFArea]->szTitle, p);
         
         InFAreaSetup=1;
         continue;
      }

      if (!strcmpi(p, "End") && InFAreaSetup)
      {
         InFAreaSetup=0;
         nCurFArea++;
         continue;
      }

      if (InFAreaSetup && !strcmpi(p, "Path"))
      {
         p = strtok (NULL, " \r\n\t;");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }
         
         if (*Config.FAreas [nCurFArea]->szPath != '\0')
         {
            printf ("Error in definition for area \"%s\" on line %ld of %s: Duplicate setting!\n", Config.Areas [nCurArea]->szTitle, lCurLine, Config.szFilename);
            Err=1;
            continue;
         }

         remove_backslash (p);
         strcpy (Config.FAreas [nCurFArea]->szPath, p);
         sprintf (Config.FAreas [nCurFArea]->szFileList, "%s\\FILES.BBS", p);
         continue;
      }

      if (InFAreaSetup && !strcmpi(p, "Filelist"))
      {
         p = strtok (NULL, " \r\n\t;");
         if (!p)
         {
            printf ("Invalid setting on line %ld of %s: %s\n", lCurLine, Config.szFilename, szInputLine);
            Err=1;
            continue;
         }
         
         strcpy (Config.FAreas [nCurFArea]->szFileList, p);
         continue;
      }
   }

   fclose (hConfigure);

   if (Err)
      return 0;

   /* Check for "bad" things.. */
   if (!nCurArea)
   {
      printf ("ERROR: No message areas defined!\n");
      return 0;
   }

   if (!nCurFArea)
   {
      printf ("ERROR: No file areas defined!\n");
      return 0;
   }

   return 1;
}

/***************************************************************************
 Function : void Parse_Address (NETADDR *pAddr, char *pszStr)
 Arguments: NETADDR *pAddr       Pointer to NETADDR struct to fill out
            char *pszStr         Pointer to NUL terminated string to build
                                 NETADDR from.
 Return   : pAddr->zone = 0 if error

 Descript.:
   Converts a string (1:163/215.38) to a netaddr structure.
 ***************************************************************************/
void Parse_Address (NETADDR *pAddr, char *pszStr)
{
   if (sscanf (pszStr, "%u:%u/%u.%u", &pAddr->zone, &pAddr->net, &pAddr->node,
                                    &pAddr->point) < 3)
   {
      pAddr->zone = 0;
      return;
   }
   if (pAddr->zone > 4095 || pAddr->net > 32766 || pAddr->node == 32766 ||
       pAddr->point > 32766)
   {
      pAddr->zone = 0;
      return;
   }
}

/***************************************************************************
 Function : char *stristr (const char *String, const char *Pattern)
 Arguments: const char *String   String to search                          
            const char *Pattern  Pattern to search for
 Return   : pointer to start of Pattern in String, or NULL if Pattern isn't
            found in String

 Descript.:
   Case-insensitive "string in string" function.
 ***************************************************************************/
char *stristr(const char *String, const char *Pattern)
{
      char *pptr, *sptr, *start;
      uint  slen, plen;

      for (start = (char *)String,
           pptr  = (char *)Pattern,
           slen  = strlen(String),
           plen  = strlen(Pattern);

           /* while string length not shorter than pattern length */

           slen >= plen;

           start++, slen--)
      {
            /* find start of pattern in string */
            while (toupper(*start) != toupper(*Pattern))
            {
                  start++;
                  slen--;

                  /* if pattern longer than string */

                  if (slen < plen)
                        return(NULL);
            }

            sptr = start;
            pptr = (char *)Pattern;

            while (toupper(*sptr) == toupper(*pptr))
            {
                  sptr++;
                  pptr++;

                  /* if end of pattern then pattern was found */

                  if ('\0' == *pptr)
                        return (start);
            }
      }
      return(NULL);
}


/***************************************************************************
 Function : unsigned long Make_MSGID (void)
 Arguments: 
 Return   : MSGID

 Descript.:
   Creates a 32-bit MSGID from the current time.
 ***************************************************************************/
unsigned long Make_MSGID (void)
{
    static unsigned long old_id = 0;
    unsigned long i;
    time_t now;
    struct tm *t;

    now = time(NULL);
    t = localtime(&now);
    i = (t->tm_sec * 10L) +
        (t->tm_min * 600L) +
        (t->tm_hour * 36000L) +
        (t->tm_mday * 864000L) +
        (t->tm_mon * 26784000L) +
        (t->tm_year * 321408000L);

    if (i <= old_id)
        i = old_id + 1;

    return (old_id = i);
}
