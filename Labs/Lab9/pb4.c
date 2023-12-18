#include <stdio.h>
#include <stdlib.h>

typedef struct vector
{
	int *v;
	int cap;
	int n;
} vector;

void init_vector(vector *a, int nr)
{
	a->v = (int *)malloc(nr * sizeof(int));
	a->cap = nr;
	a->n = 0;
}

//adauga element la vector
void adauga_vector(vector *a, int n)
{
	if(a->cap == a->n + 1)
	{
		a->cap *= 2;
		a->v = (int *)realloc(a->v, a->cap * sizeof(int));
	}
	
	a->v[a->n] = n;
	a->n = a->n + 1;
}

void scrie_vector(vector a)
{
	for(int i = 0; i < a.n; i++)
		printf("%d ", a.v[i]);

}

int main()
{
	vector v;
	init_vector(&v, 101);
	for(int i = 0; i <= 100; i++)
		adauga_vector(&v, i);

	scrie_vector(v);
	return 0;
}
