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
#include <sys/wait.h>

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
class TimeoutEntry{
private:
    const char* org_cmd_line;
    time_t timestamp;
    int duration;
    pid_t pid;
    int job_id;
public:
    TimeoutEntry(const char* cmd_line,int duration,pid_t pid, int job_id): org_cmd_line(cmd_line),duration(duration),pid(pid), job_id(job_id){
        timestamp = time(nullptr);
    }
    ~TimeoutEntry(){}
    time_t getTimeStamp(){
        return timestamp;
    }
    int getDuration(){
        return duration;
    }
    pid_t getPid(){
        return pid;
    }
    const char* getCmdLine(){
        return org_cmd_line;
    }
    int getJobId(){
        return job_id;
    }
};
class JobsList {
private:
  class JobEntry {
      char* cmd_line;
      int process_id;
      bool stopped;
      char* original_cmd_line;
      int job_id; //TODO: keep the ID even if moved to fg and then back to bg
      time_t start_time;
  public:
      JobEntry(char* cmd_line,int pid, bool stopped, const char* org_cmd_line,int id = -1):cmd_line(cmd_line),process_id(pid), stopped(stopped),job_id(id){
          start_time = time(nullptr);
          original_cmd_line = (char*)malloc(PATH_MAX);
          strcpy(original_cmd_line,org_cmd_line);
      };
      ~JobEntry(){
          free(original_cmd_line);
      }
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

  std::list<JobEntry*>* jobs_list;
  int num_jobs;
  int id_in_fg;
    // TODO: Add your data members
 public:
  JobsList(): jobs_list(new std::list<JobEntry*>),num_jobs(0),id_in_fg(0){};
  ~JobsList(){};
  void addJob(const char* cmd_line,pid_t pid, int cur_job_id,int* new_id, bool stopped = false);
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
      if (jobs_list->empty())
          return 0;
      removeFinishedJobs();
      jobs_list->sort(compareJobEntries);
      return jobs_list->back()->getJobId();
  }
  void resumeJob(int job_id);
  int getNumJobs();
  void clearJobs(){
      if (!jobs_list->empty())
        jobs_list->clear();
      num_jobs = 0;
  }
  void updateIdInFg(int job_id){
      id_in_fg = job_id;
  }

  int getJobId(){
      return jobs_list->front()->getJobId();
  }
  int getFGPid(){
      return jobs_list->front()->getProcessId();
  }
  const char* getFGCmdLine(){
      return jobs_list->front()->getOrgCmdLine();
  }
  static bool compareJobEntries(JobEntry* j1, JobEntry* j2){
      return j1->getJobId() < j2->getJobId();
  }

};

class SmallShell {
private:
    pid_t smash_pid;
    char* prev_dir;
    char* cur_dir;
    JobsList* jobs_list;
    JobsList* in_fg;
    std::string shell_name;
    std::list<TimeoutEntry*>* timeout_list;
    unsigned int overall_max_job_id;
    // TODO: Add your data members
public:
    SmallShell():jobs_list(new JobsList),in_fg(new JobsList),shell_name("smash"),timeout_list(new std::list<TimeoutEntry*>),overall_max_job_id(0){
        cur_dir =  get_current_dir_name();
        prev_dir = get_current_dir_name();
        smash_pid = getpid();
    };
    ~SmallShell();
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    std::string getName();
    void setName(std::string new_name);
    char* getDir();
    char* getPrevDir();
    void updateDirs();
    void updateOverallMax(int new_max){
        overall_max_job_id = new_max;
    }
    int getOverallMax(){
        in_fg->removeFinishedJobs();
        jobs_list->removeFinishedJobs();
        int max_in_fg = in_fg->getMaxJob();
        int max_in_bg = jobs_list->getMaxJob();
        overall_max_job_id = fmax(max_in_bg,max_in_fg);
        return overall_max_job_id;
    }
    Command *CreateCommand(const char* cmd_line);
    void executeCommand(const char *cmd_line);
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    pid_t getSmashPid(){
        return smash_pid;
    }
    static bool cmp_alarms(TimeoutEntry* t1,  TimeoutEntry* t2){
        time_t now = time(nullptr);
        time_t start1 = t1->getTimeStamp();
        int duration1 = t1->getDuration();
        int time_past1 = now - start1;
        time_t start2 = t2->getTimeStamp();
        int duration2 = t2->getDuration();
        int time_past2 = now - start2;

        int t1_time_left = duration1 - time_past1;
        int t2_time_left = duration2 - time_past2;
        return t1_time_left < t2_time_left;
    }
     void clearFinishedTimeoutProcesses(){
        std::vector<TimeoutEntry*> to_remove;

        for (auto process:*timeout_list){
            if(waitpid(process->getPid(),nullptr,WNOHANG) > 0){
                to_remove.push_back(process);
            }
        }
        for (auto process:to_remove){
            timeout_list->remove(process);
        }
    }
    bool alarmListEmpty(){
        return timeout_list->empty();
    }
    void StopFG() {
        std::cout << "smash: got ctrl-Z" << std::endl;
        jobs_list->removeFinishedJobs();
        if (!in_fg->isEmpty()){
            pid_t pid = in_fg->getFGPid();
            int new_job_id;
            jobs_list->addJob(in_fg->getFGCmdLine(), pid, in_fg->getJobId(), &new_job_id,true);
            int ret_val = killpg(pid, SIGSTOP);
            if (ret_val != 0){
                perror("smash error: kill failed");
                return;
            }
            std::cout << "smash: process " << pid << " was stopped" << std::endl;
        }

    }

