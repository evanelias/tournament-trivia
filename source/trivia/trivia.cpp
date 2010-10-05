#include <stdio.h>
#include <fstream.h>
#include "e:\doors\intrnode\gamesrv.h"
#include "source\trivia.h"

GameSettings GameSettings::info;

TriviaServer* gsGame;


void main()
{
   gsGame = new TriviaServer;
  
   gsGame->initDatabase();

   // If game maintenance should be run, do so
   if ( GameSettings::info.checkMaint() )
      GameSettings::info.maint();
      
   gsGame->run();

   // Save game data
   GameSettings::save();
}



/////////////////////////////////////////////////////////////////////////////////////////
// TriviaServer methods


TriviaServer::TriviaServer()
{
   char* szNone = "none";

   strcpy(szQuestion, szNone);
   strcpy(szAnswer, szNone);
   strcpy(szClue, szNone);
   strcpy(szDB[0], "database.enc");
   nClueNumber = 0;
   nStartTime = 0;
   nNodeInEditor = -1;
   tqDatabase = NULL;
   nDatabaseSize = 0;
   nSkipRequests = 0;
   
   nTrackLine[0] = -1;
   nTrackLine[1] = -1;
   nTrackFile[0] = -1;
   nTrackFile[1] = -1;

   for ( short n = 1; n < MAX_TRIVIA_FILES; n++ )
      strcpy(szDB[n], szNone);

   // If custom.tx but no custom.txt (ie, on fresh install), copy custom.tx
   // to custom.txt.  This mechanism prevents custom.txt from being over-written
   // upon upgrading the game.
	if ( getFileLength("custom.txt") <= 0 && getFileLength("custom.tx") > 0 )
      myCopyFile("custom.tx", "custom.txt", FALSE);
   if ( getFileLength("custom.tx") > 0 )
      myDeleteFile("custom.tx");
   
   Command::init(this);
}


// Loads (or re-loads) the question files.
void TriviaServer::initDatabase()
{
   unsigned short nCurCount = 0;
   bool bEncoded;

   // If re-loading database, delete old one from memory
   nDatabaseSize = 0;
   if ( tqDatabase != NULL )
      {
      delete[] tqDatabase;
      tqDatabase = NULL;
      }
   
   for ( short n = 0; n < MAX_TRIVIA_FILES; n++ )
      {
      if ( strlen(GameSettings::info.szExtraFiles[n]) > 0 )
         nCurCount += TriviaQuestion::countInFile(GameSettings::info.szExtraFiles[n]);
      }

   if ( !gsGame->checkReg() && nCurCount > (myReg.getMaxCore() + myReg.getMaxExtra()) )
      nCurCount = myReg.getMaxCore() + myReg.getMaxExtra() + 5;
      
   tqDatabase = new TriviaQuestion[nCurCount];

   for ( short n = 0; n < MAX_TRIVIA_FILES; n++ )
      {
      if ( strlen(GameSettings::info.szExtraFiles[n]) > 0 )
         {
         bEncoded = false;
         if ( strstr(GameSettings::info.szExtraFiles[n], ".enc") != NULL )
            bEncoded = true;
         
         indexQuestions(GameSettings::info.szExtraFiles[n], n, bEncoded);
         }
      }
}


// Parses through all questions in a given question file, and adds them to the question
// index array.
void TriviaServer::indexQuestions(char* szFile, unsigned char nFileCode, bool bEncoded)
{
   long lQuestionPos;
   char szQuestion[241], szAnswer[41];
   short nMaxRemaining = -1;
   short nLineCount = 1;
   ifstream ifsDataFile;

	if ( szFile == NULL )
   	return;

   if ( nFileCode >= MAX_TRIVIA_FILES )
      nFileCode = MAX_TRIVIA_FILES - 1;

   // There is a question max per file in unreg'd version.
   if ( !gsGame->checkReg() )
      {
      if ( strcmpi(szFile, "database.enc") == 0 )
         nMaxRemaining = myReg.getMaxCore();
      else
         {
         nMaxRemaining = myReg.getMaxCore() + myReg.getMaxExtra() - nDatabaseSize;
         if ( nMaxRemaining < 0 )
            return;
         }
      }


   // Read the correct question locations into the database.
   ifsDataFile.open(szFile);
   strcpy(szDB[nFileCode], szFile);

   while ( ifsDataFile && nMaxRemaining != 0 )
      {
      lQuestionPos = ifsDataFile.tellg();

      ifsDataFile.getline(szQuestion, 160, '\n'); //--> 161?!?
      if ( !ifsDataFile || strlen(szQuestion) < 3 )
         break;
      
      ifsDataFile.getline(szAnswer, 80, '\n');
      if ( !ifsDataFile || strlen(szAnswer) < 1 )
         break;

      tqDatabase[nDatabaseSize++].setValue(nFileCode, lQuestionPos, bEncoded, nLineCount);
      nMaxRemaining--;
      nLineCount += 2;
      }

   ifsDataFile.close();
}


