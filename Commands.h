#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <list>
#include <vector>
#include <ctime>
#include <string>
#include <set>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include "string.h"
#include "algorithm"
#include <cmath>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)
//TODO print zero jobs
//TODO reset time when readding
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



class PipeCommand : public Command {
    char* args[2];
    bool err;
public:
    PipeCommand( const char* cmd_line, bool err):Command(cmd_line),err(err){
        char* unconst_cmd_line = (char*)malloc(sizeof(cmd_line)+1);
        strcpy(unconst_cmd_line,cmd_line);
        if (err){
            args[0] = strtok(unconst_cmd_line, "|&");
            args[1] = strtok(nullptr, "|&");
        }
        else{
            args[0] = strtok(unconst_cmd_line, "|");
            args[1] = strtok(nullptr, "|");
        }

    };
    virtual ~PipeCommand() {}
    void execute() override;
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
      int job_id; //TODO: keep the ID even if moved to fg and then back to bg
      char* cmd_line;
      int process_id;
      time_t start_time;
      bool stopped;
      char* original_cmd_line;
  public:
      JobEntry(char* cmd_line,int pid, bool stopped, const char* org_cmd_line,int id = -1):cmd_line(cmd_line),process_id(pid), stopped(stopped),job_id(id){
          start_time = time(nullptr);
          original_cmd_line = (char*)malloc(sizeof(org_cmd_line)+1);
          strcpy(original_cmd_line,org_cmd_line);
      };
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
      const char* getOrgCmdLine(){
          return original_cmd_line;
      }



          // TODO: Add your data members
  };
  std::list<JobEntry*> jobs_list;
  int num_jobs;
  int id_in_fg;
    // TODO: Add your data members
 public:
  JobsList(): num_jobs(0){};
  ~JobsList(){};
  void addJob(const char* cmd_line,pid_t pid, int cur_job_id, bool stopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int job_id);
  void removeJobById(int job_id);
  JobEntry * getLastJob(int* last_job_id); // TODO:do we need it??
  JobEntry *getLastStoppedJob(int *job_id);
  int getPid(int job_id);
  bool isEmpty();
  int getMaxJob(){
      if (jobs_list.size() == 0){
          return fmax(0,id_in_fg);
      }
      return fmax(jobs_list.back()->getJobId(),id_in_fg);
  }
  void resumeJob(int job_id);
  int getNumJobs();
  void clearJobs(){
      if (jobs_list.size() != 0)
        jobs_list.clear();
      num_jobs = 0;
  }
  void updateIdInFg(int job_id){
      id_in_fg = job_id;
  }

  int getJobId(){
      return jobs_list.front()->getJobId();
  }
  int getFGPid(){
      return jobs_list.front()->getProcessId();
  }
  const char* getFGCmdLine(){
      return jobs_list.front()->getOrgCmdLine();
  }
  static bool compareJobEntries(JobEntry* j1, JobEntry* j2){
      return j1->getJobId() < j2->getJobId();
  }

};

