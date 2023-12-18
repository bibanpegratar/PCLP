# include <stdio.h>
# include <stdlib.h>
# include <string.h>
 
int main ()
{
   char c = 'a', *p, s[100];
   p = &c;
   printf ("%c %c %c", *p, *p+1, *p+2);
   s[0] = 'A'; s[1] = 'B'; s[2] = 'C'; s[3] = '\0';
   p = s;
   printf ("\n%s %s %c %s", s, p, *(p+1), p+1);
   strcpy (s, "\nacesta este un exemplu de lucru cu pointeri");
   printf ("%s", s);
   p += 17;
   for (; *p != '\0'; ++p) {
      if (*p == 'e')
         *p = 'E';
      if (*p == ' ')
         *p = '\n';
   }
   printf ("%s\n", s);
 
   return 0;
}
