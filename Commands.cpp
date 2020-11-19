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

bool _isBackgroundComamnd(const char* cmd_line) {
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

SmallShell::SmallShell() {
    shellName = "smash";
    CurDir =  get_current_dir_name();
    PrevDir = get_current_dir_name();
}

SmallShell::~SmallShell() {
    free(CurDir);
    free(PrevDir);
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
  }/*
  else if (cmd_s.find("fg") == 0){
      return new ForegroundCommand(cmd_line,jobs_list);
  }
  else if (cmd_s.find("fg") == 0){
      return new ForegroundCommand(cmd_line,jobs_list);
  }
  else if (cmd_s.find("quit") == 0){
      return new QuitCommand(cmd_line,jobs_list);
  }*/
  else {
    return nullptr;
    //new ExternalCommand(cmd_line);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  Command* cmd = CreateCommand(cmd_line);
   cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    return nullptr;
}

void JobsList::addJob(Command *cmd, bool isStopped) {
    char* unconstCmdLine = (char*)malloc(sizeof(cmd->getCmdLine()+1));
    strcpy(unconstCmdLine,cmd->getCmdLine());
    _removeBackgroundSign(unconstCmdLine);
    auto new_job = new JobEntry(unconstCmdLine);
    jobsList.push_back(new_job);
    numJobs++;
}

void JobsList::printJobsList() {
    time_t now = time(0); //time when starting print
    for (auto job : jobsList){
        double timeElapsed = difftime(now,job->getStartTime());
        cout << "[" << job->getJobId() << "] " << job->getCmdLine();
        if (job->isStopped()) {
            cout << " : " << job->getProcessId()  << " " << timeElapsed << " secs (stopped)" << endl;
        } else
            cout<< "& : " << job->getProcessId()  << " " << timeElapsed << " secs" << endl;
    }
}

void JobsList::killAllJobs() {
    for (auto job: jobsList){
        cout << job->getProcessId() << ": " << job->getCmdLine();
        if (!job->isStopped()){
            cout << "&";
        }
        cout<<endl;
        kill(job->getProcessId(),SIGKILL);
    }
    jobsList.clear();
}

void JobsList::removeFinishedJobs() {
    for (auto job:jobsList){
        if(waitpid(job->getProcessId(),nullptr,WNOHANG) > 0){
            jobsList.remove(job);
            numJobs--;
        }
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    for(auto job: jobsList) {
        if (job->getJobId() == jobId) {
            return job; // TODO: throw error if not found
        }
    }
}

void JobsList::removeJobById(int jobId) {

    for(auto job: jobsList){
        if (job->getJobId() == jobId){
            jobsList.remove(job);
            numJobs--;
            return;
        }
    }
    return; //TODO: throw error not found
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    JobEntry *foundJob = nullptr;
    for (auto job : jobsList) {
        if (job->isStopped()) {
            *jobId = job->getJobId();
            foundJob = job;
        }
    }
    if (!foundJob) {
        *jobId = -1;
    }
    return foundJob;
}

int JobsList::getPid(int jobId) {
    for (auto job: jobsList){
        if(job->getJobId() == jobId){
            return job->getProcessId();
        }
    }
    return -1;
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

void ChangePromptCommand::execute() {
    smash->setName(newName);
}

void GetCurrDirCommand::execute() {
    cout << smash->getDir() << endl ;
}
void ChangeDirCommand::execute() {
    if (!isInputValid){
        std::cout<< "smash error: cd: too many arguments"<< std::endl;
    }
    else{
        char* dash = "-";
        if(*newPath == *dash){
            if(smash->getDir() == smash->getPrevDir()){
                cout << "smash error: cd: OLDPWD not set" << endl;
            }
            else{
                int retVal = chdir(smash->getPrevDir());
                if (retVal != 0){
                    perror("smash error: chdir failed");
                }
                else{
                    smash->updateDirs();
                }
            }
        }
        else if (newPath == ".."){
            if (smash->getDir() == "/"){
                return;
            }
            char* current = (char*)malloc(sizeof(smash->getDir())+1);
            goBack(current);
            int retVal = chdir(current);
            free(current);
            if (retVal != 0){
                perror("smash error: chdir failed");
            }
            else{
                smash->updateDirs();
            }
        }
        else{
        int retVal = chdir(newPath);
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
    std::set<std::string> content;
    struct dirent *de;
    DIR *dr = opendir(smash->getDir());
    while ((de = readdir(dr)) != NULL){
        content.insert(de->d_name);
    }
    closedir(dr);
    for (auto j:content ){
        cout << j << endl;
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
    int pid = jobs_list->getPid(jobId);
    if (!validInput){
        cout<<"smash error: kill: invalid arguments"<<endl;
        return;
    }
    else if (pid == -1){
        cout<<"smash error: kill: job-id " << jobId << " does not exist"<<endl;
        return;
    }
    else{
        int retVal = kill(pid,sigNum);
        if (retVal != 0){
            perror("smash error: kill failed");
        }
        else{
            cout<< "signal number " << sigNum << " was sent to pid " << pid <<endl;
        }
    }
}

void ForegroundCommand::execute() {
    if(noArgs){
        if(jobs_list->isEmpty()){
            cout<< "smash error: fg: jobs list is empty" << endl;
        }
        else{
            int pid = jobs_list->getMaxJob()->getProcessId();
            cout<< getCmdLine() << " : "<< pid << endl;
            kill(pid,SIGCONT);
            waitpid(pid,nullptr, 0);
            jobs_list->removeJobById(jobId);
        }
    }
    else if (tooManyArgs){
        cout <<"smash error: fg: invalid arguments" << endl;
    }
    else{
        int pid = jobs_list->getPid(jobId);
        if( pid == -1){
            cout<<"smash error: fg: job-id " << jobId << " does not exist"<<endl;
        }
        else{
            cout<< getCmdLine() << " : "<< pid << endl;
            kill(pid,SIGCONT);
            waitpid(pid,nullptr, 0);
            jobs_list->removeJobById(jobId);
        }
    }
}

void BackgroundCommand::execute() {
    if(noArgs) {
        int lastStoppedId;
        if (!jobs_list->getLastStoppedJob(&lastStoppedId)) {
            cout << "smash error: bg: there is no stopped job to resume" << endl;
        } else {
            int pid = jobs_list->getJobById(lastStoppedId)->getProcessId();
            cout << getCmdLine() << " : " << pid << endl;
            kill(pid, SIGCONT);
            jobs_list->resumeJob(jobId); //TODO: maybe add &
        }
    }
    else if (tooManyArgs){
        cout <<"smash error: bg: invalid arguments" << endl;
    }
    else {
        int pid = jobs_list->getPid(jobId);
        if (pid == -1) {
            cout << "smash error: bg: job-id " << jobId << " does not exist" << endl;
        }
        else if (!jobs_list->getJobById(jobId)->isStopped()){
            cout << "smash error: bg: job-id " << jobId << " is already running in the background" << endl;
        }
        else {
            cout << getCmdLine() << " : " << pid << endl;
            kill(pid, SIGCONT);
            jobs_list->resumeJob(jobId); //TODO: maybe add &
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