// Method for creating a new node
void TriviaServer::addNode(short nNode, char* szDoorUserInfo)
{
   // When new player enters the game, re-check for reg file.
   // If found, reload question database.
   if ( myReg.load() )
      initDatabase();

   gNode[nNode] = new Player(nNode, szDoorUserInfo);
   GameThread::launch( EnterGameThread::factory(gNode[nNode], this) );
}


void TriviaServer::centralInput(InputData id)
{
   Player* pl = dynamic_cast<Player*>( gNode[id.nFrom] );
   char szText[180];
   
   // Ignore nonstandard input messages
   if ( id.nType != IP_NORMAL )
      return;

   // No input or "display" / ".d":  Redisplay question
   if ( strlen(id.szMessage) < 1 || strcmpi(id.szMessage, "display") == 0 || strcmpi(id.szMessage, ".d") == 0 )
      {
      displayQuestion(pl);
      return;
      }

   // Try input as answer for question; award points and assign new question if correct
   if ( checkForAnswer(id.szMessage) )
      {
      // Display a message to all players
      sprintf(szText, "\r\n>>> %s got the correct answer: %s!", pl->szAlias, strupr(szAnswer));
      gsGame->printAll(szText, CYAN);
            
      pl->awardPoints( pointValue() );
      nextQuestion();
      return;
      }

   // Handle commands here (after checking answer)
   char szMessageCopy[200];
   char *szFirstWord, *szArg;

   strcpy(szMessageCopy, id.szMessage);
   szFirstWord = strtok(szMessageCopy, " \t");
   szArg = strtok(NULL, "");
      
   Command* myCommand = Command::getCommand(szFirstWord);

   // If a valid command was found, do the effect
   if ( myCommand != NULL )
      myCommand->doEffect(szArg, pl);

   // If input was none of the above, echo it to the room
   else
      {
      sprintf(szText, "%s says: ", pl->szAlias);
      printAll(szText, GREEN, 0);
      printAll(id.szMessage, LWHITE);
      }
}


// Method run once per second by server
void TriviaServer::doorRound(time_t nRound)
{
   time_t nLastClue;
   time_t nQuestionFreq = GameSettings::info.nQuestionFrequency;
   time_t nClueFreq = GameSettings::info.nClueFrequency;

   /* Faster gameplay in single-player mode -- REMOVED 
   if ( nPlayersInGame <= 1 )
      {
      nQuestionFreq *= 2;
      nQuestionFreq /= 3;
      nClueFreq *= 2;
      nClueFreq /= 3;
      }
   */
   
   // If start time is still 0, no player has seen a question yet.  Hence, wait until a
   // player sees a question (set in addNode after player finishes logging in) before
   // starting to rattle off new questions.
   if ( nStartTime == 0 )
      return;
   
   //--> fix clock changes here.

   // Check if need next question
   if ( nRound - nStartTime >= nQuestionFreq )
      nextQuestion();

   // Check if new clue needed.  If a clue is actually added, re-display the question.
   nLastClue = nStartTime + (nClueFreq * nClueNumber);
   if ( nRound - nLastClue >= nClueFreq )
      {
      nextClue();
      if ( nClueNumber <= GameSettings::info.nMaxClues )
         displayQuestion();
      }

   // Save scores once per minute.
   if ( nRound % 60 == 0 )
      savePlayers();
   
   // Note: maint cannot be checked for in doorRound().  Maint requires that no players are in the game,
   // since it deletes the score file, invalidating the loaded players' file-location vars.
}


