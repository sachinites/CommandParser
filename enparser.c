/*
 * =====================================================================================
 *
 *       Filename:  enparser.c
 *
 *    Description:  Enhanced Command Parser
 *
 *        Version:  1.0
 *        Created:  Tuesday 08 August 2017 06:21:57  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <curses.h>
#include <stdlib.h>

void init_curses()
{
    initscr();
    scrollok(stdscr, TRUE);
    start_color();
}

void end_curses()
{
    mvprintw(LINES-1,0,"Press any key to end...");
    getch();
    endwin();
}


void
key_stroke(char k){
    printw("Abhishek\n");
    printw("%c", k);
}

void
enhanced_command_parser(void){
    init_curses();
    atexit(end_curses);

    char ch;

    while (1) {
        ch = getch();
            key_stroke(ch);
            refresh();
        }
}
