//Lawrence Calais
//MyOwnDaemon HW3
//Compile: gcc -o sample sample_d.c
//output: ./sample (start|stop)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#define OK              0
#define ERR_SETSID      1
#define SIGTERM_ERR     2
#define SIGHUP_ERR      3
#define ERR_FORK        4
#define ERR_CHDIR       5
#define ERR_WTF         9
#define DAEMON_NAME     "lawrence_daemon"
#define ERROR_FORMAT    "error: %s"
#define PID_FILE        "/tmp/test_daemon.pid"


//signal handler
static void _signal_handler(const int signal) {
    switch (signal) {
        case SIGHUP:
            // Handle SIGHUP if needed
            break;
        case SIGTERM:
            syslog(LOG_INFO, "received SIGTERM, exiting.");
            unlink(PID_FILE); // Remove the PID file
            closelog();
            exit(OK);
            break;
        default:
            syslog(LOG_INFO, "received unhandled signal.");
    }
}

//Perform daemon work
static void _do_work(void) {
    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_buffer[26];
        strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        syslog(LOG_INFO, "Current time: %s", time_buffer);
        sleep(1);
    }
}

//daemonize the process
void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        exit(ERR_FORK);
    }
    if (pid > 0) {
        exit(OK);
    }
    if (setsid() < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        exit(ERR_SETSID);
    }
    if (chdir("/") < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        exit(ERR_CHDIR);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    umask(0);

    int pid_file = open(PID_FILE, O_RDWR | O_CREAT, 0600);
    if (pid_file < 0) {
        syslog(LOG_ERR, "Could not open PID file %s, exiting", PID_FILE);
        exit(ERR_WTF);
    }
    if (lockf(pid_file, F_TLOCK, 0) < 0) {
        syslog(LOG_ERR, "Could not lock PID file %s, exiting", PID_FILE);
        exit(ERR_WTF);
    }

    char str[10];
    sprintf(str, "%d\n", getpid());
    write(pid_file, str, strlen(str));

    signal(SIGTERM, _signal_handler);
    signal(SIGHUP, _signal_handler);

    _do_work();

    unlink(PID_FILE);
    closelog();
}

//start daemon
void start() {
    openlog(DAEMON_NAME, LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON);
    syslog(LOG_INFO, "starting");
    daemonize();
}

//stop daemon
void stop() {
    FILE *pid_file = fopen(PID_FILE, "r");
    if (pid_file == NULL) {
        fprintf(stderr, "Could not open PID file %s, exiting\n", PID_FILE);
        exit(ERR_WTF);
    }

    int pid;
    fscanf(pid_file, "%d", &pid);
    fclose(pid_file);

    if (kill(pid, SIGTERM) < 0) {
        fprintf(stderr, "Could not send SIGTERM to process %d, exiting\n", pid);
        exit(SIGTERM_ERR);
    }

    printf("Daemon stopped successfully.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s {start|stop}\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "start") == 0) {
        start();
    } else if (strcmp(argv[1], "stop") == 0) {
        stop();
    } else {
        fprintf(stderr, "Usage: %s {start|stop}\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    return OK;
}

