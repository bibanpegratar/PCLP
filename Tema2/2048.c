#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <math.h>
#include <time.h>

#define MENU_PADDING_HEIGHT 5
#define MENU_PADDING_WIDTH 5
#define MENU_CHOICES 3
#define MENU_CHOICES_PADDING 2
#define MENU_CHOICES_TOP_OFFSET 10
#define GAME_PADDING_HEIGHT 4
#define GAME_PADDING_WIDTH 0
#define AI_DEPTH 12
#define TIMEOUT_MS 2000

typedef struct
{
    int number_in_cell, color_pair_id;
} cell_color_pair;

typedef struct
{
    int a, b;
} pair;

WINDOW *init_window(int y_max, int x_max, int window_padding_height, int window_padding_width);
WINDOW* init_square_window(int y_max, int x_max, int window_padding_height, int window_padding_width);
void operate_menu(WINDOW *menu, char choices[][9], char intro_text[7][24], int (*game_board)[4], cell_color_pair cells[12], int *score, int *has_resume);
void quit_game();
void game_loop(int (*game_board)[4], cell_color_pair cells[12], int new_game, int *score, int *has_resume);

void print_board(WINDOW* game, int (*game_board)[4], cell_color_pair cells[12], int game_y_max, int game_x_max);
void make_menu_action(char choices[][9], int highlight, int (*game_board)[4], cell_color_pair cells[12], int *score, int *has_resume);
void menu_control(WINDOW **menu, int ch, char choices[][9], int (*game_board)[4], cell_color_pair cells[12], int *has_resume, int *highlight,  int *score, int *y_max, int* x_max, int *choice_height);
int game_control(WINDOW **game, int ch, int (*game_board)[4], int *score, int *y_max, int *x_max, int *game_y_max, int *game_x_max);
void show_end_screen(WINDOW *game, int *has_resume, int *score, int *y_max, int *x_max, int *end_screen_y_max, int *end_screen_x_max);

int move_up(int (*game_board)[4], int *score);
int move_down(int (*game_board)[4], int *score);
int move_left(int (*game_board)[4], int *score);
int move_right(int (*game_board)[4], int *score);

int is_move_available(int (*game_board)[4], int score);
int is_2048(int (*game_board)[4]);
int is_game_board_full(int (*game_board)[4]);
void generate_random(int (*game_board)[4]);
int make_best_move(int (*game_board)[4], int *score, int depth);
int max_num(int a, int b, int c, int d);
int count_empty_cells(int (*game_board)[4]);


//custom colors for each cell value
void define_custom_colors() {
    init_color(8, 0, 200, 300);    // black
    init_color(9, 900, 850, 800);  // light gray
    init_color(10, 500, 500, 500); // darker gray
    init_color(11, 990, 600, 300); // orange
    init_color(12, 990, 500, 300); // darker orange
    init_color(13, 990, 400, 300); // more darker orange
    init_color(14, 990, 300, 300); // red
    init_color(15, 990, 200, 300); // dark red
    init_color(16, 990, 100, 300); // purple
    init_color(17, 900, 0, 300);   // dark purple
    init_color(18, 770, 0, 300);   // magenta
    init_color(19, 600, 0, 300);   // dark magenta
}

