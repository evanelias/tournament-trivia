/*    INTRNODE.CPP
      Copyright 2003 Evan Elias

      ObjectDoor -- Code used in communication between Door Clients and Door Server.
      Current implementation uses Windows Mailslots for IPC.  Currently, the Door Server process
      and Door Client processes must be run on the same box; this will be changed in the future
      to support BBS's that span multiple systems.
*/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "e:\doors\intrnode\intrnode.h"
#include "source\doorset.h"

// Creates a mailslot server for the given node, or for the gameserver if the
// node is -1.
HANDLE createSlot(int nNode)
{
   char szText[80];

   if ( nNode < 0 )
      sprintf(szText, "\\\\.\\mailslot\\%s", DOOR_INP_SLOT);
   else
      sprintf(szText, "\\\\.\\mailslot\\%s%d", DOOR_OUT_SLOT, nNode);

   return CreateMailslot(szText, 240, 0, NULL);
}


// Opens a pre-existing mailslot for reading.
HANDLE openSlot(int nNode)
{
   char szText[80];

   if ( nNode < 0 )
      sprintf(szText, "\\\\.\\mailslot\\%s", DOOR_INP_SLOT);
   else
      sprintf(szText, "\\\\.\\mailslot\\%s%d", DOOR_OUT_SLOT, nNode);
   
   return CreateFile(szText, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL); 

}


// Writes text to a given slot.
bool sendToSlot(HANDLE hSlot, char* szInput)
{
   DWORD cbWritten; 
 
   return WriteFile(hSlot, szInput,  strlen(szInput) + 1, &cbWritten, NULL); 
}


// Converts InputData to char*
char* InputData::toString(char* szBuffer)
{
   sprintf(szBuffer, "%d %d %s", nFrom, nType, szMessage);
   return szBuffer;
}

// Converts char* to InputData
InputData::InputData(char* szMsg)
{
   nFrom = atoi( strtok(szMsg, " ") );
   nType = atoi( strtok(NULL, " ") );
   szMessage[0] = '\0';
   char* szTemp = strtok(NULL, "");
   if ( szTemp != NULL )
      strcpy( szMessage, szTemp );
}


InputData::InputData()
{
}

// Converts OutputData to char*
char* OutputData::toString(char* szBuffer)
{
   sprintf(szBuffer, "%d %d %d %d %d %d %d %s", nType, nColor, nHp, nSp, nMf, nEnemyPercent, nHpColor, szMessage);
   return szBuffer;
}

// Converts char* to OutputData
OutputData::OutputData(char* szMsg)
{
   nType = atoi( strtok(szMsg, " ") );
   nColor = atoi( strtok(NULL, " ") );
   nHp = atoi( strtok(NULL, " ") );
   nSp = atoi( strtok(NULL, " ") );
   nMf = atoi( strtok(NULL, " ") );
   nEnemyPercent = atoi( strtok(NULL, " ") );
   nHpColor = atoi( strtok(NULL, " ") );
   szMessage[0] = '\0';
   char* szTemp = strtok(NULL, "");
   if ( szTemp != NULL )
      strcpy( szMessage, szTemp );
}

OutputData::OutputData()
{
   szMessage[0] = '\0';
   nType = OP_NORMAL;
   nColor = 7;
   nHp = 0;
   nSp = -1;
   nMf = -1;
   nEnemyPercent = -1;
   nHpColor = 7;
}


QueueNode::QueueNode(InputData idMessage)
{
   id = idMessage;
   qnNext = NULL;
}


MessageQueue::MessageQueue()
{
   qnTop = NULL;
}

// Removes the Input message at the top of the input queue.
InputData MessageQueue::dequeue()
{
   if ( isEmpty() )
      return NULL;

   InputData idReturn = qnTop->id;
   QueueNode* qnNewTop = qnTop->qnNext;
   delete qnTop;
   qnTop = qnNewTop;
   return idReturn;
}


// Adds an input message to the top of an input queue.
void MessageQueue::enqueue(InputData idMessage)
{
   if ( isEmpty() )
      {
      qnTop = new QueueNode(idMessage);
      return;
      }

   QueueNode* qnCurrent = qnTop;

   while ( qnCurrent->qnNext != NULL )
      {
      qnCurrent = qnCurrent->qnNext;
      }

   qnCurrent->qnNext = new QueueNode(idMessage);
}


bool MessageQueue::isEmpty()
{
   if ( qnTop == NULL )
      return true;
   else
      return false;
}



