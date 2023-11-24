#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CARD_LINES 5
#define CARD_COLS 5
#define MIDDLE 2
#define COLOR 100
#define BINGO_POINTS 10
#define BINGO "BINGO"
#define SWAP "SWAP"
#define SHIFT "SHIFT"
#define ASC "ASC"
#define DSC "DSC"
#define SHOW "SHOW"

//TODO: type safety
void read_card(int **card);

//return address of colored cell, if found
int* color_cell(int **card, char col, int number);

//check for bingo
int check_columns(int **card);
int check_lines(int **card);
int check_diagonals(int **card);
int has_bingo(int **card); //returns total points / card

void make_operation(char *s, int ***cards, int n_cards); //parse input for action, call function
void swap(int *a, int *b);
void swap_columns(int **card, int x, int y);        //SWAP
void rotate_down(int **card, int column, int pos);   //SHIFT 
void asc_sort(int **card, int column);                    //ASC
void desc_sort(int **card, int column);                   //CESC
void show_card(int **card);               //SHOW

//allocate and deallocate a matrix
int** create_matrix(int l, int c);
void free_matrix(int **matrix, int l);

int main()
{
	int n, m, ***cards = NULL;
	char *instruction_string;

	scanf("%d", &n); //number of cards
	cards = (int ***)malloc(n * sizeof(int **));

	if(cards)
	{
		//read cards
		for(int i = 0; i < n; i++)
		{
			cards[i] = create_matrix(CARD_LINES, CARD_COLS);
			read_card(cards[i]);
			color_cell(cards[i], 'N', cards[i][2][2]);
		}

		scanf("%d", &m); //number of instructions
		getchar(); //for \n character left by scanf

		for(int i = 0; i < m; i++)
		{
			fgets(instruction_string, sizeof(instruction_string), stdin);
			make_operation(instruction_string, cards, n);
		}
		
		//free memory
		for(int i = 0; i < n; i++)
			free_matrix(cards[i], CARD_LINES);
	}
	
	free(cards);
    return 0;
}

void read_card(int **card)
{
	for(int i = 0; i < CARD_LINES; i++)
		for(int j = 0; j < CARD_COLS; j++)
			scanf("%d", &card[i][j]);
}

int* color_cell(int **card, char col, int number)
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
			if(number == card[i][col_number])
			{
				card[i][col_number] = COLOR;
				return &card[i][col_number];
			}
	}
	else return NULL;
}

int check_columns(int **card)
{
	int cnt = 0, ok;
	for(int j = 0; j < CARD_COLS; j++)
	{
		ok = 1;
		for(int i = 0; i < CARD_LINES; i++)
			if(card[i][j] != COLOR)
			{
				ok = 0;
				break;
			}
		if(ok) cnt++;
	}
	return cnt;
}

int check_lines(int **card)
{
	int cnt = 0, ok;
	for(int i = 0; i < CARD_LINES; i++)
	{
		ok = 1;
		for(int j = 0; j < CARD_COLS; j++)
			if(card[i][j] != COLOR)
			{
				ok = 0;
				break;
			}
		if(ok) cnt++;
	}
	return cnt;
}

int check_diagonals(int **card)
{
	int cnt = 0, ok;
	
	ok = 1;
	for(int i = 0; i < CARD_LINES; i++)
		if(card[i][i] != COLOR)
		{
			ok = 0;
			break;
		}
	
	if(ok) cnt++;

	ok = 1;
	for(int i = 0; i < CARD_LINES; i++)
		if(card[i][CARD_COLS - i - 1] != COLOR)
		{
			ok = 0;
			break;
		}
	
	if(ok) cnt++;
	return cnt;
}

		
int has_bingo(int **card)
{
	int nr = 0;

	nr += check_columns(card);
	nr += check_lines(card);
	nr += check_diagonals(card);

	return nr * BINGO_POINTS;
}

void make_operation(char *s, int ***cards, int n_cards)
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

	else if(strcmp(type, SHOW) == 0)
	{
		int i = atoi(strtok(NULL, "-"));
		show_card(cards[i]);
	}

	free(cpy);
}

void swap(int *a, int *b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

void swap_columns(int **card, int x, int y)
{
	for(int i = 0; i < CARD_LINES; i++)
		swap(&card[i][x], &card[i][y]);
}

void rotate_down(int **card, int column, int pos)
{
	int *temp = (int *)malloc(CARD_LINES * sizeof(int));
	for(int i = 0; i < CARD_LINES; i++)
		temp[(i + pos) % CARD_LINES] = card[i][column];

	
	for(int i = 0; i < CARD_LINES; i++)
		card[i][column] = temp[i];
}

void asc_sort(int **card, int column)
{
	int ok = 1;
	while(ok)
	{
		ok = 0;
		for(int i = 0; i < CARD_LINES - 1; i++)
			if(card[i][column] > card[i + 1][column])
			{
				swap(&card[i][column], &card[i + 1][column]);
				ok = 1;
			}
	}
}

void desc_sort(int **card, int column)
{
	int ok = 1;
	while(ok)
	{
		ok = 0;
		for(int i = 0; i < CARD_LINES - 1; i++)
			if(card[i][column] < card[i + 1][column])
			{
				swap(&card[i][column], &card[i + 1][column]);
				ok = 1;
			}
	}
}

void show_card(int **card)
{
	for(int i = 0; i < CARD_LINES; i++)
	{
		for(int j = 0; j < CARD_COLS; j++)
		{
			if(card[i][j] == COLOR) printf("#  ");
			else printf("%2d ", card[i][j]);
		}
		printf("\n");
	}
}

int** create_matrix(int l, int c)
{
	int **matrix = (int **) malloc(l * sizeof(int *));
	if(matrix)
	{
		for(int i = 0; i < l; i++)
			matrix[i] = (int *) malloc(c * sizeof(int));
	}

	return matrix;
}

void free_matrix(int **matrix, int l)
{
	for(int i = 0; i < l; i++)
		free(matrix[i]);
	
	free(matrix);
}
