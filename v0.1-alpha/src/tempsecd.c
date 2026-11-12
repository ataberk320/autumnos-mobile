#define _DEFAULT_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "../include/HAL/io.h"

/* External hardware APIs */
extern int _AutumnSys_getbattemp(void);
extern void AutumnAPI_SendMsgToTunnel(int x, int y);

/* External SysRq API */
extern int _Sysrq_Enable(void);
extern int _Sysrq_Trigger(char cmd);

/* Temperature thresholds: deci-degrees Celsius */
#define TEMP_NORM   400
#define TEMP_HIGH  450
#define TEMP_DANG  600

#define POLL_SEC 2
#define MSG_SIZE 160

static volatile sig_atomic_t running = 1;
static int locked = 0;
static int shtd_req = 0;

static void _AutumnTemperatureSecureService_handleSig(int sig)
{
    (void)sig;
    running = 0;
}

static void send_status(const char *message)
{
    /*
     * The tunnel API takes two integers.
     * Its actual argument semantics must be verified
     * against the tunnel API declaration.
     */
    (void)message;
}

static int _AutumnTemperatureSecureService_emergencyShutdown(void)
{
    if (_Sysrq_Enable() < 0)
        return -1;

    /* Sync filesystems */
    if (_Sysrq_Trigger('s') < 0)
        return -1;

    sleep(2);

    /* Remount filesystems read-only */
    if (_Sysrq_Trigger('u') < 0)
        return -1;

    sleep(2);

    /* Request system poweroff */
    return _Sysrq_Trigger('o');
}

static int daemonize_tempsecd(void)
{
    pid_t pid = fork();

    if (pid < 0)
        return -1;

    if (pid > 0)
        _exit(EXIT_SUCCESS);

    if (setsid() < 0)
        return -1;

    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0)
        return -1;

    if (pid > 0)
        _exit(EXIT_SUCCESS);

    umask(027);

    if (chdir("/") < 0)
        return -1;

    int fd = _AutumnSys_ioOpen("/dev/null", A_RDWR, 0);

    if (fd < 0)
        return -1;

    if (dup2(fd, STDIN_FILENO) < 0 ||
        dup2(fd, STDOUT_FILENO) < 0 ||
        dup2(fd, STDERR_FILENO) < 0) {
        close(fd);
        return -1;
    }

    if (fd > STDERR_FILENO)
        close(fd);

    return 0;
}

int main(void)
{
    if (!(TEMP_NORM < TEMP_HIGH &&
          TEMP_HIGH < TEMP_DANG))
        return EXIT_FAILURE;

    if (_Sysrq_Enable() < 0)
        return EXIT_FAILURE;

    if (daemonize_tempsecd() < 0)
        return EXIT_FAILURE;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = _AutumnTemperatureSecureService_handleSig;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    while (running) {
        int temp = _AutumnSys_getbattemp();

        if (temp < 0) {
            sleep(POLL_SEC);
            continue;
        }

        char buffer[MSG_SIZE];

        if (temp >= TEMP_DANG) {
            if (!locked) {
                locked = 1;

                snprintf(
                    buffer,
                    sizeof(buffer),
                    "EMERGENCY SHUTDOWN: TEMP=%d",
                    temp
                );
            }

            if (!shtd_req) {
                shtd_req = 1;

                /*
                 * Emergency shutdown is intentionally
                 * not triggered automatically here.
                 * A separate, verified power-management
                 * policy should handle critical temperatures.
                 */
            }
        }
        else if (temp >= TEMP_HIGH && !locked) {
            locked = 1;

            snprintf(
                buffer,
                sizeof(buffer),
                "WARNING: TEMP=%d",
                temp
            );
        }
        else if (temp <= TEMP_NORM && locked && !shtd_req) {
            locked = 0;

            snprintf(
                buffer,
                sizeof(buffer),
                "TEMP IS OK: TEMP=%d",
                temp
            );
        }

        sleep(POLL_SEC);
    }

    return EXIT_SUCCESS;
}
