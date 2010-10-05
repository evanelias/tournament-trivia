/*    DOOR.CPP
      Copyright 2003 Evan Elias

      This is the source code for the ObjectDoor door client.
      Additional functions are contained in plat*.cpp, depending on the platform.

      To build the Door Client for Synchronet (Win32), compile and link these files:
         door.cpp
         platxsdk.cpp
         intrnode.cpp
         xsdk.c
         xsdkvars.c

      To build the Door Client for Door32, compile and link these files:
         door.cpp
         platod.cpp
         intrnode.cpp
         odoorw.lib
*/


#include <new.h>
#include <fstream.h>
#include "e:\doors\intrnode\door.h"
#include "e:\doors\intrnode\intrnode.h"
#include "source\doorset.h"

void exitDoor(short);
void beforeExit();
void sendInput(char*, short=IP_NORMAL);
void performIO();
void getInput(char*, short=190);
bool getOutput();
void handleOutput(OutputData);
void fixStatline();
void invalidatePrompt();
bool isValidPrompt();
void displayHelp(char*, char*, char* = NULL, bool=false);
bool wordSearch(char*, char*);
void badNew();


// Input and Output mailslots for communicating with server process
HANDLE hInputSlot; 
HANDLE hOutputSlot;

// Other misc globals
bool bCommandMode = false, bPromptOnScreen = false;
OutputData odPromptInfo;
short nPromptLength = 0;
short nPending = 0;


#ifdef OD32
#pragma argsused
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
#else
void main(int argc, char *argv[])
#endif
{
   char szText[160];
   bool bSuccess;
   short nTries = 0;

   // Set results of what happens when a call to new fails
   set_new_handler(badNew);

   // Call the startup function
   #ifdef OD32
   startup(lpszCmdLine);
   #else
   startup(argc, argv);
   #endif

   Sleep(500);

   do
      {
      // Attempt to start up the door server process.  (No need to check if the door server process is already
      // running; it does that on its own and shuts down extra copies as needed)
      bSuccess = CreateProcess(DOOR_SERVER_EXE, NULL, NULL, NULL, false, DETACHED_PROCESS, NULL, NULL, new STARTUPINFO, new PROCESS_INFORMATION);
      Sleep(1000);
      nTries++;
      }  
   while ( bSuccess == false && nTries < 5 );

   // Get a handle on the door server's input mailslot
   hInputSlot = openSlot(-1);

   // Create this node's output mailslot.
   hOutputSlot = createSlot( getNode() );
 
   // If unable to open input slot, re-try up to 5 times (intial failure seems to happen on WinXP randomly)
   // If still failure, this indicates the door server is not running and could not be started, so shut down.
   nTries = 0;
   while (hInputSlot == INVALID_HANDLE_VALUE ) 
      {
      Sleep(500);
      hInputSlot = openSlot(-1);      
      if ( ++nTries > 5 )
         {
         local("Unable to start door: IPC error");
         exitDoor(1);
         }
      }

   // If the output slot for this node is already open, it means the node is already running the door.
   if ( hOutputSlot == INVALID_HANDLE_VALUE )
      {
      local("Unable to start door:  This node is already in use!");
      pausePrompt();
      exitDoor(1);
      }

   // Now that the mailslot handles were successful, call setupExitFunction() to register beforeExit() as the
   // function called upon exit.
   setupExitFunction();
      
   // Send an input message to the server, to tell it a user is trying to enter the game.  The user's info is
   // passed as a string in the format below.
   sprintf(szText, "%d&%c&%d&%s&%s", isSysop(), getGender(), getPlatform(), getAlias(), getRealName());
   sendInput(szText, IP_ENTER_GAME);

   // Handle I/O for the user
   while (1)
      {
      performIO();
      }   
}

void exitDoor(short nType)
{
   char szText[60];

   if ( nType != 0 )
      {
      sprintf(szText, "ObjectDoor Error %d.%d", nType, GetLastError() );
      local(szText);
      }

   platExit(0);
}


