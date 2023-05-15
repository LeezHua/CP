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
/*example*/
    b=1\
00
101:a=2*(1+3)
    IF(b>10) THEN
        a=1
    ELSE IF(b>=5) THEN
        a=2
    ELSE
        GOTO 101