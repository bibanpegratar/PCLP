#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//a buffer will be used to store input for each instruction
#define BUFFER_MAX_LEN 1000
#define TITLE_MAX_LEN 50
#define AUTHOR_MAX_LEN 30
#define DESCRIPTION_MAX_LEN 510
#define INSTRUCTION_MAX_LEN 600

typedef struct book
{
    int ID, release_year;
    char *title;
    char *author;
    char *description;
} book;

void init_book(book *b, int ID, int release_year, char *title, char *author, char *description);
void read_book(book *b);
void print_book(book b);
void delete_book(book *b);

book** make_operation(char *s, book **library, int *n);
void search(book **library, int n, char *search_string, int search_year);
void replace_descriptions(book **library, int n, char *f, char *r, char *(replace_fct)(char*, char*, char*));
char* replace(char *s, char *f, char *r);
void encode(book **library, int n, int ID);
book** add_book(book **library, int *n);

void sort_library(book **library, int n);
void swap_books(book *a, book *b);
int strcmp_lowercase(char *a, char *b);
int is_integer(char *s);
void rm_newline(char *s);
char *reverse_s(char *s);

int main(void)
{
    char buffer[BUFFER_MAX_LEN], instruction[INSTRUCTION_MAX_LEN];
    int n_books, n_operations, i = 0;
    book **library;

    fgets(buffer, BUFFER_MAX_LEN, stdin);
    sscanf(buffer, "%d", &n_books);

    //allocate memory for library and each book
    library = (book **)malloc(n_books * sizeof(book *));
    if(library == NULL) fprintf(stderr, "ERRORSCREEN: library allocation failed-line:52\n");
    for(i = 0; i < n_books; i++)
    {
        library[i] = (book *)malloc(sizeof(book));
        if(library == NULL) fprintf(stderr, "ERRORSCREEN: book[%d] of library allocation failed-line:56\n", i);
        read_book(library[i]);
    }

    fgets(buffer, BUFFER_MAX_LEN, stdin);
    sscanf(buffer, "%d", &n_operations);

    for(i = 0; i < n_operations; i++)
    {
        fgets(instruction, INSTRUCTION_MAX_LEN, stdin);
        library = make_operation(instruction, library, &n_books);
    }

    printf("%d\n", n_books);
    for(i = 0; i < n_books; i++)
        print_book(*library[i]);

    //free memory
    for(i = 0; i < n_books; i++)
        delete_book(library[i]);
    free(library);

    return 0;
}

//b should be already allocated
//dynamic allocation used for title, author, description
void init_book(book *b, int ID, int release_year, char *title, char *author, char *description)
{
    b->ID = ID;
    b->release_year = release_year;

    b->title = strdup(title);
    b->author = strdup(author);
    b->description = strdup(description);
}

