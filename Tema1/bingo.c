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

void read_card(int **card);

//return address of colored cell, if found
int* color_cell(int **card, char col, int number);

//check for bingo
int check_columns(int **card);
int check_lines(int **card);
int check_diagonals(int **card);
int has_bingo(int **card); //returns total points / card

void make_operation(char *s, int **card); //parse input for action, call function
void swap_columns(int *x, int *y);        //SWAP
void rotate_down(int *column, int pos);   //SHIFT 
void asc_sort(int *v);                    //ASC
void desc_sort(int *v);                   //CESC
void show_card(int **card);               //SHOW

//allocate and deallocate a matrix
int** create_matrix(int l, int c);
void free_matix(char **matrix); //TODO

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
		for(int i = 0; i < m; i++)
		{
			fgets(instruction_string, sizeof(instruction_string), stdin);
            for(int k = 0; k < n; k++)
				make_operation(instruction_string, cards[k]);
		}
		
		//show all cards, ONLY FOR TESTING
		for(int i = 0; i < n; i++)
		{
			show_card(cards[i]);
			printf("\n");
		}

	}

    return 0;
}

void read_card(int **card)
{
	for(int i = 0; i < CARD_LINES; i++)
		for(int j = 0; j < CARD_COLS; j++)
			scanf("%d", &card[i][j]);
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

//TODO : ONLY BALL OPERATION IMPLEMENTED
void make_operation(char *s, int **card)
{
	char *cpy = (char *)malloc(sizeof(s));
	strcpy(cpy, s);

	char *type = strtok(cpy, "-");
	
	if(strlen(type) == 1 && strchr(BINGO, *type))
	{
		int number = atoi(strtok(NULL, "-"));
		color_cell(card, *type, number);  
	}

	else if(strcmp(type, SWAP) == 0)
	{
		
	}

	else if(strcmp(type, "ASC") == 0)
	{
	}
	free(cpy);
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
