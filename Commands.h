#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <list>
#include <vector>
#include <ctime>
#include <string>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)



class SmallShell {
private:
    std::string PrevDir;
    std::string CurDir;
    std::string shellName;
    JobsList* jobs_list{};
    // TODO: Add your data members
public:
    SmallShell(){
        shellName = "smash";
        //TODO init firstdir char - empty
        CurDir =  getcwd(firstdir,200);
        PrevDir = firstDir;
    };
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();
    std::string getName(){
        return shellName;
    }
    void setName(std::string newName){
        shellName = newName;
    }
    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed
};


class Command {
// TODO: Add your data members
public:
    Command(const char* cmd_line){};
    virtual ~Command();
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line);
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {
    std::string newName;
    SmallShell* smash;

// TODO: Add your data members
public:
    ChangePromptCommand(const char *cmdLine, char **plastPwd, SmallShell* smash) : BuiltInCommand(cmdLine), smash(smash) {
        if (plastPwd[1] == nullptr)
            newName = "smash";
        else
            newName = plastPwd[1];
    };
    ~ChangePromptCommand() override = default;
    void execute() override;
};


class ShowFilesCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    ShowFilesCommand (const char* cmd_line);
    virtual ~ShowFilesCommand () {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
 public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  explicit GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  ~GetCurrDirCommand() override {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line):{};
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class CommandsHistory {
 protected:
  class CommandHistoryEntry {
	  // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  CommandsHistory();
  ~CommandsHistory() {}
  void addRecord(const char* cmd_line);
  void printHistory();
};

class HistoryCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  HistoryCommand(const char* cmd_line, CommandsHistory* history);
  virtual ~HistoryCommand() {}
  void execute() override;
};

class JobsList {
 public:
  class JobEntry {
      int jobId;
      int processId;
      time_t startTime;
      bool isStopped;


   // TODO: Add your data members
  };
  std::list<JobEntry> jobsList;
  int maxJobId;
 // TODO: Add your data members
 public:
  JobsList();
  ~JobsList();
  void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

// TODO: add more classes if needed 
// maybe ls, timeout ?


#endif //SMASH_COMMAND_H_
