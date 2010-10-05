Building new MUD-style doors with the INTRNODE framework:
--------------------------------------------------------
Current files assume E:\DOORS\INTRNODE as location for the framework files,
and ..\SOURCE\ as location for source files.


1. Create a new directory for the door.  The project file for your door will
   go in this directory and compile/build from this directory.

2. Create a subdirectory called SOURCE, which will contain the door's source
   code.

3. Copy DOORSET.H to your SOURCE directory, and edit it for the proper settings
   for your door.  Different doors must have unique mailslot names.

3. To build the Synchronet XTRN.DAT version, compile and link together DOOR.CPP,
   INTRNODE.CPP, PLATXSDK.CPP, XSDK.C, and XSDKVARS.C.  Compile as a Win32 
   Console Mode program, static libraries, multithreading enabled.

4. To build the Opendoors DOOR32.SYS version, compile and link together
   DOOR.CPP, INTRNODE.CPP, PLATOD.CPP, and ODOORW.LIB.  Compile as a Win32 GUI
   program, static libraries, multithreading enabled.  

5. To build the game's multinode server, you will need to write code for:

   A node class must be derived from the abstract base GameNode.  You must
   implement the fillStats() function for setting a player's statline values.

   A server class must be derived from the abstract base GameServer.  You
   must implement the addNode() method, which takes a node # and user name,
   and creates a new GameNode-derived object.  You may also want to implement:
      centralInput()   -- Handles input for nodes that don't have own threads.
      doorRound()      -- Run by server thread once per second.

   Your code should create an object of your GameServer-derived class early on.
   When you are ready for the server to start up, use the run() method.
   
   Compile and link together GAMESRV.CPP, INTRNODE.CPP, and your own source 
   files.  Compile as a Win32 console mode program, static libraries, 
   multithreading enabled if needed.


-------------------------------------------------------------------------------

How to create a thread that handles input for a particular node:

1. Create a class that extends from GameThread.  You will need to create a run()
   method that contains the code you want to run in the thread.   You will need
   to create a static factory() method for creating objects of the thread; at
   minimum, your factory method will need to take a GameServer pointer and a 
   GameNode pointer to create the thread.

   If desired, over-ride the cleanup() method depending on where you want the
   user to go after the thread is done.  The default cleanup() method simply 
   calls:
      gn->print(NULL, LWHITE, 0, OP_COMMAND_PROMPT);

   If you wanted to send the user to a hotkey-prompt instead, for example,
   simply have cleanup() execute:
      gn->print(NULL, LWHITE, 0, OP_HOTKEY_PROMPT);
   instead.


2. When you want to create an instance of the thread, use the static method
   GameThread::launch().  For example, to create an instance of some thread
   class called ExitPromptThread for a node designated by gn in a gameserver
   designated by gs, use:
      GameThread::launch( ExitPromptThread::factory(gn, gs) );

   The thread will be created, and its run() method will be called.  Input
   for that node will now be routed to this thread, instead of going to 
   GameServer::centralInput().

   When your run() method terminates, cleanup() is called and then input
   is again routed back to GameServer::centralInput().  Be sure that cleanup()
   specifies some sort of prompt for the node (most likely a NULL one),
   otherwise his/her client will not permit any input.

   You do not need to worry about calling delete on the thread; this is done
   automatically.

3. GameThread::launch() can be called in your server's addNode() method if
   desired, to put the node in its own input thread immediately upon node
   creation.

4. A node input thread cannot launch another node input thread; this would
   cause a conflict of where the input would go.  Hence, GameThread::launch()
   cannot be called from GameThread::run() for the same node.  (In theory,
   this may work if launch() is called on a DIFFERENT node, but the call will
   fail if that node already has an input thread.)







