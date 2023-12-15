#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//TODO: Add newline to
#define BUFFER_MAX_LEN 1000
#define TITLE_MAX_LEN 150
#define AUTHOR_MAX_LEN 150
#define DESCRIPTION_MAX_LEN 550
#define INSTRUCTION_MAX_LEN 550

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

void make_operation(char *s, book **library, int *n);
void search(book **library, int n, char *search_string, int search_year);
void replace_descriptions(book **library, int n, char *f, char *r, char* (*replace_fct)(char*, char*, char*));
char* replace(char *s, char *f, char *r);
void encode(book **library, int n, int ID);
book** add_book(book **library, int *n);

void sort_library(book **library, int n);
void swap_books(book *a, book *b);
int strcmp_lowercase(char *a, char *b);
int is_integer(char *s);
void rm_newline(char *s);
//char *int_to_string(int n);
char *reverse_s(char *s);

int main()
{
    int n_books, n_operations, i = 0;
    book **library;
    char buffer[BUFFER_MAX_LEN], instruction[INSTRUCTION_MAX_LEN];

    fgets(buffer, BUFFER_MAX_LEN, stdin);
    sscanf(buffer, "%d", &n_books);

    library = (book **)malloc(n_books * sizeof(book *));
    for(i = 0; i < n_books; i++)
    {
        library[i] = (book *)malloc(sizeof(book));
        read_book(library[i]);
    }

    fgets(buffer, BUFFER_MAX_LEN, stdin);
    sscanf(buffer, "%d", &n_operations);

    for(i = 0; i < n_operations; i++)
    {
        fgets(instruction, INSTRUCTION_MAX_LEN, stdin);
        make_operation(instruction, library, &n_books);
    }

    printf("%d\n", n_books);
    for(i = 0; i < n_books; i++)
        print_book(*library[i]);

    for(i = 0; i < n_books; i++)
        delete_book(library[i]);

    free(library);
    return 0;
}

//b should be already allocated
void init_book(book *b, int ID, int release_year, char *title, char *author, char *description)
{
    b->ID = ID;
    b->release_year = release_year;

    if(strlen(title) <= TITLE_MAX_LEN)
    {
        b->title = (char *)malloc((strlen(title) + 1) * sizeof(char));
        b->title = strdup(title);
    }

    if(strlen(author) <= AUTHOR_MAX_LEN)
    {
        b->author = (char *)malloc((strlen(author) + 1) * sizeof(char));
        b->author = strdup(author);
    }
    
    if(strlen(description) <= DESCRIPTION_MAX_LEN)
    {
        b->description = (char *)malloc((strlen(description) + 2) * sizeof(char));
        
        //add newline if it doesnt exist
        size_t strlen_desc = strlen(description);
        if(description[strlen_desc - 1] != '\n')
        {
            description[strlen_desc] = '\n';
            description[strlen_desc + 1] = '\0';
        }
            
        b->description = strdup(description);
    }
}

