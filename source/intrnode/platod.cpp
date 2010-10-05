/*    PLATOD.CPP
      Copyright 2003 Evan Elias

      ObjectDoor Door Client -- OpenDoors Win32 implementations of door I/O functions.
*/


#include <string.h>
#include "source\doorset.h"
#include "e:\doors\intrnode\door.h"
#include "e:\doors\intrnode\intrnode.h"

void beforeExit();
bool correctDirectory();


// Called upon door startup, to initialize the door depending on the doorkit's needs.
void startup(LPSTR lpszCmdLine)
{
   char* szCopyright = DOOR_COPYRIGHT;

   strcpy(od_registered_to, "Evan Elias");
   od_registration_key = 2469396480L;
   od_control.od_nocopyright = TRUE;
   strcpy(od_control.od_prog_copyright, szCopyright);
   strcpy(od_control.od_prog_name, DOOR_NAME);
   strcpy(od_control.od_prog_version, DOOR_VERSION);
   od_control.od_cmd_show = SW_MINIMIZE;
   od_parse_cmd_line(lpszCmdLine);
   if ( od_control.od_force_local == TRUE )
      od_control.od_cmd_show = SW_RESTORE;
   od_init();
   od_control.od_clear_on_exit = FALSE;
   od_control.od_inactivity = 300;
   od_control.od_help_text2 = (char *)"                  Doorgame - (c) 2003 Evan Elias                               ";

   if ( !correctDirectory() )
      platExit(0);
}


// Sets the function to be called upon any door shutdown (whether by voluntary exit, timeout, carrier loss,
// whatever)
void setupExitFunction()
{
   od_control.od_before_exit = beforeExit;   
}


// Switches to door's directory.  Necessary since Door32 games under mystic/elebbs don't start in their own
// directory, and can't use batch files for this purpose under Win 95/98/ME.
// (This function only used here in the OpenDoors version.  Not referenced
// elsewhere, so no need to create this function in a non-OpenDoors plat*.cpp)
bool correctDirectory()
{
   char* szCommandLine;
   char szExeDir[150], szErrorMsg[250];
   bool bFoundError = false;

   szCommandLine = GetCommandLine();
   
   // Unable to handle command lines over 149 chars.
   if ( strlen(szCommandLine) > 149 )
      {
      bFoundError = true;
      strcpy(szExeDir, "<too long>");
      }
      
   else
      {
      // If command line begins with a quote mark, skip the quote mark; otherwise, use as is.
      if ( szCommandLine[0] == '\"' )
         strcpy(szExeDir, szCommandLine+1);
      else
         strcpy(szExeDir, szCommandLine);

      // Strip spaces from the command line
      for ( short n = 0; n < strlen(szExeDir); n++ )
         {
         if ( szExeDir[n] == ' ' )
            szExeDir[n] = '\0';
         }

      // Assume current directory is door's directory if no path given ( usually Win NT/2k/XP)
      if ( strchr(szExeDir, '\\') == NULL && strchr(szExeDir, ':') == NULL )
         return true;

      // Remove exe program name (done by finding last backslash)
      while ( strlen(szExeDir) > 2 && szExeDir[strlen(szExeDir) - 1] != '\\' )
         {
         szExeDir[strlen(szExeDir) - 1] = '\0';
         }

      // Remove trailing backslash.
      if ( strlen(szExeDir) <= 2 )
         bFoundError = true;
      else
         szExeDir[strlen(szExeDir) - 1] = '\0';         
      }

   if ( !bFoundError )
      {
      if ( SetCurrentDirectory(szExeDir) == FALSE )
         bFoundError = true;
      }

   if ( bFoundError )
      {
      sprintf(szErrorMsg, "Critical error: Door directory %s is invalid.  Please e-mail door author to report!", szExeDir);
      MessageBox(NULL, szErrorMsg, "Door problem!", MB_ICONSTOP | MB_OK | MB_TASKMODAL);
      return false;
      }

   return true;
}


