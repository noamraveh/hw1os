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
private:
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
/*
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
*/

class JobsList {
private:
  class JobEntry {
      int job_id;
      char* cmd_line;
      int process_id;
      time_t start_time;
      bool stopped;
  public:
      JobEntry(char* cmd_line):cmd_line(cmd_line){};
      ~JobEntry(){}
      time_t getStartTime(){
          return start_time;
      }
      int getJobId(){
          return job_id;
      }
      const char* getCmdLine(){
          return cmd_line;
      }
      int getProcessId(){
          return process_id;
      }
      bool isStopped(){
          return stopped;
      }
      void changeIsStopped(bool status){
          stopped = status;
      }

   // TODO: Add your data members
  };
  std::list<JobEntry*> jobs_list;
  int max_job_id;
  int num_jobs;
    // TODO: Add your data members
 public:
  JobsList(): max_job_id(1), num_jobs(0){};
  ~JobsList(){};
  void addJob(Command* cmd, bool stopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int job_id);
  void removeJobById(int job_id);
  JobEntry * getLastJob(int* last_job_id); // TODO:do we need it??
  JobEntry *getLastStoppedJob(int *job_id);
  int getPid(int job_id);
  bool isEmpty();
  JobEntry* getMaxJob(){
      return *(&*jobs_list.end());
  }
  void resumeJob(int job_id);
  int getNumJobs();
};

class SmallShell {
private:
    char* prev_dir;
    char* cur_dir;
    std::string shell_name;
    JobsList* jobs_list;
    // TODO: Add your data members
public:
    SmallShell():shell_name("smash"){
        cur_dir =  get_current_dir_name();
        prev_dir = get_current_dir_name();
    };
    ~SmallShell();
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    std::string getName();
    void setName(std::string new_name);
    char* getDir();
    char* getPrevDir();
    void updateDirs();
    Command *CreateCommand(const char* cmd_line);
    void executeCommand(const char *cmd_line);
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
    virtual ~ShowPidCommand() {}
    void execute() override;
};
class ChangePromptCommand : public BuiltInCommand {
    std::string new_name;
    SmallShell* smash;
public:
    ChangePromptCommand(const char *cmd_line, char **plast_pwd, SmallShell* smash) : BuiltInCommand(cmd_line), smash(smash) {
        if (plast_pwd[1] == nullptr)
            new_name = "smash";
        else
            new_name = plast_pwd[1];
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
    SmallShell* smash;
    char* new_path;
    bool valid_input;
public:
    ChangeDirCommand(const char* cmd_line, char** plastPwd, SmallShell* smash): BuiltInCommand(cmd_line), smash(smash), new_path(nullptr),valid_input(true){
        if(plastPwd[2]!= nullptr){
            valid_input = false;
        }
        else{
            new_path = plastPwd[1];
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
    int sig_num;
    int job_id;
    bool valid_input;
 public:
  KillCommand(const char* cmd_line,char** plast_pwd, JobsList* jobs_list):BuiltInCommand(cmd_line), jobs_list(jobs_list), valid_input(true){
      if (!plast_pwd[1]){
          valid_input = false;
          return;
      }
      else{
          sig_num = *plast_pwd[1]; //TODO: cut the "-" in the beginning
          if (!plast_pwd[2]){
              valid_input = false;
              return;
          }
          else{
              job_id = *plast_pwd[2];
              if(plast_pwd[3] != nullptr){
                  valid_input = false;
             }
          }
      }
  };

  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    JobsList* jobs_list;
    int job_id;
    bool too_many_args;
    bool no_args;
public:
    ForegroundCommand(const char* cmd_line,char** plast_pwd, JobsList* jobs_list): BuiltInCommand(cmd_line), jobs_list(jobs_list), too_many_args(false),no_args(false){
        if (!plast_pwd[1]){
            no_args = true;
            return;
        }
        else {
            job_id = (int)*plast_pwd[1]; //TODO: cut the "-" in the beginning
            if (plast_pwd[2] != nullptr) {
                too_many_args = true;
            }
        }
    };
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    JobsList* jobs_list;
    int job_id;
    bool too_many_args ;
    bool no_args;
 public:
  BackgroundCommand(const char* cmd_line,char** plast_pwd, JobsList* jobs_list): BuiltInCommand(cmd_line), jobs_list(jobs_list), too_many_args(false),no_args(false){
      if (!plast_pwd[1]){
          no_args = true;
          return;
      }
      else {
          job_id = (int)*plast_pwd[1]; //TODO: cut the "-" in the beginning
          if (plast_pwd[2] != nullptr) {
              too_many_args = true;
          }
      }
  }
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class QuitCommand : public BuiltInCommand {
    JobsList* jobs_list;
    bool kill;
public:
    QuitCommand(const char* cmd_line,char** plast_pwd, JobsList* jobs_list): BuiltInCommand(cmd_line), jobs_list(jobs_list) ,kill(false){
        if  (plast_pwd[1] == "kill"){
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