int main(int argc, char **argv)
{
    initscr();  

    //no buffer when reading input 
    cbreak();

    //disable cursor   
    curs_set(0);

    //do not display input characters
    noecho();

    //enable terminal colors
    start_color(); 

    define_custom_colors();
    
    //initialize color pairs for each type of cell in game
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // empty cell
    init_pair(2, COLOR_WHITE, 8);            // 2
    init_pair(3, COLOR_BLACK, 9);            // 4
    init_pair(4, COLOR_BLACK, 10);           // 8
    init_pair(5, COLOR_BLACK, 11);           // 16
    init_pair(6, COLOR_BLACK, 12);           // 32
    init_pair(7, COLOR_BLACK, 13);           // 64
    init_pair(8, COLOR_BLACK, 14);           // 128
    init_pair(9, COLOR_BLACK, 15);           // 256
    init_pair(10, COLOR_BLACK, 16);          // 512
    init_pair(11, COLOR_BLACK, 17);          // 1024
    init_pair(12, COLOR_BLACK, 18);          // 2048

    cell_color_pair cells[12] = 
    {
        [1].number_in_cell = 2,
        [1].color_pair_id = 2,

        [2].number_in_cell = 4,
        [2].color_pair_id = 3,

        [3].number_in_cell = 8,
        [3].color_pair_id = 4,

        [4].number_in_cell = 16,
        [4].color_pair_id = 5,

        [5].number_in_cell = 32,
        [5].color_pair_id = 6,

        [6].number_in_cell = 64,
        [6].color_pair_id = 7,

        [7].number_in_cell = 128,
        [7].color_pair_id = 8,

        [8].number_in_cell = 256,
        [8].color_pair_id = 9,

        [9].number_in_cell = 512,
        [9].color_pair_id = 10,

        [10].number_in_cell = 1024,
        [10].color_pair_id = 11,

        [11].number_in_cell = 2048,
        [11].color_pair_id = 12
    };

    //main menu options
    char choices[MENU_CHOICES][9]=
    {
        "NEW GAME",
        "RESUME  ",
        "QUIT    ",
    };

    //main menu display text
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

    int game_board [4][4] = {0};
    int score = 0;
    int has_resume = 0;

    int y_max, x_max;
    getmaxyx(stdscr, y_max, x_max);

    //create menu window
    WINDOW *menu = init_window(y_max, x_max, MENU_PADDING_HEIGHT, MENU_PADDING_WIDTH);
    operate_menu(menu, choices, intro_text, game_board, cells, &score, &has_resume);
    endwin();
    return 0;
}

//initialize a centered window with padding
WINDOW *init_window(int y_max, int x_max, int window_padding_height, int window_padding_width)
{
    /* Menu window initialization*/
    int window_height, window_width, window_start_y, window_start_x;
    window_height = y_max - (window_padding_height * y_max) / 100;
    window_width = x_max - (window_padding_width * x_max) / 100;
    window_start_y = (y_max - window_height) / 2;
    window_start_x = (x_max - window_width) / 2;

    WINDOW  *win = newwin(window_height, window_width, window_start_y, window_start_x);
    return win;
}

//initialize a squared centered window with padding
WINDOW* init_square_window(int y_max, int x_max, int window_padding_height, int window_padding_width)
{
    int window_l, window_padding;
    if(y_max < x_max)
    {
        window_padding = window_padding_height;
        y_max /= 4;
        y_max *= 4;
        window_l = y_max ;
    }
    else 
    {
        window_padding = window_padding_width;
        window_l = x_max - ((x_max - 5) % 4);
    }
    
    int window_start_y, window_start_x;
    window_l = window_l - 2 * window_padding;
    window_start_y = (y_max - window_l) / 2;
    window_start_x = (x_max - window_l * 2) / 2;

    WINDOW  *win = newwin(window_l + 1, window_l * 2 + 1, window_start_y, window_start_x);
    return win;
}

//menu screen functionality
void operate_menu(WINDOW *menu, char choices[][9], char intro_text[7][24], int (*game_board)[4], cell_color_pair cells[12], int *score, int *has_resume)
{
    int i;
    int ch = 0, highlight = 0, choice_height, current_choice_height;
    int y_max, x_max;
    keypad(menu, 1);

    while(1)
    {
        getmaxyx(menu, y_max, x_max);

        //positions of menu items, relative to padidng
        choice_height = (y_max - 2 - MENU_CHOICES_PADDING * (MENU_CHOICES - 1)) / (MENU_CHOICES - 1);

        //use special keys
        keypad(menu, 1); 
        box(menu, (int)' ' ,0); 
    
        //print intro text on upper side of menu
        wattron(menu, A_BOLD);
        wattron(menu, COLOR_PAIR(1));
        for(i = 0; i < 7; i++)
                mvwprintw(menu, i + 2, (x_max - strlen(intro_text[i]) - 2) / 2, intro_text[i]);
        wattroff(menu, A_BOLD);
        wattroff(menu, COLOR_PAIR(1));


        if(!*has_resume)
        {
            //print choices of menu, highlight selected one (without resume)
            current_choice_height = choice_height - 1;
            for(i = 0; i < MENU_CHOICES; i++)
            {
                //RESUME option is 2nd
                if(i != 1)
                {
                    if(i == highlight)
                        wattron(menu, A_REVERSE);
                    mvwprintw(menu, current_choice_height + MENU_CHOICES_TOP_OFFSET, (x_max - strlen(choices[i]) - 2) / 2, choices[i]);
                    wattroff(menu, A_REVERSE);
                    current_choice_height += MENU_CHOICES_PADDING;
                }
            }
        }

        else
        {
            //print choices of menu (with resume)
            current_choice_height = choice_height;
            for(i = 0; i < MENU_CHOICES; i++)
            {
                if(i == highlight)
                    wattron(menu, A_REVERSE);
                mvwprintw(menu, current_choice_height + MENU_CHOICES_TOP_OFFSET, (x_max - strlen(choices[i]) - 2) / 2, choices[i]);
                wattroff(menu, A_REVERSE);
                current_choice_height += MENU_CHOICES_PADDING;
            }
        }
        
        //move inside menu (arrow keys / WS, ENTER to Select)
        ch = wgetch(menu);
        menu_control(&menu, ch, choices, game_board, cells, has_resume, &highlight, score, &y_max, &x_max, &choice_height);

        if(highlight < 0) 
        {
            highlight = 0;
            beep();
        }

        if(highlight > MENU_CHOICES - 1) 
        {
            highlight = MENU_CHOICES - 1;
            beep();
        }

        if(!is_wintouched(menu))
            touchwin(menu);
    }
}

