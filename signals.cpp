#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    pid_t pid = getpid();

    SmallShell::getInstance().StopFG();
}

void ctrlCHandler(int sig_num) {
    pid_t pid = getpid();

    SmallShell::getInstance().KillFG();
}

void alarmHandler(int sig_num) {
    std::cout << "smash: got an alarm" << std::endl;
    if (!SmallShell::getInstance().alarmListEmpty()){
        SmallShell::getInstance().killAlarmedProcess();
    }
}

