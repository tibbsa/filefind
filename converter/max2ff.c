/***************************************************************************/
/* Maximus 3.x FILEAREA.CTL -> FileFind 1.0 Configuration File             */
/* Version 0.99B                                                           */
/***************************************************************************/

#include "max2ff.h"

#include <dos.h>
#include <dir.h>
#include <share.h>
#include <stdio.h>
#include <string.h>

/*
** Global Data
*/
char           szFFConfigFile [120];                  /* Config filename.  */
char           szFileAreaCtl  [120];                  /* Filearea.ctl file */
char				szBBSType [24];								/* System type.		*/

int				(*area_open_func)(char *pszFile) = NULL;
int				(*area_close_func)(void) = NULL;
int				(*area_get_file)(FileAreaInfo *pFInfo) = NULL;

/*
** Macros for calling the "(*area_xxx" functions.
*/
#define  FileOpenArea(file)	area_open_func((file))
#define  FileCloseArea()		area_close_func()
#define	FileGetNextFile(x)	area_get_file(&x)

/*
** Local Prototypes
*/
int main (int argc, char *argv[]);
void Convert (void);
int Can_Write (void);
int Set_BBS_Functions (char *pszSystemName);
void trim (char *pStr)
{
char *p;
char *q;
char szOutput[600];
int isStart=1;

	p=pStr;
	q=szOutput;
	while (*p)
	{
		if (*p==' ')
		{
			if (!isStart)
			{
				*q = ' ';
				q++;
			}
			while (*p == ' ')
				p++;
		}
		if (*p=='\t')
		{
			if (!isStart)
			{
				*q = '\t';
				q++;
			}
			while (*p == '\t')
				p++;
		}

		*q=*p;
		p++;
		q++;
		isStart=0;
	}
	*q='\0';
	/* Make sure there are no spaces at the end... */
	while (*q==' '||*q=='\t')
		q--;
	q++;
	*q='\0';
	strcpy (pStr,szOutput);
}


/*
** Maximus 3.x support functions (MAX3.C)
*/
int Max3_Open_Area (char *pszFile);
int Max3_Close_Area (void);
int Max3_Get_Next_File (FileAreaInfo *pFInfo);

/*
** Maximus 2.x support functions (MAX2.C)
*/
int Max2_Open_Area (char *pszFile);
int Max2_Close_Area (void);
int Max2_Get_Next_File (FileAreaInfo *pFInfo);

/****************************************************************************
 Function : int main (int argc, char **argv)
 Arguments: int argc          Number of commandline arguments
				char **argv       Commandline argument data
 Return   : Always 0

 Descript:
 ***************************************************************************/
int main (int argc, char **argv)
{
   /*
	** Setup up the global data to defaults.
	*/
	strcpy (szFFConfigFile, "FILEFIND.CFG");
	strcpy (szFileAreaCtl, "FILEAREA.CTL");

	printf ("BBS File Area Configuration to FileFind Import Utility\n");
	printf ("Copyright (c) 1998 by Anthony Tibbs\n\n");

	/*
	** If the user specified something on the command line (i.e.the input /
	** output filenames), dosomething with the information.
	*/
	if (argc >= 2)
	{
	int nCurrentArg;

		for (nCurrentArg=1; nCurrentArg < argc; nCurrentArg++)
		{
		const char *pszHelpKeys[]={"-?","/?","-h","-H","/h","/H", NULL};
		int n=0;

			while (pszHelpKeys[n])
			{
				if (!strcmpi (argv [nCurrentArg], pszHelpKeys [n]))
				{
               printf ("Usage: CONVERT <ctlname> <filefind name> <system>\n");
					printf ("\n   <ctlname>\t\tPath/filename of BBS filearea configuration\n");
					printf ("   <filefind name>\tPath/filename of target FILEFIND config file\n");
					printf ("	<system>\tSpecifies the type of BBS software you are using:\n");
					printf ("\t\t\tMax2\t\tMaximus 2.x\n");
					printf ("\t\t\tMax3\t\tMaximus 3.x\n\n");
					return 0;
				}
				n++;
			}
		}
	}

	if (argc != 4)
	{
      printf ("Incorrect number of arguments. Run CONVERT /h for details.\n");
		return 0;
	}

	strcpy (szFFConfigFile, argv[2]);
	strcpy (szFileAreaCtl, argv[1]);
	if (!Set_BBS_Functions (argv[3]))
	{
      printf ("You specified an invalid type of BBS.  Please run CONVERT/H for a list\n");
		printf ("of valid (supported) BBS types.\n");
		return 0;
	}

	printf ("Converting %s to %s...\n", szFileAreaCtl, szFFConfigFile);
	Convert();

	return 0;
}