    void KillFG(){
        jobs_list->removeFinishedJobs();
        std::cout << "smash: got ctrl-C" << std::endl;
        if (!in_fg->isEmpty()){
            int cur_max = SmallShell::getInstance().getOverallMax();
            int job_id = in_fg->getJobId();
            if (cur_max == job_id)
                SmallShell::getInstance().updateOverallMax(jobs_list->getMaxJob());
            pid_t pid = in_fg->getFGPid();
            int ret_val = killpg(pid, SIGKILL);
            if (ret_val != 0){
                perror("smash error: kill failed");
                return;
            }
            std::cout << "smash: process " << pid << " was killed" << std::endl;
        }
    }

    void SetAlarm(){
       // alarm(0);
        if (timeout_list->empty())
            return;
        //clearFinishedTimeoutProcesses();
        jobs_list->removeFinishedJobs();
        timeout_list->sort(cmp_alarms);
        TimeoutEntry* min_timeout = timeout_list->front();
        time_t now = time(nullptr);
        time_t start = min_timeout->getTimeStamp();
        int duration = min_timeout->getDuration();
        int time_past = now - start;
        int alarm_time = duration - time_past;
        int ret = alarm(alarm_time);

        if (ret < 0){
            perror("smash error: alarm failed");
            return;
        }
    }

