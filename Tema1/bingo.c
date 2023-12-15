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
#define INSTRUCTION_MAX_LEN 25

//an element in a bingo card
typedef struct cell
{
	int number, has_color;
	char color_character;
} cell;

cell init_cell()
{
	cell a;
	a.number = 0;
	a.has_color = 0;
	a.color_character = COLOR_CHARACTER;
	return a;
}

typedef struct player
{
	int card_index, score;
	cell **card;
} player;

//associate a letter to each column
typedef struct column_pair
{
	char name;
	int column_number;
} column_pair;

void read_card(cell **card);

cell* color_cell(cell **card, column_pair *columns, char col, int number);
int check_columns(cell **card);
int check_lines(cell **card);
int check_diagonals(cell **card);
int has_bingo(cell **card); 

void make_operation(char *s, cell ***cards, column_pair *columns, int n_cards);

void swap(cell *a, cell *b);
void swap_columns(cell **card, int x, int y);
void swap_columns_name(int x, int y, column_pair *columns);

void rotate_down(cell **card, int column, int pos);
void asc_sort(cell **card, int column);
void desc_sort(cell **card, int column);
void show_card(cell **card);

player check_for_winner(cell ***cards, int n_cards);

cell** create_matrix(int l, int c);
void free_matrix(cell **matrix, int l);

int main()
{
	int n, m;
	cell ***cards = NULL;
	char instruction[INSTRUCTION_MAX_LEN];
	player winner;

	//initialize name-column pairs 
	column_pair columns[5] = 
	{
		[0].name = 'B',
		[0].column_number = 0,

		[1].name = 'I',
		[1].column_number = 1,

		[2].name = 'N',
		[2].column_number = 2,

		[3].name = 'G',
		[3].column_number = 3,

		[4].name = 'O',
		[4].column_number = 4
	};

	scanf("%d", &n);
	cards = (cell  ***)malloc(n * sizeof(cell **));

	if(cards)
	{
		//read cards
		for(int i = 0; i < n; i++)
		{
			cards[i] = create_matrix(CARD_LINES, CARD_COLS);
			read_card(cards[i]);
			color_cell(cards[i], columns, 'N', cards[i][2][2].number);
		}

		scanf("%d", &m);
		getchar();

		for(int i = 0; i < m; i++)
		{
			fgets(instruction, INSTRUCTION_MAX_LEN, stdin);
			make_operation(instruction, cards, columns, n);
			winner = check_for_winner(cards, n);

			if(winner.score != 0)
			{
				printf("%d\n", winner.card_index);
				show_card(winner.card);
				printf("%d\n", winner.score);
				return 0;
			}
		}
		
		printf("NO WINNER\n");

		for(int i = 0; i < n; i++)
			free_matrix(cards[i], CARD_LINES);
	}
	
	free(cards);
    return 0;
}

void read_card(cell **card)
{
	for(int i = 0; i < CARD_LINES; i++)
		for(int j = 0; j < CARD_COLS; j++)
			scanf("%d", &card[i][j].number);
}

