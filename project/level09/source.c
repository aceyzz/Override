#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure qui stocke les données utilisateur
typedef struct s_message {
    char text[140];        // Message (0x8c bytes)
    char username[40];     // Username (0x28 bytes)
    int len;               // Longueur max pour text (initialisé à 140)
} t_message;

// Fonction cachée jamais appelée - notre cible
void secret_backdoor() {
    char buffer[128];
    
    fgets(buffer, 128, stdin);
    system(buffer);
}

// Lit le username avec un bug off-by-one
void set_username(t_message *msg) {
    char buffer[128];
    int i;
    
    puts(">: Enter your username");
    printf(">>: ");
    fgets(buffer, 128, stdin);
    
    // BUG : i <= 0x28 (40) au lieu de i < 0x28
    // Permet 41 itérations (i=0 à i=40) au lieu de 40
    for (i = 0; i <= 0x28 && buffer[i]; i++) {
        msg->username[i] = buffer[i];
    }
    
    printf(">: Welcome, %s", msg->username);
}

// Lit le message en utilisant msg->len comme taille
void set_msg(t_message *msg) {
    char buffer[1024];
    
    puts(">: Msg @Unix-Dude");
    printf(">>: ");
    fgets(buffer, 1024, stdin);
    
    // Utilise msg->len qui peut avoir été modifié !
    strncpy(msg->text, buffer, msg->len);
}

// Gère la logique principale
void handle_msg() {
    t_message msg;
    
    // Initialise la structure
    memset(msg.username, 0, 40);
    msg.len = 140;  // 0x8c
    
    set_username(&msg);
    set_msg(&msg);
    
    puts(">: Msg sent!");
}

int main() {
    puts("--------------------------------------------");
    puts("|   ~Welcome to l33t-m$n ~    v1337        |");
    puts("--------------------------------------------");
    
    handle_msg();
    
    return 0;
}