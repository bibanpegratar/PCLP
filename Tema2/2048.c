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
#define AI_DEPTH 5
#define END_SCREEN_DELAY 2000
#define SHOW_LAST_BOARD_DELAY 1000
#define GAMESAVE_FILENAME "gamesave.bin"

#define HAS_RESUME_FILE_POS 0
#define HIGHSCORE_FILE_POS 1
#define SCORE_FILE_POS 2
#define GAME_BOARD_FILE_POS 3

//set at 1 second for time to refresh each second if no input recieved
#define TIMEOUT_MS 1
//multiplied with TIMEOUT_MS, for making automatic move after some time
#define TIMEOUT_CNT 1

//ncurses colors work with 0-999 values for colors
//conversion value
#define RGB_TO_COLOR 1000/255
#define INTRO_COLOR_PAIR_ID 13

typedef struct
{
    int number_in_cell, color_pair_id;
} cell_color_pair;

//window creation
WINDOW *init_window(int y_max, int x_max, int window_padding_height, int window_padding_width);
WINDOW* init_square_window(int y_max, int x_max, int window_padding_height, int window_padding_width);

void operate_menu(WINDOW *menu, char choices[][9], char intro_text[7][24], int game_board[4][4], cell_color_pair cells[12], int *score, int *has_resume, int *curr_timeout_cnt, int *highscore);
void print_menu_options(WINDOW *menu, char choices[][9], int current_choice_height, int highlight, int has_resume, int x_max);
void menu_control(WINDOW **menu, int ch, char choices[][9], int game_board[4][4], cell_color_pair cells[12], int *has_resume, int *highlight,  int *score, int *y_max, int* x_max, int *choice_height, int *curr_timeout_cnt, int *highscore);
void make_menu_action(char choices[][9], int highlight, int game_board[4][4], cell_color_pair cells[12], int *score, int *has_resume, int *curr_timeout_cnt, int *highscore);
void show_end_screen(WINDOW *game, int *has_resume, int *score, int *y_max, int *x_max, int *end_screen_y_max, int *end_screen_x_max, int *highscore);
void quit_game(int *has_resume, int game_board[4][4], int *highscore, int *);

int move_up(int game_board[4][4], int *score);
int move_down(int game_board[4][4], int *score);
int move_left(int game_board[4][4], int *score);
int move_right(int game_board[4][4], int *score);

void game_loop(int game_board[4][4], cell_color_pair cells[12], int new_game, int *score, int *has_resume, int *curr_timeout_cnt, int *highscore);
int game_control(WINDOW **game, int ch, int game_board[4][4], int *score, int *y_max, int *x_max, int *game_y_max, int *game_x_max);
void print_board(WINDOW* game, int game_board[4][4], cell_color_pair cells[12], int game_y_max, int game_x_max);
int is_move_available(WINDOW* game, int game_board[4][4], int score);
int is_2048(int game_board[4][4]);
int is_game_board_full(int game_board[4][4]);
void generate_random(int game_board[4][4]);
int make_best_move(WINDOW *game, int game_board[4][4], int *score, int depth);
int max_num(int a, int b, int c, int d);
int count_empty_cells(int game_board[4][4]);
int compare_boards(int game_board[4][4], int game_board_cpy[4][4]);
void copy_boards(int game_board[4][4], int game_board_cpy[4][4]);
void rgb_init_color(int color_number, int r, int g, int b);

int set_resume_state(int has_resume);
int load_resume_state();
void set_board_state(int game_board[4][4]);
void load_board_state(int game_board[4][4]);
int set_highscore_state(int highsocre);
int load_highscore_state();
int set_score_state(int score);
int load_score_state();

