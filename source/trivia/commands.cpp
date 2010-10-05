#include <stdio.h>
#include "e:\doors\intrnode\gamesrv.h"
#include "source\trivia.h"
#include "source\commands.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Static Command members

Command**      Command::myCommands;
short          Command::nMaxCommands;
TriviaServer*  Command::myServer;


/////////////////////////////////////////////////////////////////////////////////////////
// Command methods

void Command::init(TriviaServer* aServer)
{
   short nCommandCount = 0;

   myServer = aServer;
   nMaxCommands = 15;

   myCommands = new Command*[nMaxCommands];
   myCommands[nCommandCount++] = new PlayerListCommand("who");
   myCommands[nCommandCount++] = new PlayerListCommand("players", "pl");
   myCommands[nCommandCount++] = new SkipCommand("skip", "next");
   myCommands[nCommandCount++] = new HelpCommand("help");
   myCommands[nCommandCount++] = new TellCommand("whisper", "tell");
   myCommands[nCommandCount++] = new SubmitCommand("submit", "sub");
   myCommands[nCommandCount++] = new CorrectionCommand("correction", "correct");
   myCommands[nCommandCount++] = new ScoresCommand("scores", "sc");
   myCommands[nCommandCount++] = new ScoresCommand("topten", "top");
   myCommands[nCommandCount++] = new ExitCommand("exit", "x");
   myCommands[nCommandCount++] = new ExitCommand("quit");
   myCommands[nCommandCount++] = new SysopCommand("sysop");
   myCommands[nCommandCount++] = new SysopCommand("configure", "config");
   myCommands[nCommandCount++] = new MenuCommand("menu", "?");
   myCommands[nCommandCount++] = new CheckRegCommand(";checkreg");

}


Command* Command::getCommand(char* szInput)
{
	if ( szInput == NULL )
   	return NULL;
      
   for ( short n = 0; n < nMaxCommands; n++ )
      {
      if ( strcmpi(szInput, myCommands[n]->szName) == 0 )
         return myCommands[n];
      if ( strcmpi(szInput, myCommands[n]->szAbbrev) == 0 )
         return myCommands[n];
      }

   // Partial command-matching removed.
   /*
   for ( short n = 0; n < nMaxCommands; n++ )
      {
      // Search the command name for the input text
      if ( wordSearch(myCommands[n]->szName, szInput) )
         return myCommands[n];
      }
   */

   return NULL;
}


