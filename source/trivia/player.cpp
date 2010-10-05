#include <stdio.h>
#include <fstream.h>
#include "e:\doors\intrnode\gamesrv.h"
#include "source\trivia.h"

PlayerRecord PlayerRecord::rankedRecords[10];
extern TriviaServer* gsGame;


/////////////////////////////////////////////////////////////////////////////////////////
// Player methods

// Player constructor, supplied with node info.  Loads player data from file.
Player::Player(short nNode, char* szInfo) : GameNode(nNode, szInfo)
{
   short nPlayerCounter = 0;
   bool bFound = false;
   fstream fsPlayerFile;
   PlayerRecord aRecord;
   
   nScore = 0;
   nPlayerNumber = -1;
   bWantedSkip = false;

   fsPlayerFile.open("player.dat", ios::in | ios::out | ios::binary | ios::ate );
   fsPlayerFile.seekg(0);
   
   // Try to find the player in the data file
   while ( fsPlayerFile )
      {
      fsPlayerFile.read( (char*)&aRecord, sizeof(PlayerRecord) );
      if ( !fsPlayerFile )
         break;
            
      // If real name matches, grab their score and any other stats
      if ( strcmpi(aRecord.szName, szRealName) == 0 )
         {
         bFound = true;
         nScore = aRecord.nScore;
         nPlayerNumber = nPlayerCounter;

         // If non-XSDK version, make user a sysop if saved as such.
         // (In XSDK version, user must currently be 90+ security to be sysop in game)
         if ( aRecord.bSavedSysop && nPlatform != PL_XSDK32 )
            bSysop = true;
         break;
         }

      nPlayerCounter++;
      }

   // If new player
   if ( !bFound )
      {
      nPlayerNumber = nPlayerCounter;
      strcpy(aRecord.szName, szRealName);
      strcpy(aRecord.szAlias, szAlias);
      aRecord.nScore = 0;
      aRecord.bSavedSysop = bSysop;
      fsPlayerFile.clear();
      fsPlayerFile.seekp( 0, ios::end );
      fsPlayerFile.write( (char*)&aRecord, sizeof(PlayerRecord) );
      fsPlayerFile.flush();
      }

   fsPlayerFile.close();
}


// Destructor:  Save the player to disk, if it is valid.
Player::~Player()
{
   char szText[80];
   
   if ( nPlayerNumber < 0 )
      return;
   
   save();
   bInGame = false;
   sprintf(szText, ">>> %s has left the game!", szAlias);
   gsGame->printAll(szText, MAGENTA);   
   
}


// Writes player score to file
void Player::save()
{
	ofstream ofsPlayerFile;

   if ( nPlayerNumber < 0 )
      return;

   PlayerRecord myRecord(szRealName, szAlias, nScore, bSysop);
	ofsPlayerFile.open("player.dat", ios::out | ios::binary | ios::ate);

   if ( !ofsPlayerFile )
      {
      MessageBox(NULL, "Unable to write to player file!", "Trivia", MB_ICONSTOP | MB_OK | MB_TASKMODAL);
      return;
      }

   ofsPlayerFile.seekp(nPlayerNumber * sizeof(PlayerRecord));
   ofsPlayerFile.write( (char*)&myRecord, sizeof(PlayerRecord) );
   ofsPlayerFile.flush();
   ofsPlayerFile.close();
}

// Fills an output message with the player's current stats, so that the player's
// statline prompt is always up to date.  This is a required GameUser class
// function.
void Player::fillStats(OutputData* od)
{
   od->nHp = nScore;
   od->nSp = -1;
   od->nMf = -1;
   od->nEnemyPercent = -1;
   od->nHpColor = LWHITE;
}


// Gives the player points
void Player::awardPoints(short nAmount)
{
   nScore += nAmount;
}


// Returns the player's score
short Player::getScore()
{
   return nScore;
}


/////////////////////////////////////////////////////////////////////////////////////////

PlayerRecord::PlayerRecord()
{
   szName[0] = '\0';
   szAlias[0] = '\0';
   nScore = 0;
   bSavedSysop = false;
}


PlayerRecord::PlayerRecord(char* aName, char* anAlias, short aScore, bool bIsSysop)
{
   strcpy(szName, aName);
   strcpy(szAlias, anAlias);
   nScore = aScore;
   bSavedSysop = bIsSysop;
}


bool PlayerRecord::isEmpty()
{
   if ( strlen(szName) < 1 || nScore == 0 )
      return true;
   return false;
}


// Returns an array of the top 10 player rankings.  Re-computes rankings as needed.
PlayerRecord* PlayerRecord::getRankedRecords()
{
   static time_t lastCalled = 1;
   PlayerRecord* gameRecords;
   short n = 0, nCurrentScore;
   ifstream fsPlayerFile;

   long lFileLength = getFileLength("player.dat");
   short nEntries = short(lFileLength / sizeof(PlayerRecord));

   // Only re-rank if the question has changed since the last ranking.
   if ( lastCalled == gsGame->getStartTime() )
      return rankedRecords;
   else
      lastCalled = gsGame->getStartTime();

   
   if ( nEntries <= 0 )
      return rankedRecords;
   
   gameRecords = new PlayerRecord[nEntries];

   fsPlayerFile.open("player.dat", ios::in | ios::binary );
   fsPlayerFile.seekg(0);
   
   // Get info on all players in the player file.
   while ( fsPlayerFile )
      {
      fsPlayerFile.read((char*)&gameRecords[n], sizeof(PlayerRecord));
      if ( !fsPlayerFile )
         break;

      if (gameRecords[n].bSavedSysop && !GameSettings::info.bListSysops)
         {
         gameRecords[n].nScore = 0;
         nEntries--;
         continue;
         }
         
      nCurrentScore = gsGame->getCurrentScore(gameRecords[n].szName);
      if ( nCurrentScore > gameRecords[n].nScore )
         gameRecords[n].nScore = nCurrentScore;
      
      if ( gameRecords[n].nScore == 0 )
         nEntries--;
      else
         n++;
      }
   fsPlayerFile.close();

   // Rank the players using a selection-sort.
   short nRankedCount = 0, nTopPoints, nCurHigh;
   
   do
      {
      nTopPoints = 0;
      nCurHigh = -1;
      
      // Select the top score remaining in the list.
      for ( n = 0; n < nEntries; n++ )
         {
         if ( gameRecords[n].nScore > nTopPoints )
            {
            nCurHigh = n;
            nTopPoints = gameRecords[n].nScore;
            }
         }

      // Move the top score over to the sorted-list.
      if ( nCurHigh > -1 )
         {
         rankedRecords[nRankedCount++] = gameRecords[nCurHigh];
         gameRecords[nCurHigh].nScore = 0;
         }
      }
   while ( nCurHigh > -1 && nRankedCount < 10 );

   for ( n = nRankedCount; n < 10; n++ )
      {
      rankedRecords[n].szName[0] = '\0';
      }
   
   delete[] gameRecords;
   return rankedRecords;
}

