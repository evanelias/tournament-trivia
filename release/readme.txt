TOURNAMENT TRIVIA  v1.0
=-=-=-=-=-=-=-=-=-=-=-=
(C) 2003 Evan Elias
                   

This is the Sysop Documentation file for Tournament Trivia.  This document is
also available online at http://trivia.doormud.com/sysdocs.html in hypertext
format, complete with configuration screenshots!  You may find that version of
the docs to be much easier to use.

Tournament Trivia is a 32-bit Windows door, and will run on the following BBS
software platforms:
 - Synchronet v3.xx for Windows
 - WildCat WinServer
 - Mystic BBS (for Win32)
 - EleBBS (for Win32)
 - MannSoft GameSrv
 - Any other Windows-based BBS with Door32.sys support



Upgrading from a previous version:
   Simply unzip the newer version into your existing Tournament Trivia
   directory, over-writing older files.  Your score files and custom question
   files will not be lost.


==============================================================================


INTRODUCTION 
------------
Tournament Trivia is a new 32-bit doorgame for Bulletin Board Systems.  
Players compete with each other in real-time, trying to be the first to answer
the current question.  Scores add up from day to day, and are reset at the end
of the month, with one player emerging victorious.  The game offers an 
excellent multi-node environment that also effectively functions as a chat 
room.



DISCLAIMER
----------
Users of this program must accept the following:

TOURNAMENT TRIVIA is supplied as is.  The author disclaims all warranties, 
expressed or implied.  The author assumes no liability for damages, direct or 
consequential, which may result from the use of TOURNAMENT TRIVIA.

You may not modify or disassemble this software, in whole or in part.  Use of
decompilers, hex editors, etc on TOURNAMENT TRIVIA is strictly forbidden.


==============================================================================


Contents
~~~~~~~~
1. Setup
2. Configuration options
3. Adding more questions
4. Registration
5. Miscellaneous


==============================================================================


1. Setup
~~~~~~~~

In order to set up Tournament Trivia, perform the following steps:

Step 1:  Create a directory for Tournament Trivia, and copy all of the 
         Tournament Trivia files to this directory.  Please be sure to select
         a directory name that does not contain any spaces in it.

Step 2:  You must add the door to your BBS's menus.  Specific instructions for
         every supported BBS software are included below!


Specific setup instructions:
(Reminder:  http://trivia.doormud.com/sysdocs.html has configuration screenshots
for each BBS software -- please give these a look if you encounter setup 
problems.)


Synchronet:
----------
  Tournament Trivia has a special Synchronet version (trivsync.exe) that uses
  Synchronet's proprietary XTRN.DAT instead of using DOOR32.SYS.  Like the
  Door32 version, it is a 32-bit Windows-native doorgame.  But the Synchronet
  version also adds support for Synchronet Global Commands such as on-line user
  lists, log-on/off notifications, and inter-node paging!

  To add the game to your board's menus, first run SCFG by selecting 
  BBS -> Configure in the Synchronet console.  Then go to External Programs ->
  Online Programs -> Games -> Available Online Programs, and hit the insert 
  key.  Use "Tournament Trivia" for the program name and "TRIVIA" for the 
  internal code.  Now, configure the door as follows:

         ¦----------------------------------------------------------¦
         ¦ ¦Name                       Tournament Trivia            ¦
         ¦ ¦Internal Code              TRIVIA                       ¦
         ¦ ¦Start-up Directory         C:\SBBS\XTRN\TRIVIA   <<- Example only
         ¦ ¦Command Line               trivsync                     ¦
         ¦ ¦Clean-up Command Line                                   ¦
         ¦ ¦Execution Cost             None                         ¦
         ¦ ¦Access Requirements                                     ¦
         ¦ ¦Execution Requirements                                  ¦
         ¦ ¦Multiple Concurrent Users  Yes                          ¦
         ¦ ¦Intercept I/O Interrupts   No                           ¦
         ¦ ¦Native (32-bit) Executable Yes                   <<- See below!
         ¦ ¦Modify User Data           No                           ¦
         ¦ ¦Execute on Event           No                           ¦
         ¦ ¦BBS Drop File Type         Synchronet      XTRN.DAT     ¦
         ¦ ¦Place Drop File In         Node Directory               ¦
         ¦ ¦Time Options...                                         ¦
         +----------------------------------------------------------+

         Be sure to set the "start-up directory" to whatever directory you put
         your Tournament Trivia files in.

         When selecting the Synchronet XTRN.DAT dropfile type, set "use real 
         names" to "no" in order for inter-node paging to properly display a 
         user's alias.    

         The "Native (32-bit)" option is only available in Synchronet v3.10g 
         or later. If you are using an older version of Synchronet, you MUST 
         put trivsync on the OS/2 Program List instead as described below.

         New versions of Synchronet may have additional options for doors.  
         You can safely leave those at their default values.


  If you are running Synchronet v3.10g or later, your setup for the door is now
  complete, and you can exit SCFG.

  Otherwise, you are running an older (pre-3.10g) release of Synchronet, which
  does not have the "Native (32-bit) Executable" option in SCFG.  Instead, you
  must manually add trivsync to a seperate list of 32bit doors. This can be
  done in SCFG by selecting External Programs -> OS/2 Program List and hit the
  insert key on your keyboard.  Type "trivsync" and then hit enter.  Select 
  "No" for the "Comm Port" toggle.  (This step is *required* for pre-3.10g
  Synchronet, so do not skip it!)

  Next, exit SCFG.  In older versions of Synchronet, you must now manually
  refresh the board's node/config data in order for the game to appear on your
  BBS's menus immediately.  You can do this by hitting the stop button then the
  play button on the Synchronet telnet server.

 
