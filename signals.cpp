#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    SmallShell::getInstance().StopFG();
}

void ctrlCHandler(int sig_num) {
    SmallShell::getInstance().KillFG();
}

void alarmHandler(int sig_num) {
    if (!SmallShell::getInstance().alarmListEmpty()){
        std::cout << "smash: got an alarm" << std::endl;
        SmallShell::getInstance().killAlarmedProcess();
    }
}

