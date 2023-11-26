#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CARD_LINES 5
#define CARD_COLS 5
#define MIDDLE 2
#define COLOR_CHARACTER '#'
#define BINGO_POINTS 10
#define BINGO "BINGO"
#define SWAP "SWAP"
#define SHIFT "SHIFT"
#define ASC "ASC"
#define DSC "DSC"
#define SHOW "SHOW"

struct cell
{
	int number, has_color;
	char color_character;
};

struct cell init_cell()
{
	struct cell a;
	a.number = 0;
	a.has_color = 0;
	a.color_character = COLOR_CHARACTER;
	return a;
}

struct player
{
	int card_index, score;
	struct cell **card;
};

//TODO: type safety
void read_card(struct cell **card);

//return address of colored cell, if found
struct cell* color_cell(struct cell **card, char col, int number);
//check for bingo
int check_columns(struct cell **card);
int check_lines(struct cell **card);
int check_diagonals(struct cell **card);
int has_bingo(struct cell **card); //returns total points / card

void make_operation(char *s, struct cell ***cards, int n_cards); //parse input for action, call function
void swap(struct cell *a, struct cell *b);
void swap_columns(struct cell **card, int x, int y);        //SWAP
void rotate_down(struct cell **card, int column, int pos);   //SHIFT 
void asc_sort(struct cell **card, int column);                    //ASC
void desc_sort(struct cell **card, int column);                   //CESC
void show_card(struct cell **card);               //SHOW
struct player check_for_winner(struct cell ***cards, int n_cards);

//allocate and deallocate a matrix
struct cell** create_matrix(int l, int c);
void free_matrix(struct cell **matrix, int l);

int main()
{
	int n, m;
	struct cell ***cards = NULL;
	char instruction_string[25];
	struct player winner;

	scanf("%d", &n); //number of cards
	cards = (struct cell  ***)malloc(n * sizeof(struct cell **));

	if(cards)
	{
		//read cards
		for(int i = 0; i < n; i++)
		{
			cards[i] = create_matrix(CARD_LINES, CARD_COLS);
			read_card(cards[i]);
			color_cell(cards[i], 'N', cards[i][2][2].number);
		}

		scanf("%d", &m); //number of instructions
		getchar(); //for \n character left by scanf

		for(int i = 0; i < m; i++)
		{
			fgets(instruction_string, 25, stdin);
			make_operation(instruction_string, cards, n);
			winner = check_for_winner(cards, n);
			if(winner.score)
			{
				printf("%d\n", winner.card_index);
				show_card(winner.card);
				printf("%d\n", winner.score);
				return 0;
			}
		}
		
		printf("NO WINNER\n");

		//free memory
		for(int i = 0; i < n; i++)
			free_matrix(cards[i], CARD_LINES);
	}
	
	free(cards);
    return 0;
}

void read_card(struct cell **card)
{
	for(int i = 0; i < CARD_LINES; i++)
		for(int j = 0; j < CARD_COLS; j++)
			scanf("%d", &card[i][j].number);
}

struct cell* color_cell(struct cell **card, char col, int number)
{
	int col_number = -1;

	//letter to column number
	switch(col)
	{
		case('B'): col_number = 0; break;
		case('I'): col_number = 1; break;
		case('N'): col_number = 2; break;
		case('G'): col_number = 3; break;
		case('O'): col_number = 4; break;
		default: break;
	}

	if(col_number != -1)
	{
		for(int i = 0; i < CARD_LINES; i++)
			if(number == card[i][col_number].number)
			{
				card[i][col_number].has_color = 1;
				return &card[i][col_number];
			}
	}
	return NULL;
}

int check_columns(struct cell **card)
{
	int cnt = 0, ok;
	for(int j = 0; j < CARD_COLS; j++)
	{
		ok = 1;
		for(int i = 0; i < CARD_LINES; i++)
			if(card[i][j].has_color == 0)
			{
				ok = 0;
				break;
			}
		if(ok) cnt++;
	}
	return cnt;
}

int check_lines(struct cell **card)
{
	int cnt = 0, ok;
	for(int i = 0; i < CARD_LINES; i++)
	{
		ok = 1;
		for(int j = 0; j < CARD_COLS; j++)
			if(card[i][j].has_color == 0)
			{
				ok = 0;
				break;
			}
		if(ok) cnt++;
	}
	return cnt;
}

