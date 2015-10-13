#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <signal.h>

#include "usock.h"
#include "common.h"
#include "digiled.h"

// LED-A 数码管A段         29
// LED-B 数码管B段         27
// LED-C 数码管C段         24
// LED-D 数码管D段         22
// LED-E 数码管E段         21
// LED-F 数码管F段         28
// LED-G 数码管G段         25
// LED-H 数码管DP段（点）  23
static int seg_pin[] = {29, 27, 24, 22, 21, 28, 25, 23};

//LED-COM1 数码管位选（1） 0
//LED-COM2 数码管位选（2） 2
//LED-COM3 数码管位选（3） 3
//LED-COM4 数码管位选（4） 12
static int digi_pin[] = {0, 2, 3, 12};

static uint8_t num_seg_mask[10] = {
    0b11111100,
    0b01100000,
    0b11011010,
    0b11110010,
    0b01100110,
    0b10110110,
    0b10111110,
    0b11100000,
    0b11111110,
    0b11110110,
};

static uint8_t rotation_mask[6] = {
    0b10000000,
    0b01000000,
    0b00100000,
    0b00010000,
    0b00001000,
    0b00000100,
};

static int rotation_speed = 10;

static int toggle_state;
static int pos_num[3]; /* xxx , 10 means don't display */
static int dot_pos_state[3]; /* x.xx or xx.x or xxx. */
//static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t thread;

#define SHIFT_TIMEOUT       5
#define MAXLINE             128

bool use_syslog = true;

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s -f\n"
            "   -f              Do not fork to background\n"
            "   -h              help\n"
            "\n", prog
            );
}


static inline void display_num(int pos)
{
    int i;

    if ((toggle_state & 0b10) == 0) {
        delay(SHIFT_TIMEOUT);
        return;
    }

    dot_pos_state[pos] == 1 ? digitalWrite(seg_pin[7], LOW) :
                                  digitalWrite(seg_pin[7], HIGH);
    for (i = 0; i < 7; i++) {
        pos_num[pos] == 10 ?  digitalWrite(seg_pin[i], HIGH) :
            digitalWrite(seg_pin[i], 1 ^ (1 & num_seg_mask[pos_num[pos]] >> (7 - i)));
    }

    digitalWrite(digi_pin[pos], LOW);
    delay(SHIFT_TIMEOUT);
    digitalWrite(digi_pin[pos], HIGH);
}

static inline void display_rotation(void)
{
    static int n;
    static int remain;
    int i;

    if ((toggle_state & 0b01) == 0) {
        delay(SHIFT_TIMEOUT);
        return;
    }

    if (remain == 0) {
        remain = rotation_speed;
        n++;
        n = n % 6;
    }

    for (i = 0; i < 8; i++)
        digitalWrite(seg_pin[i], 1 ^ (1 & rotation_mask[n] >> (7 - i)));

    digitalWrite(digi_pin[3], LOW);
    delay(SHIFT_TIMEOUT);
    digitalWrite(digi_pin[3], HIGH);

    remain--;
}

