/***************************************************************************/
/* Maximus 3.x FILEAREA.CTL -> FileFind 1.0 Configuration File             */
/* Version 0.99B                                                           */
/* Maximus 3.x support module.															*/
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
FILE *hMax3_FileAreas [10]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
int nCurLevel=0;
void trim (char *pStr);

/*
** Local Prototypes
*/
int Max3_Open_Area (char *pszFile);
int Max3_Close_Area (void);
int Max3_Get_Next_File (FileAreaInfo *pFInfo);

/****************************************************************************
 Function : int Max3_Open_Area (char *pszFile)
 Arguments: char *pszFile     Filename of source configuration file
 Return   : 0 if an error occurred, 1 otherwise.

 Descript:
	(Maximus 3.x)
		Opens a Max 3.x FILEAREA.CTL-style file for reading.
 ***************************************************************************/
int Max3_Open_Area (char *pszFile)
{
	trim (pszFile);
   printf ("\r%s\n", pszFile);

	if (hMax3_FileAreas [nCurLevel])
		nCurLevel++;

	hMax3_FileAreas[nCurLevel] = _fsopen (pszFile, "r", SH_DENYWR);
	if (!hMax3_FileAreas[nCurLevel])
		return 0;

	return 1;
}

/****************************************************************************
 Function : int Max3_Close_Area (void)
 Arguments:
 Return   : 1 if the area closed OK, 0 if it wasn't even open.

 Descript:
	(Maximus 3.x)
		Closes a Max 3.x FILEAREA.CTL-style file handle.
 ***************************************************************************/
int Max3_Close_Area (void)
{

	if (!hMax3_FileAreas[nCurLevel])
		return 0;
	fclose (hMax3_FileAreas[nCurLevel]);
	hMax3_FileAreas[nCurLevel]=NULL;
	if (nCurLevel)
		nCurLevel--;

	return 1;
}

/****************************************************************************
 Function : int Max3_Get_Next_File (FileAreaInfo *pFInfo);
 Arguments: FileAreaInfo...   Pointer to a file area info structure to be
										completed by this function.
 Return   : 0 if an error occurred, 1 if the structure has been filled out
				successfully.

 Descript:
	(Maximus 3.x)
		Reads the next area from FILEAREA.CTL, if possible.
 ***************************************************************************/
int Max3_Get_Next_File (FileAreaInfo *pFInfo)
{
char szLine [256];
unsigned long lLine=0L;
char *p;
int nInFileArea=0;

	if (!hMax3_FileAreas[nCurLevel])
		return 0;

	while (fgets (szLine, 256, hMax3_FileAreas[nCurLevel]) != NULL)
	{
		lLine++;
		if (*szLine == '\r' || *szLine == '\n')
			continue;

		p = strtok (szLine, " \t");
		if (!p)
			continue;

		if (!strcmpi (p, "Include"))
		{
			if (nCurLevel==8)
			{
				printf ("You are only allowed to include up to 8 levels of files!\n");
				continue;
			}
			else
			{
				p=strtok (NULL," \r\n\t");
				if (!p)
				{
					printf ("Syntax error on line %ld: invalid include file\n", lLine);
					return 0;
				}

				if (!Max3_Open_Area (p))
				{
					printf ("Error opening include file: %s\n", p);
					return 0;
				}
				else
				{
					if (!Max3_Get_Next_File (pFInfo))
					{
						Max3_Close_Area();
						continue;
					}
					else
						return 1;
				}
			}
		}

		if (!strcmpi (p, "FileArea"))
		{
			nInFileArea=1;
			*pFInfo->szTitle='\0';
			*pFInfo->szPath='\0';
			*pFInfo->szFileList='\0';
			continue;
		}

		if (!strcmpi (p, "Desc") && nInFileArea)
		{
			p=strtok (NULL, "\r\n");
			if (!p)
			{
				printf ("Syntax error on line %ld: invalid description\n", lLine);
				return 0;
			}
			strcpy (pFInfo->szTitle, p);
			continue;
		}

		if (!strcmpi (p, "Download") && nInFileArea)
		{
			p=strtok (NULL, " \t\r\n");
			if (!p)
			{
				printf ("Syntax error on line %ld: invalid path\n", lLine);
				return 0;
			}
			strcpy (pFInfo->szPath, p);
			if (!*pFInfo->szFileList)
				sprintf (pFInfo->szFileList, "%s\\FILES.BBS", pFInfo->szPath);

			continue;
		}

		if (!strcmpi (p, "Filelist") && nInFileArea)
		{
			p=strtok (NULL, " \r\n\t");
			if (!p)
			{
				printf ("Syntax error on line %ld: invalid filelist\n", lLine);
				return 0;
			}

			strcpy (pFInfo->szFileList, p);
			continue;
		}

		if (!strcmpi (p, "End") && nInFileArea)
		{
			if (!*pFInfo->szTitle || !*pFInfo->szFileList ||
				 !*pFInfo->szPath)
			{
				printf ("Area around line %ld is missing some info!\n", lLine);
				return 0;
			}

			return 1;
		}
	}

	if (nCurLevel)
	{
		Max3_Close_Area();
		return Max3_Get_Next_File (pFInfo);
	}

	return 0;
}