class SmallShell {
private:
    char* prev_dir;
    char* cur_dir;
    std::string shell_name;
    JobsList* jobs_list;
    JobsList* in_fg;
    // TODO: Add your data members
public:
    SmallShell():jobs_list(new JobsList),in_fg(new JobsList),shell_name("smash"){
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
    void StopFG() {
        std::cout << "smash: got ctrl-Z" << std::endl;
        if (!in_fg->isEmpty()){
            pid_t pid = in_fg->getFGPid();
            jobs_list->addJob(in_fg->getFGCmdLine(), pid, in_fg->getJobId(), true);
            int ret_val = kill(pid, SIGSTOP);
            std::cout << ret_val <<std::endl ;
            std::cout << pid <<std::endl;
            if (ret_val != 0){
                perror("smash error: kill failed");
                exit(0);
            }
            std::cout << "smash: process " << pid << " was stopped" << std::endl;
        }
        //add error
    }

    void KillFG(){
        std::cout << "smash: got ctrl-C" << std::endl;

        if (!in_fg->isEmpty()){
            pid_t pid = in_fg->getFGPid();
            int ret_val = kill(pid, SIGKILL);
            if (ret_val != 0){
                perror("smash error: kill failed");
                exit(0);
        }
            std::cout << "smash: process " << pid << " was killed" << std::endl;
        }
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
    ChangePromptCommand(const char *cmd_line, char **cmd_args, SmallShell* smash) : BuiltInCommand(cmd_line), smash(smash) {
        if (cmd_args[1] == nullptr)
            new_name = "smash";
        else
            new_name = cmd_args[1];
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
    bool too_many_args;
    bool no_args;
public:
    ChangeDirCommand(const char* cmd_line, char** cmd_args, SmallShell* smash): BuiltInCommand(cmd_line), smash(smash), new_path(nullptr),too_many_args(false),no_args(false){
        if (cmd_args[1] == nullptr){
            no_args = true;
        }
        if(cmd_args[2]!= nullptr){
            too_many_args = true;
        }
        else{
            new_path = cmd_args[1];
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
  KillCommand(const char* cmd_line,char** cmd_args, JobsList* jobs_list):BuiltInCommand(cmd_line), jobs_list(jobs_list), valid_input(true){
      if (!cmd_args[1]){
          valid_input = false;
          return;
      }
      else{
          sig_num = std::stoi(std::string(cmd_args[1]+1));
          if (!cmd_args[2]){
              valid_input = false;
              return;
          }
          else{
              job_id = std::stoi(std::string(cmd_args[2]));
              if(cmd_args[3] != nullptr){
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
    JobsList* in_fg;
    int job_id;
    bool too_many_args;
    bool no_args;
public:
    ForegroundCommand(const char* cmd_line,char** cmd_args, JobsList* jobs_list,JobsList* in_fg): BuiltInCommand(cmd_line), jobs_list(jobs_list),in_fg(in_fg), too_many_args(false),no_args(false){
        if (!cmd_args[1]){
            no_args = true;
            return;
        }
        else {
            job_id = std::stoi(std::string(cmd_args[1]));
            if (cmd_args[2] != nullptr) {
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
  BackgroundCommand(const char* cmd_line,char** cmd_args, JobsList* jobs_list): BuiltInCommand(cmd_line), jobs_list(jobs_list), too_many_args(false),no_args(false){
      if (!cmd_args[1]){
          no_args = true;
          return;
      }
      else {
          job_id = std::stoi(std::string(cmd_args[1]));
          if (cmd_args[2] != nullptr) {
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
    QuitCommand(const char* cmd_line,char** cmd_args, JobsList* jobs_list): BuiltInCommand(cmd_line), jobs_list(jobs_list) ,kill(false){
        if (strcmp(cmd_args[1],"kill") == 0){
            kill = true;
        }
    }
    virtual ~QuitCommand() {}
    void execute() override;
};

class ExternalCommand : public Command {
    const char* cmd_line;
    JobsList* jobs_list;
    JobsList* in_fg;
public:
    explicit ExternalCommand(const char* cmd_line,JobsList* jobs_list,JobsList* in_fg = nullptr): Command(cmd_line),cmd_line(cmd_line),jobs_list(jobs_list),in_fg(in_fg){};
    virtual ~ExternalCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    SmallShell* smash;
    JobsList* jobs_list;
    char* args[2];
    bool to_append;
    bool built_in;
public:
    RedirectionCommand(const char* cmd_line, bool to_append, bool built_in, JobsList* jobs_list, SmallShell* smash):Command(cmd_line), jobs_list(jobs_list) ,to_append(to_append), built_in(built_in), smash(smash){
        char* unconst_cmd_line = (char*)malloc(sizeof(cmd_line)+1);
        strcpy(unconst_cmd_line,cmd_line);
        args[0] = strtok(unconst_cmd_line, ">");
        args[1] = strtok(nullptr, ">");
        std::string argszero = std::string(args[0]);
        argszero.erase(std::remove(argszero.begin(), argszero.end(), ' '), argszero.end());
        strcpy(args[0], argszero.c_str());
        std::string argsone = std::string(args[1]);
        argsone.erase(std::remove(argsone.begin(), argsone.end(), ' '), argsone.end());
        strcpy(args[1], argsone.c_str());
    };
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};





/*
// TODO: add more classes if needed 
// maybe ls, timeout ?
*/

#endif //SMASH_COMMAND_H_