WildCat Winserver:
----------------- 
  You must run Tournament Trivia through a batch file.  Copy WCTRIV.BAT from
  your Tournament Trivia directory and place it wherever you normally put door
  batch files. Then, edit it as directed in the batch file's comments. 

  Then, configure your BBS to run the door as "Door32 Compatible" for Door 
  Type, and "WCTRIV.BAT" for the Batch File.  Be sure to enable the Multiuser
  toggle option.

  IMPORTANT:  This door directly interfaces with Winserver's DOOR32.DLL file
  to provide native Winserver door support.  It is vital that the door is able
  to find this file.  Please determine which of your directories contains this
  file (probably your \wc5 directory), and then add this directory to your 
  system's path if it isn't already there:   

    Windows 95/98/ME:  You can add a directory to your system path by
                       modifying AUTOEXEC.BAT.  You will then have to reboot
                       your system in order for the change to take effect.

    Windows NT/2K/XP:  Use the Control Panel to modify your system path.
                       Please see Control Panel >> System >> Advanced >> 
                       Environmental Variables >> System Variables >> Path.

  An alternative option (if you don't know how to edit your path) is to 
  simply place a copy of WildCat's DOOR32.DLL file in Tournament Trivia's 
  directory.  This should work fine, but is not recommended for upgrade 
  reasons, since DOOR32.DLL could change in future versions of Winserver.


Mystic BBS:                                                              
----------
  Assuming c:\doors\trivia is your Tournament Trivia directory, and c:\mystic
  is your Mystic directory, use a command line of:
    
     c:\doors\trivia\triv32.exe /n %3 /d c:\mystic\temp%3

  Set the menu command type to "D3  Door with DOOR32.SYS".

  IMPORTANT:  The original release of Mystic v1.07 had a bug that prevents 
  Door32 games from working properly.  You must have Mystic v1.07.2 or later
  in order for this door to run.

  WINDOWS 95/98/ME:  Do *NOT* use a batch file to call this door.  Doing so 
  will not work on these operating systems.  The door switches to its own
  directory upon start-up anyway; there is no need for a batch file!


EleBBS:
------
  Assuming c:\doors\trivia is your Tournament Trivia directory, and c:\ele is
  your EleBBS directory, use a command line of:

     c:\doors\trivia\triv32.exe -n *N -d c:\ele\NODE*N\door32.sys -socket *W

  In order for this door to work properly, you may need to lower the value for
  Modem >> Telnet >> Start Node# in ELCONFIG, since this door does not support
  node numbers higher than 99.  

  WINDOWS 95/98/ME:  Do *NOT* use a batch file to call this door.  Doing so 
  will not work on these operating systems.  The door switches to its own
  directory upon start-up anyway; thus there is no need for a batch file!



MannSoft GameSrv:
----------------
  In GameSrv, go to the Games tab and press Add.  
  Assuming Tournament Trivia is in c:\bbs\trivia and GameSrv is in 
  c:\bbs, you should use the following command line: 
  
     c:\bbs\trivia\triv32.exe /n *N /d c:\bbs\node*N /socket *H 

  For the FOSSIL setting, select "None".  Tournament Trivia uses Door32.sys,
  so it doesn't need (and can't use) a FOSSIL driver.