int main(int argc, char **argv)
{
    initscr();  

    //no buffer when reading input 
    cbreak();

    //disable cursor   
    curs_set(0);

    //do not display input characters
    noecho();

    //enable terminal colors, if colors can be changed
    if(has_colors() && can_change_color())
        start_color(); 

    //define custom colors for cells
    rgb_init_color(8, 0, 0, 25);       //dark blue
    rgb_init_color(9, 200, 200, 200);  //light gray
    rgb_init_color(10, 125, 125, 125); //dark gray
    rgb_init_color(11, 255, 180, 0);   //yellow
    rgb_init_color(12, 255, 120, 0);   //orange
    rgb_init_color(13, 255, 80, 0);    //darker orange
    rgb_init_color(14, 255, 150, 150); //pink-ish
    rgb_init_color(15, 255, 100, 100); //darker pink
    rgb_init_color(16, 255, 50, 50);   //light red
    rgb_init_color(17, 200, 0, 0);     //red
    rgb_init_color(18, 150, 0, 0);     //dark red

    //random value for menu intro text
    srand(time(NULL));
    rgb_init_color(19, rand() % 256, rand() % 256, rand() % 256); 
    
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
    init_pair(INTRO_COLOR_PAIR_ID, 19, COLOR_BLACK); //intro text

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

    int score = load_score_state();
    int has_resume = load_resume_state();
    int highscore = load_highscore_state();
    int curr_timeout_cnt = 0;
    if(has_resume != 0)
        load_board_state(game_board);

    int y_max, x_max;
    getmaxyx(stdscr, y_max, x_max);

    //create menu window
    WINDOW *menu = init_window(y_max, x_max, MENU_PADDING_HEIGHT, MENU_PADDING_WIDTH);
    operate_menu(menu, choices, intro_text, game_board, cells, &score, &has_resume, &curr_timeout_cnt, &highscore);

    endwin();
    return 0;
}

//initialize a centered window with padding
WINDOW *init_window(int y_max, int x_max, int window_padding_height, int window_padding_width)
{
    //menu window initialization
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
void operate_menu(WINDOW *menu, char choices[][9], char intro_text[7][24], int game_board[4][4], cell_color_pair cells[12], int *score, int *has_resume, int *curr_timeout_cnt, int *highscore)
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

        //draw box
        box(menu, (int)' ' ,0); 
    
        //print intro text on upper side of menu
        wattron(menu, A_BOLD);
        wattron(menu, COLOR_PAIR(INTRO_COLOR_PAIR_ID));
        for(i = 0; i < 7; i++)
                mvwprintw(menu, i + 2, (x_max - strlen(intro_text[i]) - 2) / 2, intro_text[i]);
        wattroff(menu, A_BOLD);
        wattroff(menu, COLOR_PAIR(INTRO_COLOR_PAIR_ID));

        if(!*has_resume) current_choice_height = choice_height - 1;
        else current_choice_height = choice_height;

        print_menu_options(menu, choices, current_choice_height, highlight, *has_resume, x_max);
        
        //move inside menu (arrow keys / WS, ENTER to Select)
        ch = wgetch(menu);
        menu_control(&menu, ch, choices, game_board, cells, has_resume, &highlight, score, &y_max, &x_max, &choice_height, curr_timeout_cnt, highscore);

        //verify out of bounds
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

void quit_game(int *has_resume, int game_board[4][4], int *highscore, int *score) 
{
    //save state of game
    set_resume_state(*has_resume);
    set_highscore_state(*highscore);
    set_score_state(*score);
    set_board_state(game_board);

    endwin();
    exit(0);
}

//game loop logic
void game_loop(int game_board[4][4], cell_color_pair cells[12], int new_game, int *score, int *has_resume, int *curr_timeout_cnt, int *highscore)
{
    int y_max, x_max, game_y_max, game_x_max, end_screen_y_max, end_screen_x_max, i, j;
    time_t mytime = time(NULL);
    struct tm * time_str = localtime(&mytime);
    int ch;
    char time_formated[100];

    //create game window
    getmaxyx(stdscr, y_max, x_max);
    WINDOW *game = init_square_window(y_max, x_max, GAME_PADDING_HEIGHT, GAME_PADDING_HEIGHT);
    getmaxyx(game, game_y_max, game_x_max);
    touchwin(game);
    box(game, 0 ,0);
    refresh();

    //handle no input after some time
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
        is_move_available(game, game_board, *score);
    
        ch = wgetch(game);
        
        if(ch == ERR)
        {
            //make move after TIMEOUT_CNT seconds have passed
            if(*curr_timeout_cnt >= TIMEOUT_CNT)
            {
                mvwprintw(game, 0, game_x_max - 10, "MOVING...");
                wrefresh(game);
                refresh();

                make_best_move(game, game_board, score, AI_DEPTH);
                wrefresh(game);
                refresh();
                
                box(game, 0, 0);
                mvwprintw(game, 0, game_x_max - 6, "DONE");
                wrefresh(game);
                refresh();

                generate_random(game_board);
                *curr_timeout_cnt = 0;
            }
            else 
            {
                *curr_timeout_cnt = *curr_timeout_cnt + 1;
                wrefresh(game);
                refresh();
            }
        }
        
        else
        {
            wtimeout(game, TIMEOUT_MS);
            box(game, 0, 0);
            if(game_control(&game, ch, game_board, score, &y_max, &x_max, &game_y_max, &game_x_max) == 0)
                return;
        }

        //game has ended
        if(!is_move_available(game, game_board, *score) || is_2048(game_board))
        {
            //display last version of the board for SHOW_LAST_BOARD_DELAY ms
            print_board(game, game_board, cells, game_y_max, game_x_max);
            wrefresh(game);
            refresh();
            napms(SHOW_LAST_BOARD_DELAY);
            show_end_screen(game, has_resume, score, &y_max, &x_max, &end_screen_y_max, &end_screen_x_max, highscore);
            return;
        }
    }

}

