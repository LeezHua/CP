/* This program is an example to sum 1 to 9 */  
#include <stdio.h>
int main() {
    int sum = 0; // sum
    int i = 1;
l1:
    if (i < 10) {
        sum = i + 1;
        i = i + 1;
        goto l1;
    }
    else {
        printf("The sum is %d\n", sum);
    }

    return 0;
}