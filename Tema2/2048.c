#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#define MENU_PADDING_HEIGHT 5 /* % top and bottom padding */
#define MENU_PADDING_WIDTH 5  /* % left and right padding */
#define MENU_CHOICES 3
#define MENU_CHOICES_PADDING 2
#define MENU_CHOICES_TOP_OFFSET 10

WINDOW *init_window(int y_max, int x_max, int window_padding_height, int window_padding_width);
void operate_menu(WINDOW *menu);

void start_game();
void restart_game();
void quit_game();

typedef struct choice
{
    char *choice_name;
    void (*fct) ();
} choice;

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    curs_set(0);
    raw();
    noecho();

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);


    int y_max, x_max;
    getmaxyx(stdscr, y_max, x_max);
    WINDOW *menu = init_window(y_max, x_max, MENU_PADDING_HEIGHT, MENU_PADDING_WIDTH);
    operate_menu(menu);
    endwin(); // End the curses library
    return 0;
}

WINDOW *init_window(int y_max, int x_max, int window_padding_height, int window_padding_width)
{
    /* Menu window initialization*/
    int menu_height, menu_width, menu_start_y, menu_start_x;
    menu_height = y_max - (window_padding_height * y_max) / 100;
    menu_width = x_max - (window_padding_width * x_max) / 100;
    menu_start_y = (y_max - menu_height) / 2;
    menu_start_x = (x_max - menu_width) / 2;

    WINDOW  *menu = newwin(menu_height, menu_width, menu_start_y, menu_start_x);
    refresh();
    box(menu, (int)' ' ,0);
    wrefresh(menu);
    return menu;
}

void operate_menu(WINDOW *menu)
{
    choice choices[MENU_CHOICES]=
    {
        [0].choice_name = "NEW GAME",
        [0].fct = start_game,

        [1].choice_name = "RESUME  ", 
        [1].fct = restart_game,

        [2].choice_name = "QUIT    ",
        [2].fct = quit_game
    };

    char intro_text[7][24] =
    {
        [0] = " 222   000      4  888 ",
        [1] = "2   2 0   0   4 4 8   8",
        [2] = "    2 0   0  4  4 8   8",
        [3] = " 222  0   0 44444  888 ",
        [4] = " 2    0   0     4 8   8",
        [5] = "2     0   0     4 8   8",
        [6] = " 2222  000      4  888 "
    };

    int ch = 0, highlight = 0, choice_height, current_choice_height;
    int y_max, x_max;
    getmaxyx(menu, y_max, x_max);
    choice_height = (y_max - 2 - MENU_CHOICES_PADDING * (MENU_CHOICES - 1)) / (MENU_CHOICES - 1);
    keypad(menu, 1);

    while(1)
    {
        /* print intro_text on menu*/
        wattron(menu, A_BOLD);
        wattron(menu, COLOR_PAIR(1));
        for(int i = 0; i < 7; i++)
                mvwprintw(menu, i + 2, (x_max - strlen(intro_text[i]) - 2) / 2, intro_text[i]);
        wattroff(menu, A_BOLD);
        wattroff(menu, COLOR_PAIR(1));
        
        /* print choices of menu*/
        current_choice_height = choice_height;
        for(int i = 0; i < MENU_CHOICES; i++)
        {
            if(i == highlight)
                wattron(menu, A_REVERSE);

            mvwprintw(menu, current_choice_height + MENU_CHOICES_TOP_OFFSET, (x_max - strlen(choices[i].choice_name) - 2) / 2, choices[i].choice_name);
            wattroff(menu, A_REVERSE);
            current_choice_height += MENU_CHOICES_PADDING;
        }

        /* move inside menu (arrow keys / WS, ENTER to Select)*/
        ch = wgetch(menu);

        switch(ch)
        {
            case KEY_UP:
            case (int)'W':
            case (int)'w':
                highlight--;
                break;
            
            case KEY_DOWN:
            case (int)'S':
            case (int)'s':
                highlight++;
                break;

            case 10:
                untouchwin(menu);
                wclear(menu);
                wrefresh(menu);
                refresh();
                choices[highlight].fct();
                break;
            
            case (int)'Q':
            case (int)'q':
                return;
                break;

            default:
                break;
        }

        if(highlight < 0) highlight = 0;
        if(highlight > MENU_CHOICES - 1) highlight = MENU_CHOICES - 1;

        if(!is_wintouched(menu))
        {
            touchwin(menu);
            box(menu, (int)' ' ,0);
            refresh();
        }
    }
}

void quit_game() 
{
    endwin();
    exit(0);
}

void restart_game() {return;}
void start_game() 
{
    int y_max, x_max, game_y_max, game_x_max;
    getmaxyx(stdscr, y_max, x_max);
    WINDOW *game = init_window(y_max, x_max, 30, 30);
    getmaxyx(game, game_y_max, game_x_max);
    touchwin(game);
    refresh();
    box(game, 0 ,0);

    for(int i = 1; i <= 3; i++)
    {
        wmove(game, i * ((game_y_max + 1) / 4) , 1);
        whline(game, ACS_HLINE, game_x_max - 2);
    }

    for(int i = 1; i <= 3; i++)
    {
        wmove(game, 1, i * (game_x_max + 1) / 4);
        wvline(game, ACS_VLINE, game_y_max - 2);
    }
    wrefresh(game);

    int ch;

    while(1)
    {
        ch = wgetch(game);


        if(ch == (int)'q' || ch == (int)'Q') 
        {
            untouchwin(game);
            return;
        }
    }
}





