#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int decrypt(int key)
{
    char buf[] = "Q}|u`sfg~sf{}|a3";
    size_t len = strlen(buf);

    for (size_t i = 0; i < len; ++i)
        buf[i] ^= (char)key;

    if (strcmp(buf, "Congratulations!") == 0)
        return system("/bin/sh");
    else
        return puts("\nInvalid Password");
}

static int test(int a1, int a2)
{
    int diff = a2 - a1;

    switch (diff) {
        case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 9:
        case 16: case 17: case 18:
        case 19: case 20: case 21:
            return decrypt(diff);
        default:
            return decrypt(rand());
    }
}

int main(void)
{
    int input;

    srand(time(NULL));

    puts("***********************************");
    puts("*\t\tlevel03\t\t**");
    puts("***********************************");

    printf("Password:");
    if (scanf("%d", &input) != 1)
        return 1;

    test(input, 0x1337d00d);
    return 0;
}