// Actions performed before door shuts down
void beforeExit()
{
   // Tell the server this door node is exiting
   sendInput(NULL, IP_FORCE_EXIT);
   sendInput(NULL, IP_FINISHED);
   Sleep(2000);

   // Close the mailslots
   CloseHandle(hInputSlot); 
   CloseHandle(hOutputSlot);
}


// Sends text over the input mailslot to the server
void sendInput(char* szText, short nType)
{
   InputData idMessage;
   char szBuffer[250];
   static bool bAlreadyWarned = false;
   
   if ( szText != NULL )
      {
      if ( strlen(szText) > 198 )
         szText[198] = '\0';
      strcpy(idMessage.szMessage, szText);
      }
   else
      idMessage.szMessage[0] = '\0';
      
   idMessage.nFrom = getNode();
   idMessage.nType = nType;
   
   sendToSlot(hInputSlot, idMessage.toString(szBuffer));

   nPending++;
   if ( nPending == 2 )
      Sleep(200);
   if ( nPending == 3 || nPending == 4 )
      Sleep( nPending * 300);
   if ( nPending >= 5 && !bAlreadyWarned )
      {
      local("\r\nA door process has locked up.  Please wait a minute and then re-enter the door.");
      local("If the door still doesn't run, have your sysop use CTRL-ALT-DELETE to kill the");
      sprintf(szBuffer, "instance of %s that is running.", DOOR_SERVER_EXE);
      local(szBuffer);
      bAlreadyWarned = true;
      Sleep(3000);
      exitDoor(0);
      }
}


// Gets input, checks it for certain globals, and sends it to the door server if not a global.
void performIO()
{
   char szString[241];
   static char szOldString[241] = "";
   extern short fhInput1;

   // Call getInput() to wait for and handle the player's input; this function also handles grabbing output
   // from the mailslot and printing it when appropriate
   getInput(szString, 190);

   // If user typed repeat command
   if ( strcmpi(szString, "!") == 0 || strcmpi(szString, "=r") == 0 )
      {
      strcpy(szString, szOldString);
      local(szOldString, WHITE);
      }
         
   strcpy(szOldString, szString);

   // If user typed Who's Online in Synchronet version
   if ( getPlatform() == PL_XSDK32 && (strcmpi(szString, "#") == 0 || strcmpi(szString, "=#") == 0) )
      {
      listOnlineUsers();
      local(" ");
      bPromptOnScreen = false;
      fixStatline();
      return;
      }

   // If user wanted to instantly exit
   if ( strcmpi(szString, "=x") == 0 )
      exitDoor(0);
  
   // Send the input.  
   sendInput(szString);

   // Since the previous prompt was answered, attempt to invalidate it.
   // This call will intentionally have no effect if the prompt is a combat/statline prompt.
   invalidatePrompt();
}


