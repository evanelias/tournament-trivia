/*    PLATXSDK.CPP
      Copyright 2003 Evan Elias

      ObjectDoor Door Client -- Synchronet XSDK Win32 implementations of door I/O functions.
*/

#include <string.h>
#include "e:\doors\intrnode\door.h"
#include "e:\doors\intrnode\intrnode.h"

void beforeExit();

short nNoLocalWindow = 1;


// Called upon door startup, to initialize the door depending on the doorkit's needs.
void startup(int argc, char *argv[])
{
   char* p;
   
   p=getenv("SBBSNODE");
   if(p)
      strcpy(node_dir,p);
   else 
      {
      printf("\nUnable to run door.\n");
      printf("- If you are trying to run door in local mode, this cannot be done with the\n");
      printf("  Synchronet XTRN.DAT version.  You must run the door through the BBS, or use\n");
      printf("  the Door32 version in local mode instead.\n");
      printf("- If you are running the door remotely, be sure it is configured as a 32bit\n  door in SCFG.  See door's documentation!\n");
      exit(0); 
      }

   for ( short n = 1; n < argc; n++ )
      {
      if ( strcmpi(argv[n], "-window") == 0 || strcmpi(argv[n], "/window") == 0 )
         nNoLocalWindow = 0;
      }
   
   initdata();

   if ( nNoLocalWindow == 1 )
      FreeConsole();

   sec_warn = 270;
   sec_timeout = 300;
}


// Sets the function to be called upon any door shutdown (whether by voluntary exit, timeout, carrier loss,
// whatever)
void setupExitFunction()
{
   atexit(beforeExit);
}


// Gets an input key, if one is present; returns 0 if no input key waiting.
char inputKey()
{
   // Note: XSDK's inkey() already calls Sleep(); no need to do so here.
   return inkey(0);
}


// Displays a string to the remote user.  (This is the door's main output function)
void local(char* szString, short nColor, short nNewLines)
{
   short n;

   attr(nColor);

   if ( szString != NULL )
      rputs(szString);

   if ( nColor != 7 )
      attr(7);

   for(n = 0; n < nNewLines; n++)
      {
      rputs("\r\n");
      }
}


void newline()
{
   attr(7);
   rputs("\r\n");
}


// Prints a single character
void printChar(char cChar)
{
   outchar(cChar);
}


// Displays centered text
void center(char* szText, short nColor, short nNewlines)
{
   char szHolder[100];
   short nEmpty, n, nTrueLength;

   if ( strlen(szText) >= 79 )
      local(szText, nColor, nNewlines);
   else
      {
      nTrueLength = strlen(szText);
      while ( nTrueLength > 0 && (szText[nTrueLength-1] == '\n' || szText[nTrueLength-1] == '\r') )
         nTrueLength--;
         
      nEmpty = 40 - (short)(nTrueLength / 2);  
      for (n = 0; n < nEmpty; n++)
         szHolder[n] = ' ';
      szHolder[nEmpty] = '\0';
      local(szHolder, 7, 0);
      local(szText, nColor, nNewlines);
      }
}


// Displays the specified ANSI file.
void showAnsi(char *szFileName)
{
   printfile(szFileName);
}


// Backspaces the given number of times (60 at most)
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

   rputs(szStr);

   
   /*attr(LCYAN);
   rputs("Backspace\n\r");*/   
}


// Checks for carrier loss, exit door if so
void checkCarrier()
{
   checkline();
}


// Checks if user is out of time, exit door if so
void checkTimeLeft()
{
   checktimeleft();
}


// Displays a hit-any-key type prompt and waits for a key.
void pausePrompt(short nClear, short nCenter)
{
   char* szText = "[Hit any key to continue]";

   time_t timeout = time(NULL);

   if ( nCenter != 1 )
      local(szText, LWHITE, 0);
   else
      center(szText, LWHITE, 0);

   while ( inkey(0) == 0 )
      {
      checkCarrier();
      checkTimeLeft();
      inactiveCheck(timeout);
      Sleep(0);
      }

   
   if (nClear == 1)
      clearScreen();
   else
      local(" ");
}


