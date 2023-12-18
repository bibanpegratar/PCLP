#include <stdio.h>

int shl(int a, int b)
{
	return a << b;
	
}

int shr(int a, int b)
{
	return a >> b;
}

void f(int *v, int (*shiftOp) (int, int), int n, int b)
{
	int i;
	for(i = 0; i < n; i++)
		v[i] = (*shiftOp)(v[i], b);
}

int main()
{
	int v[10] = {0, 1, 2, 3, 4, 5};
	f(v, shl, 6, 2);
	for(int i = 0; i < 10; i++)
		printf("%d ", v[i]);
	return 0;
}