void TriviaServer::nextQuestion()
{
   short nTries = 0, nRandom;
   bool bFirstQuestion = false;
   Player* pl;
   
   // If question timer is still 0, this is the first question.  (This is used for displaying
   // the question differently)
   if ( nStartTime == 0 )
      bFirstQuestion = true;
   
   // Get a random question; try to get one that hasn't been used yet this session.
   do
      {
      nRandom = dice(0, nDatabaseSize-1);
      }
   while ( tqDatabase[nRandom].bUsed && ++nTries < 5 );

   // Set the question and answer strings appropriately; mark this question as used.
   tqDatabase[nRandom].getStrings(szQuestion, szAnswer);
   tqDatabase[nRandom].bUsed = true;

   // Track this question and previous question.
   nTrackFile[0] = nTrackFile[1];
   nTrackLine[0] = nTrackLine[1];
   nTrackFile[1] = tqDatabase[nRandom].getFileCode();
   nTrackLine[1] = tqDatabase[nRandom].getLine();
   
   // Fix clue string
   nClueNumber = 0;
   strcpy(szClue, szAnswer);
   for ( short n = 0; n < strlen(szClue); n++ )
      {
      if ( isalnum(szClue[n]) != 0 )
         szClue[n] = CLUE_CHAR;
      }
   
   nStartTime = time(NULL);
   nSkipRequests = 0;

   for ( short n = 0; n < MAX_NODE; n++ )
      {
      if ( gNode[n] != NULL )
         {
         pl = dynamic_cast<Player*>(gNode[n]);
         pl->bWantedSkip = false;
         }
      }
   
   // Display the question to all players. (2nd arg indicates it's a new question; display as such)
   displayQuestion(NULL, true, bFirstQuestion);
}


// Gives next clue, revealing new letters.
void TriviaServer::nextClue()
{
   short nWordLocation[10], nWordCounter = 1, nLettersRevealed;
   short nNotYetRevealed = 0, nAlreadyRevealed = 0;
   
   // Increment the clue number.
   nClueNumber++;

   // If already given away all clues, skip this clue.
   if ( nClueNumber > GameSettings::info.nMaxClues )
      return;

   // If more than half of the letters are already revealed, skip this clue.
   for ( short n = 0; n < strlen(szClue); n++ )
      {
      if ( szClue[n] == CLUE_CHAR )
         nNotYetRevealed++;
      if ( isalnum(szClue[n]) != 0 )
         nAlreadyRevealed++;
      }

   if ( nAlreadyRevealed > nNotYetRevealed )
      return;

   // Figure out how many letters to reveal.  Will either be equal to clue # (ie, 3 letters
   // for clue 3), OR nNotYetRevealed/5, whichever is smaller.
   nLettersRevealed = nClueNumber;
   if ( nLettersRevealed > nNotYetRevealed / 5 )
      nLettersRevealed = nNotYetRevealed / 5;
   if ( nLettersRevealed == 0 && nAlreadyRevealed == 0 && nNotYetRevealed > 1 )
      nLettersRevealed = 1;
      
   // Find the location of each word in the clue.  (Note: skip last character; easier than checking if
   // found space is the last character each time)
   nWordLocation[0] = 0;
   for ( short n = 0; n < strlen(szAnswer) - 1; n++ )
      {
      if ( !isalnum(szAnswer[n]) && nWordCounter < 10 )
         nWordLocation[nWordCounter++] = n+1;
      }

   // Give the clues
   for ( short n = 0; n < nLettersRevealed; n++ )
      {
      // Start w/ first letter of a random word; or, if only one word, a random letter.
      short nCurLetter;
      if ( nWordCounter > 1 )
         nCurLetter = nWordLocation[ dice(0, nWordCounter-1) ];
      else
         nCurLetter = dice(0, strlen(szClue)-1);
      
      // Find a letter that hasn't been revealed yet in this word, if possible.
      while ( szClue[nCurLetter] != CLUE_CHAR && nCurLetter < strlen(szClue)-1 )
         {
         nCurLetter++;
         }
      
      // Reveal the letter.
      szClue[nCurLetter] = szAnswer[nCurLetter];
      }
}


