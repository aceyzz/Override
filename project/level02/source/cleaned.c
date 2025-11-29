#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    FILE *file;
    char username[100];
    char password[100];
    char stored[41];
    int items_read;

    memset(username, 0, sizeof(username));
    memset(stored, 0, sizeof(stored));
    memset(password, 0, sizeof(password));

    file = fopen("/home/users/level03/.pass", "r");
    if (file == NULL)
    {
        fwrite("ERROR: failed to open password file\n", 1, 36, stderr);
        exit(1);
    }

    items_read = fread(stored, 1, 41, file);
    stored[strcspn(stored, "\n")] = '\0';

    if (items_read != 41)
    {
        fwrite("ERROR: failed to read password file\n", 1, 36, stderr);
        fwrite("ERROR: failed to read password file\n", 1, 36, stderr);
        exit(1);
    }

    fclose(file);

    puts("===== [ Secure Access System v1.0 ] =====");
    puts("/***************************************\\");
    puts("| You must login to access this system. |");
    puts("\\**************************************/");
    printf("--[ Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    printf("--[ Password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    puts("*****************************************");

    if (strncmp(stored, password, 41) == 0)
    {
        printf("Greetings, %s!\n", username);
        system("/bin/sh");
        return 0;
    }

    printf(username);
    puts(" does not have access!");
    exit(1);
}
