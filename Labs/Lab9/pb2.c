#include <stdio.h>
#include <time.h>

int main()
{
	time_t t_time = time(NULL);
	char *human_time = ctime(&t_time);

	printf("%s", human_time);
	return 0;
}