int check_diagonals(struct cell **card)
{
	int cnt = 0, ok;
	
	ok = 1;
	for(int i = 0; i < CARD_LINES; i++)
		if(card[i][i].has_color == 0)
		{
			ok = 0;
			break;
		}
	
	if(ok) cnt++;

	ok = 1;
	for(int i = 0; i < CARD_LINES; i++)
		if(card[i][CARD_COLS - i - 1].has_color == 0)
		{
			ok = 0;
			break;
		}
	
	if(ok) cnt++;
	return cnt;
}

		
int has_bingo(struct cell **card)
{
	int nr = 0;

	nr += check_columns(card);
	nr += check_lines(card);
	nr += check_diagonals(card);

	return nr * BINGO_POINTS;
}

void make_operation(char *s, struct cell ***cards, int n_cards)
{
	char *cpy = (char *)malloc(sizeof(s));
	strcpy(cpy, s);

	char *type = strtok(cpy, "-");
	
	if(strlen(type) == 1 && strchr(BINGO, *type))
	{
		int number = atoi(strtok(NULL, "-"));
		for(int i = 0; i < n_cards; i++) 
			color_cell(cards[i], *type, number);  
	}

	else if(strcmp(type, SWAP) == 0)
	{
		int x = atoi(strtok(NULL, "-"));
		int y = atoi(strtok(NULL, "-"));
		for(int i = 0; i < n_cards; i++)
			swap_columns(cards[i], x, y);
	}

	else if(strcmp(type, ASC) == 0)
	{
		int column = atoi(strtok(NULL, "-"));
		for(int i = 0; i < n_cards; i++)
			asc_sort(cards[i], column);
	}

	else if(strcmp(type, DSC) == 0)
	{
		int column = atoi(strtok(NULL, "-"));
		for(int i = 0; i < n_cards; i++) 
			desc_sort(cards[i], column);
	}

	else if(strcmp(type, SHIFT) == 0)
	{
		int column = atoi(strtok(NULL, "-"));
		int pos = atoi(strtok(NULL, "-"));
		for(int i = 0; i < n_cards; i++)
			rotate_down(cards[i], column, pos);
	}

	else if(strcmp(type, SHOW) == 0)
	{
		int i = atoi(strtok(NULL, "-"));
		show_card(cards[i]);
	}

	free(cpy);
}

void swap(struct cell *a, struct cell *b)
{
	struct cell t = *a;
	*a = *b;
	*b = t;
}

void swap_columns(struct cell **card, int x, int y)
{
	for(int i = 0; i < CARD_LINES; i++)
		swap(&card[i][x], &card[i][y]);
}

void rotate_down(struct cell **card, int column, int pos)
{
	struct cell *temp = (struct cell *)malloc(CARD_LINES * sizeof(struct cell));
	for(int i = 0; i < CARD_LINES; i++)
		temp[(i + pos) % CARD_LINES] = card[i][column];
	
	for(int i = 0; i < CARD_LINES; i++)
		card[i][column] = temp[i];

	free(temp);
}

void asc_sort(struct cell **card, int column)
{
	int ok = 1;
	while(ok)
	{
		ok = 0;
		for(int i = 0; i < CARD_LINES - 1; i++)
			if(card[i][column].number > card[i + 1][column].number)
			{
				swap(&card[i][column], &card[i + 1][column]);
				ok = 1;
			}
	}
}

void desc_sort(struct cell **card, int column)
{
	int ok = 1;
	while(ok)
	{
		ok = 0;
		for(int i = 0; i < CARD_LINES - 1; i++)
			if(card[i][column].number < card[i + 1][column].number)
			{
				swap(&card[i][column], &card[i + 1][column]);
				ok = 1;
			}
	}
}

void show_card(struct cell **card)
{
	for(int i = 0; i < CARD_LINES; i++)
	{
		for(int j = 0; j < CARD_COLS; j++)
		{
			if(card[i][j].has_color == 1) printf("#  ");
			else printf("%2d ", card[i][j].number);
		}
		printf("\n");
	}
}

struct player check_for_winner(struct cell ***cards, int n_cards)
{
	struct player winner;
	int score;
	
	winner.card_index = -1;
	winner.score = 0;
	winner.card = NULL;

	for(int i = 0; i < n_cards; i++)
	{
		score = has_bingo(cards[i]);
		if(score > winner.score)
		{
			winner.card_index = i;
			winner.score = score;
			winner.card = cards[i];
		}
	}

	return winner;
}

struct cell** create_matrix(int l, int c)
{
	struct cell **matrix = (struct cell **) malloc(l * sizeof(struct cell *));
	if(matrix)
	{
		for(int i = 0; i < l; i++)
		{
			matrix[i] = (struct cell *) malloc(c * sizeof(struct cell));
			for(int j = 0; j < c; j++) matrix[i][j] = init_cell();
		}
	}

	return matrix;
}

void free_matrix(struct cell **matrix, int l)
{
	for(int i = 0; i < l; i++)
		free(matrix[i]);
	
	free(matrix);
}
