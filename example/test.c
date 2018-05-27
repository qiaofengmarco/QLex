#include <stdio.h>
#include <stdlib.h>
#include <string.h>
unsigned int max_capacity = 2;
char* temp;
int yyleng = 0;
void cat(char **a, const char b)
{
    if (max_capacity < yyleng + 2)
    {
        max_capacity += 1000;
        if (temp != NULL)
        {
            free(temp);
            temp = NULL;
        }
        temp = (char*) malloc(sizeof(char) * max_capacity);
        strcpy(temp, *a);
        free(*a);
        *a = temp;
    }
    (*a)[yyleng++] = b;
    (*a)[yyleng] = '\0';
}
int main()
{
    char* a = (char*) calloc(max_capacity, sizeof(char));
    a[0] = 'b';
    a[1] = '\0';
    yyleng = 1;
    cat(&a, 'a');
    cat(&a, 'c');
    cat(&a, 'd');
    printf("%s\n", a);
    getchar();
    return 0;
}