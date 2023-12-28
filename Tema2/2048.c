#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <math.h>
#include <time.h>

#define MENU_PADDING_HEIGHT 5 /* % top and bottom padding */
#define MENU_PADDING_WIDTH 5  /* % left and right padding */
#define MENU_CHOICES 3
#define MENU_CHOICES_PADDING 2
#define MENU_CHOICES_TOP_OFFSET 10
#define GAME_PADDING_HEIGHT 4 //in columns(dont change)
#define GAME_PADDING_WIDTH 0
#define AI_DEPTH 12

#define TIMEOUT_MS 2000 // 1000 milliseconds = 1 second

typedef struct
{
    int number_in_cell, color_pair_id;
} cell_color_pair;

WINDOW *init_window(int y_max, int x_max, int window_padding_height, int window_padding_width);
void operate_menu(WINDOW *menu, char choices[][9], char intro_text[7][24], int (*game_board)[4], cell_color_pair cells[12], int *score, int *has_resume);

void game_loop(int (*game_board)[4], cell_color_pair cells[12], int new_game, int *score, int *has_resume);
void quit_game();

void make_menu_action(char choices[][9], int highlight, int (*game_board)[4], cell_color_pair cells[12], int *score, int *has_resume);
void generate_random(int (*game_board)[4]);
int is_game_board_full(int (*game_board)[4]);
int is_move_available(int (*game_board)[4], int score);

void move_up(int (*game_board)[4], int *score);
void move_left(int (*game_board)[4], int *score);
void move_down(int (*game_board)[4], int *score);
void move_right(int (*game_board)[4], int *score);
int make_best_move(int (*game_board)[4], int *score, int depth);
int max_num(int a, int b, int c, int d);

// Define custom color attributes for each value
void define_custom_colors() {
    init_color(8, 0, 200, 300);         // Black background
    init_color(9, 900, 850, 800);   // Light gray background
    init_color(10, 500, 500, 500);   // Darker gray background (adjusted intensities)
    init_color(11, 990, 600, 300);   // Orange background
    init_color(12, 990, 500, 300);  // Darker orange background
    init_color(13, 990, 400, 300);  // Even darker orange background
    init_color(14, 990, 300, 300);  // Red background
    init_color(15, 990, 200, 300); // Dark red background
    init_color(16, 990, 100, 300); // Purple background
    init_color(17, 900, 0, 300);   // Dark purple background
    init_color(18, 770, 0, 300);  // Magenta background
    init_color(19, 600, 0, 300);  // Dark magenta background
}

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    curs_set(0);
    raw();
    noecho();
    
    start_color();
    define_custom_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // Empty cell
    init_pair(2, COLOR_WHITE, 8);        // Value 2
    init_pair(3, COLOR_BLACK, 9);           // Value 4
    init_pair(4, COLOR_BLACK, 10);           // Value 8
    init_pair(5, COLOR_BLACK, 11);          // Value 16
    init_pair(6, COLOR_BLACK, 12);          // Value 32
    init_pair(7, COLOR_BLACK, 13);          // Value 64
    init_pair(8, COLOR_BLACK, 14);         // Value 128
    init_pair(9, COLOR_BLACK, 15);         // Value 256
    init_pair(10, COLOR_BLACK, 16);         // Value 512
    init_pair(11, COLOR_BLACK, 17);       // Value 1024
    init_pair(12, COLOR_BLACK, 18);       // Value 2048

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

    char choices[MENU_CHOICES][9]=
    {
        "NEW GAME",
        "RESUME  ",
        "QUIT    ",
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

    int game_board [4][4] = {0};
    int score = 0;
    int has_resume = 0;

    int y_max, x_max;
    getmaxyx(stdscr, y_max, x_max);
    WINDOW *menu = init_window(y_max, x_max, MENU_PADDING_HEIGHT, MENU_PADDING_WIDTH);
    operate_menu(menu, choices, intro_text, game_board, cells, &score, &has_resume);
    endwin();
    return 0;
}

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

