/*    INTRNODE.H
      Copyright 2003 Evan Elias

      ObjectDoor -- Header for data types used in communicating between the Door Client
      and Door Server.
*/

#ifndef INTRNODEH_DEF
#define INTRNODEH_DEF

#include <windows.h>
#include "source\doorset.h"

// Constants for InputData::nType
const short IP_NORMAL         = 0;                 // Standard hotkey input               
const short IP_ENTER_GAME     = 1;                 // Enter game request (supplies user name in szMessage)
const short IP_FORCE_EXIT     = 2;                 // Indicates instant forced exit from hangup or lockup.
const short IP_FINISHED       = 3;                 // Indicates server should erase the node.

// Constants for OutputData::nType
const short OP_NORMAL         = 0;                 // Standard output
const short OP_HOTKEY_PROMPT  = 1;                 // Set current prompt; expect a hotkey
const short OP_COMMAND_PROMPT = 2;                 // Set current prompt; expect a string command
const short OP_FORCE_PAUSE    = 3;                 // Indicates a forced pause for the node
const short OP_EXIT_NODE      = 4;                 // Tells node to exit the game
const short OP_CLEAR_SCREEN   = 5;                 // Tells node to clear the screen.
const short OP_CENTER_TEXT    = 6;                 // Standard output, text is to be centered.
const short OP_DISPLAY_ANSI   = 7;                 // Display indicated ANSI file.
const short OP_DISPLAY_HLP    = 8;                 // Display an entry in an HLP file.

// Constants for door client platforms
const short PL_LOCAL          = 0;
const short PL_XSDK32         = 1;
const short PL_OPENDOORS32    = 2;


// General functions
HANDLE createSlot(int);
HANDLE openSlot(int);
bool sendToSlot(HANDLE, char*);


// Class for node input.  Can be converted to/from a character array for use
// in mailslots.
class InputData
{
   public:
      short nFrom;
      short nType;
      char szMessage[200];
      char* toString(char*);
      InputData(char*);
      InputData();
};

// Class for node output.  Can be converted to/from a character array for use
// in mailslots.
class OutputData
{
   public:
      short nType;
      short nColor;
      short nHp, nSp, nMf;
      short nEnemyPercent;
      short nHpColor;
      char szMessage[200];
      char* toString(char*);
      OutputData(char*);
      OutputData();
};


// Class for a message queue node.  Stores a single input message waiting to be
// processed, and has a pointer to the next QueueNode.
class QueueNode
{
   public:
      InputData id;
      QueueNode* qnNext;
      QueueNode(InputData);
};


// Class for a MessageQueue, the data structure used for storing input messages
// that are waiting to be processed.
class MessageQueue
{
   private:
      QueueNode* qnTop;
   public:
      InputData dequeue();
      void enqueue(InputData);
      bool isEmpty();
      MessageQueue();
};
  


#endif
