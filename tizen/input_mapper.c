/*
 * Samsung TV Remote → Virtual Gamepad Input Mapper
 *
 * Samsung Smart Remote의 IR/CEC 키 이벤트를 표준
 * Linux uinput 가상 게임패드로 매핑한다.
 *
 * Sober는 /dev/input/event* (evdev)로 게임패드를 읽으므로
 * uinput으로 가상 게임패드를 만들면 자동으로 인식된다.
 *
 * 빌드: aarch64-linux-gnu-gcc -O2 -o input_mapper input_mapper.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>

/* ------------------------------------------------------------------ */
/*  Samsung Smart Remote 키코드 (Tizen / Linux evdev)                  */
/* ------------------------------------------------------------------ */
#define SAMSUNG_KEY_OK          KEY_ENTER
#define SAMSUNG_KEY_BACK        KEY_BACK
#define SAMSUNG_KEY_HOME        KEY_HOMEPAGE
#define SAMSUNG_KEY_UP          KEY_UP
#define SAMSUNG_KEY_DOWN        KEY_DOWN
#define SAMSUNG_KEY_LEFT        KEY_LEFT
#define SAMSUNG_KEY_RIGHT       KEY_RIGHT
#define SAMSUNG_KEY_RED         KEY_RED
#define SAMSUNG_KEY_GREEN       KEY_GREEN
#define SAMSUNG_KEY_YELLOW      KEY_YELLOW
#define SAMSUNG_KEY_BLUE        KEY_BLUE
#define SAMSUNG_KEY_VOLUMEUP    KEY_VOLUMEUP
#define SAMSUNG_KEY_VOLUMEDOWN  KEY_VOLUMEDOWN
#define SAMSUNG_KEY_PLAY        KEY_PLAY
#define SAMSUNG_KEY_PAUSE       KEY_PAUSE
#define SAMSUNG_KEY_STOP        KEY_STOP
#define SAMSUNG_KEY_FF          KEY_FASTFORWARD
#define SAMSUNG_KEY_REW         KEY_REWIND
#define SAMSUNG_KEY_1           KEY_1
#define SAMSUNG_KEY_2           KEY_2
#define SAMSUNG_KEY_3           KEY_3
#define SAMSUNG_KEY_4           KEY_4

/* ------------------------------------------------------------------ */
/*  매핑 테이블: Samsung Remote → Xbox 호환 게임패드 버튼               */
/* ------------------------------------------------------------------ */
typedef struct {
    int src_key;       /* Samsung remote 키코드 */
    int dst_type;      /* EV_KEY 또는 EV_ABS */
    int dst_code;      /* 대상 버튼/축 코드 */
    int dst_value;     /* EV_ABS일 때 값 (-32767, 0, 32767) */
} KeyMap;

static const KeyMap KEY_MAPPING[] = {
    /* 방향키 → DPAD (ABS_HAT0X/Y) */
    { SAMSUNG_KEY_UP,    EV_ABS, ABS_HAT0Y, -32767 },
    { SAMSUNG_KEY_DOWN,  EV_ABS, ABS_HAT0Y,  32767 },
    { SAMSUNG_KEY_LEFT,  EV_ABS, ABS_HAT0X, -32767 },
    { SAMSUNG_KEY_RIGHT, EV_ABS, ABS_HAT0X,  32767 },

    /* OK/Enter → A 버튼 */
    { SAMSUNG_KEY_OK,    EV_KEY, BTN_SOUTH,  1 },

    /* Back → B 버튼 */
    { SAMSUNG_KEY_BACK,  EV_KEY, BTN_EAST,   1 },

    /* 컬러 버튼 → X, Y, LB, RB */
    { SAMSUNG_KEY_RED,    EV_KEY, BTN_WEST,  1 },
    { SAMSUNG_KEY_GREEN,  EV_KEY, BTN_NORTH, 1 },
    { SAMSUNG_KEY_YELLOW, EV_KEY, BTN_TL,    1 },
    { SAMSUNG_KEY_BLUE,   EV_KEY, BTN_TR,    1 },

    /* Play/Pause → Start */
    { SAMSUNG_KEY_PLAY,  EV_KEY, BTN_START,  1 },
    { SAMSUNG_KEY_PAUSE, EV_KEY, BTN_START,  1 },

    /* Home → Select */
    { SAMSUNG_KEY_HOME,  EV_KEY, BTN_SELECT, 1 },
};
#define KEY_MAPPING_COUNT (sizeof(KEY_MAPPING) / sizeof(KEY_MAPPING[0]))

/* ------------------------------------------------------------------ */
/*  uinput 가상 게임패드 생성                                           */
/* ------------------------------------------------------------------ */
static int uinput_fd = -1;

