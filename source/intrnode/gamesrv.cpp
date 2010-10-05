#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <conio.h>
#include <dir.h>
#include <new.h>
#include "e:\doors\intrnode\gamesrv.h"
#include "source\doorset.h"

void handleThread(void*);
void startRoundThread(void*);
void badNew();


/////////////////////////////////////////////////////////////////////////////////////////
// GameServer methods

// GameServer constructor
GameServer::GameServer()
{
   // Set the function for bad new call
   set_new_handler(badNew);

   // Open the input slot
   hInputSlot = createSlot(-1);

   if ( hInputSlot == INVALID_HANDLE_VALUE )
      exit(0);

   // Initialize all nodes to NULL.
   for ( int n = 0; n < MAX_NODE; n++ )
      {
      gNode[n] = NULL;
      }

   // Initialize the game's CRITICAL_SECTION object.
   InitializeCriticalSection(&csCritical);
      
   nCriticalCount = 0;
   nPlayersInGame = 0;
}


// Prints a message to all on-line nodes that are flagged as being in the game.
void GameServer::printAll(char* szText, short nColor, short nNewlines, short nType)
{
   for ( short n = 0; n < MAX_NODE; n++ )
      {
      if ( gNode[n] != NULL && gNode[n]->bInGame )
         gNode[n]->print(szText, nColor, nNewlines, nType);
      }
}


// Same as printAll() but uses word wrap.
void GameServer::printAllWordWrap(char* szText, short nColor, short nNewlines, short nOffset, bool bCarryIndent)
{
   for ( short n = 0; n < MAX_NODE; n++ )
      {
      if ( gNode[n] != NULL && gNode[n]->bInGame )
         gNode[n]->printWordWrap(szText, nColor, nNewlines, nOffset, bCarryIndent);
      }
}


// Repeatedly read input from the input slot.
// Returns when last player exits game.
void GameServer::run()
{
   DWORD nNextMessageSize, nMessagesLeft, nBytesRead;      
   InputData idMessage;
   char szBuffer[245];

   // Call randomize() in the GameServer's operational thread (usually same thread as main())
   myRandomize();

   while ( true )
      {
      nMessagesLeft = 0;
      nNextMessageSize = MAILSLOT_NO_MESSAGE;

      // Check the status of the slot.
      GetMailslotInfo(hInputSlot, NULL, &nNextMessageSize, &nMessagesLeft, NULL);
      
      // If there's no input, sleep and then return to top of loop.
      if ( nNextMessageSize == MAILSLOT_NO_MESSAGE )
         {
         Sleep(100);
         continue;
         }

      // If there's a message, read it and process it.
      if ( nMessagesLeft > 0 )
         {
         // Ignore invalid message sizes (see MS Knowledge Base Q192276)
         if ( nNextMessageSize < 0 || nNextMessageSize > 500 )
            continue;

         ReadFile(hInputSlot, szBuffer, nNextMessageSize, &nBytesRead, NULL); 
         idMessage = szBuffer;         
         enterCritical();
         handleInput(idMessage);
         leaveCritical();
         
         // If last player just exited, shut down the server.
         if ( idMessage.nType == IP_FINISHED && nPlayersInGame == 0 )
            {
            CloseHandle(hInputSlot);
            DeleteCriticalSection(&csCritical);
            return;
            }
         }
      }
}


