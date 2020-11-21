#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);

}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = nullptr;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}
void goBack(char* current){
    *(strrchr(current, '/') + 1) = 0;
}
// TODO: Add your implementation for classes in Commands.h 

SmallShell::~SmallShell() {
    free(cur_dir);
    free(prev_dir);
}
std::string SmallShell::getName() {
    return shell_name;
}
void SmallShell::setName(std::string new_name) {
    shell_name = new_name;
}
char* SmallShell::getDir(){
    return cur_dir;
}
char * SmallShell::getPrevDir() {
    return prev_dir;
}
void SmallShell::updateDirs() {
    free(prev_dir);
    prev_dir = cur_dir;
    cur_dir = get_current_dir_name();
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = string(cmd_line);
  char* cmd_args[20];
  _parseCommandLine(cmd_line,cmd_args);
  if (cmd_s.find("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line,this);
  }
  else if (cmd_s.find("chprompt") == 0){
      return new ChangePromptCommand(cmd_line,cmd_args,this);
  }
  else if (cmd_s.find("ls") == 0){
      return new ShowFilesCommand(cmd_line,this);
  }
  else if (cmd_s.find("showpid") == 0){
      return new ShowPidCommand(cmd_line);
  }
  else if (cmd_s.find("cd") == 0){
      return new ChangeDirCommand(cmd_line,cmd_args,this);
  }
  else if (cmd_s.find("jobs") == 0){
      return new JobsCommand(cmd_line,jobs_list);
  }
  else if (cmd_s.find("kill") == 0){
      return new KillCommand(cmd_line,cmd_args,jobs_list);
  }
  else if (cmd_s.find("fg") == 0){
      return new ForegroundCommand(cmd_line,cmd_args,jobs_list);
  }
  else if (cmd_s.find("bg") == 0){
      return new ForegroundCommand(cmd_line,cmd_args,jobs_list);
  }
  else if (cmd_s.find("quit") == 0){
      return new QuitCommand(cmd_line,cmd_args,jobs_list);
  }
  else {
      return new ExternalCommand(cmd_line,jobs_list); //TODO:External command
  }
}

void SmallShell::executeCommand(const char *cmd_line) {
  Command* cmd = CreateCommand(cmd_line);
   cmd->execute();
}

void JobsList::addJob(Command *cmd,pid_t pid, bool is_stopped) {
    char* un_const_cmd_line = (char*)malloc(sizeof(cmd->getCmdLine()+1));
    strcpy(un_const_cmd_line,cmd->getCmdLine());
    _removeBackgroundSign(un_const_cmd_line);
    int max_job_id;
    if (getMaxJob() == nullptr){
        max_job_id=0;
    }
    else{
        max_job_id = getMaxJob()->getJobId();
    }
    auto new_job = new JobEntry(un_const_cmd_line,max_job_id+1, pid, false);
    jobs_list.push_back(new_job);
    num_jobs++;
}

void JobsList::printJobsList() {
    time_t now = time(0); //time when starting print
    for (auto job : jobs_list){
        double timeElapsed = difftime(now,job->getStartTime());
        cout << "[" << job->getJobId() << "] " << job->getCmdLine();
        if (job->isStopped()) {
            cout << " : " << job->getProcessId()  << " " << timeElapsed << " secs (stopped)" << endl;
        } else
            cout<< "& : " << job->getProcessId()  << " " << timeElapsed << " secs" << endl;
    }
}

void JobsList::killAllJobs() {
    for (auto job: jobs_list){
        cout << job->getProcessId() << ": " << job->getCmdLine();
        if (!job->isStopped()){
            cout << "&";
        }
        cout<<endl;
        int ret_val = kill(job->getProcessId(),SIGKILL);
        if(ret_val != 0)
            perror("smash error: kill failed");
    }
    jobs_list.clear();
}
void JobsList::removeFinishedJobs() {
    for (auto job:jobs_list){
        if(waitpid(job->getProcessId(),nullptr,WNOHANG) > 0){
            jobs_list.remove(job);
            num_jobs--;
        }
    }
}

JobsList::JobEntry *JobsList::getJobById(int job_id) {
    for (auto job: jobs_list) {
        if (job->getJobId() == job_id) {
            return job; // TODO: throw error if not found
        }
    }
}

void JobsList::removeJobById(int job_id) {

    for(auto job: jobs_list){
        if (job->getJobId() == job_id){
            jobs_list.remove(job);
            num_jobs--;
            return;
        }
    }
    return; //TODO: throw error not found
}

JobsList::JobEntry *JobsList::getLastJob(int *last_job_id) {
    return nullptr;
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *job_id) {
    JobEntry *found_job = nullptr;
    for (auto job : jobs_list) {
        if (job->isStopped()) {
            *job_id = job->getJobId();
            found_job = job;
        }
    }
    if (!found_job) {
        *job_id = -1;
    }
    return found_job;
}

int JobsList::getPid(int job_id) {
    for (auto job: jobs_list){
        if(job->getJobId() == job_id){
            return job->getProcessId();
        }
    }
    return -1;
}
bool JobsList::isEmpty() {
    return jobs_list.empty();
}
void JobsList::resumeJob(int job_id) {
    getJobById(job_id)->changeIsStopped(false);
}
int JobsList::getNumJobs() {
    return num_jobs;
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

void ChangePromptCommand::execute() {
    smash->setName(new_name);
}

void GetCurrDirCommand::execute() {
    cout << smash->getDir() << endl ;
}

void ChangeDirCommand::execute() {
    if(no_args){
        return;
    }
    if (too_many_args){
        std::cout<< "smash error: cd: too many arguments"<< std::endl;
    }
    else{
        char* dash = "-";
        if(*new_path == *dash){
            if(smash->getDir() == smash->getPrevDir()){
                cout << "smash error: cd: OLDPWD not set" << endl;
            }
            else{
                int ret_val = chdir(smash->getPrevDir());
                if (ret_val != 0){
                    perror("smash error: chdir failed");
                }
                else{
                    smash->updateDirs();
                }
            }
        }
        else if (new_path == ".."){
            if (smash->getDir() == "/"){
                return;
            }
            char* current = (char*)malloc(sizeof(smash->getDir())+1);
            goBack(current);
            int ret_val = chdir(current);
            free(current);
            if (ret_val != 0){
                perror("smash error: chdir failed");
            }
            else{
                smash->updateDirs();
            }
        }
        else{
        int retVal = chdir(new_path);
            if (retVal != 0){
                perror("smash error: chdir failed");
            }
            else{
                smash->updateDirs();
            }
        }
    }
}

void ShowFilesCommand::execute() {

    struct dirent **namelist;
    int n;
    int i = 0;
    n = scandir(".", &namelist, NULL, alphasort);
    if (n < 0)
        perror("smash error:scandir failed");
    else {
        while (i < n) {
            printf("%s\n", namelist[i]->d_name);
            free(namelist[i]);
            ++i;
        }
        free(namelist);
    }
}

void JobsCommand::execute() {
    if(!jobs_list){
        return;
    }
    jobs_list->removeFinishedJobs();
    jobs_list->printJobsList();
}

void KillCommand::execute() {
    int pid = jobs_list->getPid(job_id);
    if (!valid_input){
        cout<<"smash error: kill: invalid arguments"<<endl;
        return;
    }
    else if (pid == -1){
        cout<<"smash error: kill: job-id " << job_id << " does not exist"<<endl;
        return;
    }
    else{
        int ret_val = kill(pid,sig_num);
        if (ret_val != 0){
            perror("smash error: kill failed");
        }
        else{
            cout<< "signal number " << sig_num << " was sent to pid " << pid <<endl;
        }
    }
}

void ForegroundCommand::execute() {
    if(no_args){
        if(jobs_list->isEmpty()){
            cout<< "smash error: fg: jobs list is empty" << endl;
        }
        else{
            int pid = jobs_list->getMaxJob()->getProcessId();
            cout<< getCmdLine() << " : "<< pid << endl;
            kill(pid,SIGCONT);
            waitpid(pid,nullptr, 0);
            jobs_list->removeJobById(job_id);
        }
    }
    else if (too_many_args){
        cout <<"smash error: fg: invalid arguments" << endl;
    }
    else{
        int pid = jobs_list->getPid(job_id);
        if( pid == -1){
            cout<<"smash error: fg: job-id " << job_id << " does not exist"<<endl;
        }
        else{
            cout<< getCmdLine() << " : "<< pid << endl;
            kill(pid,SIGCONT);
            waitpid(pid,nullptr, 0);
            jobs_list->removeJobById(job_id);
        }
    }
}

void BackgroundCommand::execute() {
    if(no_args) {
        int lastStoppedId;
        if (!jobs_list->getLastStoppedJob(&lastStoppedId)) {
            cout << "smash error: bg: there is no stopped job to resume" << endl;
        } else {
            int pid = jobs_list->getJobById(lastStoppedId)->getProcessId();
            cout << getCmdLine() << " : " << pid << endl;
            kill(pid, SIGCONT);
            jobs_list->resumeJob(job_id); //TODO: maybe add &
        }
    }
    else if (too_many_args){
        cout <<"smash error: bg: invalid arguments" << endl;
    }
    else {
        int pid = jobs_list->getPid(job_id);
        if (pid == -1) {
            cout << "smash error: bg: job-id " << job_id << " does not exist" << endl;
        }
        else if (!jobs_list->getJobById(job_id)->isStopped()){
            cout << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
        }
        else {
            cout << getCmdLine() << " : " << pid << endl;
            kill(pid, SIGCONT);
            jobs_list->resumeJob(job_id); //TODO: maybe add &
        }
    }
}

void QuitCommand::execute() {
    if (kill){
        cout<< "sending SIGKILL signal to "<< jobs_list->getNumJobs() << "jobs:"<<endl;
        jobs_list->killAllJobs();
        }
    exit(0);
}

void ExternalCommand::execute() {
    char* un_const_cmd_line = (char*)malloc(sizeof(cmd_line+1));
    strcpy(un_const_cmd_line,cmd_line);
    char arg0[] = "/bin/bash";
    char arg1[] = "-c";
    char* args[] = {arg0,arg1,un_const_cmd_line,nullptr};
    pid_t child_pid = fork();
    cout<< child_pid<<endl;
    if (child_pid == -1){
        perror("smash error: fork failed");
    }
    else if(child_pid > 0){
        char* modified_cmd_line = (char*)malloc(sizeof(cmd_line)+1);
        strcpy(modified_cmd_line,cmd_line);
        _removeBackgroundSign(modified_cmd_line);
        int diff = strcmp(cmd_line,modified_cmd_line);
        if (diff){
            jobs_list->addJob(this,child_pid,false);
        }
        wait(nullptr);
    }
    else{
       // setpgrp()
        execv("/bin/bash",args);

    }

}