Command::Command(char* szMyName, char* szMyAbbrev)
{
   strcpy(szName, szMyName);

   if ( szMyAbbrev == NULL )
      szAbbrev[0] = '\0';
   else
      strcpy(szAbbrev, szMyAbbrev);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Command-derivative methods

#pragma argsused
void PlayerListCommand::doEffect(char* szParam, Player* pl)
{
   myServer->listOnlinePlayers(pl);
}


#pragma argsused
void SkipCommand::doEffect(char* szParam, Player* pl)
{
   // Can only request a skip once.
   if ( pl->bWantedSkip )
      {
      pl->print("You already requested that this question be skipped!", LRED);
      return;
      }

   /* Skip disabled in single-player mode -- REMOVED
   if ( myServer->nPlayersInGame < 2 )
      {
      pl->print("\r\nThe SKIP command may only be used if more than one player is in the game.", WHITE, 2);
      return;
      }
   */
   
   char szText[150];
   pl->bWantedSkip = true;
   sprintf(szText, ">>> %s has requested that this question be skipped.", pl->szAlias);
   myServer->printAll(szText, MAGENTA);
   myServer->requestSkip();
}


#pragma argsused
void HelpCommand::doEffect(char* szParam, Player* pl)
{
   pl->newline();
   if ( szParam == NULL )
      pl->displayHlp("user.hlp", "instructions");
   else
      pl->displayHlp("user.hlp", szParam, "Sorry, there is no help entry for that topic.", true);
   pl->newline();
}


// Sends a private message to a player.  This method is somewhat complex because player names can span multiple
// words, so an intelligent comparison must be done between player names and the entered parameter.
void TellCommand::doEffect(char* szIntactParam, Player* pl)
{
   char szParamCopy[240], * szUsedName, szLowerName[80], szText[240];
   char* szError = "Syntax:   TELL <user> <message>";
   short nBestMatchNode = -1, nBestMatchCount = -1, nBestDupe = -1;
   short nCurPos, nWordEnd;
   bool bDoneLoop;

   if ( szIntactParam == NULL || strlen(szIntactParam) < 2 || strchr(szIntactParam, ' ') == NULL )
      {
      pl->print(szError, LRED);
      return;
      }

   strcpy(szParamCopy, szIntactParam);
   sprintf(szText, " %s", strtok(szParamCopy, " "));
   strlwr(szText);
      
   // See which matches are best
   for ( short n = 0; n < MAX_NODE; n++ )
      {
      if ( myServer->gNode[n] == NULL || !(myServer->gNode[n]->bInGame) )
         continue;
         
      // Look at the first word of the parameter, and see if it is found at the beginning of any word in the
      // player's name.
      sprintf(szLowerName, " %s", myServer->gNode[n]->szAlias);
      strlwr(szLowerName);
      szUsedName = strstr(szLowerName, szText); 

      // If not found, continue to next user; otherwise, skip leading space in name.
      if ( szUsedName == NULL )
         continue;
      else
         szUsedName++;

      bDoneLoop = false;
      nCurPos = nWordEnd = 0;
      
      // This loop compares characters in the parameter with characters in the player's name.  Note that we
      // start at whatever position in the player's name was found to match the input, so we can skip
      // some words at the beginning of the player's name as needed.
      while ( !bDoneLoop )
         {
         if ( nCurPos >= strlen(szIntactParam) )
            {
            // error out, name match w/o message
            pl->print(szError, LRED);
            return;
            }

         // Potential perfect name match.  Ensure that current word in param doesn't continue.
         if ( nCurPos >= strlen(szUsedName) )
            {
            if ( szIntactParam[nCurPos] == ' ' )
               {
               nBestMatchNode = n;
               nBestMatchCount = nCurPos-1;
               n = MAX_NODE;
               }
            if ( szIntactParam[nCurPos] != ' ' && nWordEnd > nBestMatchCount )
               {
               nBestMatchCount = nWordEnd;
               nBestMatchNode = n;
               }

            break;
            }

         // If a non-matching character found, evaluate the situation
         if ( tolower(szIntactParam[nCurPos]) != szUsedName[nCurPos] )
            {
            // If this match is just as good as some other match, make note of it
            if ( szIntactParam[nCurPos] == ' ' && nCurPos-1 == nBestMatchCount && nBestMatchCount > -1 )
               nBestDupe = nCurPos - 1;
            
            // If new word in param, see if the current text before new word was the best name match
            if ( szIntactParam[nCurPos] == ' ' && nCurPos-1 > nBestMatchCount )
               {
               nBestMatchCount = nCurPos - 1;
               nBestMatchNode = n;
               }
                   
            // If not new word in param, backtrack to previous full matching word and see if it's the best match
            if ( szIntactParam[nCurPos] != ' ' && nWordEnd > nBestMatchCount )
               {
               nBestMatchCount = nWordEnd;
               nBestMatchNode = n;
               }

            bDoneLoop = true;
            }

         // If this is the end of a word, record it as such
         if ( !bDoneLoop && szIntactParam[nCurPos] == ' ' )
            nWordEnd = nCurPos-1;

         nCurPos++;
         }
      }

   if ( nBestMatchNode == -1 || strlen(szIntactParam) <= nBestMatchCount+2 )
      {
      pl->print(szError, LRED);
      return;
      }

   // If best match was present in multiple player names, abort
   if ( nBestDupe == nBestMatchCount ) 
      {
      pl->print("Multiple players match that name; please be more specific.", LRED);
      return;
      }
     
   sprintf(szText, "Message sent to %s.", myServer->gNode[nBestMatchNode]->szAlias);
   pl->print(szText, MAGENTA);
   sprintf(szText, "%s tells you: ", pl->szAlias);
   myServer->gNode[nBestMatchNode]->print(szText, LGREEN, 0);
   myServer->gNode[nBestMatchNode]->print(szIntactParam+nBestMatchCount+2, LWHITE);
}


#pragma argsused
void SubmitCommand::doEffect(char* szParam, Player* pl)
{
   pl->displayHlp("user.hlp", "submission");
}


#pragma argsused
void CorrectionCommand::doEffect(char* szParam, Player* pl)
{
   char szFileName[20], szText[120], szHeader[30];
   short nLine;

   for ( short n = 0; n < 2; n++ )
      {   
      nLine = myServer->getQuestionInfo(szFileName, n);

      if ( nLine > 0 )
         {
         if ( n == 0 )
            strcpy(szHeader, "PREVIOUS QUESTION");
         else
            strcpy(szHeader, "CURRENT QUESTION");

         pl->newline();
         pl->underline(szHeader, "Ä", LCYAN, BLUE);

         sprintf(szText, "  %s question %d", szFileName, nLine);
         pl->print(szText, LWHITE, 0);
         sprintf(szText, "version %s", szFileName);
         pl->displayHlp("correct.hlp", szText);
         pl->print("\r\n  Report at: ", LWHITE, 0);
         pl->displayHlp("correct.hlp", szFileName, "http://trivia.doormud.com/correct.html");
         }
      }

   pl->newline();
}


#pragma argsused
void ScoresCommand::doEffect(char* szParam, Player* pl)
{
   myServer->displayScores(pl);
}


#pragma argsused
void ExitCommand::doEffect(char* szParam, Player* pl)
{
   GameThread::launch(ExitPromptThread::factory(pl, myServer));
}


#pragma argsused
void SysopCommand::doEffect(char* szParam, Player* pl)
{
   if ( pl->bSysop )
      GameThread::launch(SysopThread::factory(pl, myServer));
   else
      pl->print("\r\nYou do not have sysop access or local-mode access.", LRED, 2);
}


#pragma argsused
void MenuCommand::doEffect(char* szParam, Player* pl)
{
  pl->newline();
  pl->displayScreen("menu");
  pl->newline();
}

#pragma argsused
void CheckRegCommand::doEffect(char* szParam, Player* pl)
{
   myServer->getReg().displayRegInfo(pl);
}







                     