// Displays the question to the given node.  If the parameter is NULL, displays
// the question to all nodes instead.
void TriviaServer::displayQuestion(GameNode* pl, bool bNewQuestion, bool bFirstQuestion)
{
   // If no question yet, get one and return.  (Returning is a must, since
   // otherwise, this player would see the question twice)
   if ( nStartTime == 0 )
      {
      nextQuestion();
      return;
      }
      
   if ( bNewQuestion )
      {
      if ( bFirstQuestion )
         printAll("\r\n\r\n* * *  FIRST QUESTION * * *", YELLOW);
      else
         printAll("\r\n\r\n* * *  NEW QUESTION * * *", YELLOW);
      }

   char* szClueNums[3] = { "First ", "Second ", "Third " };
   char szClueHeader[20], szClueText[50];
   szClueHeader[0] = '\0';

   if ( nClueNumber >= 1 && nClueNumber <= 3 )
      strcpy(szClueHeader, szClueNums[nClueNumber-1]);
   if ( nClueNumber >= GameSettings::info.nMaxClues && GameSettings::info.nMaxClues > 1 )
      strcpy(szClueHeader, "Last ");
   strcat(szClueHeader, "Clue:");
   sprintf(szClueText, "%-16s", szClueHeader);
      
   if ( pl != NULL )
      {      
      pl->print("\r\nQuestion:       ", WHITE, 0);
      pl->printWordWrap(szQuestion, LWHITE, 1, 16, true);
      pl->print(szClueText, WHITE, 0);
      pl->print(szClue, LBLUE);
      }
   else
      {
      printAll("\r\nQuestion:       ", WHITE, 0);
      printAllWordWrap(szQuestion, LWHITE, 1, 16, true);
      printAll(szClueText, WHITE, 0);
      printAll(szClue, LBLUE);
      }
}


void TriviaServer::savePlayers()
{
   Player* pl;

   for ( int n = 0; n < MAX_NODE; n++ )
      {
      if ( gNode[n] != NULL && gNode[n]->bInGame )
         {
         pl = dynamic_cast<Player*>(gNode[n]);
         pl->save();
         }
      }
}



void TriviaServer::listOnlinePlayers(GameNode* pl)
{
   char szText[200];
   bool bFirstPlayer = true;

   pl->print("Players on-line:  ", GREEN, 0);
   szText[0] = '\0';

   for ( short n = 0; n < MAX_NODE; n++ )
      {
      if ( gNode[n] != NULL && gNode[n]->bInGame && strlen(szText)+strlen(gNode[n]->szAlias) < 197 )
         {
         if ( !bFirstPlayer )
            strcat(szText, ", ");
         strcat(szText, gNode[n]->szAlias);
         bFirstPlayer = false;
         }
      }

   pl->print(szText, MAGENTA);
   
}


// Checks if supplied string is the correct answer to the question.
// If so, returns true.  Otherwise, reveals correct letters and then
// returns false.
bool TriviaServer::checkForAnswer(char* szTry)
{
	if ( szTry == NULL )
   	return false;
   // If answer is correct
   if ( strcmpi(szTry, szAnswer) == 0 )
      return true;

   // Otherwise, answer is false, handle accordingly by revealing letters from beginning of word
   short nMaxChar = strlen(szTry);
   if ( nMaxChar > strlen(szAnswer) )
      nMaxChar = strlen(szAnswer);
   
   for ( short n = 0; n < nMaxChar; n++ )
      {
      if ( tolower(szAnswer[n]) == tolower(szTry[n]) )
         szClue[n] = szAnswer[n];
      else
         return false;
      }
   
   return false;
}


// Returns point value of current question.  Point value depends on number of
// clues given out so far.
short TriviaServer::pointValue()
{
   short nPointValue = 3;

   nPointValue -= nClueNumber;
   
   if ( nPlayersInGame < 2 || nPointValue < 1 )
      return 1;

   return nPointValue;
}


// Returns the file name of the given database codenum.  Used in TriviaQuestion
// to load the actual question text.
char* TriviaServer::getDBName(short nFileCode)
{
   if ( nFileCode < 0 || nFileCode > MAX_TRIVIA_FILES )
      nFileCode = 0;

   if ( strlen(szDB[nFileCode]) < 1 )
      return "database.enc";
      
   return szDB[nFileCode];
}


// Use to get information on either the current question (nWhich of 1) or the previous question
// (nWhich of 0).  Puts the question's file name into szBuffer, and also returns the line number
// of the question.   This function is mainly used by the CORRECTION command to provide the
// user with where-to-send-corrections info.
short TriviaServer::getQuestionInfo(char* szBuffer, short nWhich)
{
   if ( nWhich < 0 || nWhich > 1 )
      return 0;

   strcpy(szBuffer, getDBName(nTrackFile[nWhich]));
   return (nTrackLine[nWhich]);
}


// Returns the Time that this question was chosen.  Used by the score-ranking function
// to determine if a new question has elapsed since the last time scores were tabulated.
// This way, scores are only re-ranked if it's possible that someone's score went up.
short TriviaServer::getStartTime()
{
   return nStartTime;
}


unsigned short TriviaServer::getDatabaseSize()
{
   return nDatabaseSize;
}