static void gen_display_image(int integer)
{
    int i100000, i10000, i1000, i100, i10, i1;

    i1      = integer % 10;
    integer /= 10;
    i10     = integer % 10;
    integer /= 10;
    i100    = integer % 10;
    integer /= 10;
    i1000   = integer % 10;
    integer /= 10;
    i10000  = integer % 10;
    integer /= 10;
    if (integer > 9) {
        i100000 = 10; /* over range */
    } else {
        i100000 = integer % 10;
    }

    if (i100000 > 9) {
        /* 999. */
        pos_num[0] = 9;
        pos_num[1] = 9;
        pos_num[2] = 9;
        dot_pos_state[0] = 0;
        dot_pos_state[1] = 0;
        dot_pos_state[2] = 1;
    } else if (i100000 > 0) {
        /* xxx. */
        pos_num[0] = i100000;
        pos_num[1] = i10000;
        pos_num[2] = i1000;
        dot_pos_state[0] = 0;
        dot_pos_state[1] = 0;
        dot_pos_state[2] = 1;
    } else if (i10000 > 0) {
        /* xx.x */
        pos_num[0] = i10000;
        pos_num[1] = i1000;
        pos_num[2] = i100;
        dot_pos_state[0] = 0;
        dot_pos_state[1] = 1;
        dot_pos_state[2] = 0;
    } else if (i1000 > 0) {
        /* x.xx */
        pos_num[0] = i1000;
        pos_num[1] = i100;
        pos_num[2] = i10;
        dot_pos_state[0] = 1;
        dot_pos_state[1] = 0;
        dot_pos_state[2] = 0;
    } else if (i100 > 0) {
        /* xxx */
        pos_num[0] = i100;
        pos_num[1] = i10;
        pos_num[2] = i1;
        dot_pos_state[0] = 0;
        dot_pos_state[1] = 0;
        dot_pos_state[2] = 0;
    } else if (i10 > 0) {
        /* _xx */
        pos_num[0] = 10;
        pos_num[1] = i10;
        pos_num[2] = i1;
        dot_pos_state[0] = 0;
        dot_pos_state[1] = 0;
        dot_pos_state[2] = 0;
    } else {
        /* __x */
        pos_num[0] = 10;
        pos_num[1] = 10;
        pos_num[2] = i1;
        dot_pos_state[0] = 0;
        dot_pos_state[1] = 0;
        dot_pos_state[2] = 0;
    }
    LOGD("num:%d %d %d, dot:%d %d %d", pos_num[0], pos_num[1], pos_num[2],
        dot_pos_state[0], dot_pos_state[1], dot_pos_state[2]);
}

static void pin_init(void)
{
    int i;

    wiringPiSetup();

    for (i = 0; i < 4; i++) {
        pinMode(digi_pin[i], OUTPUT);
        digitalWrite(digi_pin[i], HIGH);
    }

    for (i = 0; i < 8; i++) {
        pinMode(seg_pin[i], OUTPUT);
        digitalWrite(seg_pin[i], HIGH);
    }

    gen_display_image(0);
}

static void* thread_func(void *arg)
{
    int i;
    while (1) {
        for (i = 0; i < 4; i++) {
            if (i == 3) {
                display_rotation();
            } else {
                display_num(i);
            }

            if (toggle_state == 0) {
                /* avoid overload */
                sleep(1);
            }
        }
    }
}

static void msg_handler(char *cmdline)
{
    char **tokens = NULL;
    size_t numtokens = 0;
    int opt;
    int i;

    LOGD("Get a message: %s", cmdline);

    tokens = strsplit(cmdline, " ", &numtokens);

    optind = 1;
    while ((opt = getopt(numtokens, tokens, "t:i:r:")) != -1)
    {
        switch(opt)
        {
            case 't':
                /*
                 * toggle state:
                 *   0b11 - enable all display
                 *   0b10 - only digital display
                 *   0b01 - only rotation display
                 *   0b00 - disable all display
                 */

                /* LED灯有点问题，做特殊处理 */
                for (i = 0; i < 8; i++) {
                    pinMode(seg_pin[i], OUTPUT);
                    digitalWrite(seg_pin[i], HIGH);
                }

                toggle_state = atoi(optarg);
                LOGD("toggle_state: %d\n", toggle_state);
                break;
            case 'i':
                /*
                 * integer
                 *   effective range is 0 ~ 999000
                 */
                gen_display_image(atoi(optarg));
                break;
            case 'r':
                /*
                 * rotate speed:
                 *   X - 1 ~ 10
                 */
                rotation_speed = atoi(optarg);
                if (rotation_speed > 10) {
                    rotation_speed = 10;
                }
                if (rotation_speed <= 0) {
                    rotation_speed = 1;
                }
                rotation_speed = 11 - rotation_speed;
                break;
            default:
                LOGD("Invalid message");
                return;
        }
    }

    tokensfree(tokens, numtokens);
    tokens = NULL;
}