void read_book(book *b)
{
    int ID = -1, release_year = -1;
    char buffer[BUFFER_MAX_LEN], title[TITLE_MAX_LEN], author[AUTHOR_MAX_LEN], description[DESCRIPTION_MAX_LEN];

    //ID
    fgets(buffer, BUFFER_MAX_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    sscanf(buffer, "%d", &ID);

    //title
    fgets(title, TITLE_MAX_LEN, stdin);
    title[strcspn(title, "\n")] = '\0';

    //author
    fgets(author, AUTHOR_MAX_LEN, stdin);
    author[strcspn(author, "\n")] = '\0';

    //release_year
    fgets(buffer, BUFFER_MAX_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    sscanf(buffer, "%d", &release_year);

    //description
    fgets(description, DESCRIPTION_MAX_LEN, stdin);

    if(ID == -1 || release_year == -1) return;
    else init_book(b, ID, release_year, title, author, description);
}

book** add_book(book **library, int *n)
{
    book **temp_library = (book **)realloc(library, (*n + 1) * sizeof(book *));
    
    if (temp_library == NULL)
    {
        fprintf(stderr, "ERRORSCREEN: temp_library reallocation failed-line:125");
        return library;
    }

    else
    {
        library = temp_library;
        library[*n] = (book *)malloc(sizeof(book));

        if (library[*n] == NULL)
            fprintf(stderr, "ERRORSCREEN: book[%d] of temp_library reallocation failed-line:138", *n);
        else
            read_book(library[*n]);

        *n = *n + 1;
        return library;
    }
}

void print_book(book b)
{
    printf("%d\n", b.ID);
    printf("%s\n", b.title);
    printf("%s\n", b.author);
    printf("%d\n", b.release_year);
    printf("%s", b.description);
}

void delete_book(book *b)
{
    free(b->title);
    free(b->author);
    free(b->description);
    free(b);
}

//parses the given 's' instruction
book** make_operation(char *s, book **library, int *n)
{
    char *cpy = strdup(s);
    rm_newline(cpy);

    char *type = strtok(cpy, " ");
    if(strcmp(type, "SEARCH") == 0)
    {
        char *param = strtok(NULL, "\n");
        rm_newline(param);

        if(is_integer(param) == 1)
            search(library, *n, NULL, atoi(param));
        else
            search(library, *n, param, -1);
    }

    else if(strcmp(type, "REPLACE") == 0)
    {
        char *param1 = strtok(NULL, " ");
        char *param2 = strtok(NULL, " \n");
        replace_descriptions(library, *n, param1, param2 , replace);
    }

    else if(strcmp(type, "ENCODE") == 0)
    {
        int param = atoi(strtok(NULL, "\n"));
        encode(library, *n, param);
    }

    else if(strcmp(type, "ADD_BOOK") == 0)
        library = add_book(library, n);

    free(cpy);
    return library;

}

//search by author -> search_year = -1
//searcb by year   -> search_string = NULL
void search(book **library, int n, char *search_string, int search_year)
{
    int cnt = 0, i = 0;

    //make a copy of library
    book **new_library = (book **)malloc(n * sizeof(book *));
    if(new_library == NULL) fprintf(stderr, "ERRORSCREEN: new_library reallocation failed-line:212");
    for(i = 0; i < n; i++)
    {
        new_library[i] = (book *)malloc(sizeof(book));
        if (new_library[i] == NULL) fprintf(stderr, "ERRORSCREEN: new_library[%d] reallocation failed-line:216", i);
    }

    //copy all elements
    for(i = 0; i < n; i++)
        init_book(new_library[i], library[i]->ID, library[i]->release_year, library[i]->title, library[i]->author, library[i]->description);

    sort_library(new_library, n);

    //search by string
    if(search_string != NULL)
    {
        for(i = 0; i < n; i++)
            if(strcmp_lowercase(search_string, new_library[i]->title) ==  1 || strcmp_lowercase(search_string, new_library[i]->author) == 1)
            {
                print_book(*new_library[i]);
                printf("\n");
                cnt++;
            }
    }

    //search by year
    else if(search_year != -1)
    {
        for(i = 0; i < n; i++)
            if(search_year == new_library[i]->release_year)
            {
                print_book(*new_library[i]);
                printf("\n");
                cnt++;
            }
    }

    //free memory
    for(i = 0; i < n; i++)
        delete_book(new_library[i]);
    free(new_library);

    if(cnt == 0) printf("NOT FOUND\n\n");
}

//replace all occurences of f with r in all books' descriptions
void replace_descriptions(book **library, int n, char *f, char *r, char *(replace_fct)(char *, char*, char*))
{
    int i = 0;
    for(i = 0; i < n; i++)
        library[i]->description = (*replace_fct)(library[i]->description, f, r);
}

//replaces all occurences of s1 in s with s2
//retuns a dynamically allocated string, frees the old string
char *replace(char *s, char *s1, char *s2)
{
    unsigned long diff_size = (strlen(s2) - strlen(s1)) * sizeof(char);
    size_t init_size = (strlen(s) + 1) * sizeof(char);
    size_t new_s_size = 0;

    char *new_s = NULL;
    
    //used for freeing memory held by s
    char *init_s = s;
    char *t = NULL, *p = NULL;
    int cnt = 0;

    //count occurences of s1 in cpy_s
    t = strstr(s, s1);
    while(t)
    {
        cnt++;
        t = strstr(t + strlen(s1), s1);
    }

    if(cnt > 0)
    {
        //diiference in size between old and new string
        diff_size *= cnt;
        new_s = (char *)malloc((init_size + diff_size + 2) * sizeof(char));
        if(new_s == NULL) fprintf(stderr, "ERRORSCREEN: new_s reallocation failed-line:290");
        new_s[0] = '\0';

        p = strtok(s, " \n");
        while(p)
        {
            if(strcmp(p, s1) == 0) strcat(new_s, s2);
            else strcat(new_s, p);

            //words are separated by space
            strcat(new_s, " ");
            p = strtok(NULL, " \n");
        }

        //reached end of word, add newline and terminator
        new_s_size = strlen(new_s);
        new_s[new_s_size - 1] = '\n';
        new_s[new_s_size] = '\0';

        //free old string
        free(init_s);
        return new_s;
    }

    return s;
}

//modify each book's description:
//  - replace the string of consecutive occurences with: size of sequence + character
//  - size of sequence is converted in hex (capital letters)
//  - reverse the string
void encode(book **library, int n, int ID)
{
    int i = 0, cnt = 0;
    size_t curr_size = 0;
    char *new_desc = NULL, *new_desc_cpy = NULL;
    char *p = NULL, cnt_string[5] = "", char_to_string[2] = "", *init_p;

    new_desc = (char *)malloc(1 * sizeof(char));
    if(new_desc == NULL) fprintf(stderr, "ERRORSCREEN: new_desc reallocation failed-line:327");
    new_desc[0] = '\0';

    for(i = 0; i < n; i++)
    {
        //search by ID
        if(library[i]->ID == ID)
        {
            //copy descritpion to p
            p = strdup(library[i]->description);
            init_p = p;
            while(*p != '\0')
            {
                cnt = 0;
                if(isalpha(*p))
                {
                    //count consecutive characters
                    cnt = 1;
                    while(*p == *(p + 1))
                    {
                        cnt++;
                        p = p + 1;
                    }
                }

                //convert size of sequence to hex
                sprintf(cnt_string, "%X", cnt);

                //convert character to string
                char_to_string[0] = *p;
                char_to_string[1] = '\0';

                //calculate extra size for new characters
                curr_size += (strlen(cnt_string) + strlen(char_to_string) + 1) * sizeof(char);
                new_desc_cpy = (char *)realloc(new_desc, curr_size);
                if(new_desc_cpy == NULL) fprintf(stderr, "ERRORSCREEN: new_desc_cpy reallocation failed-line:363");

                if(new_desc_cpy != NULL)
                {
                    new_desc = new_desc_cpy;
                    strcat(new_desc, char_to_string);
                    if(cnt > 0) strcat(new_desc, cnt_string);
                }
                 p = p + 1;
            }

            //reached end of the string
            if(*p == '\0')
            {
                rm_newline(new_desc);
                new_desc = reverse_s(new_desc);
                free(library[i]->description);
                free(init_p);
                library[i]->description = strdup(new_desc);
            }
        }
    }
    free(new_desc);
}

//sorts library by ID
void sort_library(book **library, int n)
{
    int i = 0, j = 0;
    for(i = 0; i < n; i++)
        for(j = i + 1; j < n; j++)
            if(library[i]->ID > library[j]->ID)
                swap_books(library[i], library[j]);
}

void swap_books(book *a, book *b)
{
    book t = *a;
    *a = *b;
    *b = t;
}

//case-insensitive strcmp
int strcmp_lowercase(char *a, char *b)
{
    unsigned int i = 0;
    if(strlen(a) == strlen(b))
    {
        for(i = 0; i < strlen(a); i++)
            if(tolower(a[i]) != tolower(b[i]))
                return 0;
        return 1;
    }
    return 0;
}

int is_integer(char *s)
{
    unsigned int i = 0;
    while(i <= strlen(s))
    {
        if(s[i] == ' ');
        else if(s[i] >= '0' && s[i] <= '9') return 1;
        else return 0;
        i++;
    }
    return 0;
}

//removes newline from end of string
void rm_newline(char *s)
{
    size_t length = strlen(s);
    if (length > 0 && s[length - 1] == '\n')
    {
        s[length - 1] = '\0';
    }
}

//s should by dynamically allocated
//returns dynamically allocated string
char *reverse_s(char *s)
{
    size_t i = 0;
    char *cpy = (char *)malloc((strlen(s) + 2) * sizeof(char));
    if(cpy == NULL) fprintf(stderr, "ERRORSCREEN: cpy reallocation failed-line:446");

    cpy[strlen(s) + 1] = '\0';

    for(i = 0; i < strlen(s); i++)
        cpy[strlen(s) - i - 1] = s[i];

    //put newline from s to end of cpy
    if(s[strlen(s) - 1] != '\n')
        cpy[strlen(s)] = '\n';

    //free old memory
    free(s);
    return cpy;
}