// Processes input messages and redirects them to the proper node thread.
void GameServer::handleInput(InputData idMessage)
{
   // If message is from a totally out-of-range node number, ignore it.
   if ( idMessage.nFrom < 0 || idMessage.nFrom >= MAX_NODE )
      return;

   // If an enter-game message:
   if ( idMessage.nType == IP_ENTER_GAME )
      {
      if ( gNode[idMessage.nFrom] != NULL )
         {
         gNode[idMessage.nFrom]->print("Error:  Node already in game.  Please wait 5 seconds and re-enter game.");
         gNode[idMessage.nFrom]->exitGame();
         return;
         }

      // Create the new node and then return.  Start round thread if needed.  Verify the player isn't a dupe.
      nPlayersInGame++;
      addNode(idMessage.nFrom, idMessage.szMessage);

      if ( nPlayersInGame == 1 )
         _beginthread(startRoundThread, 4096, this);

      for ( short n = 0; n < MAX_NODE; n++ )
         {
         if ( gNode[n] == NULL || n == idMessage.nFrom )
            continue;
         if ( strcmpi(gNode[n]->szRealName, gNode[idMessage.nFrom]->szRealName) == 0 )
            {
            gNode[idMessage.nFrom]->print("You are on-line on multiple nodes!\r\nPlease wait 5 seconds and re-enter the game.");
            gNode[n]->print("You are on-line on multiple nodes!\r\nPlease wait 5 seconds and re-enter the game.");
            gNode[idMessage.nFrom]->exitGame();
            gNode[n]->exitGame();
            }
         }
     
      return;
      }
      
   // If input is from an invalid node, ignore it.
   if ( gNode[idMessage.nFrom] == NULL )
      return;

   // If an exit-game message:
   if ( idMessage.nType == IP_FINISHED )
      {
      // Kill the node.  If the node has a thread, wait until the
      // thread is terminated (by a IP_FORCE_EXIT) before proceeding.
      while ( gNode[idMessage.nFrom]->bHasThread )
         {
         leaveCritical();
         Sleep(200);
         enterCritical();
         }
      CloseHandle(gNode[idMessage.nFrom]->hOutputSlot);
      delete gNode[idMessage.nFrom];
      gNode[idMessage.nFrom] = NULL;
      nPlayersInGame--;
      return;
      }
      
   // Otherwise, message is IP_NORMAL or IP_FORCE_EXIT.  In either case, route the
   // InputData to the appropriate node, and let the node's thread take care of it.   
   // If not using separate threads for each node, have an outside function handle
   // the message instead.
   if ( gNode[idMessage.nFrom]->bHasThread )
      gNode[idMessage.nFrom]->mqInput.enqueue(idMessage);
   else
      centralInput(idMessage);
}


// Virtual function for handling input on nodes that don't have their own
// threads.  Over-ride to add functionality if desired.
#pragma argsused
void GameServer::centralInput(InputData id)
{
}


// Virtual function for handling a round event.  Over-ride to add functionality
// if desired.
#pragma argsused
void GameServer::doorRound(time_t nRound)
{
}



// Used to enter a critical section.  Useful for tasks in which a thread is
// modifying global resources or game data, to prevent other threads from
// doing so.
void GameServer::enterCritical()
{
   nCriticalCount++;
   EnterCriticalSection(&csCritical);
}


// Used to leave a critical section.
void GameServer::leaveCritical()
{
   LeaveCriticalSection(&csCritical);
   nCriticalCount--;
}



/////////////////////////////////////////////////////////////////////////////////////////
// GameNode methods


// GameNode constructor.  Opens the output slot and assigns default values.
GameNode::GameNode(short nNumber, char* szUserInfo)
{
   hOutputSlot = openSlot(nNumber);
   nIndex = nNumber;
   bInGame = false;
   bHasThread = false;

   // Read sysop status flag
   if ( atoi( strtok(szUserInfo, "&") ) == 1 )
      bSysop = true;
   else
      bSysop = false;

   // Read gender
   char* szTemp = strtok(NULL, "&");
   cGender = szTemp[0];

   // Read platform
   nPlatform = atoi( strtok(NULL, "&") );

   // Read alias
   strcpy( szAlias, strtok(NULL, "&") );
   
   // Read real name
   strcpy( szRealName, strtok(NULL, "") );
}


// Destructor (virtual -- override if needed)
GameNode::~GameNode()
{
}




