#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";
#define NUM_BUILT_IN_CMD 10

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
bool isBuiltIn(char* cmd_name){
    std::string cmd(cmd_name);
    char* commands[] = {"chprompt","ls", "showpid", "pwd","cd","jobs","kill","fg","bg","quit"};
    for (int i = 0 ; i < NUM_BUILT_IN_CMD ; i++){
        if (cmd.find(commands[i])!=-1){
            return true;
        }
    }
    return false;
}
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
     bool builtIn = isBuiltIn(cmd_args[0]);
    if (cmd_s.find(">>")!=-1) {
        return new RedirectionCommand(cmd_line,true,builtIn,jobs_list,this);
    }
    else if (cmd_s.find(">")!=-1) {
        return new RedirectionCommand(cmd_line,false,builtIn,jobs_list,this);
    }
    else if (cmd_s.find("|&")!=-1) {
        return new PipeCommand(cmd_line,true);
    }
    else if (cmd_s.find("|")!=-1) {
        return new PipeCommand(cmd_line,false);
    }
     else if (cmd_s.find("pwd") == 0) {
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
         return new JobsCommand(cmd_line,jobs_list,in_fg);
     }
     else if (cmd_s.find("kill") == 0){
         return new KillCommand(cmd_line,cmd_args,jobs_list);
     }
     else if (cmd_s.find("fg") == 0){
         return new ForegroundCommand(cmd_line,cmd_args,jobs_list,in_fg);
     }
     else if (cmd_s.find("bg") == 0){
         return new BackgroundCommand(cmd_line,cmd_args,jobs_list);
     }
     else if (cmd_s.find("quit") == 0){
         return new QuitCommand(cmd_line,cmd_args,jobs_list);
     }
    else if (cmd_s.find("timeout") == 0) {
        return new TimeoutCommand(cmd_line, cmd_args, timeout_list, this, jobs_list, in_fg);
    }
    else {
         return new ExternalCommand(cmd_line,jobs_list,in_fg); //TODO:External command
    }
}

void SmallShell::executeCommand(const char *cmd_line) {
  Command* cmd = CreateCommand(cmd_line);
   cmd->execute();
}

void JobsList::addJob(const char* cmd_line,pid_t pid,int cur_job_id, int* new_id,bool is_stopped) {
    removeFinishedJobs();
    char* un_const_cmd_line = (char*)malloc(sizeof(cmd_line)+1);
    strcpy(un_const_cmd_line,cmd_line);
    _removeBackgroundSign(un_const_cmd_line);
    if (cur_job_id == -1) {
        int new_job_id = getMaxJob()+1;
        auto new_job = new JobEntry(un_const_cmd_line, pid, is_stopped, cmd_line, new_job_id);
        *new_id = new_job_id;
        updateIdInFg(new_job_id);
        jobs_list.push_back(new_job);
    }
    else {
        auto new_job = new JobEntry(un_const_cmd_line, pid, is_stopped, cmd_line, cur_job_id);
        *new_id = cur_job_id;
        updateIdInFg(cur_job_id);
        jobs_list.push_back(new_job);
    }
    num_jobs++;
}

void JobsList::printJobsList() {
    time_t now = time(0); //time when starting print
    jobs_list.sort(compareJobEntries);
    for (auto job : jobs_list){
        double timeElapsed = difftime(now,job->getStartTime());
        cout << "[" << job->getJobId() << "] " << job->getOrgCmdLine() <<  " : " << job->getProcessId()  << " " << timeElapsed << " secs ";
        if (job->isStopped()) {
            cout <<"(stopped)" << endl;
        } else
            cout << endl;
    }
}

void JobsList::killAllJobs() {
    for (auto job: jobs_list){
        cout << job->getProcessId() << ": " << job->getCmdLine();
        if (!job->isStopped()){
            cout << "&";
        }
        cout<<endl;
    }
    for (auto job : jobs_list){
        int ret_val = kill(job->getProcessId(),SIGKILL);
        if(ret_val != 0)
            perror("smash error: kill failed");
        exit(0);
    }
    jobs_list.clear();
}
void JobsList::removeFinishedJobs() {
    std::list<JobEntry*> to_remove;

    for (auto job:jobs_list){
        if(waitpid(job->getProcessId(),nullptr,WNOHANG) > 0){
            to_remove.push_back(job);
        }
    }
    for (auto job:to_remove){
        jobs_list.remove(job);
        num_jobs--;
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
                    exit(0);
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
                exit(0);
            }
            else{
                smash->updateDirs();
            }
        }
        else{
        int retVal = chdir(new_path);
            if (retVal != 0){
                perror("smash error: chdir failed");
                exit(0);
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
    if (n < 0){
        perror("smash error:scandir failed");
        exit(0);
    }
    else{
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
    in_fg->updateIdInFg(0);
    jobs_list->updateIdInFg(0);
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
            exit(0);
        }
        else{
            cout<< "signal number " << sig_num << " was sent to pid " << pid <<endl;
        }
    }
}