WINDOW* init_square_window(int y_max, int x_max, int window_padding_height, int window_padding_width)
{
    /* Menu window initialization*/
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
    
    int window_height, window_width, window_start_y, window_start_x;
    window_l = window_l - 2 * window_padding;
    window_start_y = (y_max - window_l) / 2;
    window_start_x = (x_max - window_l * 2) / 2;

    WINDOW  *win = newwin(window_l + 1, window_l * 2 + 1, window_start_y, window_start_x);
    return win;
}

void operate_menu(WINDOW *menu, char choices[][9], char intro_text[7][24], int (*game_board)[4], cell_color_pair cells[12], int *score, int *has_resume)
{
    int ch = 0, highlight = 0, choice_height, current_choice_height;
    int y_max, x_max;
    keypad(menu, 1);

    while(1)
    {
        getmaxyx(menu, y_max, x_max);
        choice_height = (y_max - 2 - MENU_CHOICES_PADDING * (MENU_CHOICES - 1)) / (MENU_CHOICES - 1);
        keypad(menu, 1);
        box(menu, (int)' ' ,0);
    
        /* print intro_text on menu*/
        wattron(menu, A_BOLD);
        wattron(menu, COLOR_PAIR(1));
        for(int i = 0; i < 7; i++)
                mvwprintw(menu, i + 2, (x_max - strlen(intro_text[i]) - 2) / 2, intro_text[i]);
        wattroff(menu, A_BOLD);
        wattroff(menu, COLOR_PAIR(1));

        if(!*has_resume)
        {
            /* print choices of menu*/
            current_choice_height = choice_height - 1;
            for(int i = 0; i < MENU_CHOICES; i++)
            {
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
            /* print choices of menu*/
            current_choice_height = choice_height;
            for(int i = 0; i < MENU_CHOICES; i++)
            {
                if(i == highlight)
                    wattron(menu, A_REVERSE);
                mvwprintw(menu, current_choice_height + MENU_CHOICES_TOP_OFFSET, (x_max - strlen(choices[i]) - 2) / 2, choices[i]);
                wattroff(menu, A_REVERSE);
                current_choice_height += MENU_CHOICES_PADDING;
            }
        }
        
        /* move inside menu (arrow keys / WS, ENTER to Select)*/
        ch = wgetch(menu);

        switch(ch)
        {
            /* move up */
            case KEY_UP:
            case (int)'W':
            case (int)'w':
                if(!*has_resume && highlight == 2) highlight -= 2;
                else highlight--;
                break;
            
            /* move down */
            case KEY_DOWN:
            case (int)'S':
            case (int)'s':
                if(!*has_resume && highlight == 0) highlight += 2;
                else highlight++;
                break;

            /* choose option*/
            case 10:
                /* unfocus menu */
                untouchwin(menu);
                wclear(menu);
                wrefresh(menu);
                refresh();

                /* call function according to highlighted option */
                make_menu_action(choices, highlight, game_board, cells, score, has_resume);

                /* resize window after funxtion exists */
                wclear(menu);
                wrefresh(menu);
                refresh();
                getmaxyx(stdscr, y_max, x_max);
                menu = init_window(y_max, x_max, MENU_PADDING_HEIGHT, MENU_PADDING_WIDTH);
                choice_height = (y_max - 2 - MENU_CHOICES_PADDING * (MENU_CHOICES - 1)) / (MENU_CHOICES - 1);
                touchwin(menu);
                box(menu, (int)' ' ,0);
                refresh();

                break;
            
            case (int)'Q':
            case (int)'q':
                return;
                break;

            /* resize window */
            case KEY_RESIZE:
                wclear(menu);
                wrefresh(menu);
                refresh();
                getmaxyx(stdscr, y_max, x_max);
                menu = init_window(y_max, x_max, MENU_PADDING_HEIGHT, MENU_PADDING_WIDTH);
                choice_height = (y_max - 2 - MENU_CHOICES_PADDING * (MENU_CHOICES - 1)) / (MENU_CHOICES - 1);
                touchwin(menu);
                box(menu, (int)' ' ,0);
                refresh();
                break;

            default:
                break;
        }

        if(highlight < 0) {highlight = 0; beep();}
        if(highlight > MENU_CHOICES - 1) {highlight = MENU_CHOICES - 1; beep();}
        if(!is_wintouched(menu))
        {
            touchwin(menu);
        }
    }
}

void quit_game() 
{
    endwin();
    exit(0);
}

void game_loop(int (*game_board)[4], cell_color_pair cells[12], int new_game, int *score, int *has_resume)
{
    int y_max, x_max, game_y_max, game_x_max, col_pair_no, end_screen_y_max, end_screen_x_max;
    time_t mytime = time(NULL);
    struct tm * time_str = localtime(&mytime);
    int timeout_cnt = 0;
    int ch, prev_ch;

    getmaxyx(stdscr, y_max, x_max);
    WINDOW *game = init_square_window(y_max, x_max, GAME_PADDING_HEIGHT, GAME_PADDING_HEIGHT);
    getmaxyx(game, game_y_max, game_x_max);
    touchwin(game);
    box(game, 0 ,0);
    refresh();
    wtimeout(game, TIMEOUT_MS);
    //nodelay(game, TRUE);

    if(new_game)
    {
        //initialize board for newgame
        for(int i = 0; i < 4; i++)
            for(int j = 0; j < 4; j++)
                game_board[i][j] = 0;

        generate_random(game_board);
        generate_random(game_board);
        *score = 0;
    }

    while(1)
    {
        keypad(game, 1);
        wrefresh(game);
        
        //draw horizontal lines
        for(int i = 1; i <= 3; i++)
        {
            wmove(game, i * game_y_max / 4 , 1);
            whline(game, ACS_HLINE, game_x_max - 2);
            // wprintw(game, "%d", i * game_y_max / 4);
        }

        //draw vertical lines
        for(int i = 1; i <= 3; i++)
        {
            wmove(game, 1, i * game_x_max / 4);
            wvline(game, ACS_VLINE, game_y_max - 2);
        }

        // wmove(game, game_y_max - 1, 1);
        // wprintw(game, "%d", game_y_max);

        /* print elements of board */
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                if(game_board[i][j] != 0)
                {
                    /* color each cell accordingly */
                    for(int k = i * game_y_max / 4 + 1; k < (i + 1) * game_y_max / 4; k++)
                        for(int l = j * game_x_max / 4 + 1; l < (j + 1) * game_x_max / 4; l++)
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
                    wmove(game, i * game_y_max / 4 + game_y_max / 8, j * game_x_max / 4 + game_x_max / 8 - (strlen(dgt) / 2));
                    wprintw(game, "%d", game_board[i][j]);
                    wattroff(game, COLOR_PAIR(col_pair_no));
                }

                else
                {
                    for(int k = i * game_y_max / 4 + 1; k < (i + 1) * game_y_max / 4; k++)
                        for(int l = j * game_x_max / 4 + 1; l < (j + 1) * game_x_max / 4; l++)
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
        mytime = time(NULL);
        time_str = localtime(&mytime);
        mvwprintw(game, game_y_max - 1, 2, "Time: %d:%d:%d", time_str->tm_hour, time_str->tm_min, time_str->tm_sec);
        wrefresh(game);
        refresh();
        mvwprintw(game, game_y_max - 1, game_x_max - 1 - 15, "Score: %d", *score);
        wrefresh(game);
        refresh();
        
        // mvwprintw(game, 1, 0, "Window dimension(x): %d", game_x_max);
        ch = wgetch(game);


        if(ch == ERR)
        {
            mvwprintw(game, 0, game_x_max / 2 - 5, "MOVING...");
            wrefresh(game);
            refresh();

            make_best_move(game_board, score, AI_DEPTH);
            wrefresh(game);
            refresh();
            
            box(game, 0, 0);
            mvwprintw(game, 0, game_x_max / 2 - 2, "DONE");
            wrefresh(game);
            refresh();

            generate_random(game_board);
            //qnapms(250);

            // box(game, 0, 0);
            // wrefresh(game);
            // refresh();
        }
        
        else
        {
            wtimeout(game, TIMEOUT_MS);
            box(game, 0, 0);
            switch (ch)
            {
                case (int)'q':
                case (int)'Q':
                    untouchwin(game);
                    wclear(game);
                    refresh();
                    return;
                    break;

                case (int)'w':
                case (int)'W':
                    move_up(game_board, score);
                    generate_random(game_board);
                    wrefresh(game);
                    mvwprintw(game, 0, game_x_max / 2 - 1, "UP");
                    refresh();
                    break;

                case (int)'a':
                case (int)'A':
                    move_left(game_board, score);
                    generate_random(game_board);
                    wrefresh(game);
                    mvwprintw(game, 0, game_x_max / 2 - 3, "LEFT");
                    refresh();
                    break;

                case (int)'s':
                case (int)'S':
                    move_down(game_board, score);
                    generate_random(game_board);
                    wrefresh(game);
                    mvwprintw(game, 0, game_x_max / 2 - 2, "DOWN");
                    refresh();
                    break;

                case (int)'d':
                case (int)'D':
                    move_right(game_board, score);
                    generate_random(game_board);
                    wrefresh(game);
                    mvwprintw(game, 0, game_x_max / 2 - 3, "RIGHT");
                    refresh();
                    break;

                case KEY_RESIZE:
                    wclear(game);
                    wrefresh(game);
                    refresh();
                    getmaxyx(stdscr, y_max, x_max);
                    game = init_square_window(y_max, x_max, GAME_PADDING_HEIGHT, GAME_PADDING_HEIGHT);
                    getmaxyx(game, game_y_max, game_x_max);
                    touchwin(game);
                    box(game, 0 ,0);
                    refresh();
                    break;

                default:
                    break;
            }
        }

        if(!is_move_available(game_board, *score))
        {
            untouchwin(game);
            wclear(game);
            refresh();
            *has_resume = 0;

            WINDOW *end_screen = init_square_window(y_max, x_max, GAME_PADDING_HEIGHT, GAME_PADDING_HEIGHT);
            getmaxyx(end_screen, end_screen_y_max, end_screen_x_max);
            touchwin(end_screen);
            box(end_screen, 0 ,0);
            mvwprintw(end_screen, end_screen_y_max / 2, end_screen_x_max / 2, "FINAL SCORE: %d", *score);
            beep();
            wrefresh(end_screen);
            refresh();

            napms(2500);
            mvwprintw(end_screen, end_screen_y_max / 2 + 1, end_screen_x_max / 2, "Press any key to exit");
            wrefresh(end_screen);
            refresh();
            getch();

            return;
        }
    }

}

void make_menu_action(char choices[][9], int highlight, int (*game_board)[4], cell_color_pair cells[12], int *score, int *has_resume)
{
    switch (highlight)
    {
        /* start */
        case 0:
            *has_resume = 1;
            game_loop(game_board, cells, 1, score, has_resume);
            break;
        
        /* resume */
        case 1:
            game_loop(game_board, cells, 0, score, has_resume);
            break;

        /* quit */
        case 2:
            quit_game();
            break;

        default:
            break;
    }
}

int is_move_available(int (*game_board)[4], int score)
{
    int new_score = score;
    if(is_game_board_full(game_board))
    {
        int game_board_cpy[4][4];
        for(int i = 0; i < 4; i++)
            for(int j = 0; j < 4; j++)
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

void move_up(int (*game_board)[4], int *score)
{
    for (int j = 0; j < 4; j++)
    {
        for (int i = 1; i < 4; i++)
        {
            if (game_board[i][j] != 0)
            {
                int k = i;
                while (k > 0 && game_board[k - 1][j] == 0)
                {
                    // Move the non-zero element up
                    game_board[k - 1][j] = game_board[k][j];
                    game_board[k][j] = 0;
                    k--;
                }
                if (k > 0 && game_board[k - 1][j] == game_board[k][j])
                {
                    // Merge identical elements
                    game_board[k - 1][j] *= 2;
                    *score = *score + game_board[k - 1][j];
                    game_board[k][j] = 0;
                }
            }
        }
    }
}

void move_down(int (*game_board)[4], int *score)
{
    for (int j = 0; j < 4; j++)
    {
        for (int i = 2; i >= 0; i--)
        {
            if (game_board[i][j] != 0)
            {
                int k = i;
                while (k < 3 && game_board[k + 1][j] == 0)
                {
                    // Move the non-zero element down
                    game_board[k + 1][j] = game_board[k][j];
                    game_board[k][j] = 0;
                    k++;
                }
                if (k < 3 && game_board[k + 1][j] == game_board[k][j])
                {
                    // Merge identical elements
                    game_board[k + 1][j] *= 2;
                    *score = *score + game_board[k + 1][j];
                    game_board[k][j] = 0;
                }
            }
        }
    }
}

void move_left(int (*game_board)[4], int *score)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 1; j < 4; j++)
        {
            if (game_board[i][j] != 0)
            {
                int k = j;
                while (k > 0 && game_board[i][k - 1] == 0)
                {
                    // Move the non-zero element to the left
                    game_board[i][k - 1] = game_board[i][k];
                    game_board[i][k] = 0;
                    k--;
                }
                if (k > 0 && game_board[i][k - 1] == game_board[i][k])
                {
                    // Merge identical elements
                    game_board[i][k - 1] *= 2;
                    *score = *score + game_board[i][k - 1];
                    game_board[i][k] = 0;
                }
            }
        }
    }
}

void move_right(int (*game_board)[4], int *score)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 2; j >= 0; j--)
        {
            if (game_board[i][j] != 0)
            {
                int k = j;
                while (k < 3 && game_board[i][k + 1] == 0)
                {
                    // Move the non-zero element to the right
                    game_board[i][k + 1] = game_board[i][k];
                    game_board[i][k] = 0;
                    k++;
                }
                if (k < 3 && game_board[i][k + 1] == game_board[i][k])
                {
                    // Merge identical elements
                    game_board[i][k + 1] *= 2;
                    *score = *score + game_board[i][k + 1];
                    game_board[i][k] = 0;
                }
            }
        }
    }
}

void generate_random(int (*game_board)[4])
{
    int i, j, value = 0;

    time_t t;
    srand((unsigned) time(&t));

    while(1 && !is_game_board_full(game_board))
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
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            if(game_board[i][j] == 0)
                return 0;
    
    return 1;
}

int make_best_move(int (*game_board)[4], int *score, int depth)
{
    if (!is_move_available(game_board, *score) || depth <= 0)
        return 0;

    int max_score = -1;
    int best_move = -1;

    // Try moving in each direction and evaluate the resulting score
    for (int move = 0; move < 4; move++)
    {
        int temp_board[4][4];
        int temp_score = *score;

        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
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

        // Evaluate the move and update the best move if needed
        int move_score = temp_score + make_best_move(temp_board, &temp_score, depth - 1);
        if (max_score < move_score)
        {
            max_score = move_score;
            best_move = move;
        }
    }

    // Apply the best move
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

    return max_score;
}


int max_num(int a, int b, int c, int d)
{
    int mx = a;
    if(mx < b) mx = b;
    if(mx < c) mx = c;
    if(mx < d) mx = d;
    return mx;
}