#include <stdio.h>

int main(void)
{
    int i = 0;
    while (1) {                 /* infinite loop so we can hit the line many times */
        printf("enter the text\n");
        ++i;
    }
    return 0;
}