/****************************************************************************
 Function : void Convert (void)
 Arguments:
 Return   :

 Descript:
   Actually performs the conversion operation.  This process is quite simple
	to do, actually.

   LogiPlan:
   *  Read line
   *  If token 0 == "filearea"
   *     Read line
   *     If token 0 == "end"
	*        if !have_desc OR !have_download err
	*        if !have_filelist set filelist to "path\\files.bbs"
   *        write area
   *     return to big loop
   *     If token 0 == "desc", set filearea title to token 1
   *     If token 0 == "download", set filearea path to token 1
   *     If token 0 == "filelist", set filearea list to token 1
   *     Skipline
 ***************************************************************************/
void Convert (void)
{
FILE *hOutput;
FileAreaInfo fInfo;

	/* Check to see if the output file exists.  If so, prompt to overwrite. */
	if (!Can_Write())
		return;

	hOutput = _fsopen (szFFConfigFile, "w", SH_DENYWR);
	if (!hOutput)
	{
		printf ("ERROR: \aUnable to open %s!\n", szFFConfigFile);
		return;
	}
	/*
	** Open the FILEAREA.CTL file.
	*/
	if (!FileOpenArea (szFileAreaCtl))
	{
		printf ("ERROR: \aUnable to open %s!\n", szFileAreaCtl);
		return;
	}

	/*
	** Loop through the input file until there are no more areas to read.
	** Since everything is done through pointers & "drop-in support
	** functions", this routine should be very generic.
	*/
	while (FileGetNextFile (fInfo))
	{
		trim (fInfo.szTitle);
		trim (fInfo.szPath);
		trim (fInfo.szFileList);
      printf ("   %s\n", fInfo.szTitle);
		fprintf (hOutput, "FileArea %s\n", fInfo.szTitle);
		fprintf (hOutput, "\tPath\t\t%s\n", fInfo.szPath);
		fprintf (hOutput, "\tFileList\t%s\n", fInfo.szFileList);
		fprintf (hOutput, "End\n\n");
	}

	fclose (hOutput);
	FileCloseArea();
}

/****************************************************************************
 Function : int Can_Write (void)
 Arguments:
 Return   : 0 if we CAN'T overwrite, 1 if we can.

 Descript:
	Since the user may not want to overwrite their existing configuration
	file, we want to prompt to overwrite it if it already exists.
0 ***************************************************************************/
int Can_Write (void)
{
struct ffblk ffb;
int Done;

	Done=findfirst (szFFConfigFile, &ffb, 0);

	if (Done == 0)
	{
		printf ("Warning: FileFind configuration file (%s) already exists.\n",szFFConfigFile);
		printf ("         Do you want to overwrite it? [Y/N]");
		while (Done==0)
		{
		char ch;
			ch = getch();
			if (ch =='Y' || ch == 'y')
			{
				printf (" Yes!\n");
				return 1;
			}
			if (ch == 'N' || ch == 'n')
			{
				printf (" No!\n");
				return 0;
			}
		}
	}
	return 1;
}

/****************************************************************************
 Function : int Set_BBS_Functions (char *pszSystemName);
 Arguments: char *pszSystemName System type (max2, max3, etc.)
 Return   : 0 if we got an invalid system name, 1 otherwise.

 Descript:
	Initializes the "pointers to functions" listed at the top of this code
	file.  Uses the appropriate functions for the desired system.  This
	makes it >real< easy to implement new formats as required - just add
	another module, and away you go.
 ***************************************************************************/
int Set_BBS_Functions (char *pszSystemName)
{
	if (*pszSystemName == '\0' || !pszSystemName)
		return 0;

	/* Supported systems:  Maximus 3.x													*/
	printf ("Format: ");
	if ((strcmpi (pszSystemName, "Max3")) == 0)
	{
		printf ("Maximus 3.x FILEAREA.CTL\n");
		area_open_func = Max3_Open_Area;
		area_close_func = Max3_Close_Area;
		area_get_file = Max3_Get_Next_File;
		return 1;
	}
   if ((strcmpi (pszSystemName, "Max2")) == 0)
   {
      printf ("Maximus 2.x FILEAREA.CTL\n");
      area_open_func = Max2_Open_Area;
      area_close_func = Max2_Close_Area;
      area_get_file = Max2_Get_Next_File;
		return 1;
	}

	printf ("Unknown\n");
	return 0;
}