void quit_game() 
{
    endwin();
    exit(0);
}

//game loop logic
void game_loop(int (*game_board)[4], cell_color_pair cells[12], int new_game, int *score, int *has_resume)
{
    int y_max, x_max, game_y_max, game_x_max, end_screen_y_max, end_screen_x_max, i, j;
    time_t mytime = time(NULL);
    struct tm * time_str = localtime(&mytime);
    int ch;
    char time_formated[100];

    getmaxyx(stdscr, y_max, x_max);
    WINDOW *game = init_square_window(y_max, x_max, GAME_PADDING_HEIGHT, GAME_PADDING_HEIGHT);
    getmaxyx(game, game_y_max, game_x_max);
    touchwin(game);
    box(game, 0 ,0);
    refresh();
    wtimeout(game, TIMEOUT_MS);

    if(new_game)
    {
        //initialize board for newgame
        for(i = 0; i < 4; i++)
            for(j = 0; j < 4; j++)
                game_board[i][j] = 0;

        generate_random(game_board);
        generate_random(game_board);
        *score = 0;
    }

    while(1)
    {
        keypad(game, 1);
        wrefresh(game);
        
        //draw horizontal lines for board
        for(i = 1; i <= 3; i++)
        {
            wmove(game, i * game_y_max / 4 , 1);
            whline(game, ACS_HLINE, game_x_max - 2);
        }

        //draw vertical lines for board
        for(i = 1; i <= 3; i++)
        {
            wmove(game, 1, i * game_x_max / 4);
            wvline(game, ACS_VLINE, game_y_max - 2);
        }

        //print game board
        print_board(game, game_board, cells, game_y_max, game_x_max);
    
        //print current time
        mytime = time(NULL);
        time_str = localtime(&mytime);
        strftime(time_formated, sizeof(time_formated), "%H:%M:%S", time_str);
        mvwprintw(game, game_y_max - 1, 2, "Time: %s", time_formated);
        wrefresh(game);
        refresh();

        //print current score
        mvwprintw(game, game_y_max - 1, game_x_max - 1 - 15, "Score: %d", *score);
        wrefresh(game);
        refresh();

        //movement legend
        mvwprintw(game, 0, 1, "Move - WASD, Pause menu - Q");
    
        ch = wgetch(game);
        //if no character recieved after TIMEOUT_MS, make move automatically
        if(ch == ERR)
        {
            mvwprintw(game, 0, game_x_max - 11, "MOVING...");
            wrefresh(game);
            refresh();

            make_best_move(game_board, score, AI_DEPTH);
            wrefresh(game);
            refresh();
            
            box(game, 0, 0);
            mvwprintw(game, 0, game_x_max - 6, "DONE");
            wrefresh(game);
            refresh();

            //make random move
            generate_random(game_board);
        }
        
        else
        {
            wtimeout(game, TIMEOUT_MS);
            box(game, 0, 0);
            if(game_control(&game, ch, game_board, score, &y_max, &x_max, &game_y_max, &game_x_max) == 0)
                return;
        }

        //game has ended
        if(!is_move_available(game_board, *score) && !is_2048(game_board))
        {
            show_end_screen(game, has_resume, score, &y_max, &x_max, &end_screen_y_max, &end_screen_x_max);
            return;
        }
    }

}

