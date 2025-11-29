#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char username[256];

int verify_user_name(void)
{
    puts("verifying username....\n");
    return strncmp("dat_wil", username, 7);
}

int verify_user_pass(const char *password)
{
    return strncmp("admin", password, 5);
}

int main(int argc, char **argv)
{
    char password[64];

    memset(password, 0, sizeof(password));

    puts("********* ADMIN LOGIN PROMPT *********");
    printf("Enter Username: ");
    fgets(username, sizeof(username), stdin);

    if (verify_user_name() != 0)
    {
        puts("nope, incorrect username...\n");
        return 1;
    }

    puts("Enter Password: ");
    fgets(password, 100, stdin);
    verify_user_pass(password);
    puts("nope, incorrect password...\n");

    return 1;
}