void make_menu_action(char choices[][9], int highlight, int game_board[4][4], cell_color_pair cells[12], int *score, int *has_resume, int *curr_timeout_cnt, int *highscore)
{
    switch (highlight)
    {
        //start
        case 0:
            *has_resume = 1;
            game_loop(game_board, cells, 1, score, has_resume, curr_timeout_cnt, highscore);
            break;
        
        //resume
        case 1:
            game_loop(game_board, cells, 0, score, has_resume, curr_timeout_cnt, highscore);
            break;

        //quit
        case 2:
            quit_game(has_resume, game_board, highscore, score);
            break;

        default:
            break;
    }
}

void menu_control(WINDOW **menu, int ch, char choices[][9], int game_board[4][4], cell_color_pair cells[12], int *has_resume, int *highlight,  int *score, int *y_max, int* x_max, int *choice_height, int *curr_timeout_cnt, int *highscore)
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
                make_menu_action(choices, *highlight, game_board, cells, score, has_resume, curr_timeout_cnt, highscore);

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

int game_control(WINDOW **game, int ch, int game_board[4][4], int *score, int *y_max, int *x_max, int *game_y_max, int *game_x_max)
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
        case KEY_UP:
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
        case KEY_LEFT:
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
        case KEY_DOWN:
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
        case KEY_RIGHT:
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

void show_end_screen(WINDOW *game, int *has_resume, int *score, int *y_max, int *x_max, int *end_screen_y_max, int *end_screen_x_max, int *highscore)
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

    if(*score > *highscore)
    {
        *highscore = *score;
        mvwprintw(end_screen, *end_screen_y_max / 2, *end_screen_x_max / 2, "NEW HIGH SCORE: %d", *highscore);
        beep();
        wrefresh(end_screen);
        refresh();
    }

    else
    {
        mvwprintw(end_screen, *end_screen_y_max / 2, *end_screen_x_max / 2, "HIGH SCORE: %d", *highscore);
        mvwprintw(end_screen, *end_screen_y_max / 2 + 1, *end_screen_x_max / 2, "FINAL SCORE: %d", *score);
        beep();
        wrefresh(end_screen);
        refresh();
    }

    

    //wait before letting user press key to exit
    napms(END_SCREEN_DELAY);
    mvwprintw(end_screen, *end_screen_y_max / 2 + 2, *end_screen_x_max / 2, "Press any key to exit");

    //flush any characters pressed before prompt appears
    flushinp();
    napms(50);
    wrefresh(end_screen);
    refresh();
    getch();
}

