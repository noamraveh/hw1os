#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <list>
#include <vector>
#include <ctime>
#include <string>
#include <set>
#include <dirent.h>
#include <unistd.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

class Command {
// TODO: Add your data members
    const char* cmd_line;
public:
    Command(const char* cmd_line):cmd_line(cmd_line){};
    virtual ~Command() {};
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    const char* getCmdLine(){
        return cmd_line;
    }
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line):Command(cmd_line){};
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


class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() {}
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

/*
class HistoryCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  HistoryCommand(const char* cmd_line, CommandsHistory* history);
  virtual ~HistoryCommand() {}
  void execute() override;
};
 */

class JobsList {
  class JobEntry {
      int jobId;
      char* cmd_line;
      int processId;
      time_t startTime;
      bool stopped;
  public:
      JobEntry(char* cmd_line):cmd_line(cmd_line){};
      ~JobEntry(){}
      time_t getStartTime(){
          return startTime;
      }
      int getJobId(){
          return jobId;
      }
      const char* getCmdLine(){
          return cmd_line;
      }
      int getProcessId(){
          return processId;
      }
      bool isStopped(){
          return stopped;
      }
      void changeIsStopped(bool status){
          stopped = status;
      }

   // TODO: Add your data members
  };
  std::list<JobEntry*> jobsList;
  int maxJobId;
  int numJobs;
    // TODO: Add your data members
 public:
  JobsList(): maxJobId(1), numJobs(0){};
  ~JobsList(){};
  void addJob(Command* cmd, bool stopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  int getPid(int jobId);
  bool isEmpty(){
      return jobsList.empty();
  }
  JobEntry* getMaxJob(){
      return *(&*jobsList.end());
  }
  void resumeJob(int jobId){
      getJobById(jobId)->changeIsStopped(false);
  }
  int getNumJobs(){
      return numJobs;
  }
  // TODO: Add extra methods or modify existing ones as needed
};

class SmallShell {
private:
    char* PrevDir;
    char* CurDir;
    std::string shellName;
    JobsList* jobs_list{};
    // TODO: Add your data members
public:
    SmallShell();
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
    char* getDir(){
        return CurDir;
    }
    char* getPrevDir(){
        return PrevDir;
    }
    void updateDirs(){
        free(PrevDir);
        PrevDir = CurDir;
        CurDir = get_current_dir_name();
    }

    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed
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

class GetCurrDirCommand : public BuiltInCommand {
    SmallShell* smash;
public:
    explicit GetCurrDirCommand(const char* cmd_line, SmallShell* smash):BuiltInCommand(cmd_line), smash(smash) {};
    ~GetCurrDirCommand() override {}
    void execute() override;
};
class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
    SmallShell* smash;
    char* newPath;
    bool isInputValid;
public:
    ChangeDirCommand(const char* cmd_line, char** plastPwd, SmallShell* smash): BuiltInCommand(cmd_line), smash(smash), newPath(nullptr),isInputValid(true){
        if(plastPwd[2]!= nullptr){
            isInputValid = false;
        }
        else{
            newPath = plastPwd[1];
        }
    } ;
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class ShowFilesCommand : public BuiltInCommand {
    SmallShell* smash;
public:
    ShowFilesCommand (const char* cmd_line, SmallShell* smash):BuiltInCommand(cmd_line), smash(smash){};
    virtual ~ShowFilesCommand () {}
    void execute() override;
};

class JobsCommand : public BuiltInCommand {
    JobsList* jobs_list;
public:
    JobsCommand(const char* cmd_line,JobsList* jobs_list):BuiltInCommand(cmd_line), jobs_list(jobs_list){};
    virtual ~JobsCommand() {}
    void execute() override;
};


class KillCommand : public BuiltInCommand {
    JobsList* jobs_list;
    int sigNum;
    int jobId;
    bool validInput;
 public:
  KillCommand(const char* cmd_line,char** plastPwd, JobsList* jobs_list):BuiltInCommand(cmd_line), jobs_list(jobs_list), validInput(true){
      if (!plastPwd[1]){
          validInput = false;
          return;
      }
      else{
          sigNum = *plastPwd[1]; //TODO: cut the "-" in the beginning
          if (!plastPwd[2]){
              validInput = false;
              return;
          }
          else{
              jobId = *plastPwd[2];
              if(plastPwd[3] != nullptr){
                  validInput = false;
             }
          }
      }

  };

  virtual ~KillCommand() {}
  void execute() override;
};
class ForegroundCommand : public BuiltInCommand {
    JobsList* jobs_list;
    int jobId;
    bool tooManyArgs;
    bool noArgs;
public:
    ForegroundCommand(const char* cmd_line,char** plastPwd, JobsList* jobs): BuiltInCommand(cmd_line), jobs_list(jobs), tooManyArgs(false),noArgs(false){
        if (!plastPwd[1]){
            noArgs = true;
            return;
        }
        else {
            jobId = (int)*plastPwd[1]; //TODO: cut the "-" in the beginning
            if (plastPwd[2] != nullptr) {
                tooManyArgs = true;
            }
        }
    };
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    JobsList* jobs_list;
    int jobId;
    bool tooManyArgs;
    bool noArgs;
 public:
  BackgroundCommand(const char* cmd_line,char** plastPwd, JobsList* jobs): BuiltInCommand(cmd_line), jobs_list(jobs), tooManyArgs(false),noArgs(false){
      if (!plastPwd[1]){
          noArgs = true;
          return;
      }
      else {
          jobId = (int)*plastPwd[1]; //TODO: cut the "-" in the beginning
          if (plastPwd[2] != nullptr) {
              tooManyArgs = true;
          }
      }
  };
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class QuitCommand : public BuiltInCommand {
    JobsList* jobs_list;
    bool kill;
public:
    QuitCommand(const char* cmd_line,char** plastPwd, JobsList* jobs): BuiltInCommand(cmd_line), jobs_list(jobs) ,kill(false){
        if  (plastPwd[1] == "kill"){
            kill = true;
        }
    }
    virtual ~QuitCommand() {}
    void execute() override;
};
/*
// TODO: add more classes if needed 
// maybe ls, timeout ?
*/

#endif //SMASH_COMMAND_H_
