#include <stdio.h>
#include <process.h>
#include "e:\doors\intrnode\gamesrv.h"
#include "source\trivia.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TriviaThread

TriviaThread::TriviaThread(GameNode* aNode, GameServer* aServer) : GameThread(aNode, aServer)
{
   pl = dynamic_cast<Player*>(gn);
   myServer = dynamic_cast<TriviaServer*>(aServer);
}



///////////////////////////////////////////////////////////////////////////////////////////////
// EnterGameThread

EnterGameThread::EnterGameThread(GameNode* aNode, GameServer* aServer) : TriviaThread(aNode, aServer)
{
} 


GameThread* EnterGameThread::factory(GameNode* aNode, GameServer* aServer)
{
   return new EnterGameThread(aNode, aServer);
}

void EnterGameThread::run()
{
   char szText[120], szFor[20], szOwner[80];
   
   pl->clearScreen();
   sprintf(szText, "TOURNAMENT TRIVIA v%s", DOOR_VERSION);
   pl->textBox(szText, LCYAN|(BLUE<<4), YELLOW|(BLUE<<4), true);

   sprintf(szText, "%s   http://trivia.doormud.com", DOOR_COPYRIGHT);
   pl->center(szText, LWHITE);
   if ( pl->nPlatform == PL_XSDK32 )
      sprintf(szFor, "Synchronet");
   if ( pl->nPlatform == PL_OPENDOORS32 )
      sprintf(szFor, "Door32");
   sprintf(szText, "%s build, compiled on %s", szFor, DOOR_COMPILE);
   pl->center(szText, LWHITE);

   itoa(myServer->getDatabaseSize(), szText, 10);
   strcpy(szFor, "--");
   if ( (strlen(myServer->getReg().getRegName()) + strlen(szText)) % 2 == 0 )
      strcpy(szFor, "-");

   if ( myServer->checkReg() )
      {
      sprintf(szOwner, "Registered to %s", myServer->getReg().getRegName());
      if ( strstr(szOwner, myServer->getReg().getRegName()) == NULL )
         exit(0);
      }
   else
      strcpy(szOwner, "Unregistered Version");
      
   sprintf(szText, "%s %s %u questions available", szOwner, szFor, myServer->getDatabaseSize());
   pl->center(szText, WHITE,2);
   
   myServer->displayScores(pl);
   sprintf(szText, "Last month's winner was %s with %d points!", GameSettings::info.szPreviousWinner, GameSettings::info.nPreviousHighScore);  
   if ( strcmpi(GameSettings::info.szPreviousWinner, "none") != 0 )
      pl->center(szText, LCYAN, 2);
   pl->newline();
      
   if ( pl->getScore() == 0 )
      {
      getStr(szText, "Would you like instructions on how to play?  [y/n]: ");

      if ( szText[0] == 'y' || szText[0] == 'Y' )
         {
         pl->displayHlp("user.hlp", "instructions");
         pause();
         }
      }

   else
      {
      // Pause the thread; print pause message centered.
      pause(true);
      }
 
   // Game entrance notification
   sprintf(szText, ">>> %s has entered the game!", pl->szAlias);
   myServer->printAll(szText, MAGENTA);   

   pl->clearScreen();
   pl->bInGame = true;
   pl->displayScreen("menu");
   pl->newline();
   if ( pl->bSysop )
      {
      if ( pl->nPlatform == PL_OPENDOORS32 )
         pl->displayHlp("user.hlp", "local");
      else
         pl->displayHlp("user.hlp", "sysop");
      }
   myServer->listOnlinePlayers(pl);
   myServer->displayQuestion(pl);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ExitPromptThread

ExitPromptThread::ExitPromptThread(GameNode* aNode, GameServer* aServer) : TriviaThread(aNode, aServer) 
{ 
}

GameThread* ExitPromptThread::factory(GameNode* aNode, GameServer* aServer)
{
   return new ExitPromptThread(aNode, aServer);
}


void ExitPromptThread::run()
{
   char szAnswer[81];

   gn->newline();
   getStr(szAnswer, "Are you sure you want to exit?  [y/n]: ");
   if ( szAnswer[0] == 'y' || szAnswer[0] == 'Y' )
      {
      gn->print("\r\nThanks for playing Tournament Trivia! ", GREEN, 2);
      if ( !myServer->checkReg() )
         {
         gn->print("\r\nThis ", LWHITE, 0);
         gn->print("UNREGISTERED ", WHITE, 0);
         gn->print("copy of the game has a limited number of questions; the full");
         gn->print("version has many more.  You can help your sysop register the game at a");
         gn->print("discounted price by contributing new trivia questions!  Please visit");
         gn->print("http://trivia.doormud.com/submit.html", WHITE, 0);
         gn->print(" for more information.", LWHITE, 2);
         gn->pausePrompt();
         }
      gn->exitGame();
      }
}                      



///////////////////////////////////////////////////////////////////////////////////////////////////
// SysopThread

SysopThread::SysopThread(GameNode* aNode, GameServer* aServer) : TriviaThread(aNode, aServer)
{
} 

GameThread* SysopThread::factory(GameNode* aNode, GameServer* aServer)
{
   return new SysopThread(aNode, aServer);
}

void SysopThread::run()
{
   char szText[90], szInput[80];
   short nNewValue, nUsedFiles;
   
   gn->bInGame = false;
   
   while ( true )
      {
      gn->clearScreen();
      gn->textBox("Sysop Config Menu", WHITE);

      sprintf(szText, "Question Frequency:           %ld seconds", GameSettings::info.nQuestionFrequency);
      gn->menuOption('F', szText);
      sprintf(szText, "Max number of clues:          %d", GameSettings::info.nMaxClues);
      gn->menuOption('M', szText);
      strcpy(szText,  "Show sysops on score list:    ");
      if ( GameSettings::info.bListSysops )
         strcat(szText, "Yes");
      else
         strcat(szText, "No");
      gn->menuOption('S', szText);
      gn->newline();
      
      for ( short n = 0; n < MAX_TRIVIA_FILES; n++ )
         {
         sprintf(szText, "Question file #%2d:            ", n+1);
         if ( strlen(GameSettings::info.szExtraFiles[n]) > 1 )
            strcat(szText, GameSettings::info.szExtraFiles[n]);
         else
            {
            if ( n > 4 && strlen(GameSettings::info.szExtraFiles[n-1]) <= 0 )
               continue;
            strcat(szText, "[none]");
            }
         if ( n < 9 )
            gn->menuOption('1'+n, szText);
         else
            gn->menuOption('0', szText);
         }
      gn->newline();
      gn->menuOption('X', "Return to game\r\n");
      
      getStr(szInput, "[Enter a selection]: ", LWHITE, 5);
      szInput[0] = tolower(szInput[0]);
      gn->newline();
   
      switch ( szInput[0] )
         {
         case 'f':
            getStr(szInput, "Enter new value, from 25 seconds to 75 seconds:  ");
            nNewValue = atoi(szInput);
            if ( nNewValue >= 25 && nNewValue <= 75 )
               {
               GameSettings::info.nQuestionFrequency = nNewValue;
               GameSettings::info.nClueFrequency = GameSettings::info.nQuestionFrequency / (GameSettings::info.nMaxClues+1);
               }
            break;
         case 'm':
            getStr(szInput, "Enter new value, from 0 clues to 4 clues:  ");
            nNewValue = atoi(szInput);
            if ( nNewValue >= 0 && nNewValue <= 4 )
               {
               GameSettings::info.nMaxClues = nNewValue;
               GameSettings::info.nClueFrequency = GameSettings::info.nQuestionFrequency / (GameSettings::info.nMaxClues+1);
               }
            break;
         case 's':
            GameSettings::info.bListSysops = !(GameSettings::info.bListSysops);
            break;
         default:
            if ( szInput[0] < '0' || szInput[0] > '9' )
               {
               gn->bInGame = true;
               return;
               }
            if ( szInput[0] == '0' )
               szInput[0] = '9' + 1;
               
            getStr(GameSettings::info.szExtraFiles[szInput[0] - '1'], "Enter new file, or ENTER for none:  ", LWHITE, 14);
            nUsedFiles = 0;
            for ( short n = 0; n < MAX_TRIVIA_FILES; n++ )
               {
               if ( strlen(GameSettings::info.szExtraFiles[n]) > 1 )
                  {
                  if ( getFileLength(GameSettings::info.szExtraFiles[n]) > 0 )
                     nUsedFiles++;
                  }
               }

            // Ensure that there's at least one valid file
            if ( nUsedFiles == 0 )
               strcpy(GameSettings::info.szExtraFiles[0], "database.enc");
        
            if ( !myServer->checkReg() && strcmpi(GameSettings::info.szExtraFiles[szInput[0] - '1'], "database.enc") != 0 )
               {
               gn->print("\r\nPlease note:", LRED);
               gn->print("- In this unregistered copy of the game, only the first 150 questions from");
               gn->print("  third-party / custom trivia files will be used.", LWHITE, 2);
               gn->pausePrompt();
               }

            // Re-init database
            myServer->initDatabase();
         }
      }
}








