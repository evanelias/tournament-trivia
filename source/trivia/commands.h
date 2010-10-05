#ifndef COMMAND_H_DEF
#define COMMAND_H_DEF

// Command-derivative classes

class PlayerListCommand : public Command
{
   public:
      PlayerListCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class SkipCommand : public Command
{
   public:
      SkipCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class HelpCommand : public Command
{
   public:
      HelpCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class MenuCommand : public Command
{
   public:
      MenuCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};   

class TellCommand : public Command
{
   public:
      TellCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class SubmitCommand : public Command
{
   public:
      SubmitCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class CorrectionCommand : public Command
{
   public:
      CorrectionCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class ScoresCommand : public Command
{
   public:
      ScoresCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class ExitCommand : public Command
{
   public:
      ExitCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class SysopCommand : public Command
{
   public:
      SysopCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

class CheckRegCommand : public Command
{
   public:
      CheckRegCommand(char* szMyName, char* szMyAbbrev =NULL) : Command(szMyName, szMyAbbrev) { }
      void doEffect(char*, Player*);
};

#endif