#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER 1024

struct termios savetty;
struct termios tty;

// an array of children processes
int pidArray[4];

char *readString();

void parentProcess();

void changeRegister();

void invertString();

void replaceSymbols();

void convertToKOI8();


int main(void) {
    // get parent`s process id
    pid_t ppid = getpid();

    for (int i = 0; i < 4; i++) {
        // forking a process
        pid_t pid = fork();
        switch (pid) {
            case -1:
                perror("Fork failed");
                exit(1);
            case 0:
                // handling children processes
                switch (i) {
                    case 0:
                        // setting signal handler
                        signal(SIGUSR1, changeRegister);
                        while (1) {
                            // stop the process until the signal goes off
                            pause();
                            // send the signal to the parent`s process
                            kill(ppid, SIGUSR1);
                        }
                        exit(0);
                    case 1:
                        signal(SIGUSR1, invertString);
                        while (1) {
                            pause();
                            kill(ppid, SIGUSR1);
                        }
                        exit(0);
                    case 2:
                        signal(SIGUSR1, replaceSymbols);
                        while (1) {
                            pause();
                            kill(ppid, SIGUSR1);
                        }
                        exit(0);
                    case 3:
                        signal(SIGUSR1, convertToKOI8);
                        while (1) {
                            pause();
                            kill(ppid, SIGUSR1);
                        }
                        exit(0);
                }
                break;
            default:
                // set an each child`s process id
                pidArray[i] = pid;
        }
    }


    if (getpid() == ppid) {
        signal(SIGUSR1, parentProcess);
        // call parentProcess for first user input
        parentProcess();
        while (1) {
            pause();
        }
    }

}


char *readString() {
    char *string = (char *) malloc(sizeof(char) * BUFFER);
    read(0, string, BUFFER);
    return string;
}

void changeRegister() {
    printf("Please, enter a string to change register:\n");
    char *str = readString();
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        } else if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32;
        }
        i++;
    }
    write(1, str, i);
}

void invertString() {
    printf("Please, enter a string to invert:\n");
    char *str = readString();
    int counter = 0;
    while (str[counter++] != '\0');
    int strLength = counter;
    counter -= 3;
    int i = 0;
    for (; i < counter; ++i, --counter) {
        char tmp = str[i];
        str[i] = str[counter];
        str[counter] = tmp;
    }
    write(1, str, strLength + 1);
}

void replaceSymbols() {
    printf("Please, enter a string to swap neighbors:\n");
    char *str = readString();
    int i = 0;
    while (str[i] != '\0' && str[i] != '\n') {
        if (i % 2 == 1) {
            char tmp = str[i - 1];
            str[i - 1] = str[i];
            str[i] = tmp;
        }
        i++;
    }
    write(1, str, i + 1);
}

void convertToKOI8() {
    printf("Please, enter a string to convert to KOI-8:\n");
    char *str = readString();
    int counter = 0;
    while (str[counter] != '\0') {
        if (str[counter] != ' ' && str[counter] != '\n') {
            str[counter] |= 128;
        }
        ++counter;
    }
    write(1, str, counter);
}

void parentProcess() {

    if (!isatty(0)) {
        fprintf(stderr, "stdin not terminal\n");
        exit(1);
    };

    char childNumber;

    tcgetattr(0, &tty);
    savetty = tty;
    tty.c_lflag &= ~(ICANON | ECHO | ISIG);
    tty.c_cc[VMIN] = 1;
    tcsetattr(0, TCSAFLUSH, &tty);

    if (read(0, &childNumber, 1) < 0) {
        perror("File read error in reading data\n");
        exit(1);
    }

    tcsetattr(0, TCSAFLUSH, &savetty);

    if (childNumber - '0' >= 0 && childNumber - '0' <= 3) {
        kill(pidArray[childNumber - '0'], SIGUSR1);
    } else {
        perror("You entered wrong character. Please right a number in range 0-3");
        exit(1);
    }
}