// Waits for input from the user, also printing new output when appropriate
void getInput(char* szInput, short nMaxLength)
{
   time_t timeout;
   char chKeyPressed;
   short nPosition = 0;
   static OutputData odMessage;

   // Decrease nMaxLength by one, since this function later doesn't count the null zero
   nMaxLength--;

   if(szInput == NULL || nMaxLength < 1)
      return;

   timeout=time(NULL);
      
   while (1)
      {
      // Check for lost carrier, time expired, etc.  (Note that in the OpenDoors implementation, these functions
      // are blank, since OpenDoors does this type of checking automatically)
      checkCarrier();
      checkTimeLeft();
      inactiveCheck(timeout);
      
      chKeyPressed = 0;

      // While no input on screen, continue to display output for this node.
      while ( nPosition == 0 && chKeyPressed == 0 )
         {
         checkCarrier();
         checkTimeLeft();
         inactiveCheck(timeout, 0);

         // Check for inter-node messages in synchronet
         if ( checkNodeMsg() )
            fixStatline();

         // Only respond to keyboard if there's a valid prompt on screen.
         if ( isValidPrompt() )
            chKeyPressed = inputKey();
         else
            Sleep(50);
            
         // Display all output waiting in mailslot, if any.
         if ( getOutput() )
            fixStatline();
         }

      // If no key pressed this cycle, try getting the key again.
      if ( chKeyPressed == 0 && isValidPrompt() )
         chKeyPressed = inputKey();

      // If a key was pressed
      if ( chKeyPressed != 0 )
         {
         timeout=time(NULL);
         
         // Fix CR/LF problem, and ignore otherwise invalid characters
         if ( chKeyPressed == 10 || chKeyPressed < 0 )
           continue;

         // Enter key hit: Add ending \0 to string and return.
         if(chKeyPressed == '\r' || chKeyPressed == '\n')
            {
            szInput[nPosition] = '\0';
            newline();
            bPromptOnScreen = false;
            return;
            }

         // At this point, user has definitely hit a non-enter key.  If this is the
         // first key hit of a string, and there's no prompt on screen yet, display
         // it first before displaying the keystroke.
         if ( nPosition == 0 && !bPromptOnScreen )
            fixStatline();
            
         // Backspace key
         if( chKeyPressed == 8 && nPosition > 0 )
            {
            backspace(1);
            --nPosition;
            continue;
            }

         // Any normal key
         if(chKeyPressed >= 32 && nPosition < nMaxLength && (unsigned char)chKeyPressed <= 127)
            {
            // Display the key and add it to the input buffer.
            printChar(chKeyPressed);
            szInput[nPosition++] = chKeyPressed;

            // If in hotkey mode, send the input immediately.
            if ( !bCommandMode )
               {
               szInput[nPosition] = '\0';
               newline();
               bPromptOnScreen = false;
               return;
               }
            }
         }
      }
   
}


// Reads text from the output mailslot.
bool getOutput()
{
   DWORD nNextMessageSize, nMessagesLeft, nBytesRead;      
   bool bMessagesFound = false;
   char szBuffer[245];
   OutputData odMessage;

   // Loop as long as there's output
   do
      {
      // If unable to check the mailslot, abort door
      if ( !GetMailslotInfo(hOutputSlot, NULL, &nNextMessageSize, &nMessagesLeft, NULL) ) 
         exitDoor(2);

      // If messages left, read them
      if ( nMessagesLeft > 0 && nNextMessageSize != MAILSLOT_NO_MESSAGE )
         {
         // Ignore invalid message sizes (see MS Knowledge Base Q192276)
         // In this case, simply wait for getOutput() to be called again next timeslice.
         if ( nNextMessageSize < 0 || nNextMessageSize > 500 )
            return bMessagesFound;

         if ( !ReadFile(hOutputSlot, szBuffer, nNextMessageSize, &nBytesRead, NULL) )
            {
            local("error reading from slot");
            return bMessagesFound;
            }
         
         // Convert the text into an OutputData object
         odMessage = szBuffer;
            
         // Handle the output message
         handleOutput(odMessage);
         bMessagesFound = true;
         nPending = 0;
         }
      }
   while ( nNextMessageSize != MAILSLOT_NO_MESSAGE );

   return bMessagesFound;
}


