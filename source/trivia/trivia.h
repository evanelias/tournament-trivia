#ifndef TRIVIA_H_DEF
#define TRIVIA_H_DEF

#include <time.h>
#include "source\doorset.h"

#define MAX_TRIVIA_FILES 10
#define CLUE_CHAR        '.'


class Player : public GameNode
{
   private:
      short nScore;
      short nPlayerNumber;
   public:
      bool bWantedSkip;
      Player(short, char*);
      ~Player();
      void save();
      void fillStats(OutputData*);
      void awardPoints(short);
      short getScore();
};

class TriviaQuestion
{
   private:
      unsigned char nFileCode;
      long lLocationInFile;
      bool bEncoded;
      short nLine;
   public:
      bool bUsed;
      TriviaQuestion();
      void setValue(unsigned char, long, bool, short);
      void getStrings(char*, char*);
      short getFileCode();
      short getLine();
      static unsigned short countInFile(char*);
};


class RegInfo
{
   private:
      unsigned char* szHolder;
      short nLength;
      char szRegdTo[80];
      bool bInit;
      unsigned char decode(unsigned char, long);
   public:
      RegInfo();
      bool load();
      char* getRegName();
      short getMaxCore();
      short getMaxExtra();
      void displayRegInfo(Player*);
};


class TriviaServer : public GameServer
{
   private:
      char szQuestion[161];
      char szAnswer[81];
      char szClue[81];
      short nClueNumber;
      short nSkipRequests;
      time_t nStartTime;
      TriviaQuestion* tqDatabase;
      unsigned short nDatabaseSize;
      short nTrackLine[2];
      short nTrackFile[2];
      char szDB[MAX_TRIVIA_FILES][15];
      RegInfo myReg;
      void indexQuestions(char*, unsigned char, bool=false);
      void addNode(short, char*);
      void centralInput(InputData id);
      void doorRound(time_t);
      void nextQuestion();
      void nextClue();
      bool checkForAnswer(char*);
      short pointValue();
      void savePlayers();
   public:
      TriviaServer();
      void initDatabase();
      void displayQuestion(GameNode* = NULL, bool=false, bool=false);
      void listOnlinePlayers(GameNode*);
      short nNodeInEditor;
      char* getDBName(short);
      short getQuestionInfo(char*, short=1);
      short getStartTime();
      unsigned short getDatabaseSize();
      short getCurrentScore(char*);
      RegInfo getReg();
      void requestSkip();
      void displayScores(GameNode*);
      bool checkReg();
};

class GameSettings
{
   public:
      short nCurMonth;
      short nDifficulty;
      char szPreviousWinner[60];
      short nPreviousHighScore;
      short nMaxClues;
      time_t nClueFrequency;
      time_t nQuestionFrequency;
      bool bVerifySubmissions;
      char szExtraFiles[MAX_TRIVIA_FILES][21];
      bool bListSysops;
      short nPlayerTimeout;
      static GameSettings info;
      GameSettings();
      static void save();
      static void maint();
      static bool checkMaint();
};


class Command
{
   private:
      char szName[20];
      char szAbbrev[10];
      static Command** myCommands;
      static short nMaxCommands;
   protected:
      static TriviaServer* myServer;
   public:
      virtual void doEffect(char*, Player*) = 0;
      static void init(TriviaServer*);
      static Command* getCommand(char*);
      Command(char*, char* =NULL);
};


class PlayerRecord
{
   private:
      static PlayerRecord rankedRecords[10];
   public:
      char szName[60];
      char szAlias[60];
      short nScore;
      bool bSavedSysop;
      PlayerRecord();
      PlayerRecord(char*, char*, short, bool);
      bool isEmpty();
      static PlayerRecord* getRankedRecords();
};


class TriviaThread : public GameThread
{
   protected:
      TriviaThread(GameNode*, GameServer*);
      TriviaServer* myServer;
      Player* pl;
};


class EnterGameThread : public TriviaThread
{
   private:
      EnterGameThread(GameNode*, GameServer*);
   public:
      static GameThread* factory(GameNode*, GameServer*);
      void run();
};


class ExitPromptThread : public TriviaThread
{
   private:
      ExitPromptThread(GameNode*, GameServer*);
   public:
      static GameThread* factory(GameNode*, GameServer*);
      void run();
};
   

class SysopThread : public TriviaThread
{
   private:
      SysopThread(GameNode*, GameServer*);
   public:
      static GameThread* factory(GameNode*, GameServer*);
      void run();
};

   

#endif