// Prints a message to a given node.
void GameNode::print(char* szText, short nColor, short nNewlines, short nType)
{
   OutputData odMessage;
   char szBuffer[250];
   
   // Need to convert szText to an output message.
   odMessage.szMessage[0] = '\0';
   if ( szText != NULL )
      {
      if ( strlen(szText) >= 200 - 2*nNewlines )
         szText[199 - 2*nNewlines] = '\0';
      strcpy(odMessage.szMessage, szText);
      }

   // Add the specified number of newlines (1 by default)
   for ( short n = 0; n < nNewlines; n++ )
      {
      strcat(odMessage.szMessage, "\r\n");
      }

   // Set the message color, type, and player stat info
   odMessage.nColor = nColor;
   odMessage.nType = nType;
   fillStats(&odMessage);
  
   // Send the message to the slot, after converting it to a string.
   sendToSlot(hOutputSlot, odMessage.toString(szBuffer));
}



// Prints a message to a given node; wraps words
void GameNode::printWordWrap(char* szText, short nColor, short nNewlines, short nOffset, bool bCarryIndent)
{
   char szBuffer[81], szCarryOffset[81];
   char* szPosition = szText;
   short nMarker;

   if ( bCarryIndent )
      {
      for ( short n = 0; n < nOffset; n++ )
         {
         szCarryOffset[n] = ' ';
         }
      szCarryOffset[nOffset] = '\0';
      }
   
   while ( strlen(szPosition) > (79 - nOffset) )
      {
      nMarker = 79 - nOffset;

      while ( szPosition[nMarker] != ' ' )
         {
         nMarker--;
         if ( nMarker == 0 )
            {
            print(szPosition, nColor, nNewlines);
            return;
            }
         }

      strncpy(szBuffer, szPosition, nMarker);
      szBuffer[nMarker] = '\0';
      print(szBuffer, nColor, 1);
      szPosition += nMarker+1;

      if ( bCarryIndent && strlen(szPosition) > 0 )
         print(szCarryOffset, LWHITE, 0);
      else
         nOffset = 0;
      }

   if (strlen(szPosition) > 0 )
      print(szPosition, nColor, nNewlines); 
}


// Prints a newline
void GameNode::newline()
{
   print(" \r\n", LWHITE, 0);
}


// Tells the node to pause for a keystroke
void GameNode::pausePrompt()
{
   print(NULL, 7, 0, OP_FORCE_PAUSE);
}

// Tells the node to exit
void GameNode::exitGame()
{
   print(NULL, 7, 0, OP_EXIT_NODE);
}

// Tells the node to clear its screen
void GameNode::clearScreen()
{
   print(NULL, 7, 0, OP_CLEAR_SCREEN);
}

// Prints centered text on node's screen
void GameNode::center(char* szText, short nColor, short nNewlines)
{
   print(szText, nColor, nNewlines, OP_CENTER_TEXT);
}

// Displays an ANSI file for the node
void GameNode::displayScreen(char* szFileName)
{
   print(szFileName, 7, 0, OP_DISPLAY_ANSI);
}

// Displays an HLP file entry for the node
void GameNode::displayHlp(char* szFileName, char* szEntry, char* szError, bool bPartialMatch)
{
   char szText[180];

   short nTotalLength = 5 + strlen(szFileName) + strlen(szEntry);
   if ( szError == NULL )
      nTotalLength += 4;
   else
      nTotalLength += strlen(szError);

   if ( nTotalLength > 175 )
      return;
  
   sprintf(szText, "%s&%s&", szFileName, szEntry);
    
   if ( szError == NULL )
      strcat(szText, "none");
   else
      strcat(szText, szError);

   if ( bPartialMatch )
      strcat(szText, "&y");
   else
      strcat(szText, "&n");

   print(szText, 7, 0, OP_DISPLAY_HLP);
}


