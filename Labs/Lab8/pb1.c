#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *my_replace(char *s, char *s1, char *s2)
{
	int cnt = 0;
	char  *i_s = s, *j_s, *i_temp;
	while(strstr(i_s, s1))
	{
		cnt++;
		i_s++;
	}

	i_s = s;

	char *temp = (char *)malloc((strlen(s) + cnt * strlen(s2)) * sizeof(char));
	i_temp = temp;
	strcpy(temp, "");
	j_s = strstr(s, s1);
	while(j_s)
	{
		while(i_s != j_s)
			*i_temp++ = *i_s++;

		for(int i = 0; i < strlen(s2); i++)
			*i_temp++ = s2[i];

		i_s += strlen(s1);
		j_s = strstr(j_s + 1, s1);
	}

	printf("%s", temp);
}

int main()
{
	char s1[100], s2[100], s[100];
	fgets(s1, sizeof(s1), stdin);
	fgets(s2, sizeof(s2), stdin);
	fgets(s, sizeof(s), stdin);
	
	my_replace(s, s1, s2);
	return 0;
}