// Returns the current score of an on-line player, or -1 if no player of the given name
// is on-line.  Used by the score-ranking function to obtain up-to-date scores for on-line
// players.
short TriviaServer::getCurrentScore(char* szPlayerName)
{
   Player* pl;

	if ( szPlayerName == NULL )
   	return -1;

   for ( short n = 0; n < MAX_NODE; n++ )
      {
      if ( gNode[n] != NULL && gNode[n]->bInGame )
         {
         if ( strcmpi(gNode[n]->szRealName, szPlayerName) == 0 )
            {
            pl = dynamic_cast<Player*>(gNode[n]);
            return pl->getScore();
            }
         }
      }

   return -1;
}


RegInfo TriviaServer::getReg()
{
   return myReg;
}

// Run to notify server that a new player has requested this question be skipped.
// If 2+ players in game, and all players have requested to skip this question, 
// it is immediately skipped.  If only 1 player in the game, the current question
// is simply sped-up to half its normal wait time.
void TriviaServer::requestSkip()
{
   if ( nPlayersInGame == 1 )
      {
      nStartTime -= GameSettings::info.nQuestionFrequency / 2;
      nClueNumber = (time(NULL) - nStartTime) / GameSettings::info.nClueFrequency;
      return;
      }
      
   if ( ++nSkipRequests >= nPlayersInGame )
      nextQuestion();
}


// Displays high scores to the given player.
void TriviaServer::displayScores(GameNode* pl)
{
   PlayerRecord* myRecords = PlayerRecord::getRankedRecords();

   char szText[80];
   bool bOneColumn = false;
   pl->newline();
   pl->underline("THIS MONTH'S HIGH SCORES", "Ä", LCYAN, BLUE, true);

   if ( myRecords[5].isEmpty() )
      bOneColumn = true;
   
   for ( int n = 0; n < 5; n++ )
      {
      szText[n] = '\0';

      if ( myRecords[n].isEmpty() )
         sprintf(szText, " %d. %-18s [     points]", n+1, " ");
      else
         {
         if ( strlen(myRecords[n].szAlias) > 18 )
            myRecords[n].szAlias[18] = '\0';
         sprintf(szText, " %d. %-18s [%4d points]", n+1, myRecords[n].szAlias, myRecords[n].nScore);
         }

      if ( bOneColumn )
         {
         pl->center(szText, CYAN);
         continue;
         }
         
      strcat(szText, "    ");
      pl->print(szText, CYAN, 0);
      pl->print("³ ", BLUE, 0);

      if ( myRecords[n+5].isEmpty() )
         sprintf(szText, "%2d. %-18s [     points] ", n+6, " ");
      else
         {
         if ( strlen(myRecords[n+5].szAlias) > 18 )
            myRecords[n+5].szAlias[18] = '\0';
         sprintf(szText, "%2d. %-18s [%4d points] ", n+6, myRecords[n+5].szAlias, myRecords[n+5].nScore);
         }
      pl->print(szText, CYAN);
      }

   pl->newline();
}


bool TriviaServer::checkReg()
{
   if ( strlen(myReg.getRegName()) > 0 )
      return true;
   return false;
}


/////////////////////////////////////////////////////////////////////////////////////////
// TriviaQuestion methods

TriviaQuestion::TriviaQuestion()
{
   nFileCode = 0;
   lLocationInFile = 0;
   bUsed = false;
   bEncoded = false;
   nLine = 0;
}

void TriviaQuestion::setValue(unsigned char nFile, long lPos, bool bIsEncoded, short nMyLine)
{
   nFileCode = nFile;
   lLocationInFile = lPos;
   bEncoded = bIsEncoded;
   nLine = nMyLine;
}


void TriviaQuestion::getStrings(char* szQ, char* szA)
{
   ifstream ifsDataFile;

   ifsDataFile.open( gsGame->getDBName(nFileCode) );

   if ( !ifsDataFile )
      {
      MessageBox(NULL, "Unable to read from question file!", "Trivia", MB_ICONSTOP | MB_OK | MB_TASKMODAL);
      return;
      }
   
   ifsDataFile.seekg(lLocationInFile);
   ifsDataFile.getline(szQ, 160, '\n');  //--> ?
   ifsDataFile.getline(szA, 80, '\n');   //--> ?!
   ifsDataFile.close();

   if ( bEncoded )
      {
      short n, nLoopMax = strlen(szQ);

      for ( n = 0; n < nLoopMax; n++ )
         {
         if ( n % 2 == 0 )
            szQ[n] -= 1 + (n % 4);
         else
            szQ[n] -= 2;
         }

      nLoopMax = strlen(szA);
      for ( n = 0; n < nLoopMax; n++ )
         {
         if ( n % 2 == 0 )
            szA[n] -= 1 + (n%4);
         else
            szA[n] -= 2;
         }
      }

   // Remove trailing spaces from answer
   while ( szA[ strlen(szA) - 1] == ' ' )
      {
      szA[ strlen(szA) - 1] = '\0';
      }
}