void make_menu_action(char choices[][9], int highlight, int (*game_board)[4], cell_color_pair cells[12], int *score, int *has_resume)
{
    switch (highlight)
    {
        //start
        case 0:
            *has_resume = 1;
            game_loop(game_board, cells, 1, score, has_resume);
            break;
        
        //resume
        case 1:
            game_loop(game_board, cells, 0, score, has_resume);
            break;

        //quit
        case 2:
            quit_game();
            break;

        default:
            break;
    }
}

void menu_control(WINDOW **menu, int ch, char choices[][9], int (*game_board)[4], cell_color_pair cells[12], int *has_resume, int *highlight,  int *score, int *y_max, int* x_max, int *choice_height)
{
    switch(ch)
        {
            //move up
            case KEY_UP:
            case (int)'W':
            case (int)'w':
                if(!*has_resume && *highlight == 2) *highlight = *highlight - 2;
                else *highlight = *highlight - 1;
                break;
            
            //move down
            case KEY_DOWN:
            case (int)'S':
            case (int)'s':
                if(!*has_resume && *highlight == 0) *highlight = *highlight + 2;
                else *highlight = *highlight + 1;
                break;

            //choose option
            case 10:
                //unfocus menu
                untouchwin(*menu);
                wclear(*menu);
                wrefresh(*menu);
                refresh();

                //call function according to highlighted option
                make_menu_action(choices, *highlight, game_board, cells, score, has_resume);

                //resume is highlighted when refocusing pause menu
                if(highlight == 0) highlight++;

                //resize window after funxtion exists
                wclear(*menu);
                wrefresh(*menu);
                refresh();
                getmaxyx(stdscr, *y_max,* x_max);
                *menu = init_window(*y_max, *x_max, MENU_PADDING_HEIGHT, MENU_PADDING_WIDTH);
                *choice_height = (*y_max - 2 - MENU_CHOICES_PADDING * (MENU_CHOICES - 1)) / (MENU_CHOICES - 1);
                touchwin(*menu);
                box(*menu, (int)' ' ,0);
                refresh();

                break;

            //resize window when window size is changed
            case KEY_RESIZE:
                wclear(*menu);
                wrefresh(*menu);
                refresh();
                getmaxyx(stdscr, *y_max, *x_max);
                *menu = init_window(*y_max, *x_max, MENU_PADDING_HEIGHT, MENU_PADDING_WIDTH);
                *choice_height = (*y_max - 2 - MENU_CHOICES_PADDING * (MENU_CHOICES - 1)) / (MENU_CHOICES - 1);
                touchwin(*menu);
                box(*menu, (int)' ' ,0);
                refresh();
                break;

            default:
                break;
        }

}

int game_control(WINDOW **game, int ch, int (*game_board)[4], int *score, int *y_max, int *x_max, int *game_y_max, int *game_x_max)
{
    switch(ch)
    {
        //quit game
        case (int)'q':
        case (int)'Q':
            untouchwin(*game);
            wclear(*game);
            refresh();
            return 0;
            break;

        //move up
        case (int)'w':
        case (int)'W':
            if(move_up(game_board, score))
                generate_random(game_board);
            wrefresh(*game);
            mvwprintw(*game, 0, *game_x_max - 3, "UP");
            refresh();
            return 1;
            break;

        //move left
        case (int)'a':
        case (int)'A':
            if(move_left(game_board, score))
                generate_random(game_board);
            wrefresh(*game);
            mvwprintw(*game, 0, *game_x_max - 5, "LEFT");
            refresh();
            return 1;
            break;

        //move down
        case (int)'s':
        case (int)'S':
            if(move_down(game_board, score))
                generate_random(game_board);
            wrefresh(*game);
            mvwprintw(*game, 0, *game_x_max - 5, "DOWN");
            refresh();
            return 1;
            break;

        //move right
        case (int)'d':
        case (int)'D':
            if(move_right(game_board, score))
                generate_random(game_board);
            wrefresh(*game);
            mvwprintw(*game, 0, *game_x_max - 6, "RIGHT");
            refresh();
            return 1;
            break;

        //resize window when window size is changed
        case KEY_RESIZE:
            wclear(*game);
            wrefresh(*game);
            refresh();
            getmaxyx(stdscr, *y_max, *x_max);
            *game = init_square_window(*y_max, *x_max, GAME_PADDING_HEIGHT, GAME_PADDING_HEIGHT);
            getmaxyx(*game, *game_y_max, *game_x_max);
            touchwin(*game);
            box(*game, 0 ,0);
            refresh();
            return 1;
            break;

        default:
            return 1;
            break;
    }
    
}