void print_board(WINDOW* game, int game_board[4][4], cell_color_pair cells[12], int game_y_max, int game_x_max)
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

                //center number in cell
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
int is_move_available(WINDOW* game, int game_board[4][4], int score)
{
    int ok = 0;
    int new_score = score;
    mvwprintw(game, 0, 1, "AVAILABLE: ");
    wrefresh(game);
    refresh();
    
    //initialize board_copy before each simulated move
    //check if score is different or if positions on board have changed

    //UP
    int game_board_cpy[4][4];
    copy_boards(game_board, game_board_cpy);

    move_up(game_board_cpy, &new_score);
    if(new_score != score || !compare_boards(game_board, game_board_cpy))
    { 
        ok = 1;
        mvwprintw(game, 0, 13, "UP");
        wrefresh(game);
        refresh();
    }

    //DOWN
    new_score = score;
    copy_boards(game_board, game_board_cpy);

    move_down(game_board_cpy, &new_score);
    if(new_score != score || !compare_boards(game_board, game_board_cpy))
    {
        ok = 1;
        mvwprintw(game, 0, 16, "DOWN");
        wrefresh(game);
        refresh();
    }

    //LEFT
    new_score = score;
    copy_boards(game_board, game_board_cpy);

    move_left(game_board_cpy, &new_score);
    if(new_score != score || !compare_boards(game_board, game_board_cpy))
    {
        ok = 1;
        mvwprintw(game, 0, 21, "LEFT");
        wrefresh(game);
        refresh();
    }

    //RIGHT
    new_score = score;
    copy_boards(game_board, game_board_cpy);

    move_right(game_board_cpy, &new_score);
    if(new_score != score || !compare_boards(game_board, game_board_cpy))
    {
        ok = 1;
        mvwprintw(game, 0, 26, "RIGHT");
        wrefresh(game);
        refresh();
    }

    return ok;
}

int is_2048(int game_board[4][4])
{
    int i, j;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(game_board[i][i] == 2048)
                return 1;
    return 0;
}

int move_up(int game_board[4][4], int *score)
{
    int i, j;
    int copy_board[4][4];
    copy_boards(game_board, copy_board);

    //shift all 0 elements
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

    //merge cells 2 by 2
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
    
    //shift all 0 elements again
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

    //set_board_state(game_board);
    //check if move was valid, by comparing boards
    return !compare_boards(game_board, copy_board);
}

int move_down(int game_board[4][4], int *score)
{
    int i, j;
    int copy_board[4][4];
    copy_boards(game_board, copy_board);

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

    //set_board_state(game_board);
    //check if move was valid, by comparing boards
    return !compare_boards(game_board, copy_board);
}

int move_left(int game_board[4][4], int *score)
{
    int i, j;
    int copy_board[4][4];
    copy_boards(game_board, copy_board);

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

    //set_board_state(game_board);
    //check if move was valid, by comparing boards
    return !compare_boards(game_board, copy_board);
}

int move_right(int game_board[4][4], int *score)
{
    int i, j;
    int copy_board[4][4];
    copy_boards(game_board, copy_board);

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

    //set_board_state(game_board);
    //check if move was valid, by comparing boards
    return !compare_boards(game_board, copy_board);
}