// Displays a message in a box
void GameNode::textBox(char* szBoxTitle, short nTextColor, short nBoxColor, bool bCenter)
{
   char szText[80], szHolder[50];
   short n;

   if ( strlen(szBoxTitle) < 1 || strlen(szBoxTitle) > 75 )
      return;

   if ( bCenter)
      {
      short nEmpty = 38 - (short)(strlen(szBoxTitle) / 2);  
      for (n = 0; n < nEmpty; n++)
         szHolder[n] = ' ';
      szHolder[nEmpty] = '\0';
      print(szHolder, 7, 0);
      }
   
   for (n = 0; n < strlen(szBoxTitle) + 4; n++)
      szText[n] = 'Í';
   szText[0] = 'É';
   szText[ strlen(szBoxTitle) + 3 ] = '»';
   szText[ strlen(szBoxTitle) + 4 ] = '\0';

   print(szText, nBoxColor);

   if ( bCenter )
      print(szHolder, 7, 0);
   print("º ", nBoxColor, 0);
   print(szBoxTitle, nTextColor, 0); 
   print(" º", nBoxColor);
      
   if ( bCenter )
      print(szHolder, 7, 0);
   szText[0] = 'È';
   szText[ strlen(szBoxTitle) + 3 ] = '¼';
   print(szText, nBoxColor, 2);
}


// Displays a menu option
void GameNode::menuOption(char cKey, char* szText, short nKeyColor, short nArrowColor, short nTextColor)
{
   char szHolder[5];
   sprintf(szHolder, " %c", cKey);
   print(szHolder, nKeyColor, 0);
   print("  ", nArrowColor, 0);
   print(szText, nTextColor);
}


// Displays underlined text
void GameNode::underline(char* szText, char* szUnderline, short nTextColor, short nUnderColor, bool bCenter)
{
   char* szHolder;

   szHolder = new char[ strlen(szText) + 5 ];
   szHolder[0] = '\0';

   // Make the underline long enough
   while ( strlen(szHolder) < strlen(szText) )
      {
      strcat(szHolder, szUnderline);
      }

   // Chop off extra/uneven characters from the end of the underline
   // (This only happens if szUnderline is more than 1 character long)
   while ( strlen(szHolder) > strlen(szText) )
      {
      szHolder[ strlen(szHolder)-1 ] = '\0';
      }

   if ( bCenter )
      {
      center(szText, nTextColor);
      center(szHolder, nUnderColor);
      }
   else
      {
      print(szText, nTextColor);
      print(szHolder, nUnderColor);
      }
   
   delete szHolder;
}



/////////////////////////////////////////////////////////////////////////////////////////
// GameThread methods


GameThread::GameThread(GameNode* aNode, GameServer* aServer)
{
   gn = aNode;
   gs = aServer;
}


// Default cleanup method for a thread.  Over-ride to have something different happen
// upon thread termination.
void GameThread::cleanup()
{
   gn->print(NULL, LWHITE, 0, OP_COMMAND_PROMPT);
}


// Sets a new prompt for the user and then gets an entire string from the input
// queue for this thread's node.  If no string present, waits until one is.
char* GameThread::getStr(char* szBuffer, char* szPromptText, short nPromptCol, short nMaxLen)
{
   // Display the new prompt
   gn->print(szPromptText, nPromptCol, 0, OP_COMMAND_PROMPT);

   gs->leaveCritical();
   
   // While there's no input messages waiting, sleep.
   while ( gn->mqInput.isEmpty() )
      {
      Sleep(50);
      }

   gs->enterCritical();
      
   // Retreive the first waiting input message
   InputData idMessage = gn->mqInput.dequeue();

   // If it's an IP_FORCE_EXIT message, shut down this thread.
   if ( idMessage.nType == IP_FORCE_EXIT )
      throw ThreadException();

   // If the message is too long, shorten it.
   if ( strlen(idMessage.szMessage) > nMaxLen && nMaxLen < 190 )
      idMessage.szMessage[nMaxLen] = '\0';
      
   // Copy the message to the buffer and return it.
   strcpy(szBuffer, idMessage.szMessage);
   return szBuffer;
}