void show_end_screen(WINDOW *game, int *has_resume, int *score, int *y_max, int *x_max, int *end_screen_y_max, int *end_screen_x_max)
{
    untouchwin(game);
    wclear(game);
    refresh();
    *has_resume = 0;

    //initialize end screen
    WINDOW *end_screen = init_square_window(*y_max, *x_max, GAME_PADDING_HEIGHT, GAME_PADDING_HEIGHT);
    getmaxyx(end_screen, *end_screen_y_max, *end_screen_x_max);
    touchwin(end_screen);
    box(end_screen, 0 ,0);

    //print score
    mvwprintw(end_screen, *end_screen_y_max / 2, *end_screen_x_max / 2, "FINAL SCORE: %d", *score);
    beep();
    wrefresh(end_screen);
    refresh();

    //wait before letting user press key to exit
    napms(2500);
    mvwprintw(end_screen, *end_screen_y_max / 2 + 1, *end_screen_x_max / 2, "Press any key to exit");
    wrefresh(end_screen);
    refresh();
    getch();
}

void print_board(WINDOW* game, int (*game_board)[4], cell_color_pair cells[12], int game_y_max, int game_x_max)
{
    int i, j, k, l;
    int col_pair_no;
    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            if(game_board[i][j] != 0)
            {
                //color each cell accordingly
                for(k = i * game_y_max / 4 + 1; k < (i + 1) * game_y_max / 4; k++)
                    for(l = j * game_x_max / 4 + 1; l < (j + 1) * game_x_max / 4; l++)
                    {
                        if(k < game_y_max - 1 && l < game_x_max - 1)
                        {
                            wmove(game, k, l);
                            col_pair_no = cells[(int)log2(game_board[i][j])].color_pair_id;
                            wattron(game, COLOR_PAIR(col_pair_no));
                            wprintw(game, " ");

                        }  
                    }
                    
                char dgt[5];
                sprintf(dgt, "%d", game_board[i][i]);

                //try to center number in cell
                wmove(game, i * game_y_max / 4 + game_y_max / 8, j * game_x_max / 4 + game_x_max / 8 - (strlen(dgt) / 2));

                wprintw(game, "%d", game_board[i][j]);
                wattroff(game, COLOR_PAIR(col_pair_no));
            }

            else
            {
                for(k = i * game_y_max / 4 + 1; k < (i + 1) * game_y_max / 4; k++)
                    for(l = j * game_x_max / 4 + 1; l < (j + 1) * game_x_max / 4; l++)
                    {
                        if(k < game_y_max - 1 && l < game_x_max - 1)
                        {
                            wmove(game, k, l);
                            wattron(game, COLOR_PAIR(1));
                            wprintw(game, " ");
                        }  
                    }
            } 
        }
    }
}

//checks if score changes after a move, if not, game is over
int is_move_available(int (*game_board)[4], int score)
{
    int i, j;
    int new_score = score;
    if(is_game_board_full(game_board))
    {
        int game_board_cpy[4][4];
        for(i = 0; i < 4; i++)
            for(j = 0; j < 4; j++)
            game_board_cpy[i][j] = game_board[i][j];

        move_up(game_board_cpy, &new_score);
        if(new_score != score) return 1;

        move_down(game_board_cpy, &new_score);
        if(new_score != score) return 1;

        move_left(game_board_cpy, &new_score);
        if(new_score != score) return 1;

        move_right(game_board_cpy, &new_score);
        if(new_score != score) return 1;

        return 0;
    }
    else return 1;
}

int is_2048(int (*game_board)[4])
{
    int i, j;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(game_board[i][i] == 2048)
                return 1;
    return 0;
}