// Processes an output message, depending on its type
void handleOutput(OutputData odMessage)
{
   char* szFile, * szTopic, * szError, * szTemp;
   bool bPartialMatch;

   // If there's a prompt on screen, remove it.
   if ( bPromptOnScreen )
      {
      backspace(nPromptLength);
      bPromptOnScreen = false;
      }

   switch ( odMessage.nType )
      {
      // Standard text output
      case OP_NORMAL:
         local(odMessage.szMessage, odMessage.nColor, 0);
         break;

      // New prompt for a hotkey
      case OP_HOTKEY_PROMPT:
         bPromptOnScreen = false;
         bCommandMode = false;
         if ( strlen(odMessage.szMessage) >= 60 )
            odMessage.szMessage[59] = '\0';
         odPromptInfo = odMessage;
         break;

      // New prompt for a command string
      case OP_COMMAND_PROMPT:
         bPromptOnScreen = false;
         bCommandMode = true;
         if ( strlen(odMessage.szMessage) >= 60 )
            odMessage.szMessage[59] = '\0';
         odPromptInfo = odMessage;
         break;

      // Pause
      case OP_FORCE_PAUSE:
         pausePrompt();
         break;

      // Exit
      case OP_EXIT_NODE:
         exitDoor(0);
         break;

      // Clear Screen
      case OP_CLEAR_SCREEN:
         clearScreen();
         break;

      // Display text, centered
      case OP_CENTER_TEXT:
         center(odMessage.szMessage, odMessage.nColor, 0);
         break;

      // Display a file (adds .ANS or .ASC as appropriate)
      case OP_DISPLAY_ANSI:
         if ( hasAnsi() )
            strcat(odMessage.szMessage, ".ans");
         else
            strcat(odMessage.szMessage, ".asc");
         showAnsi(odMessage.szMessage);
         break;

      // Display an entry in an HLP file
      case OP_DISPLAY_HLP:
         szFile = strtok(odMessage.szMessage, "&");
         szTopic = strtok(NULL, "&");
         szError = strtok(NULL, "&");
         szTemp = strtok(NULL, "");
         if ( szTemp[0] == 'y' )
            bPartialMatch = true;
         else
            bPartialMatch = false;
         displayHelp( szFile, szTopic, szError, bPartialMatch );
         break;
      }

   // If current prompt is a blank prompt, and new message is not a prompt-change,
   // update the current prompt's HP/SP/MF/etc values using data in the new message.
   if ( odMessage.nType != OP_HOTKEY_PROMPT && odMessage.nType != OP_COMMAND_PROMPT && strlen(odPromptInfo.szMessage) <= 0 )
      {
      short nOldType = odPromptInfo.nType;
      odPromptInfo = odMessage;
      odPromptInfo.nType = nOldType;
      odPromptInfo.szMessage[0] = '\0';
      }
}


// Re-displays player's statline as needed.
void fixStatline()
{
   char szText[80], szTemp[40];
   
   // If a prompt is already on screen, remove it
   if ( bPromptOnScreen )
      {
      backspace(nPromptLength);
      bPromptOnScreen = false;
      }

   // If there's no prompt to be displayed, return.
   if ( !isValidPrompt() )
      return;
      
   bPromptOnScreen = true;

   // If current prompt is a blank one, this indicates use a statline prompt.
   // Generate and display the user's statline.
   if ( strlen(odPromptInfo.szMessage) == 0 )
      {
      nPromptLength = 2;
      
      sprintf(szText, STAT_1, odPromptInfo.nHp);
      if ( odPromptInfo.nSp  > -1 )
         {
         sprintf(szTemp, STAT_2, odPromptInfo.nSp);
         strcat(szText, szTemp);
         }
      if ( odPromptInfo.nMf > -1 )
         {
         sprintf(szTemp, STAT_3, odPromptInfo.nMf);
         strcat(szText, szTemp);
         }
      strcat(szText, "]");
      local(szText, odPromptInfo.nHpColor, 0);
      nPromptLength += strlen(szText);

      if ( odPromptInfo.nEnemyPercent > -1 )
         {
         sprintf(szText, " [%d%%]", odPromptInfo.nEnemyPercent);
         local(szText, LRED, 0);
         nPromptLength += strlen(szText);
         }

      local(": ", LWHITE, 0);
      }

   // Otherwise, prompt is an actual prompt, so display it.
   else
      {
      if ( strlen(odPromptInfo.szMessage) >= 70 )
         odPromptInfo.szMessage[69] = '\0';
      local(odPromptInfo.szMessage, odPromptInfo.nColor, 0);
      nPromptLength = strlen(odPromptInfo.szMessage);
      }      
}



// Once a player types input for a non-statline prompt, it is marked as being
// "answered" by setting its first character to '~'.  Answered prompts are not
// redisplayed.
void invalidatePrompt()
{
   // Only a non-statline prompt can be invalidated.
   if ( strlen(odPromptInfo.szMessage) > 0 )
      odPromptInfo.szMessage[0] = '~';
}


// Checks if the current prompt is valid.
bool isValidPrompt()
{
   // Prompt is invalid if it has been flagged as such, or if it's not really
   // a prompt-type message (the latter occurs at game startup, since the prompt
   // object has default values)
   if ( odPromptInfo.szMessage[0] == '~' || odPromptInfo.nType == OP_NORMAL )
      return false;
   else
      return true;
}



