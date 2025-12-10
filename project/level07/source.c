#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void clear_stdin(void) {
    char c = 0;
    
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}

unsigned int get_unum(void) {
    unsigned int num = 0;
    
    fflush(stdout);
    scanf("%u", &num);
    clear_stdin();
    return num;
}

int store_number(int *buffer) {
    unsigned int number;
    unsigned int index;
    
    printf(" Number: ");
    number = get_unum();
    printf(" Index: ");
    index = get_unum();
    
    // Protection 1 : index divisible par 3
    // Protection 2 : byte le plus haut != 0xb7
    if ((index % 3 == 0) || ((number >> 24) == 0xb7)) {
        puts(" *** ERROR! ***");
        puts("   This index is reserved for wil!");
        puts(" *** ERROR! ***");
        return 1;
    }
    
    // Vulnérabilité : pas de vérification des limites !
    buffer[index] = number;
    return 0;
}

int read_number(int *buffer) {
    unsigned int index;
    
    printf(" Index: ");
    index = get_unum();
    printf(" Number at data[%u] is %u\n", index, buffer[index]);
    return 0;
}

int main(int argc, char **argv, char **envp) {
    int buffer[100];
    char command[20];
    int ret = 0;
    
    // Initialisation du buffer
    memset(buffer, 0, sizeof(buffer));
    
    // Effacement des arguments et variables d'environnement
    // (protection contre l'injection de shellcode via env)
    while (*argv) {
        memset(*argv, 0, strlen(*argv));
        argv++;
    }
    while (*envp) {
        memset(*envp, 0, strlen(*envp));
        envp++;
    }
    
    // Message de bienvenue
    puts("----------------------------------------------------");
    puts("  Welcome to wil's crappy number storage service!   ");
    puts("----------------------------------------------------");
    puts(" Commands:                                          ");
    puts("    store - store a number into the data storage    ");
    puts("    read  - read a number from the data storage     ");
    puts("    quit  - exit the program                        ");
    puts("----------------------------------------------------");
    puts("   wil has reserved some storage :>                 ");
    puts("----------------------------------------------------");
    
    // Boucle principale
    while (1) {
        printf("Input command: ");
        ret = 1;
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;  // Enlève le '\n'
        
        if (strncmp(command, "store", 5) == 0) {
            ret = store_number(buffer);
        }
        else if (strncmp(command, "read", 4) == 0) {
            ret = read_number(buffer);
        }
        else if (strncmp(command, "quit", 4) == 0) {
            return 0;
        }
        
        if (ret == 0) {
            printf(" Completed %s command successfully\n", command);
        } else {
            printf(" Failed to do %s command\n", command);
        }
        
        memset(command, 0, sizeof(command));
    }
    
    return 0;
}