static int create_virtual_gamepad(void) {
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) {
        uinput_fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
    }
    if (uinput_fd < 0) {
        perror("[input_mapper] Cannot open /dev/uinput");
        return -1;
    }

    /* 버튼 이벤트 활성화 */
    ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_SOUTH);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_EAST);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_NORTH);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_WEST);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TL);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TR);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TL2);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TR2);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_START);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_SELECT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_THUMBL);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_THUMBR);

    /* 축 이벤트 활성화 (스틱, 트리거, DPAD) */
    ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS);

    struct uinput_abs_setup abs_setup;

    /* 왼쪽 스틱 X/Y */
    memset(&abs_setup, 0, sizeof(abs_setup));
    abs_setup.code = ABS_X;
    abs_setup.absinfo.minimum = -32767;
    abs_setup.absinfo.maximum =  32767;
    abs_setup.absinfo.flat    =  128;
    abs_setup.absinfo.fuzz    =  16;
    ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);
    abs_setup.code = ABS_Y;
    ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);

    /* 오른쪽 스틱 */
    abs_setup.code = ABS_RX;
    ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);
    abs_setup.code = ABS_RY;
    ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);

    /* 트리거 L2/R2 */
    abs_setup.code    = ABS_Z;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = 255;
    abs_setup.absinfo.flat    = 0;
    abs_setup.absinfo.fuzz    = 0;
    ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);
    abs_setup.code = ABS_RZ;
    ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);

    /* D-PAD HAT */
    abs_setup.code            = ABS_HAT0X;
    abs_setup.absinfo.minimum = -32767;
    abs_setup.absinfo.maximum =  32767;
    abs_setup.absinfo.flat    = 0;
    abs_setup.absinfo.fuzz    = 0;
    ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);
    abs_setup.code = ABS_HAT0Y;
    ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);

    /* 동기 이벤트 */
    ioctl(uinput_fd, UI_SET_EVBIT, EV_SYN);

    /* 디바이스 정보 설정 */
    struct uinput_setup setup;
    memset(&setup, 0, sizeof(setup));
    setup.id.bustype = BUS_VIRTUAL;
    setup.id.vendor  = 0x04e8; /* Samsung vendor ID */
    setup.id.product = 0x1234;
    setup.id.version = 1;
    strncpy(setup.name, "Samsung TV Virtual Gamepad (TizenRoblox)",
            UINPUT_MAX_NAME_SIZE - 1);
    setup.ff_effects_max = 0;

    if (ioctl(uinput_fd, UI_DEV_SETUP, &setup) < 0) {
        /* Fallback for older kernels */
        struct uinput_user_dev udev;
        memset(&udev, 0, sizeof(udev));
        strncpy(udev.name, setup.name, UINPUT_MAX_NAME_SIZE - 1);
        udev.id = setup.id;
        write(uinput_fd, &udev, sizeof(udev));
    }

    if (ioctl(uinput_fd, UI_DEV_CREATE) < 0) {
        perror("[input_mapper] UI_DEV_CREATE failed");
        close(uinput_fd);
        uinput_fd = -1;
        return -1;
    }

    printf("[input_mapper] Virtual gamepad created: /dev/uinput\n");
    return 0;
}

static void emit_event(int type, int code, int value) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type  = type;
    ev.code  = code;
    ev.value = value;
    write(uinput_fd, &ev, sizeof(ev));
}

static void emit_sync(void) {
    emit_event(EV_SYN, SYN_REPORT, 0);
}

/* ------------------------------------------------------------------ */
/*  Samsung remote 이벤트 소스 감지                                     */
/* ------------------------------------------------------------------ */
static int find_samsung_remote(void) {
    DIR *dir = opendir("/dev/input");
    if (!dir) return -1;

    struct dirent *ent;
    char path[64];
    int found_fd = -1;

    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) != 0) continue;

        snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        char name[256] = {0};
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);

        /* Samsung 리모컨 식별 */
        if (strstr(name, "Samsung") || strstr(name, "samsung") ||
            strstr(name, "TV Remote") || strstr(name, "RC") ||
            strstr(name, "CEC")) {
            printf("[input_mapper] Found Samsung remote: %s (%s)\n", path, name);
            found_fd = fd;
            break;
        }

        /* KEY_RED 지원 여부로도 감지 */
        unsigned long key_bits[KEY_MAX/8/sizeof(long) + 1] = {0};
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits);
        if (key_bits[KEY_RED / (8*sizeof(long))] & (1UL << (KEY_RED % (8*sizeof(long))))) {
            printf("[input_mapper] Found remote by KEY_RED: %s (%s)\n", path, name);
            found_fd = fd;
            break;
        }

        close(fd);
    }
    closedir(dir);
    return found_fd;
}