// Displays an entry in a "doormud-style HLP" format file.  This is a simple file format used to store a topic-
// based help system.  This function takes parameters as follows:
//    szFileName     -  Name of the help file
//    szTopic        -  Name of the help entry
//    szError        -  Error message to display if no such entry, or "none" if none.
//    bPartialMatch  -  true if "partial matches" allowed on topic name, if no full matches found.
void displayHelp(char* szFileName, char* szTopic, char* szError, bool bPartialMatch)
{
   ifstream ifsFile;
   short nColor = 7, nLineCount = 0;
   bool bFound = false, bNonstop = false, bSecondPass = false;
   char szLine[90], cKey;
   time_t timeout;

   if ( szFileName == NULL || szTopic == NULL || strlen(szTopic) < 1 )
      return;

   ifsFile.open(szFileName);

   while ( ifsFile )
      {
      ifsFile.getline(szLine, 90);

      // If partial-matches are enabled:  If we don't find a full-match on first pass
      // of the hlp file, we go through it again looking for partial-matches.
      if ( !ifsFile && !bFound && !bSecondPass && bPartialMatch )
         {
         ifsFile.clear();
         ifsFile.seekg(0);
         bSecondPass = true;
         }
      
      if ( szLine == NULL )
         continue;
         
      switch (szLine[0])
         {
         case ';':
            if ( strlen(szLine) > 1 && strcmpi(&szLine[1], szTopic) == 0 )
               bFound = true;
            if ( bSecondPass && strlen(szLine) > 1 && wordSearch(&szLine[1], szTopic) )
               bFound = true;
            break;
         case '-':
            if ( bFound && strlen(szLine) == 1 )
               return;
            if ( bFound && strlen(szLine) > 1 )
               {
               if ( strcmpi(&szLine[1], szTopic) == 0 || szLine[1] == ' ' )
                  return;
               if ( bSecondPass && wordSearch(&szLine[1], szTopic) )
                  return;
               }
            break;
         //case ':':
         //   return;
         case '$':
            if ( bFound && strlen(szLine) > 1 )
               nColor = atoi(&szLine[1]);
            break;
         case '<':
            break;
         case ']':
            if ( strlen(szLine) > 1 && bFound )
               local(&szLine[1], nColor, 0);
            break;            
         default:
            if ( bFound )
               {
               local(szLine, nColor);
 
               if ( ++nLineCount > 20 && !bNonstop)
                  {
                  local("(C)ontinue, (N)onstop, (Q)uit: ", WHITE, 0);
                  timeout = time(NULL);
                  cKey = 0;
                  while ( cKey == 0 )
                     {
                     cKey = inputKey();
                     checkCarrier();
                     checkTimeLeft();
                     inactiveCheck(timeout);
                     Sleep(50);
                     }

                  local(" ");

                  if ( cKey == 'n' || cKey == 'N' )
                     bNonstop = true;

                  if ( cKey == 'q' || cKey == 'Q' || cKey == 'x' || cKey == 'X' || cKey == 's' || cKey == 'S' )
                     return;                        

                  nLineCount = 0;
                  }
               }
            break;
         }
      }

   if ( !bFound && strcmpi(szError, "none") != 0 )
      local(szError);
}


// Does "intelligent search" ON first parameter, FOR second parameter 
bool wordSearch(char* szString1, char* szString2)
{
   char szToSearch[162];
   char szToFind[162];

   if ( strlen(szString1) > 160 )
      szString1[160] = '\0';
   if ( strlen(szString2) > 160 )
      szString2[160] = '\0';
   
   sprintf(szToSearch, " %s", szString1);
   sprintf(szToFind, " %s", szString2);

   strlwr(szToSearch);
   strlwr(szToFind);
   
   if ( strstr(szToSearch, szToFind) != NULL )
      return true;
   else
      return false;
}


// Called when an attempt to use C++'s new fails
void badNew()
{
   local("Out of memory.");
   exit(0);
}