void ForegroundCommand::execute() {
    jobs_list->removeFinishedJobs();
    jobs_list->updateIdInFg(-1);
    in_fg->updateIdInFg(-1);
    if(no_args){
        if(jobs_list->isEmpty()){
            cout<< "smash error: fg: jobs list is empty" << endl;
        }
        else{
            int job_id = jobs_list->getMaxJob();
            int pid = jobs_list->getPid(job_id);
            cout<< jobs_list->getJobById(job_id)->getOrgCmdLine() << " : "<< pid << endl;
            in_fg->clearJobs();
            in_fg->updateIdInFg(0);
            jobs_list->updateIdInFg(0);
            int new_job_id;
            in_fg->addJob(jobs_list->getJobById(job_id)->getOrgCmdLine(),pid,job_id,&new_job_id,true);
            in_fg->updateIdInFg(new_job_id);
            jobs_list->updateIdInFg(new_job_id);
            kill(pid,SIGCONT);
            waitpid(pid,nullptr, 0);
            jobs_list->removeJobById(job_id);
            jobs_list->updateIdInFg(job_id);
            in_fg->updateIdInFg(job_id);
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
            cout<< jobs_list->getJobById(job_id)->getOrgCmdLine()  << " : "<< pid << endl;
            in_fg->clearJobs();
            in_fg->updateIdInFg(0);
            jobs_list->updateIdInFg(0);
            int new_job_id;
            in_fg->addJob(jobs_list->getJobById(job_id)->getOrgCmdLine(),pid,job_id,&new_job_id,true);
            in_fg->updateIdInFg(new_job_id);
            jobs_list->updateIdInFg(new_job_id);
            kill(pid,SIGCONT);
            waitpid(pid,nullptr, 0);
            jobs_list->removeJobById(job_id);
            jobs_list->updateIdInFg(job_id);
            in_fg->updateIdInFg(job_id);
        }
    }
}

void BackgroundCommand::execute() {
    jobs_list->removeFinishedJobs();
    if(no_args) {
        int lastStoppedId;
        if (!jobs_list->getLastStoppedJob(&lastStoppedId)) {
            cout << "smash error: bg: there is no stopped job to resume" << endl;
        } else {
            int pid = jobs_list->getJobById(lastStoppedId)->getProcessId();
            cout << jobs_list->getJobById(job_id)->getOrgCmdLine() << " : " << pid << endl;
            kill(pid, SIGCONT);
            jobs_list->resumeJob(job_id);
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
            cout << jobs_list->getJobById(job_id)->getOrgCmdLine() << " : " << pid << endl;
            kill(pid, SIGCONT);
            jobs_list->resumeJob(job_id);
        }
    }
}

void QuitCommand::execute() {
    if (kill){
        cout<< "sending SIGKILL signal to "<< jobs_list->getNumJobs() << " jobs:"<<endl;
        jobs_list->killAllJobs();
        }
    exit(0);
}

void ExternalCommand::execute() {
    char* un_const_cmd_line = (char*)malloc(sizeof(cmd_line)+1);
    strcpy(un_const_cmd_line,cmd_line);
    char arg0[] = "/bin/bash";
    char arg1[] = "-c";
    _removeBackgroundSign(un_const_cmd_line);
    char* args[] = {arg0,arg1,un_const_cmd_line,nullptr};
    pid_t child_pid = fork();
    if (child_pid == -1){
        perror("smash error: fork failed");
        exit(0);
    }
    else if(child_pid > 0){
        char* modified_cmd_line = (char*)malloc(sizeof(cmd_line)+1);
        strcpy(modified_cmd_line,cmd_line);
        _removeBackgroundSign(modified_cmd_line);
        int diff = strcmp(cmd_line,modified_cmd_line);
        if (diff){
            int new_job_id;
            jobs_list->addJob(cmd_line,child_pid,-1,&new_job_id,false);
        }
        if(!_isBackgroundCommand(cmd_line)){
            int new_job_id;
            in_fg->clearJobs();
            in_fg->updateIdInFg(0);
            jobs_list->updateIdInFg(0);
            in_fg->addJob(cmd_line,child_pid,-1,&new_job_id,false);
            in_fg->updateIdInFg(new_job_id);
            jobs_list->updateIdInFg(new_job_id);
            waitpid(child_pid, nullptr,WUNTRACED);
        }
    }
    else{
        setpgrp();
        execv("/bin/bash",args);

    }

}