// Gets an input key, if one is present; returns 0 if no input key waiting.
char inputKey()
{
   char cKey = od_get_key(FALSE);

   // Ignore telnet control commands by looking for IAC character
   if ( (unsigned char)cKey == 255 )
      {
      // Ignore following char (indicating command code)
      Sleep(50);
      cKey = od_get_key(FALSE);

      // If an option, ignore option code
      if ( (unsigned char)cKey > 250 && (unsigned char)cKey < 255 )
         {
         Sleep(50);
         od_get_key(FALSE);
         }
      cKey = 0;
      }

   if ( cKey == 0 )
      Sleep(50);
   return cKey;
}


// Displays a string to the user.
void local(char* szString, short nColor, short nNewLines)
{
   short n;

   od_set_attrib(nColor);

   if ( szString != NULL )
      od_disp_str(szString);

   if ( nColor != 7 )
      od_set_attrib(7);

   for(n = 0; n < nNewLines; n++)
      {
      od_disp_str("\r\n");
      }
}


void newline()
{
   od_set_attrib(7);
   od_disp_str("\r\n");
}


// Prints a single character
void printChar(char cChar)
{
   od_putch(cChar);
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
   od_send_file(szFileName);
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

   od_disp_str(szStr);
}


// Checks for carrier loss, exit door if so.
// No need to implement under OpenDoors; the doorkit handles this automatically in another thread.
void checkCarrier()
{
}


// Checks if user is out of time, exit door if so.
// No need to implement under OpenDoors; the doorkit handles this automatically in another thread.
void checkTimeLeft()
{
}


// Displays a hit-any-key type prompt and waits for a key.
void pausePrompt(short nClear, short nCenter)
{
   char* szText = "[Hit any key to continue]";

   if ( nCenter != 1 )
      local(szText, LWHITE, 0);
   else
      center(szText, LWHITE, 0);

   od_get_key(TRUE);
   
   if (nClear == 1)
      clearScreen();
   else
      newline();
}


void clearScreen()
{
   od_clr_scr();
}


// Returns true if the user has ANSI color graphics, false otherwise.
bool hasAnsi()
{
   if ( od_control.user_ansi == TRUE )
      return true;

   return false;
}


// Returns 1 if user is sysop, or 0 if not.  Returns as a short.
// In the OpenDoors version, a sysop is currently defined as someone running the door in
// local mode.
short isSysop()
{
   if ( od_control.od_force_local == TRUE || od_control.baud == 0 )
      return 1;
   else
      return 0;
}


// Returns a pointer to the user's real name.
char* getRealName()
{
   return od_control.user_name;
}


// Returns a pointer to the user's alias, if it exists.  If it doesn't,
// returns a pointer to the user's real name.
char* getAlias()
{
   if ( strlen(od_control.user_handle) > 1 )
      return od_control.user_handle;
   else
      return od_control.user_name;
}


// Returns the user's gender: 'm' if male, 'f' is female, or 'n' if unknown.
char getGender()
{
   // DOOR32.SYS does not have gender information, hence we don't know the user's gender.
   return 'n';
}


// Returns the user's node/line number.
short getNode()
{
   return od_control.od_node;
}


// Returns a constant representing this platform.  (These constants defined in DOOR.H)
short getPlatform()
{
   return PL_OPENDOORS32;
}


// Checks if the user has been inactive for too long.
// No need to implement under OpenDoors; the doorkit handles this automatically in another thread.
#pragma argsused
bool inactiveCheck(time_t timeout, short nFixLine)
{
   return false;
}


// Checks for (and displays) any inter-node global messages that are waiting.
// Not supported in OpenDoors.
bool checkNodeMsg()
{
   return false;
}


// Displays a list of users on-line on the BBS.
// Not supported in OpenDoors.
void listOnlineUsers()
{
}


// Exit the door, as necessary for the doorkit.
void platExit(short nExitVal)
{
   od_exit(nExitVal, FALSE);
}