//color a cell, given by COLUMN_NAME-NUMBER pair
cell* color_cell(cell **card, column_pair *columns, char col, int number)
{
	int col_number = -1;

	//find corresponding column index
	switch(col)
	{
		case('B'): col_number = columns[0].column_number; break;
		case('I'): col_number = columns[1].column_number; break;
		case('N'): col_number = columns[2].column_number; break;
		case('G'): col_number = columns[3].column_number; break;
		case('O'): col_number = columns[4].column_number; break;
		default: break;
	}

	//color all cells that on the row that have given number
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

//scans for bingo on column
int check_columns(cell **card)
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

//scans for bingo on lines
int check_lines(cell **card)
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

//scans for bingo on diagonals
int check_diagonals(cell **card)
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

//checks for bingo, returns score
int has_bingo(cell **card)
{
	int nr = 0;

	nr += check_columns(card);
	nr += check_lines(card);
	nr += check_diagonals(card);

	return nr * BINGO_POINTS;
}

//parse operation string, call corresponding function
void make_operation(char *s, cell ***cards, column_pair *columns, int n_cards)
{
	char *cpy = (char *)malloc(sizeof(s));
	strcpy(cpy, s);

	char *type = strtok(cpy, "-");
	
	//check if it is a color-cell action type
	if(strlen(type) == 1 && strchr(BINGO, *type))
	{
		int number; sscanf(strtok(NULL, "-"), "%d", &number);
		for(int i = 0; i < n_cards; i++) 
			color_cell(cards[i], columns, *type, number);  
	}

	//make special operation
	else if(strcmp(type, SWAP) == 0)
	{
		int x; sscanf(strtok(NULL, "-"), "%d", &x);
		int y; sscanf(strtok(NULL, "-"), "%d", &y);
		for(int i = 0; i < n_cards; i++)
			swap_columns(cards[i], x, y);

		//needed only once for all cards
		swap_columns_name(x, y, columns);
	}

	else if(strcmp(type, ASC) == 0)
	{
		int column; sscanf(strtok(NULL, "-"), "%d", &column);
		for(int i = 0; i < n_cards; i++)
			asc_sort(cards[i], column);
	}

	else if(strcmp(type, DSC) == 0)
	{
		int column; sscanf(strtok(NULL, "-"), "%d", &column);
		for(int i = 0; i < n_cards; i++) 
			desc_sort(cards[i], column);
	}

	else if(strcmp(type, SHIFT) == 0)
	{
		int column; sscanf(strtok(NULL, "-"), "%d", &column);
		int pos; sscanf(strtok(NULL, "-"), "%d", &pos);
		for(int i = 0; i < n_cards; i++)
			rotate_down(cards[i], column, pos);
	}

	else if(strcmp(type, SHOW) == 0)
	{
		int i; sscanf(strtok(NULL, "-"), "%d", &i);
		show_card(cards[i]);
	}

	free(cpy);
}

void swap(cell *a, cell *b)
{
	cell t = *a;
	*a = *b;
	*b = t;
}

//swaps cells of 2 columns
void swap_columns(cell **card, int x, int y)
{
	if(x != y)
		for(int i = 0; i < CARD_LINES; i++)
			swap(&card[i][x], &card[i][y]);
}

//swaps each column's name index in columns vector
void swap_columns_name(int x, int y, column_pair *columns)
{
	int i_x = 0, i_y = 0;

	//search column with index x
	for(int i = 0; i < 5; i++)
		if(x == columns[i].column_number)
		{
			i_x = i;
			break;
		}

	//search column with index y
	for(int i = 0; i < 5; i++)
		if(y == columns[i].column_number)
		{
			i_y = i;
			break;
		}

	//swap indexes
	columns[i_x].column_number = y;
	columns[i_y].column_number = x;
}

//SHIFT cells down by "pos" positions
void rotate_down(cell **card, int column, int pos)
{
	cell *temp = (cell *)malloc(CARD_LINES * sizeof(cell));
	for(int i = 0; i < CARD_LINES; i++)
		temp[(i + pos) % CARD_LINES] = card[i][column];
	
	for(int i = 0; i < CARD_LINES; i++)
		card[i][column] = temp[i];

	free(temp);
}

//ASC
void asc_sort(cell **card, int column)
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

//DSC
void desc_sort(cell **card, int column)
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

//SHOW
void show_card(cell **card)
{
	for(int i = 0; i < CARD_LINES; i++)
	{
		for(int j = 0; j < CARD_COLS; j++)
		{
			if(card[i][j].has_color == 1) printf("# ");
			else printf("%d ", card[i][j].number);
		}
		printf("\n");
	}
}

//verifies if a player has a maximum score, returns first occurence
player check_for_winner(cell ***cards, int n_cards)
{
	player winner;
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

cell** create_matrix(int l, int c)
{
	cell **matrix = (cell **) malloc(l * sizeof(cell *));
	if(matrix)
	{
		for(int i = 0; i < l; i++)
		{
			matrix[i] = (cell *) malloc(c * sizeof(cell));
			for(int j = 0; j < c; j++) matrix[i][j] = init_cell();
		}
	}

	return matrix;
}

void free_matrix(cell **matrix, int l)
{
	for(int i = 0; i < l; i++)
		free(matrix[i]);
	
	free(matrix);
}
