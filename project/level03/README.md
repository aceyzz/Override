Le niveau précédent nous a donné le mot de passe SSH pour `level03`, donc on peut enfin se connecter et voir ce qui nous attend.  
L’objectif ici est de comprendre ce que le binaire attend comme “password”, et surtout pourquoi il n’acceptera jamais une chaîne classique. Ce niveau a l’air mystérieux au début, mais en réalité tout repose sur un XOR très simple et une lecture attentive du code.

## Premières observations

On trouve le binaire `/home/users/level03/level03`.  
Un rapide coup d’œil aux protections :
```
Partial RELRO   Canary found   NX enabled   No PIE
```

Donc le stack est protégé par un canary, NX est activé (pas d’exécution sur la pile), et l’ASLR est désactivé car pas de PIE.  
En gros : pas de buffer overflow classique, pas de shellcode sur la pile.  

La seule direction raisonnable est d’analyser ce que le programme attend réellement.

## Lancement du programme

Quand on l’exécute :

```bash
./level03
***********************************
*               level03         **
***********************************
Password:
```

On tape n’importe quoi → “Invalid Password”.

À ce stade, aucune indication sur ce qu’il attend vraiment.  
Donc direction désassembleur (eh merce [DogBolt](https://dogbolt.org)).

<br>

## Analyse rapide du fonctionnement

Le `main()` est court. Il affiche la bannière, demande un entier via `scanf("%d")`, puis appelle :

```c
test(user_input, 322424845);
```

La fonction `test()` fait simplement un calcul :

- elle prend `322424845 - user_input`
- si ce résultat est compris dans la liste {1..9, 16..21}, elle appelle `decrypt()` avec cette valeur
- sinon elle appelle `decrypt(rand())`

Ça limite énormément l’espace de recherche :  
on ne peut passer que **21 valeurs possibles** comme XOR vraiment utilisées.

Le vrai cœur du niveau est la fonction `decrypt()`.

<br>

## Analyse de decrypt()

La fonction commence par copier une chaîne codée :
```c
strcpy(buffer, "Q}|u`sfg~sf{}|a3");
```

Ensuite elle applique un XOR entre chaque caractère et la valeur passée en paramètre :
```c
buffer[i] ^= key;
```

Puis elle compare le résultat avec :
```
"Congratulations!"
```

Si ça matche, elle exécute :
```c
system("/bin/sh");
```

Donc le but est clair :  
**trouver le XOR `key` qui transforme `"Q}|u`sfg~sf{}|a3"` en `"Congratulations!"`.**

<br>

## Trouver la valeur du XOR

On prend le premier caractère :
```
'Q' ^ key = 'C'
```

On inverse :
```
key = 'Q' ^ 'C'
key = 18
```

On vérifie sur d’autres caractères : c’est bien cohérent.
Ensuite, cette valeur provient de l’expression :
```
322424845 - user_input = key
```

Donc :
```
user_input = 322424845 - 18
user_input = 322424827
```

C’est littéralement *la* valeur attendue par le programme.

<br>

## Tentative avec la valeur trouvée

```bash
./level03
Password: 322424827
```

Le programme déchiffre correctement la chaîne, obtient `"Congratulations!"` et lance un `/bin/sh` setuid.  
On est donc dans un shell effectif `level04`.

## Récupération du mot de passe de level04

Ensuite c’est trivial :
```bash
cd /home/users/level04
cat .pass
# resultat
kgv3tkEb9h2mLkRsPkXRfc2mHbjMxQzvb2FrgKkf
```

<br>

Ce niveau était un leurre : aucune exploitation mémoire, aucun exploit ROP, juste un XOR simple dissimulé dans une boucle.  
Le jeu consistait à trouver la bonne valeur parmi les possibilités limitées imposées par `test()`, puis à la propager au bon endroit.