==============================================================================


2. Configuration Options
~~~~~~~~~~~~~~~~~~~~~~~~
The game has a special "config" command that is usable to change the game's
settings.  This command is available only to sysops, not to ordinary users.
In order for the game to recognize you as a sysop, you can do one of the 
following:

  - Enter the door in local mode ("triv32.exe -l"). In local mode, you are
    able to use the "config" command. You can also grant sysop access to a 
    specific user account, by using "triv32.exe -l -username xxx" where xxx is
    the real name of the player.  

  - In the Synchronet version of the door, the game reads the user's Security
    Level from the dropfile.  Any user with 90+ security in Synchronet is
    considered a Sysop in the door.


To enter the game configuration menu, type "config" from any game prompt.  From
this menu, you can change the following game settings:

[F] Question Frequency 
       This setting determines how long an unanswered question remains before
       it is replaced with a new question.  (Default: 50 seconds)

[M] Maximum number of clues
       This setting determines the maximum number of clues that may be
       displayed for a question.  The actual number of clues may be lower 
       than this value, since clues stop being displayed if most of the
       answer is already filled in.  (Default: 3 clues)

[S] Show sysops on score list
       If set to "no", players with sysop access will not be listed on the
       score list. (Default: yes -- sysops ARE listed along with normal
       players)

