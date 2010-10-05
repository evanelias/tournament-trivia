// DOORSET.H:  Defined strings for IO mailslots, server exe name, etc.
// Customize + change for each doorgame.

// Only include DOORSET.H once.
#ifndef DOORSET_H_DEF
#define DOORSET_H_DEF

// Door information:  Name, version, compile date, input mailslot, output 
//   mailslot, server exe name
#define DOOR_NAME       "Tournament Trivia"
#define DOOR_COPYRIGHT  "(c) 2003 Evan Elias"
#define DOOR_VERSION    "1.0"
#define DOOR_COMPILE    "07/04/03"
#define DOOR_INP_SLOT   "trvinput"
#define DOOR_OUT_SLOT   "trvout"
#define DOOR_SERVER_EXE "trivsrv.exe"

// Round timer
#define ROUND_TIME      1000

// Statline variables
#define STAT_1          "[ Score:%4d "
#define STAT_2          " "
#define STAT_3          " "

#endif
