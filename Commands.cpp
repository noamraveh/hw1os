#include <unistd.h>
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

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
    LastWorkingDir = ;//add address of initial dir
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = string(cmd_line);
  char* cmd_args[20];
  _parseCommandLine(cmd_line,cmd_args);
  if (cmd_s.find("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (cmd_s.find("chprompt") == 0){
      return new ChangePromptCommand(cmd_line,cmd_args,this);
  }
  else if (cmd_s.find("ls") == 0){
      return new ShowFilesCommand(cmd_line);
  }
  else if (cmd_s.find("showpid") == 0){
      return new ShowPidCommand(cmd_line);
  }
  else if (cmd_s.find("cd") == 0){
      return new ChangeDirCommand(cmd_line,cmd_args);
  }
  else if (cmd_s.find("jobs") == 0){
      return new JobsCommand(cmd_line,jobs_list);
  }
  else if (cmd_s.find("kill") == 0){
      return new KillCommand(cmd_line,jobs_list);
  }
  else if (cmd_s.find("fg") == 0){
      return new ForegroundCommand(cmd_line,jobs_list);
  }
  else if (cmd_s.find("fg") == 0){
      return new ForegroundCommand(cmd_line,jobs_list);
  }
  else if (cmd_s.find("quit") == 0){
      return new QuitCommand(cmd_line,jobs_list);
  }
  else {
    return new ExternalCommand(cmd_line);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    return nullptr;
}

void JobsList::addJob(Command *cmd, bool isStopped) {


}

void JobsList::printJobsList() {
    time_t now = time(0); //time when starting print
    for (auto job : jobsList){
        double timeElapsed = difftime(now,job.startTime);
        cout << "[" << job.jobId << "] " << job.command.cmdName << " : " << job.processId  << " " << timeElapsed << " secs" ;
        if (job.isStopped) {
            cout << " (stopped)" << endl;
        } else
            cout << endl;
    }
}

void JobsList::killAllJobs() {
    jobsList.clear();
}

void JobsList::removeFinishedJobs() {

}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    auto it = jobsList.begin();
    while (it != jobsList.end()){
        if (it->jobId == jobId)
            return &*it;
        it++;
    }
    if (it->jobId == jobId)
        return &*it;
    else
        //throw error not found


}

void JobsList::removeJobById(int jobId) {
    auto it = jobsList.begin();
    while (it != jobsList.end()){
        if (it->jobId == jobId)
            jobsList.erase(it);
        it++;
    }
    if (it->jobId == jobId)
        jobsList.erase(it);
    else
    //throw error not found
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    auto it = jobsList.end();
    while (it != jobsList.begin()){
        if (it->isStopped) {
            *jobId = it->jobId;
            return &*it;
        }
        it++;
    }
    if (it->isStopped){
        *jobId = it->jobId;
        return &*it;
    } else
        //error
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

void ChangePromptCommand::execute() {
    smash->setName(newName);
}

void GetCurrDirCommand::execute() {
    cout << getcwd() << endl ;
}