static bool get_next_connection(int fd)
{
    int client_fd;
    ssize_t n;
    char buf[MAXLINE];

    client_fd = accept(fd, NULL, 0);
    if (client_fd < 0) {
        switch (errno) {
        case ECONNABORTED:
        case EINTR:
            return true;
        default:
            LOGE("accept error: %s", strerror(errno));
            return true;
        }
    }

    int i;
    int offset = 0;
    while ((n = read(client_fd, buf, MAXLINE)) > 0) {
        for (i = 0; i < n; i++) {
            if (buf[i] == '\0') {
                msg_handler(buf + offset);
                offset = i + 1;
            }
        }
    }

    return true;
}

static void pin_fini(void)
{
    int i;
    for (i = 0; i < 8; i++) {
        pinMode(seg_pin[i], OUTPUT);
        digitalWrite(seg_pin[i], HIGH);
    }
    for (i = 0; i < 4; i++) {
        pinMode(digi_pin[i], OUTPUT);
        digitalWrite(digi_pin[i], HIGH);
    }
    exit(0);
}

static void setup_signals(void)
{
    struct sigaction s;

    memset(&s, 0, sizeof(s));

    s.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &s, NULL);

    s.sa_handler = pin_fini;
    sigaction(SIGTERM, &s, NULL);
    sigaction(SIGKILL, &s, NULL);
}

int main(int argc, const char *argv[])
{
    int res;
    int nofork = 0;
    int cur_fd = 0;
    int opt;
    int serv_fd;
    struct sockaddr_un servaddr;
    bool next;

    while ((opt = getopt(argc, argv, "f")) != -1)
    {
        switch(opt)
        {
            /* no fork */
            case 'f':
                use_syslog = false;
                nofork = 1;
                break;
			case 'h':
                usage(argv[0]);
				exit(0);
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (use_syslog) {
        openlog("digiledd", LOG_CONS, LOG_DAEMON);
    }

    int pid_file = open("/var/run/digiledd.pid", O_CREAT | O_RDWR, 0666);
    res = flock(pid_file, LOCK_EX | LOCK_NB);
    if (res) {
        if (EWOULDBLOCK == errno) {
            fprintf(stderr, "Another digiledd is running\n");
            exit(EXIT_FAILURE);
        }
    }

    /* fork (if not disabled) */
    if (!nofork)
    {
        switch (fork())
        {
            case -1:
                LOGE("fork(): %s", strerror(errno));
                exit(1);

            case 0:
                /* daemon setup */
                if (chdir("/"))
                    LOGE("chdir(): %s", strerror(errno));

                if ((cur_fd = open("/dev/null", O_WRONLY)) > -1)
                    dup2(cur_fd, 0);

                if ((cur_fd = open("/dev/null", O_RDONLY)) > -1)
                    dup2(cur_fd, 1);

                if ((cur_fd = open("/dev/null", O_RDONLY)) > -1)
                    dup2(cur_fd, 2);

                break;

            default:
                exit(0);
        }
    }

    setup_signals();

    pin_init();

    res = pthread_create(&thread, NULL, thread_func, NULL);
    if (res != 0) {
        LOGE("Thread creation failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    LOGI("digiledd started");

    /* get ready for IPC */
    unlink(DIGILED_UNIX_SOCKET);
    serv_fd = usock(USOCK_UNIX | USOCK_SERVER, DIGILED_UNIX_SOCKET, NULL);
    if (serv_fd < 0) {
        LOGE("usock: %s", strerror(errno));
    }

    do {
        next = get_next_connection(serv_fd);
    } while (next);

    unlink(DIGILED_UNIX_SOCKET);
    return 0;
}
