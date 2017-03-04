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
extern unsigned _stklen;
char szConfigFile [120];
int nCurFArea=0;
int Include_Nesting_Level=0;

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
   strcpy (szConfigFile, "FILEFIND.CFG");

   /* Display the program name/version/copy. */
   printf (PROG);
   printf (COPY);
   if (CanShare())
      printf ("SHARE detected!\n");
   
   /* Initialize variables. */
   Init_Vars();

   if (argc == 2)
   {
      strcpy (szConfigFile, argv[1]);
      printf ("Using alternate configuration file \"%s\"\n", szConfigFile);
   }

   /* Read the configuration file. */
   if (!Configure())
   {
      Errorlevel = 1;
      goto Exit;
   }

   Errorlevel = Do_It();

Exit:
   /* Deallocate configuration memory. */
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

   hConfigure = fopen (szConfigFile, "r");
   if (!hConfigure)
   {
      printf ("Configuration file, %s, not found / cannot be opened!\n", szConfigFile);
      return 0;
   }
   lCurLine=0L;

   while (fgets (szInputLine, 512, hConfigure) != NULL && !Err)
   {
      lCurLine++;
      if (*szInputLine == '\r' || *szInputLine == '\n' ||
          *szInputLine == '\0')
         continue;

      /* Trim it off. */
      strtrim (szInputLine);
      p = strtok (szInputLine, " \t");
      if (!p)
         continue;

      if (!strcmpi(p, "Include"))
      {
      char s[125];

         if (Include_Nesting_Level == 5)
         {
            printf ("Include nesting level too high on line %ld of %s!\n", lCurLine, szConfigFile);
            Err=1;
            continue;
         }
         Include_Nesting_Level++;
         p=strtok (NULL, " \r\n\t;");
         if (!p)
         {
            printf ("Include filename not specified on line %ld of %s!\n", lCurLine, szConfigFile);
            Err=1;
            continue;
         }
         strtrim (p);
         strcpy (s, szConfigFile);
         strcpy (szConfigFile, p);

         if (!Configure())
         {
            strcpy (szConfigFile, s);
            Err=1;
            continue;
         }
         strcpy (szConfigFile, s);
         Include_Nesting_Level--;
         continue;
      }

      if (!strcmpi(p, "Filelist"))
      {
         p = strtok (NULL, "\r\n\t;");
         if (!p)
            continue;
         Config.FAreas [nCurFArea] = (char *)malloc (strlen (p) + 1);
         if (!Config.FAreas [nCurFArea])
         {
            printf ("Out of memory!");
            Err=1;
            continue;
         }

         strcpy (Config.FAreas [nCurFArea], p);
         nCurFArea++;
         continue;
      }
   }

   fclose (hConfigure);

   if (Err)
      return 0;

   /* Check for "bad" things.. */
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