    void killAlarmedProcess(){
        TimeoutEntry* min_timeout = timeout_list->front();
        pid_t pid_to_kill = min_timeout->getPid();
        int min_timeout_job_id = min_timeout->getJobId();
        jobs_list->removeFinishedJobs();
        int cur_max = SmallShell::getInstance().getOverallMax();
        if (cur_max == min_timeout_job_id)
            SmallShell::getInstance().updateOverallMax(jobs_list->getMaxJob());

        if (!jobs_list->getJobById(min_timeout_job_id) && !in_fg->getJobById(min_timeout_job_id)){
            timeout_list->remove(min_timeout);
            SetAlarm();
            return;
        }
        std::cout << "smash: " << min_timeout->getCmdLine() <<" timed out!" <<std::endl;
        int ret = killpg(pid_to_kill,SIGKILL);
        if (ret != 0){
            perror("smash error: kill failed");
            return;
        }
        timeout_list->remove(min_timeout);
        jobs_list->removeFinishedJobs();

        SetAlarm();
    }
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line):Command(cmd_line){};
    virtual ~BuiltInCommand() {}
};
class ShowPidCommand : public BuiltInCommand {
    SmallShell* smash;
public:
    ShowPidCommand(const char* cmd_line, SmallShell* smash): BuiltInCommand(cmd_line), smash(smash){};
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
    JobsList* in_fg;
public:
    JobsCommand(const char* cmd_line,JobsList* jobs_list,JobsList* in_fg):BuiltInCommand(cmd_line), jobs_list(jobs_list),in_fg(in_fg){};
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
    bool invalid_job;
    if (!cmd_args[1] || *cmd_args[1] != '-' ){
        valid_input = false;
        return;
    }
    else {
        //check if signum is legit
        std::string str(cmd_args[1]);
        for (unsigned int i = 1;i<str.length(); i++){
            if (!isdigit(*(cmd_args[1] + i)))
                valid_input = false;
        }
        if (valid_input)
            sig_num = std::stoi(std::string(cmd_args[1] + 1));

        if (!cmd_args[2]) {
            valid_input = false;
            return;
        } else {
            //check if job is negative
            bool is_neg = *cmd_args[2] == '-';
            //check if job is legit
            std::string str(cmd_args[2]);
            for (unsigned int i = 1;i<str.length(); i++){
                if (!isdigit(*(cmd_args[2] + i))) {
                    valid_input = false;
                    invalid_job = true;
                }
            }

            if (!invalid_job){
                if (is_neg) {
                    job_id = std::stoi(std::string(cmd_args[2] + 1));
                    job_id = job_id * -1;
                }
                else
                    job_id = std::stoi(std::string(cmd_args[2]));

            }

            if (cmd_args[3] != nullptr) {
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
        if (!cmd_args[1])
            kill = false;
        else if (strcmp(cmd_args[1],"kill") == 0){
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
    bool to_append;
    bool built_in;
    JobsList* jobs_list;
    SmallShell* smash;
    JobsList* in_fg;
    std::list<TimeoutEntry*>* timeout_list;
    char* args[2];
    char* cmd_args[20];
    bool is_bg;
public:
    RedirectionCommand(const char* cmd_line, bool to_append, bool built_in, JobsList* jobs_list, SmallShell* smash,JobsList* in_fg, char** cmd_args_in, std::list<TimeoutEntry*>* timeout_list);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};
class TimeoutCommand:public Command{
    const char* cmd_line;
    std::list<TimeoutEntry*>* timeout_list;
    SmallShell* smash;
    JobsList* jobs_list;
    JobsList* in_fg;
    char* cmd_to_exe;
    int duration;
    bool valid_input;
public:
    TimeoutCommand(const char* cmd_line, char** args, std::list<TimeoutEntry*>* timeout_list, SmallShell* smash, JobsList* jobs_list, JobsList* in_fg): Command(cmd_line),cmd_line(cmd_line), timeout_list(timeout_list), smash(smash), jobs_list(jobs_list), in_fg(in_fg){
        if (!args[1]){ // checks if written just: TIMEOUT
            valid_input = false;
            return;
        }
        else{
            if(*args[1] == '-'){  // checks if duration is negative
                valid_input = false;
                return;
            }
            std::string str(args[1]);
            for (unsigned int i = 1;i<str.length(); i++){
                if (!isdigit(*(args[1] + i))){
                    valid_input = false;
                    return;
                    }
                }
            // time is provided and legit
            duration = std::stoi(std::string(args[1]));

            if(!args[2]){
                valid_input = false;
                return;
            }
            std::string str1(args[2]);
            std::string str2 (" ");
            std::string full(str1+str2);
            int i = 3;
            while (args[i]){
                std::string str3(args[i]);
                str3 = str3 + " ";
                full.append(str3);
                i++;
            }
            cmd_to_exe = (char*) malloc (PATH_MAX);
            strcpy(cmd_to_exe,full.c_str());
            valid_input = true;
        }
    }
    virtual ~TimeoutCommand() {
        free(cmd_to_exe);
    }
    void execute() override;
};

class PipeCommand : public Command {
    char* args[2];
    bool err;
    SmallShell* smash;
public:
    PipeCommand( const char* cmd_line, bool err, SmallShell* smash);
    virtual ~PipeCommand() {}
    void execute() override;
};
class CopyDirCommand : public BuiltInCommand {
    char* old_path;
    char* new_path;
    bool is_bg;
    JobsList* jobs_list;
    JobsList* in_fg;
    bool valid_input;
public:
    CopyDirCommand(const char *cmd_line, char **cmd_args,JobsList* jobs_list, JobsList* in_fg);
    ~CopyDirCommand() override = default;
    void execute() override;
};





/*
// TODO: add more classes if needed 
// maybe ls, timeout ?
*/

#endif //SMASH_COMMAND_H_