// Sets a new prompt for the user and then gets a single keypress (hotkey).
// May wait for input, depending on value of bWait.  
char GameThread::getKey(char* szPromptText, short nPromptCol, bool bWait)
{
   // Display the new prompt
   gn->print(szPromptText, nPromptCol, 0, OP_HOTKEY_PROMPT);

   if ( bWait )
      gs->leaveCritical();
   
   // While there's no input, if bWait is true, pause until input is present.
   // But if there's no input and bWait is false, return 0.
   while ( gn->mqInput.isEmpty() )
      {
      if ( bWait )
         Sleep(50);
      else
         return 0;
      }

   if ( bWait )
      gs->enterCritical();
      
   // Retreive the first waiting input message
   InputData idMessage = gn->mqInput.dequeue();

   // If it's an IP_FORCE_EXIT message, shut down this thread.
   if ( idMessage.nType == IP_FORCE_EXIT )
      throw ThreadException();

   // Return the keystroke, which will be the first character of the input message.
   // Make it lowercase first.
   return tolower( idMessage.szMessage[0] );
}

// Tells the thread to pause for a keystroke
void GameThread::pause(bool bCenter)
{
   char szText[70];
   szText[0] = '\0';
   if ( bCenter )
      sprintf(szText, "%-27s", " ");
   strcat(szText, "[Hit any key to continue]");
   getKey(szText);
}


// Static method used to launch a dedicated-node thread for temporarily handling a node's IO.
void GameThread::launch(GameThread* aThread)
{
   // Don't allow multiple threads for a single node.
   if ( !aThread->gn->bHasThread )
      _beginthread(handleThread, 4096, aThread);   
}



/////////////////////////////////////////////////////////////////////////////////////////
// Functions

// handleThread is a friend function of GameThread(), hence it can access private
// GameThread members.
void handleThread(void* vArg)
{
   GameThread* gt = static_cast<GameThread*>(vArg);

   // Have to call randomize() once for each thread in BC5, apparently.
   myRandomize();

   gt->gs->enterCritical();
   gt->gn->bHasThread = true;

   // Run the thread, until it either exits naturally or gets interrupted (whether
   // intentionally by the server, or via a player hang-up)
   try
      {
      gt->run();
      }
   catch ( GameThread::ThreadException )
      {
      // No need to actually do anything here; we simply needed to break
      // out of run().
      }

   gt->cleanup();
   gt->gn->bHasThread = false;
   gt->gs->leaveCritical();
   delete gt;
   _endthread();
}


// Starting point for the round-ticker thread
void startRoundThread(void* gsVoidServer)
{
   GameServer* gsServer = static_cast<GameServer*>(gsVoidServer);

   // Have to call randomize() once for each thread in BC5, apparently.
   myRandomize();
   
   while ( gsServer->nPlayersInGame > 0 )
      {
      Sleep(ROUND_TIME);
      gsServer->enterCritical();
      gsServer->doorRound( time(NULL) );
      gsServer->leaveCritical();
      }
}


// Returns the length of a closed file, or 0 if invalid filename.
long getFileLength(char* filespec)
{
   struct ffblk f;

   if(findfirst(filespec,&f,0)==NULL)
      return(f.ff_fsize);
   return(0);
}


// Deletes a file with the given name.  Wildcards are supported.
void myDeleteFile(char* szName)
{
   WIN32_FIND_DATA wfdFound;
   HANDLE hdHandle;
   short nTries = 0;
   
   hdHandle = FindFirstFile(szName, &wfdFound);

   if (hdHandle == INVALID_HANDLE_VALUE)
      return;

   DeleteFile(wfdFound.cFileName);
 
   while ( FindNextFile(hdHandle, &wfdFound) != FALSE && nTries < 100 )
      {
      DeleteFile(wfdFound.cFileName);
      nTries++;
      }

   FindClose(hdHandle);
}


// Copies a file.  Wildcards are not supported.
void myCopyFile(char* szSource, char* szDestination, BOOL bFailIfThere)
{
   CopyFile(szSource, szDestination, bFailIfThere);   
}


// Returns a random number between nMin and nMax.  Be sure to call myRandomize()
// before using *in a given thread*.
short dice(short nMin, short nMax)
{
   int n;

   if ( nMin >= nMax )
      return nMin;

   n = (random(nMax - nMin + 1)) + nMin;

   return (short(n));
}


// Seeds the random number generator, and discards first few rolls.
void myRandomize()
{
   randomize();
   random(33);
   random(100);
   random(1234);
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


// Function called when new fails
void badNew()
{
   ExitProcess(0);
}






