int move_up(int (*game_board)[4], int *score)
{
    int i, j;
    int copy_board[4][4];
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            copy_board[i][j] = game_board[i][j];

    for(j = 0; j < 4; j++)
    {
        for(i = 1; i < 4; i++)
        {
            if(game_board[i][j] != 0)
            {
                int k = i;
                while(k > 0 && game_board[k - 1][j] == 0)
                {
                    //move the non-zero element up
                    game_board[k - 1][j] = game_board[k][j];
                    game_board[k][j] = 0;
                    k--;
                }
            }
        }
    }

    for(j = 0; j < 4; j++)
        for(i = 1; i < 4; i++)
        {
            int k = i;
            if(k > 0 && game_board[k - 1][j] == game_board[k][j] && game_board[k][j] != 0)
            {
                //merge identical elements
                game_board[k - 1][j] *= 2;
                *score = *score + game_board[k - 1][j];
                game_board[k][j] = 0;
                k--;
            }
        }
    
    for(j = 0; j < 4; j++)
    {
        for(i = 1; i < 4; i++)
        {
            if(game_board[i][j] != 0)
            {
                int k = i;
                while(k > 0 && game_board[k - 1][j] == 0)
                {
                    //move the non-zero element up
                    game_board[k - 1][j] = game_board[k][j];
                    game_board[k][j] = 0;
                    k--;
                }
            }
        }
    }

    //check if move was valid, by comparing boards
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(copy_board[i][j] != game_board[i][j])
                return 1;

    return 0;
}

int move_down(int (*game_board)[4], int *score)
{
    int i, j;
    int copy_board[4][4];
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            copy_board[i][j] = game_board[i][j];

    for(j = 0; j < 4; j++)
    {
        for(i = 2; i >= 0; i--)
        {
            if(game_board[i][j] != 0)
            {
                int k = i;
                while(k < 3 && game_board[k + 1][j] == 0)
                {
                    //move the non-zero element down
                    game_board[k + 1][j] = game_board[k][j];
                    game_board[k][j] = 0;
                    k++;
                }
            }
        }
    }

    for(j = 0; j < 4; j++)
    {
        for(i = 2; i >= 0; i--)
        {
            int k = i;
            if(k < 3 && game_board[k + 1][j] == game_board[k][j] && game_board[k][j] != 0)
            {
                //merge identical elements
                game_board[k + 1][j] *= 2;
                *score = *score + game_board[k + 1][j];
                game_board[k][j] = 0;
                k++;
            }
        }
    }

    for(j = 0; j < 4; j++)
    {
        for(i = 2; i >= 0; i--)
        {
            if(game_board[i][j] != 0)
            {
                int k = i;
                while(k < 3 && game_board[k + 1][j] == 0)
                {
                    //move the non-zero element down
                    game_board[k + 1][j] = game_board[k][j];
                    game_board[k][j] = 0;
                    k++;
                }
            }
        }
    }

    //check if move was valid, by comparing boards
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(copy_board[i][j] != game_board[i][j])
                return 1;

    return 0;
}

int move_left(int (*game_board)[4], int *score)
{
    int i, j;
    int copy_board[4][4];
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            copy_board[i][j] = game_board[i][j];

    for(i = 0; i < 4; i++)
    {
        for(j = 1; j < 4; j++)
        {
            if(game_board[i][j] != 0)
            {
                int k = j;
                while(k > 0 && game_board[i][k - 1] == 0)
                {
                    //move the non-zero element to the left
                    game_board[i][k - 1] = game_board[i][k];
                    game_board[i][k] = 0;
                    k--;
                }
            }
        }
    }

    for(i = 0; i < 4; i++)
    {
        for(j = 1; j < 4; j++)
        {
            int k = j;
            if(k > 0 && game_board[i][k - 1] == game_board[i][k] && game_board[i][k] != 0)
            {
                //merge identical elements
                game_board[i][k - 1] *= 2;
                *score = *score + game_board[i][k - 1];
                game_board[i][k] = 0;
                k--;
            }
        }
    }

    for(i = 0; i < 4; i++)
    {
        for(j = 1; j < 4; j++)
        {
            if(game_board[i][j] != 0)
            {
                int k = j;
                while(k > 0 && game_board[i][k - 1] == 0)
                {
                    //move the non-zero element to the left
                    game_board[i][k - 1] = game_board[i][k];
                    game_board[i][k] = 0;
                    k--;
                }
            }
        }
    }

    //check if move was valid, by comparing boards
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(copy_board[i][j] != game_board[i][j])
                return 1;

    return 0;
}