[#] Question File 
       This setting allows you to configure what question files are used
       by Tournament Trivia.  By default, the only question files used are
       DATABASE.ENC and CUSTOM.TXT; you can add up to eight other files.
       Modifying CUSTOM.TXT and/or creating your own custom question files
       is easy; please see Section 3 of this document for more information.
       In the near future, you will also be able to download third-party
       question files.  



==============================================================================


3. Adding More Questions
~~~~~~~~~~~~~~~~~~~~~~~~

Default question file:
  The game's stock question database is DATABASE.ENC.  The copy of DATABASE.ENC
  that comes with Tournament Trivia v1.0 contains about 800 questions.  Newer,
  larger versions of DATABASE.ENC are regularly released on-line at
  http://trivia.doormud.com/sysop.html -- each new release of the file increases
  the size of the database, adding many new questions.

  Please note that in the unregistered version of Tournament Trivia, only 400 
  questions from DATABASE.ENC are made available to players.  


Using third-party question files:
  Several new question files from third-party sources will soon be available
  for Tournament Trivia.  You can freely download these files and add them to
  your game via the "config" command (see Section 2 of these docs, above).
  Links to third-party question files will be available on-line at 
  http://trivia.doormud.com/sysop.html in the near future.

  In the unregistered version of Tournament Trivia, only 150 total questions
  from third party files (and/or custom question files) are made available
  to players.


Adding custom questions:
  Adding your own custom questions is easy; see CUSTOM.TXT as an example of
  some BBS-related custom trivia questions.  With a text editor, simply add
  new questions into CUSTOM.TXT, having question lines alternate with answer
  lines. Be sure to press <ENTER> at the end of each question or answer, and
  ONLY at the end. It is recommended that you use a quality text editor like 
  TextPad or EditPad, ie, one that displays line numbers and has an option to
  *disable word wrap*. With these settings, questions should always be on odd
  line numbers, and answers on even line numbers.
  
  Questions are limited to 160 characters, and answers are limited to 80
  characters.
  
  After adding questions, if you notice the game acting strangely (displaying
  answers instead of questions), it means one of your custom question files
  has an <ENTER> line in an incorrect spot, and/or there's a question exceeding
  160 characters, and/or there's an answer exceeding 80 characters.  Please 
  verify that your custom files conform to the correct line formatting.


==============================================================================


4. Registration
~~~~~~~~~~~~~~~

Tournament Trivia is shareware.  There are limitations on the number of
questions in the game if your copy is unregistered: 
 - Only 400 questions from the game's standard question database are made 
   available to players.  
 - If you choose to add additional custom question files and/or third-party
   question files, only 150 total questions from these sources will be used.

Thus, at most, there are only 550 available questions in the unregistered
version of the game.


Registration for Tournament Trivia normally costs $12 (US dollars).  However,
you can register the game at a discounted rate if you have submitted new trivia
questions for the game:

# of acceptable     | Registration
questions submitted | Cost
--------------------|-------------
None (full price)   |  $12
10  questions       |  $10
20  questions       |  $8
40  questions       |  $5
80+ questions       |  FREE

For more information, please see the on-line question submission page:
  ->  http://trivia.doormud.com/submit.html
To check how many acceptable questions you have already submitted, please see:
  ->  http://trivia.doormud.com/helplist.html
Discounted rates are subject to change; please see the question submission
page for up-to-date rates.


HOW TO REGISTER ON-LINE:
Registering on-line is the fastest and most convenient method to purchase
Tournament Trivia.  You can pay via credit card or PayPal transfer.  For more
information about how to register on-line, please see:
  ->  http://trivia.doormud.com/register.html


HOW TO REGISTER BY POSTAL MAIL:
1.  Print and fill out the registration form (REGISTER.TXT).  If you do not
    have a printer, you may handwrite the information.  
2.  Include a check or money order for $12 US, or less if you have a discount.
    If using a check, make it payable to "Evan Elias".  (If you qualify for a
    free registration, you may skip this step!)
3.  E-mail RhythmNp@aol.com and inquire about the current registration mailing
    address. Put the payment and registration form in an envelope and mail it 
    to the provided address.


==============================================================================


5. Miscellaneous
~~~~~~~~~~~~~~~~

Notes on the Synchronet version (TRIVSYNC.EXE)
  - By default, the Synchronet version of Tournament Trivia does not create
    a local display window.  This feature is intentional, to reduce taskbar
    clutter.  If you would prefer to have a local display window, a /window
    command line option is available.
  - The Synchronet-specific version of this game, trivsync.exe, does not 
    support local mode play.  To play in local mode, run the door32 exe's 
    local mode, "triv32.exe /l".


Notes on the DOOR32 version (TRIV32.EXE)
  - This version provides an interactive local display window by default.
    The window automatically starts as minimized, unless you are running the
    door in local mode.  If desired, you can eliminate the window entirely 
    by using the /silent command line option.
  - This version of Tournament Trivia supports several command line options:
       /D or /DROPFILE  - Door information file directory and/or filename.
       /N or /NODE      - Sets the node number to use.  
       /SOCKET x        - Needed in EleBBS and GameSrv to specify the socket.
       /L or /LOCAL     - Causes door to operate in local mode, without 
                          requiring a door information (drop) file.  
       /USERNAME        - Use in local mode to specify your user name.
       /SILENT          - Operate in silent mode, with no local display window.
       /GRAPHICS        - Unless followed by 0 or N, turns on ANSI mode.
                          Useful if, for whatever reason, the game is not 
                          detecting users' ANSI settings properly.


Known issues/bugs in this version:
  - If the nodes of your BBS are spread across multiple Windows computers via
    a LAN, this door might not function properly.  An example would be if you
    have one computer serve nodes 1 to 10 and have another, separate networked
    computer serve nodes 11 through 20, using a shared drive for the BBS.  
    Setups like this are pretty rare for a BBS, but if you are using one, 
    please e-mail RhythmNp@aol.com for more info on the problem.

  - You cannot install multiple, separate copies of Tournament Trivia on the 
    same BBS. This will be fixed in a future release.

  - When running the door in Local Mode, on some systems, the user name that
    you enter in the dialog box is ignored and your user name will instead
    always come up as "sysop".  This is a bug in the doorkit used to create
    the game; it will be fixed in the next release.  For now, to avoid the
    problem you can use the /username command line option, ie 
    "triv32.exe /l /username John Doe", to use a different local user name.

  - On Wildcat Winserver, if a user drops carrier while in the door, it may 
    lock the node and temporarily prevent the user from re-logging on.  This
    problem cannot be fixed at the present time, because it is caused by a
    doorkit flaw.  If/when a new C++ doorkit with Winserver support is
    available, this problem will be fixed.


Support:
-=-=-=- 
Please consult one of the following sources if you are having difficulty setting
up the game or if you have any questions, comments, ideas, etc --

 - Visit the official webpage:       http://trivia.doormud.com
 - E-mail the game's programmer:     RhythmNp@aol.com



Thank You's:
-=-=-=-=-=-
Web Hosting       -- Allied WebProducts (www.awphosting.com)
32-bit Doorkits   -- Rob Swindell and Brian Pirie

Also a huge thanks to all the sysops and players who beta-tested this game
and submitted trivia questions!


<End of file>