short TriviaQuestion::getFileCode()
{
   return nFileCode;
}

short TriviaQuestion::getLine()
{
   return nLine;
}


unsigned short TriviaQuestion::countInFile(char* szFile)
{
   unsigned short nCurCount = 0;
   ifstream ifsDataFile;

	if ( szFile == NULL )
   	return 0;

   ifsDataFile.open(szFile);

   while ( ifsDataFile )
      {
      ifsDataFile.ignore(160, '\n'); //--> ?!

      if ( ifsDataFile )
         {
         ifsDataFile.ignore(80, '\n');
         if ( ifsDataFile )
            nCurCount++;
         }
      }

   ifsDataFile.close();

   if ( !gsGame->checkReg() )
      {
      if ( strcmpi(szFile, "database.enc") == 0 )
         {
         if ( nCurCount > gsGame->getReg().getMaxCore() )
            nCurCount = gsGame->getReg().getMaxCore();
         }
      }

   return nCurCount;
}



/////////////////////////////////////////////////////////////////////////////////////////
// GameSettings methods

GameSettings::GameSettings()
{
   ifstream ifsSettingsFile;
   char* szText;
   char szFullDate[12];
   
   // Set current month
   _strdate(szFullDate);
   szText = strtok(szFullDate, "/");
   nCurMonth = atoi(szText);

   // No previous winner
   strcpy(szPreviousWinner, "none");
   nPreviousHighScore = 0;

   // Default sysop config settings
   nMaxClues = 3;
   nClueFrequency = 12;
   nQuestionFrequency = 50;
   bVerifySubmissions = true;
   bListSysops = true;
   nPlayerTimeout = 300;

   // Default question files
   strcpy(szExtraFiles[0], "database.enc");
   strcpy(szExtraFiles[1], "custom.txt");

   for ( short n = 2; n < MAX_TRIVIA_FILES; n++ )
      {
      szExtraFiles[n][0] = '\0';
      }
  
   ifsSettingsFile.open("settings.dat", ios::in | ios::binary);
   if ( ifsSettingsFile )
      {
      ifsSettingsFile.read( (char*)this, sizeof(GameSettings) );
      ifsSettingsFile.close();
      }
}

void GameSettings::save()
{
   ofstream ofsSettingsFile;

   ofsSettingsFile.open("settings.dat", ios::out | ios::binary | ios::trunc);
   if ( ofsSettingsFile )
      {
      ofsSettingsFile.write( (char*)&info, sizeof(GameSettings) );
      ofsSettingsFile.flush();
      ofsSettingsFile.close();
      }
}


// Performs game maintenance.  Run once per month.
void GameSettings::maint()
{
   char* szText;
   char szFullDate[10];
   
   // Set current month
   _strdate(szFullDate);
   szText = strtok(szFullDate, "/");
   info.nCurMonth = atoi(szText);

   PlayerRecord* myRecords = PlayerRecord::getRankedRecords();

   // If no score this month
   if ( myRecords[0].isEmpty() )
      {
      strcpy(info.szPreviousWinner, "none");
      info.nPreviousHighScore = 0;
      }

   // Otherwise, save high score and delete player file & clear rankings.
   else
      {
      strcpy(info.szPreviousWinner, strupr(myRecords[0].szAlias));
      info.nPreviousHighScore = myRecords[0].nScore;
      myDeleteFile("player.dat");

      for ( int n = 0; n < 10; n++ )
         {
         myRecords[n].szName[0] = '\0';
         myRecords[n].szAlias[0] = '\0';
         myRecords[n].nScore = 0;
         }
      }
}


// Returns true if maint should be run, otherwise returns false.
bool GameSettings::checkMaint()
{
   char* szText;
   char szFullDate[12];
   
   // Check current month
   _strdate(szFullDate);
   szText = strtok(szFullDate, "/");

   if ( info.nCurMonth != atoi(szText) )
      return true;
   else
      return false;
}

   
