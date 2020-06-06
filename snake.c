#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <sys/select.h>
#include <termios.h>

#define STDIN 0

long speed_in_microseconds;

struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler and set the new terimal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getchr()
{
    int r;
    unsigned char c;
    if((r = read(STDIN, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
} 

char *getstri(int size, char *c)
{
    int r;
    if((r = read(STDIN, c, size)) < 0) {
        return NULL;
    }

    return c;
}


#define LEFT KEY_LEFT
#define RIGHT KEY_RIGHT
#define UP KEY_UP
#define DOWN KEY_DOWN

#define OLD 0
#define NEW 1

typedef struct {
    int x;
    int y;
} position;

typedef struct {
    position pos;
} apple;


struct segment {
    position pos;
    struct segment *next;
};

typedef struct segment segment;

typedef struct {
    int length;
    segment *head;
} snake;

#define NUM_APPLES 3
#define MAX_APPLES 32

apple apples[MAX_APPLES];
int num_apples;
int mx, my;
void apples_print(apple apples[], int len);

void init() {
    speed_in_microseconds = 200000;
    mx = my = 0;
    getmaxyx(stdscr, my, mx);

    srand(time(0));
    for(int i = 0; i < NUM_APPLES; i++) {
        apples[i].pos.x = (rand() % mx);
        apples[i].pos.y = (rand() % my);
    }
    num_apples = NUM_APPLES;
    apples_print(apples, num_apples);
}

apple new_apple() {
    apple z;
    z.pos.x = (rand() % mx);
    z.pos.y = (rand() % my);
    return z;
}

void destroy_snake(snake *s);

snake *new_snake(int length, int x, int y) {
    snake *s = malloc(sizeof(snake));

    segment *head = malloc(sizeof(segment));;
    position pos;

    pos.x = x;
    pos.y = y;

    head->pos = pos;
    segment *cur = head;
    for(int i = 1; i < length; i++) {
        cur->next = malloc(sizeof(segment));
        position p = {.x = x-i, .y = y};
        cur->next->pos = p;
        cur = cur->next;
    }
    cur->next = NULL;

    s->head = head;

    s->length = length;
    return s;
}

void snake_display(snake *s, WINDOW *w)
{
    int x, y;
    wclear(w);

    apples_print(apples, num_apples);
    segment *cur = s->head;
    while(cur) {
        x = cur->pos.x;
        y = cur->pos.y;
        mvwprintw(w, y, x, "#");
        cur = cur->next;
    }


    wrefresh(w);
}

void you_died(snake *s) {
    mvprintw(40,40, "you died!\n");
    getch();
}

int collision(snake *s, int x, int y) {
    segment *head = s->head; 
    if(x > mx || y > my || x < 0 || y < 0)
        return 1;
    while(head) {
        if(head->pos.x == x && head->pos.y == y)
            return 1;
        head = head->next;
    }

    return 0;
}

int gets_apple(int x, int y) {
    for(int i = 0; i < num_apples; i++) {
        if(apples[i].pos.x == x && apples[i].pos.y == y) {
            return i;
        }
    }
    return 0;
}

int snake_advance(snake *s, WINDOW *w, int c) {
    int x, y;

    segment *cur = s->head;
    x = cur->pos.x;
    y = cur->pos.y;

    switch(c) {
        case LEFT:
            x -= 1;
            if(x == cur->next->pos.x) return 1;
            break;
        case RIGHT:
            x += 1; 
            if(x == cur->next->pos.x) return 1;
            break;
        case UP:
            y -= 1;
            if(y == cur->next->pos.y) return 1;
            break;
        case DOWN:
            y += 1;
            if(y == cur->next->pos.y) return 1;
            break;
    }

    if(collision(s, x, y)) {
        you_died(s);
        return 0;
    }


    position p[2];
    p[NEW].x = x; p[NEW].y = y;
    while(cur->next) {
        p[OLD] = cur->pos;
        cur->pos = p[NEW];
        p[NEW] = p[OLD];
        cur = cur->next;
    }
    cur->pos = p[NEW];

    snake_display(s, w);

    int u;
    if((u = gets_apple(x, y))) {
        s->length++;
        segment *new_tail = malloc(sizeof(segment));
        cur->next = new_tail;
        apples[u] = new_apple();
        if(num_apples + 1 < MAX_APPLES) {
            apples[num_apples] = new_apple();
            num_apples++;
        }
    }

    return 1;
}

void destroy_snake(snake *s) {
    segment *tmp;
    segment *head = s->head;
    while (head != NULL) {
       tmp = head;
       head = head->next;
       free(tmp);
    }

    free(s);
}

void apples_print(apple apples[], int len) {
    for(int i = 0; i < len; i++) {
        mvprintw(apples[i].pos.y, apples[i].pos.x, "o");
    }

    refresh();
}

int main(int argc, char *argv[])
{
    set_conio_terminal_mode();
    initscr();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    curs_set(0);

    init();
    int x, y;
    x = mx / 2;
    y = my / 2;

    snake *s = new_snake(16, x, y);
    snake_display(s, stdscr);

    size_t size = 2;
    char *c = calloc(size + 1, sizeof(char));
    int dir = RIGHT;

    while(*c != 'q') {
        *c = 0;
        while(!kbhit()) {
            if(!snake_advance(s, stdscr, dir)) goto cleanup;
            usleep(speed_in_microseconds);
        }

        *c = getchr();
        if(*c == 27) { // <esc>
            *c = (getchr(), getchr());
            switch(*c) {
                case 'A':
                    dir = UP;
                    break;
                case 'B':
                    dir = DOWN;
                    break;
                case 'C':
                    dir = RIGHT;
                    break;
                case 'D':
                    dir = LEFT;
                    break;
            }
        }

    }

    goto cleanup;

cleanup:
    destroy_snake(s);
    free(c);
    clear();
    endwin();
    exit(0);
}