void read_book(book *b)
{
    int ID = -1, release_year = -1;
    char buffer[BUFFER_MAX_LEN], title[TITLE_MAX_LEN], author[AUTHOR_MAX_LEN], description[DESCRIPTION_MAX_LEN];

    fgets(buffer, BUFFER_MAX_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    sscanf(buffer, "%d", &ID);
    
    fgets(title, TITLE_MAX_LEN, stdin);
    title[strcspn(title, "\n")] = '\0';
    
    fgets(author, AUTHOR_MAX_LEN, stdin);
    author[strcspn(author, "\n")] = '\0';

    fgets(buffer, BUFFER_MAX_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    sscanf(buffer, "%d", &release_year);
    
    fgets(description, DESCRIPTION_MAX_LEN, stdin);
    description[strcspn(description, "\n")] = '\0';
    
    if(ID == -1 || release_year == -1)
    {
        return;
    }

    else init_book(b, ID, release_year, title, author, description);
}

book** add_book(book **library, int *n)
{
    *n = *n + 1;
    book **temp_library = (book **)realloc(library, (*n) * sizeof(book *));
    
    //if (temp_library == NULL);
        //fprintf(stderr, "ERRORSCREEN: temp_library reallocation failed");
    
    //else
    //{
        library = temp_library;
        library[*n - 1] = (book *)malloc(sizeof(book));
        
        //if (library[*n - 1] == NULL)
            //fprintf(stderr, "ERRORSCREEN: malloc for book failed");
        
        //else
            read_book(library[*n - 1]);
    //}
    
    return library;
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

void make_operation(char *s, book **library, int *n)
{
    //for the last operation
    rm_newline(s);
    
    char *cpy = (char *)malloc(sizeof(s));
    cpy = strdup(s);

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
    {
        add_book(library, n);
    }

    free(cpy);

}

void search(book **library, int n, char *search_string, int search_year)
{
    int cnt = 0, i = 0;
    
    //make a copy of library and sort it
    book **new_library = (book **)malloc(n * sizeof(book *));
    for(i = 0; i < n; i++)
        new_library[i] = (book *)malloc(sizeof(book));
    
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

    for(i = 0; i < n; i++)
        delete_book(new_library[i]);
    free(new_library);
    
    if(cnt == 0) printf("NOT FOUND\n\n");
}

//replace all occurences of f with r in all books' descriptions
void replace_descriptions(book **library, int n, char *f, char *r, char* (*replace_fct)(char*, char*, char*))
{
    int i = 0;
    for(i = 0; i < n; i++)
        library[i]->description = (*replace_fct)(library[i]->description, f, r);
}

char *replace(char *s, char *s1, char *s2) 
{
    int diff_size = (strlen(s2) - strlen(s1)) * sizeof(char);
    size_t init_size = (strlen(s) + 1) * sizeof(char);
    size_t new_s_size = 0;
    
    char *new_s = NULL, *cpy_s = NULL;
    char *t = NULL, *p = NULL;
    int cnt = 0;
    
    cpy_s = (char *)malloc(strlen(s) * sizeof(char));
    cpy_s = strdup(s);
    
    //count occurences
    t = strstr(cpy_s, s1);
    while(t)
    {
        cnt++;
        t = strstr(t + strlen(s1), s1);
    }
    
    if(cnt > 0)
    {
        diff_size *= cnt;
        new_s = (char *)malloc(init_size + diff_size + 1);
        p = strtok(cpy_s, " \n");
        while(p)
        {
            if(strcmp(p, s1) == 0) strcat(new_s, s2);
            else strcat(new_s, p);
            strcat(new_s, " ");
            p = strtok(NULL, " \n");
        }
        new_s_size = strlen(new_s);
        new_s[new_s_size - 1] = '\n';
        new_s[new_s_size] = '\0';
        return new_s;
    }
    
    return s;
}

void encode(book **library, int n, int ID)
{
    int i = 0, k = 0, cnt = 0;
    size_t curr_size = 0;
    char *new_desc = NULL, *new_desc_cpy = NULL;
    char *p = NULL, cnt_string[5], char_to_string[2];
    
    for(i = 0; i < n; i++)
    {
        if(library[i]->ID == ID)
        {
            p = (char *)malloc(strlen(library[i]->description) * sizeof(char));
            p = strdup(library[i]->description);
            
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
                        p += 1;
                    }
                }
                
                sprintf(cnt_string, "%X", cnt);
                
                //make character a string
                char_to_string[0] = *p;
                char_to_string[1] = '\0';
                
                //calculate size for new characters
                curr_size += (strlen(cnt_string) + strlen(char_to_string)) * sizeof(char);
                new_desc_cpy = (char *)realloc(new_desc, curr_size);
                
                if(new_desc_cpy) new_desc = new_desc_cpy;
                //else fprintf(stderr, "error reallocating in encode:new_desc");
                
                strcat(new_desc, char_to_string);
                if(cnt > 0) strcat(new_desc, cnt_string);
                p = p + 1;
            }
                
            //reached end of the string
            if(*p == '\0')
            {
                if(*(p-1) != '\n') strcat(new_desc, "\n");
                free(library[i]->description);
                new_desc = reverse_s(new_desc);
                library[i]->description = new_desc;
                return;
            }
        }
    }
    //if(i == n) fprintf(stderr, "encode: ID not found\n");
}

void sort_library(book **library, int n)
{
    int i = 0;
    for(i = 0; i < n; i++)
        for(int j = i + 1; j < n; j++)
            if(library[i]->ID > library[j]->ID)
                swap_books(library[i], library[j]);
}

void swap_books(book *a, book *b)
{
    book t = *a;
    *a = *b;
    *b = t;
}

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

void rm_newline(char *s)
{
    size_t length = strlen(s);
    if (length > 0 && s[length - 1] == '\n')
    {
        s[length - 1] = '\0';
    }
}

//free s after usage
//char *int_to_string(int n)
//{
//    int cpy = n, cnt = 0, i = 0;
//    char *s = NULL;
//    
//    while(n)
//    {
//        cnt++;
//        n /= 10;
//    }
//    
//    i = cnt - 1;
//    n = cpy;
//    s = (char *)malloc((cnt + 1) * sizeof(int));
//    
//    s[cnt - 1] = '\0';
//    while(n)
//    {
//        s[i] = (n % 10) + '0';
//        n /= 10;
//        i--;
//    }
//    return s;
//}

char *reverse_s(char *s)
{
    size_t i = 0;
    char *cpy = (char *)malloc(strlen(s) * sizeof(char));
    
    for(i = 0; i < strlen(s) - 1; i++)
        cpy[strlen(s) - i - 2] = s[i];
    
    //put newline from s to end of cpy
    if(s[strlen(s) - 1] == '\n')
        cpy[strlen(s) - 1] = '\n';
    
    //put last char of s to first char of cpy
    else cpy[0] = s[strlen(s) - 1];
    
    free(s);
    s = cpy;
    return cpy;
}

