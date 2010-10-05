#ifndef GAMESRVH_DEF
#define GAMESRVH_DEF

#include <time.h>
#include "e:\doors\intrnode\intrnode.h"

// General constants
const short MAX_NODE         = 150;


#ifndef __CONIO_H
const short    BLACK = 0;
const short    BLUE = 1;
const short    GREEN = 2;
const short    CYAN = 3;
const short    RED = 4;
const short    MAGENTA = 5;
const short    BROWN = 6;
const short    YELLOW = 14;
const short    WHITE = 15;
const short    DARKGRAY = 8;
#endif

const short     LWHITE = 7;
const short     LBLUE = 9;
const short     LGREEN = 10;
const short     LCYAN = 11;
const short     LRED = 12;
const short     LMAGENTA = 13;


// General functions useful for server
long getFileLength(char*);
void myDeleteFile(char*);
void myCopyFile(char*, char*, BOOL=FALSE);
short dice(short, short);
void myRandomize();
bool wordSearch(char*, char*);


class GameNode
{
   protected:
      short nIndex;
   public:
      char szRealName[60];
      char szAlias[60];
      char cGender;
      bool bSysop;
      bool bInGame;
      bool bHasThread;
      short nPlatform;
      HANDLE hOutputSlot;
      MessageQueue mqInput;
      GameNode(short, char*);
      virtual ~GameNode();
      virtual void fillStats(OutputData*) = 0;
      void print(char*, short=LWHITE, short=1, short=OP_NORMAL);
      void printWordWrap(char*, short=LWHITE, short=1, short=0, bool=false);
      void newline();
      void pausePrompt();
      void exitGame();
      void clearScreen();
      void center(char*, short=LWHITE, short=1);
      void displayScreen(char*);
      void displayHlp(char*, char*, char* =NULL, bool=false);
      void textBox(char*, short=LCYAN, short=LBLUE, bool=false);
      void menuOption(char, char*, short=LCYAN, short=LBLUE, short=LWHITE);
      void underline(char*, char*, short = 7, short = 7, bool=false);
};


class GameServer
{
   protected:
      CRITICAL_SECTION csCritical;
      HANDLE hInputSlot;
      void handleInput(InputData);
      virtual void addNode(short, char*) = 0;
      virtual void centralInput(InputData);
   public:
      GameNode* gNode[MAX_NODE];          // Current location & info of players in game
      short nPlayersInGame;               // Number of players currently in game.
      short nCriticalCount;
      GameServer();
      virtual void doorRound(time_t);
      void printAll(char*, short=LWHITE, short=1, short=OP_NORMAL);
      void printAllWordWrap(char*, short=LWHITE, short=1, short=0, bool=false);
      void run();
      void enterCritical();
      void leaveCritical();
};

class GameThread
{
   protected:
      class ThreadException { };
      GameNode* gn;
      GameServer* gs;
      GameThread(GameNode*, GameServer*); 
      char* getStr(char*, char* =NULL, short=7, short=79);
      char getKey(char* =NULL, short=7, bool=true);
      void pause(bool=false);
   public:
      static void launch(GameThread*);
      virtual void run() = 0;
      virtual void cleanup();
      friend void handleThread(void*);
};



#endif

