#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    initscr();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    curs_set(0);
    char *str = "butt";
    int x, y;
    int mx, my; mx = my = 0;
    getmaxyx(stdscr, my, mx);
    x = mx / 2;
    y = my / 2;
    clear();
    mvprintw(y, x, str);
    refresh();
    int c;
    while((c = getch()) != 'q') {
        switch (c) {
            case KEY_LEFT:
                x -= ((x-1) >= 0 ? 1 : 0);
                break;
            case KEY_RIGHT:
                x += ((x+1+strlen(str)) > mx ? 0 : 1);
                break;
            case KEY_UP:
                y -= ((y-1) >= 0 ? 1 : 0);
                break;
            case KEY_DOWN:
                y += ((y+1 > my - 1) ? 0 : 1);
                break;
        }
        clear();
        mvprintw(y, x, str);
    }

    int i = endwin();
    return i;
}