int move_right(int (*game_board)[4], int *score)
{
    int i, j;
    int copy_board[4][4];
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            copy_board[i][j] = game_board[i][j];

    for(i = 0; i < 4; i++)
    {
        for(j = 2; j >= 0; j--)
        {
            if(game_board[i][j] != 0)
            {
                int k = j;
                while(k < 3 && game_board[i][k + 1] == 0)
                {
                    //move the non-zero element to the right
                    game_board[i][k + 1] = game_board[i][k];
                    game_board[i][k] = 0;
                    k++;
                }
            }
        }
    }

    for(i = 0; i < 4; i++)
    {
        for(j = 2; j >= 0; j--)
        {
            int k = j;
            if(k < 3 && game_board[i][k + 1] == game_board[i][k] && game_board[i][k] != 0)
            {
                //merge identical elements
                game_board[i][k + 1] *= 2;
                *score = *score + game_board[i][k + 1];
                game_board[i][k] = 0;
                k++;
            }
        }
    }

    for(i = 0; i < 4; i++)
    {
        for(j = 2; j >= 0; j--)
        {
            if(game_board[i][j] != 0)
            {
                int k = j;
                while(k < 3 && game_board[i][k + 1] == 0)
                {
                    //move the non-zero element to the right
                    game_board[i][k + 1] = game_board[i][k];
                    game_board[i][k] = 0;
                    k++;
                }
            }
        }
    }

    //check if move was valid, by comparing boards
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(copy_board[i][j] != game_board[i][j])
                return 1;

    return 0;
}


//generate a 2 or 4 randomly on the board
void generate_random(int (*game_board)[4])
{
    int i, j, value = 0;

    time_t t;
    srand((unsigned) time(&t));

    while(!is_game_board_full(game_board))
    {
        value = 0;
        i = rand() % 4;
        j = rand() % 4;
        while(value == 0) value = 2 * (rand() % 3);
        if(game_board[i][j] == 0) 
        {
            game_board[i][j] = value;
            break;
        }
    }

}

int is_game_board_full(int (*game_board)[4])
{
    int i, j;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(game_board[i][j] == 0)
                return 0;
    
    return 1;
}


//recursive algorithm that checks for move that gives the most empty cells
//recursive algorithm that checks for move that gives the most empty cells
int make_best_move(int (*game_board)[4], int *score, int depth)
{
    int i, j, move;
    if(!is_move_available(game_board, *score) || depth <= 0)
        return 0;

    int max_empty_cells = -1;
    int best_move = -1;

    //move in each direction and evaluate the resulting score
    for(move = 0; move < 4; move++)
    {
        int temp_board[4][4];
        int temp_empty_cells = count_empty_cells(game_board);
        int temp_score = *score;

        for(i = 0; i < 4; i++)
            for(j = 0; j < 4; j++)
                temp_board[i][j] = game_board[i][j];

        switch(move)
        {
            case 0:
                move_up(temp_board, &temp_score);
                break;
            case 1:
                move_down(temp_board, &temp_score);
                break;
            case 2:
                move_left(temp_board, &temp_score);
                break;
            case 3:
                move_right(temp_board, &temp_score);
                break;
        }

        //evaluate the move and update the best move if needed
        int move_empty_cells = temp_empty_cells + make_best_move(temp_board, &temp_score, depth - 1);
        if(max_empty_cells < move_empty_cells)
        {
            max_empty_cells = move_empty_cells;
            best_move = move;
        }
    }

    //make best move on main game board
    switch(best_move)
    {
        case 0:
            move_up(game_board, score);
            break;

        case 1:
            move_down(game_board, score);
            break;

        case 2:
            move_left(game_board, score);
            break;

        case 3:
            move_right(game_board, score);
            break;
    }

    return max_empty_cells;
}

int max_num(int a, int b, int c, int d)
{
    int mx = a;
    if(mx < b) mx = b;
    if(mx < c) mx = c;
    if(mx < d) mx = d;
    return mx;
}

int count_empty_cells(int (*game_board)[4])
{
    int i, j, cnt = 0;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(game_board[i][j] == 0)
                cnt++;

    return cnt;
}