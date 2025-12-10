#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>

#define PTRACE_TRACEME 0

int auth(char *login, unsigned int serial)
{
    size_t login_len;
    unsigned int hash;
    int i;
    
    // Retire le '\n' à la fin du login (si présent)
    login[strcspn(login, "\n")] = '\0';
    
    // Obtient la longueur du login (max 32 caractères)
    login_len = strnlen(login, 32);
    
    // Vérifie que le login fait plus de 5 caractères
    if (login_len < 6) {
        return 1;
    }
    
    // Anti-debugging : détecte si le programme est tracé par GDB
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1) {
        puts("\x1b[32m.---------------------------.");
        puts("\x1b[31m| !! TAMPERING DETECTED !!  |");
        puts("\x1b[32m'---------------------------.");
        return 1;
    }
    
    // Initialise le hash avec le 4ème caractère du login (index 3)
    // Formule : (login[3] XOR 0x1337) + 0x5eeded
    hash = ((unsigned int)login[3] ^ 0x1337) + 0x5eeded;
    
    // Boucle sur chaque caractère du login pour calculer le hash final
    for (i = 0; i < login_len; i++) {
        // Vérifie que le caractère est imprimable (> 31 en ASCII)
        if (login[i] < ' ') {  // ' ' = 32 en ASCII
            return 1;
        }
        
        // Algorithme de hachage :
        // hash = hash + ((login[i] XOR hash) modulo 0x539)
        hash = hash + (((unsigned int)login[i] ^ hash) % 0x539);
    }
    
    // Compare le hash calculé avec le serial fourni
    if (serial == hash) {
        return 0;  // Authentification réussie
    }
    
    return 1;  // Échec
}

int main(int argc, char **argv)
{
    char login[32];           // Buffer pour le login (esp + 0x2c)
    unsigned int serial;      // Notre serial input (esp + 0x28)
    int auth_result;          // Résultat de auth()
    
    puts("***********************************");
    puts("*\t\tlevel06\t\t  *");
    puts("***********************************");
    
    printf("-> Enter Login: ");
    fgets(login, 32, stdin);
    
    puts("***********************************");
    puts("***** NEW ACCOUNT DETECTED ********");
    puts("***********************************");
    
    printf("-> Enter Serial: ");
    __isoc99_scanf("%u", &serial);
    
    // Appelle auth() avec le login et le serial
    auth_result = auth(login, serial);
    
    if (auth_result == 0) {
        puts("Authenticated!");
        system("/bin/sh");
        return 0;
    }
    
    return 1;
}