void clearScreen()
{
   cls();
}


// Returns true if the user has ANSI color graphics, false otherwise.
bool hasAnsi()
{
   if ( (user_misc & ANSI) > 0 )
      return true;

   return false;

}


// Returns 1 if user is sysop, or 0 if not.  Returns as a short (NOT a bool).
short isSysop()
{
   // XSDK by default defines a sysop as security level of 90+
   if ( user_level >= sysop_level  )
      return 1;
   else
      return 0;
}


// Returns a pointer to the user's real name.
char* getRealName()
{
   return user_realname;
}


// Returns a pointer to the user's alias, if it exists.  If it doesn't,
// returns a pointer to the user's real name.
char* getAlias()
{
   if ( strlen(user_name) > 1 )
      return user_name;
   else
      return user_realname;
}


// Returns the user's gender: 'm' if male, 'f' is female, or 'n' if unknown.
char getGender()
{
   if ( user_sex == 'F' )
      return 'f';
   if ( user_sex == 'M' )
      return 'm';

   return 'n';
}



// Returns the user's node/line number.
short getNode()
{
   return node_num;
}


// Returns a constant representing this platform.  (These constants defined in DOOR.H)
short getPlatform()
{
   return PL_XSDK32;
}


// Checks if the user has been inactive for too long
bool inactiveCheck(time_t timeout, short nFixLine)
{
   static time_t ttPrevious;
   static bool bAlreadyWarned = false;
   static bool bTimedOut = false;
   char szText[60];
   time_t now;

   if ( bTimedOut )
      return true;

   now = time(NULL);

   if ( bAlreadyWarned && timeout != ttPrevious )
      bAlreadyWarned = false;
   
   if ( (now - timeout) >= (time_t)sec_warn && !bAlreadyWarned )
      {
      if ( nFixLine == 1 )
         SAVELINE;
      sprintf(szText, "Warning -- Inactivity timeout in %d seconds!", int(sec_timeout - sec_warn));
      local(szText, LRED);
      bAlreadyWarned = true;
      if ( nFixLine == 1 )
         RESTORELINE;
      }

   if ( (now - timeout) >= (time_t)sec_timeout )
      {
      local("Inactivity timeout.");
      bTimedOut = true;
      exit(0);
      return true;
      }

   ttPrevious = timeout;
   return false;
}


// Checks for (and displays) any inter-node messages that are waiting.
// Under Synchronet, these include inter-node pages, user log-on/off notifications, new email notifications, etc.
bool checkNodeMsg()
{
   bool bReturn = false;
   static time_t ttLastChecked = 0, ttCurrent;
   node_t node;
   extern bool bPromptOnScreen;
   extern short nPromptLength;
   
   ttCurrent = time(NULL);

   // Only check for new inter-node messages every 3 seconds.
   if ( ttCurrent <= (ttLastChecked + 2) )
      return false;

   ttLastChecked = ttCurrent;

   // Use XSDK calls to get the node messages...
   
   if(!ctrl_dir[0])
      return false;
   getnodedat(node_num,&node,0);
   
   if(node.misc&NODE_MSGW || node.misc&NODE_NMSG)
      {
      // Remove the user's prompt if it's on the screen and there's inter-node messages
      if ( bPromptOnScreen )
         {
         backspace(nPromptLength);
         bPromptOnScreen = false;
         }

      bReturn = true;
      }

   if(node.misc&NODE_MSGW)
      getsmsg(user_number);      /* getsmsg clears MSGW flag */

   if(node.misc&NODE_NMSG)       /* getnmsg clears NMSG flag */
      getnmsg();

   if(node.misc&NODE_INTR)
      exit(0);

   return bReturn;
}


// Displays a list of users on-line on the BBS.
void listOnlineUsers()
{
   whos_online(1);
}


// Exit the door, as necessary for the doorkit.
void platExit(short nExitVal)
{
   exit(nExitVal);
}