//generate a 2 or 4 randomly on the board
void generate_random(int game_board[4][4])
{
    int i, j, value = 0;

    srand(time(NULL));

    //try and generate while there is free space on board
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

int is_game_board_full(int game_board[4][4])
{
    int i, j;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(game_board[i][j] == 0)
                return 0;
    
    return 1;
}

//recursive algorithm that checks for move that gives the most empty cells
int make_best_move(WINDOW *game, int game_board[4][4], int *score, int depth)
{
    int i, j, move;
    if(!is_move_available(game, game_board, *score) || depth <= 0)
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
        int move_empty_cells = temp_empty_cells + make_best_move(game, temp_board, &temp_score, depth - 1);
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

int count_empty_cells(int game_board[4][4])
{
    int i, j, cnt = 0;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(game_board[i][j] == 0)
                cnt++;

    return cnt;
}

//TODO: write in README
//binary file fomat
//  - first int in binary file is for has_resume
//  - second int in binary file is for highest score
//  - next 16 ints are for the last modified board after exit
//always call set functions in this order (set_resume_state creates file)

//creates gamesave file and writes has_resume state
//game state is only saved to file when quitting to pause menu
int set_resume_state(int has_resume)
{
    FILE *game_save;
    int has_resume_copy = has_resume;
    game_save = fopen(GAMESAVE_FILENAME, "wb");

    if(game_save != NULL)
    {
        fwrite(&has_resume_copy, sizeof(has_resume_copy), 1, game_save);
        fclose(game_save);
        return has_resume_copy;
    }

    else return -1;
}

//reads resume_state from gamesave file
int load_resume_state()
{
    FILE *game_save;
    int has_resume;
    game_save = fopen(GAMESAVE_FILENAME, "rb");

    //set default resume state to 0
    if(game_save == NULL)
    {
        has_resume = set_resume_state(0);
    }

    else
    {
        //fseek(game_save, HAS_RESUME_FILE_POS * sizeof(int), SEEK_SET);
        fread(&has_resume, sizeof(has_resume), 1, game_save);
        fclose(game_save);
    }
    return has_resume;
}

int set_highscore_state(int highscore)
{
    FILE *game_save;
    int highscore_copy = highscore;
    game_save = fopen(GAMESAVE_FILENAME, "ab");
    
    if(game_save != NULL)
    {
        fwrite(&highscore_copy, sizeof(int), 1, game_save);
        fclose(game_save);
    }

    return highscore_copy;
}

int load_highscore_state()
{
    FILE *game_save;
    int highscore = 0;
    game_save = fopen(GAMESAVE_FILENAME, "rb");

    if(game_save != NULL)
    {
        fseek(game_save, HIGHSCORE_FILE_POS * sizeof(int), SEEK_SET);
        fread(&highscore, sizeof(highscore), 1, game_save);
        fclose(game_save);
    }

    return highscore;
}

int set_score_state(int score)
{
    FILE *game_save;
    int score_copy = score;
    game_save = fopen(GAMESAVE_FILENAME, "ab");
    
    if(game_save != NULL)
    {
        fwrite(&score_copy, sizeof(int), 1, game_save);
        fclose(game_save);
    }

    return score_copy;
}

int load_score_state()
{
    FILE *game_save;
    int score = 0;
    game_save = fopen(GAMESAVE_FILENAME, "rb");

    if(game_save != NULL)
    {
        fseek(game_save, SCORE_FILE_POS * sizeof(int), SEEK_SET);
        fread(&score, sizeof(score), 1, game_save);
        fclose(game_save);
    }

    return score;
}

//append after resume_state, the game_board
void set_board_state(int game_board[4][4])
{
    FILE *game_save;
    game_save = fopen(GAMESAVE_FILENAME, "ab");

    if(game_save != NULL)
    {
        fwrite(game_board, sizeof(int[4][4]), 1, game_save);
        fclose(game_save);
    }
}

//read game_board
void load_board_state(int game_board[4][4])
{
    FILE *game_save;
    game_save = fopen(GAMESAVE_FILENAME, "rb");

    if(game_save != NULL)
    {
        fseek(game_save, GAME_BOARD_FILE_POS * sizeof(int), SEEK_SET);
        fread(game_board, sizeof(int[4][4]), 1, game_save);
        fclose(game_save);        
    }
}

//returns 1 if boards are equal
int compare_boards(int game_board[4][4], int game_board_cpy[4][4])
{
    int i, j;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            if(game_board[i][j] != game_board_cpy[i][j])
                return 0;
    return 1;
}

//copies game_board into game_board_copy
void copy_boards(int game_board[4][4], int game_board_copy[4][4])
{
    int i, j;
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            game_board_copy[i][j] = game_board[i][j];
}

void print_menu_options(WINDOW *menu, char choices[][9], int current_choice_height, int highlight, int has_resume, int x_max)
{
    int i;
    for(i = 0; i < MENU_CHOICES; i++)
    {
        //RESUME option is 2nd
        if(!(has_resume == 0 && i == 1))
        {
            if(i == highlight)
                wattron(menu, A_REVERSE);
            mvwprintw(menu, current_choice_height + MENU_CHOICES_TOP_OFFSET, (x_max - strlen(choices[i]) - 2) / 2, choices[i]);
            wattroff(menu, A_REVERSE);
            current_choice_height += MENU_CHOICES_PADDING;
        }
    }
}

//wrapper function for init_color for easier usage
void rgb_init_color(int color_number, int r, int g, int b)
{
    int conv_r, conv_g, conv_b;
    conv_r = round(r * RGB_TO_COLOR);
    conv_g = round(g * RGB_TO_COLOR);
    conv_b = round(b * RGB_TO_COLOR);
    init_color(color_number, conv_r, conv_g, conv_b);
}