void RedirectionCommand::execute() {
    if (!built_in) {
        ExternalCommand *cmd = new ExternalCommand(this->getCmdLine(), jobs_list);
        cmd->execute();
    } else {
        if (to_append) {
            int fd = open(args[1], O_WRONLY | O_CREAT | O_APPEND, 0666);
            if (fd == -1) {
                perror("smash error: open failed");
                exit(0);
            }
            int old_std = dup(1);
            if (old_std == -1) {
                perror("smash error: dup failed");
                exit(0);
            }
            if (dup2(fd, 1) == -1) {
                perror("smash error: dup2 failed");
                exit(0);
            }
            smash->executeCommand(args[0]);
            int ret_val = close(fd);
            if (ret_val) {
                perror("smash error: close failed");
                exit(0);
            }
            if (dup2(old_std, 1) == -1) {
                perror("smash error: dup2 failed");
                exit(0);
            }

        } else {
            int fd = open(args[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1) {
                perror("smash error: open failed");
                exit(0);
            }
            int old_std = dup(1);
            if (old_std == -1) {
                perror("smash error: dup failed");
                exit(0);
            }
            if (dup2(fd, 1) == -1) {
                perror("smash error: dup2 failed");
                exit(0);
            }
            smash->executeCommand(args[0]);
            int ret_val = close(fd);
            if (ret_val) {
                perror("smash error: close failed");
                exit(0);
            }
            if (dup2(old_std, 1) == -1) {
                perror("smash error: dup2 failed");
                exit(0);
            }
        }
    }
}

void PipeCommand::execute() {
    int fd[2];
    int ret_val = pipe(fd);
    if (ret_val == -1) {
        perror("smash error: pipe failed");
        exit(0);
    }
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("smash error: fork failed");
        exit(0);
    }
    if (pid1 == 0) { // first child
        if (err) {
            dup2(fd[1], 2);
        } else {
            dup2(fd[1], 1);
        }
        if (close(fd[0]) == -1 || close(fd[1]) == -1) {
            perror("smash error : close failed");
            exit(0);
        }

        //execv(....)// TODO: fill
    }
    pid_t pid2 = fork();
    if (pid1 < 0) {
        perror("smash error: fork failed");
        exit(0);
    }
    if (pid2 == 0) { //second child
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        //execv(....)// TODO: fill
    }
    close(fd[0]);
    close(fd[1]);
}


void TimeoutCommand::execute() {
    /*char* cmd = (char*)malloc(sizeof(cmd_line)+1);
    strcpy(cmd,cmd_line);
    std::string str(cmd);
    int first_num_index = str.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ");
    std::string str1(cmd + first_num_index);
    int command_index = str1.find_first_not_of("0123456789 ");
    std::string str2(cmd + first_num_index + command_index);
    char* parsed_cmd_line = (char *) malloc(sizeof(cmd + first_num_index + command_index) + 1);
    strcpy(parsed_cmd_line, str2.c_str());
    //parsed_cmd_line holds process cmd_line to send to external
     */

    char* un_const_cmd_line = (char*)malloc(sizeof(cmd_to_exe)+1);
    strcpy(un_const_cmd_line,cmd_to_exe);
    char arg0[] = "/bin/bash";
    char arg1[] = "-c";
    _removeBackgroundSign(un_const_cmd_line);
    //un_const = ex command without sign (send to bash)
    //parsed = ex command with sign ( add to jobs list)
    char* args[] = {arg0,arg1,un_const_cmd_line,nullptr};
    pid_t child_pid = fork();
    if (child_pid == -1){
        perror("smash error: fork failed");
        exit(0);
    }
    else if(child_pid > 0){
        char* modified_cmd_line = (char*)malloc(sizeof(cmd_to_exe)+1);
        strcpy(modified_cmd_line,cmd_to_exe);
        _removeBackgroundSign(modified_cmd_line);
        int diff = strcmp(cmd_to_exe,modified_cmd_line);
        TimeoutEntry* timeout_entry = new TimeoutEntry(cmd_line,duration,child_pid);
        timeout_list->push_back(timeout_entry);
        smash->SetAlarm();
        if (diff){
            int new_job_id;
            jobs_list->addJob(cmd_line,child_pid,-1,&new_job_id,false);
        }
        if(!_isBackgroundCommand(cmd_line)){
            int new_job_id;
            in_fg->clearJobs();
            in_fg->updateIdInFg(0);
            jobs_list->updateIdInFg(0);
            in_fg->addJob(cmd_line,child_pid,-1,&new_job_id,false);
            in_fg->updateIdInFg(new_job_id);
            jobs_list->updateIdInFg(new_job_id);
            waitpid(child_pid, nullptr,WUNTRACED);
        }
    }
    else{
        setpgrp();
        execv("/bin/bash",args);
    }

}