/* ------------------------------------------------------------------ */
/*  리모컨 → 가상 게임패드 이벤트 변환                                  */
/* ------------------------------------------------------------------ */
static void handle_remote_key(int key_code, int key_value) {
    /* key_value: 1=press, 0=release, 2=repeat */
    for (size_t i = 0; i < KEY_MAPPING_COUNT; i++) {
        if (KEY_MAPPING[i].src_key != key_code) continue;

        if (KEY_MAPPING[i].dst_type == EV_KEY) {
            /* 버튼: press/release 그대로 전달 */
            int val = (key_value > 0) ? 1 : 0;
            emit_event(EV_KEY, KEY_MAPPING[i].dst_code, val);
            emit_sync();
        } else if (KEY_MAPPING[i].dst_type == EV_ABS) {
            /* 방향키: press=값, release=0 */
            int val = (key_value > 0) ? KEY_MAPPING[i].dst_value : 0;
            emit_event(EV_ABS, KEY_MAPPING[i].dst_code, val);
            emit_sync();
        }
        return;
    }
}

/* ------------------------------------------------------------------ */
/*  실제 게임패드 패스스루 (USB/BT 게임패드 지원)                        */
/* ------------------------------------------------------------------ */
static int find_gamepads(int *fds, int max_fds) {
    DIR *dir = opendir("/dev/input");
    if (!dir) return 0;

    struct dirent *ent;
    int count = 0;

    while ((ent = readdir(dir)) != NULL && count < max_fds) {
        if (strncmp(ent->d_name, "event", 5) != 0) continue;

        char path[64];
        snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        /* 게임패드 확인: BTN_GAMEPAD 지원 */
        unsigned long key_bits[KEY_MAX/8/sizeof(long) + 1] = {0};
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits);

        int has_btn_south = key_bits[BTN_SOUTH / (8*sizeof(long))] &
                            (1UL << (BTN_SOUTH % (8*sizeof(long))));

        if (has_btn_south) {
            char name[256] = {0};
            ioctl(fd, EVIOCGNAME(sizeof(name)), name);
            printf("[input_mapper] Found gamepad: %s (%s)\n", path, name);
            fds[count++] = fd;
        } else {
            close(fd);
        }
    }
    closedir(dir);
    return count;
}

/* ------------------------------------------------------------------ */
/*  메인 이벤트 루프                                                    */
/* ------------------------------------------------------------------ */
static volatile int running = 1;

static void sig_handler(int sig) {
    (void)sig;
    running = 0;
}

int main(void) {
    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    printf("[input_mapper] TizenRoblox Input Mapper starting...\n");

    if (create_virtual_gamepad() < 0) {
        fprintf(stderr, "[input_mapper] Failed to create virtual gamepad. "
                        "Check /dev/uinput permissions.\n");
        return 1;
    }

    /* epoll 설정 */
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("[input_mapper] epoll_create1");
        return 1;
    }

    /* Samsung 리모컨 감지 */
    int remote_fd = find_samsung_remote();
    if (remote_fd >= 0) {
        struct epoll_event ev = { .events = EPOLLIN, .data.fd = remote_fd };
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, remote_fd, &ev);
    } else {
        printf("[input_mapper] Samsung remote not found, remote mapping disabled.\n");
    }

    /* 실제 게임패드 감지 (최대 8개) */
    int gamepad_fds[8];
    int gamepad_count = find_gamepads(gamepad_fds, 8);
    printf("[input_mapper] Found %d physical gamepad(s)\n", gamepad_count);
    /* 게임패드는 Sober가 직접 읽으므로 여기서는 모니터만 */

    printf("[input_mapper] Running. Virtual gamepad active.\n");
    printf("[input_mapper] Samsung remote → gamepad mapping active.\n");

    struct epoll_event events[16];

    while (running) {
        int n = epoll_wait(epoll_fd, events, 16, 1000);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == remote_fd) {
                struct input_event ev;
                while (read(remote_fd, &ev, sizeof(ev)) == sizeof(ev)) {
                    if (ev.type == EV_KEY) {
                        handle_remote_key(ev.code, ev.value);
                    }
                }
            }
        }

        /* 연결된 새 게임패드 주기적 탐지 */
        static int scan_counter = 0;
        if (++scan_counter >= 10) {
            scan_counter = 0;
            int new_fds[8] = {0};
            int new_count = find_gamepads(new_fds, 8);
            if (new_count > gamepad_count) {
                printf("[input_mapper] New gamepad detected!\n");
                gamepad_count = new_count;
                for (int j = 0; j < new_count; j++) close(new_fds[j]);
            } else {
                for (int j = 0; j < new_count; j++) close(new_fds[j]);
            }
        }
    }

    /* 정리 */
    if (remote_fd >= 0) close(remote_fd);
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
    close(epoll_fd);

    printf("[input_mapper] Stopped.\n");
    return 0;
}
