#include <fstream.h>
#include <conio.h>
#include <string.h>
#include "source\door.h"
#include "source\intrnode.h"

short nMyNode = 0;
char szMyUsername[30];

#pragma argsused
void startup(int argc, char *argv[])
{
   strcpy(szMyUsername, "Local User");
   //--> fix

}


char inputKey()
{
   char c;

   if ( kbhit() )
      {
      textcolor(7);
      textbackground(0);
      c = getch();
      return (c);
      }

   else
      return 0;
}


// Displays a string for this instance/node of the door.
void local(char* szString, short nColor, short nNewLines, short nBackColor)
{
   short n;

   textcolor(nColor);
   textbackground(nBackColor);

   if ( szString != NULL )
      cprintf("%s", szString);

   for(n = 0; n < nNewLines; n++)
      {
      textcolor(7);
      textbackground(0);
      if (nColor != 7 ) 
         cprintf(" \r\n");
      else
         cprintf("\r\n");
      }
}


void newline()
{
   cprintf("\r\n");
}


void printChar(char cChar)
{
   putch(cChar);
}


void center(char* szText, short nColor, short nNewlines, short nBackColor)
{
   char szHolder[100];
   short nEmpty, n;

   if ( strlen(szText) >= 79 )
      local(szText, nColor, nNewlines, nBackColor);
   else
      {
      nEmpty = 40 - (short)(strlen(szText) / 2);  
      for (n = 0; n < nEmpty; n++)
         szHolder[n] = ' ';
      szHolder[nEmpty] = '\0';
      strcat(szHolder, szText);
      local(szHolder, nColor, nNewlines, nBackColor);
      }
}


#pragma argsused
void showAnsi(char *szFileName)
{
   char szBuffer[100];
   ifstream ifsMyFile(szFileName);

   while ( ifsMyFile )
      {
      ifsMyFile.getline(szBuffer, 81);
      local(szBuffer);
      }
}


void backspace(short nTimes)
{
   short n;
   char szStr[190];

   if ( nTimes > 60 )
      nTimes = 60;
      
   strcpy(szStr, "\b \b");
   
   for (n = 1; n < nTimes; n++)
      {
      strcat(szStr, "\b \b");
      }

   cprintf(szStr);
}

void checkCarrier()
{
}

void checkTimeLeft()
{
}

void pausePrompt(short nClear, short nCenter)
{
   char* szText = "[Hit any key to continue]";

   if ( nCenter != 1 )
      local(szText, LWHITE, 0);
   else
      center(szText, LWHITE, 0);

   getch();
   
   if (nClear == 1)
      clearScreen();
   else
      local(" ");
}


void clearScreen()
{
   clrscr();
}


bool hasAnsi()
{
   return false;
}


// Returns 1 if user is sysop, or 0 if not.  Returns as a short (NOT a bool)
// since the value must be used in sprintf to send to the server.
short isSysop()
{
   return 1;
}


// Returns a pointer to the user's real name.
char* getRealName()
{
   return szMyUsername;
}


// Returns the user's gender: 'm' if male, 'f' is female, or 'n' if unknown.
char getGender()
{
   return 'n';
}



short getNode()
{
   return nMyNode;
}


short getPlatform()
{
   return PL_LOCAL;
}


#pragma argsused
bool inactiveCheck(time_t timeout, short nFixLine)
{
   return false;
}


bool checkNodeMsg()
{
   return false;
}


void listOnlineUsers()
{
}
