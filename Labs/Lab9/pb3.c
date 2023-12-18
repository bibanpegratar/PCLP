#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char* int_to_char(int a)
{
	int size = snprintf(NULL, 0, "%d", a);
	char *str = malloc(size + 1);
	snprintf(str, size + 1, "%d", a);
	return str;
}

char* timestr(struct tm t, char *time)
{
	time = (char *)malloc(9 * sizeof(char));
	strcpy(time, "");

	strcat(time, int_to_char(t.tm_hour));
	strcat(time, ":");
	strcat(time, int_to_char(t.tm_min));
	strcat(time, ":");
	strcat(time, int_to_char(t.tm_sec));
}

int main()
{
	struct tm tt = localtime(time(NULL));
	char *t = timestr(tt, t);
	printf("%s", t);
	return 0;
}
