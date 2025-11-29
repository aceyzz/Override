Le niveau précédent nous a filé le mot de passe SSH pour l’utilisateur `level01` :

`uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL`

Ici, le but c’est de trouver et d’exploiter une faille dans un programme setuid pour choper le mot de passe de l’utilisateur suivant, `level02`.

## Connexion et premières observations

Après s’être connecté en SSH avec `level01`, on tombe sur un message d’accueil et un résumé des protections du binaire. Le fichier à regarder, c’est `/home/users/level01/level01`.

En lançant `file`, on voit que c’est un exécutable ELF 32 bits, pas stripé, avec les droits setuid et setgid. Ça veut dire que le programme tourne avec les droits de `level01`, même si on l’exécute depuis un shell limité.

Côté protections, il n’y a pas de canary, pas de PIE, et NX est désactivé. En gros, le binaire est vulnérable au buffer overflow et à l’injection de shellcode.

<br>

## Lancement du programme

Quand on lance le binaire, il affiche une invite de login admin et demande un nom d’utilisateur puis un mot de passe :

```bash
./level01
********* ADMIN LOGIN PROMPT *********
Enter Username:
```

<br>

## Analyse rapide du fonctionnement

En regardant le binaire avec un désassembleur (`objdump -d level01` par exemple), on repère trois trucs :

1. Une fonction `verify_user_name` qui lit le nom d’utilisateur dans un buffer global et le compare à une chaîne fixe.
2. Une fonction `verify_user_pass` qui compare le mot de passe à une autre chaîne fixe.
3. La fonction `main` qui gère tout : affichage, lecture du login, vérif, puis lecture et vérif du mot de passe.

Le nom d’utilisateur est lu dans un buffer global, puis vérifié. Si c’est bon, on passe au mot de passe, qui lui est lu dans un buffer local sur la pile. Le souci, c’est que le programme lit plus de caractères que la taille réelle du buffer. On a notre faille.

<br>

## Découverte des identifiants codés en dur

Avec `gdb`, on peut voir directement les chaînes utilisées pour la vérif dans `verify_user_name` et `verify_user_pass`. En affichant les adresses, on trouve :

Pour le nom d’utilisateur :

```gdb
(gdb) x/s 0x80486a8
0x80486a8: "dat_wil"
```

En mettant `dat_wil` comme login, le programme valide et demande le mot de passe.

Pour le mot de passe :

```gdb
(gdb) x/s 0x80486b0
0x80486b0: "admin"
```

Donc le combo est :

Nom d’utilisateur : `dat_wil`  
Mot de passe : `admin`

Mais même avec ça, la vérif du mot de passe échoue. En fait, même si on passe la vérif, le programme se termine sans donner le mot de passe de `level02`. C’est un piège, le vrai intérêt c’est le buffer overflow.

## Mise en évidence de la vulnérabilité

Dans le code de `main`, on voit que :

- Le nom d’utilisateur est lu dans un buffer global d’environ 100 octets, avec `fgets` qui peut lire jusqu’à 256 caractères. Pas trop grave ici.
- Pour le mot de passe, c’est un buffer local sur la pile de 64 octets, mais `fgets` peut lire jusqu’à 100 caractères dedans.

Donc :

- Buffer local de 64 octets.
- `fgets` qui lit jusqu’à 99 caractères + le `'\0'`.
- Pas de canary.
- Adresse de retour juste au-dessus du buffer sur la pile.

Si on lit 100 octets dans un buffer de 64, on déborde sur la pile et on peut écraser l’adresse de retour de `main`. Comme NX est désactivé, on peut injecter du shellcode et faire pointer l’exécution dessus. (NX c'est la protection qui empêche d’exécuter du code dans des zones de données comme la heap ou la stack.)

## Stratégie d’exploitation

Le plan :

On met le shellcode dans le buffer global `a_user_name` (avec le login) à une adresse connue, genre `0x0804a040`. On commence par `"dat_wil"`, puis un NOP sled, puis le shellcode qui lance `/bin/sh`. (NOP sled = une série d’instructions qui ne font rien, pour faciliter l’atterrissage du saut vers le shellcode.)

Le mot de passe sert à déclencher le buffer overflow. On met du padding (80 octets, trouvé avec un pattern cyclique dans un débogueur), puis l’adresse de retour qui pointe dans le NOP sled du buffer global `0x0804a050`.

Quand `main` retourne, ça saute dans le NOP sled, puis le shellcode, et on a un shell avec les droits setuid de `level01`.

## Construction du payload avec Python

Pour automatiser, on fait un petit script Python `exploit.py` qui crée deux fichiers :

- `/tmp/user01` avec le login et le shellcode.
- `/tmp/pass01` avec le débordement et l’adresse de retour.

Le shellcode est un classique 32 bits pour `execve("/bin/sh")`, sans octets nuls, après un NOP sled

Pour le « username » :

1. On écrit `"dat_wil"`.
2. On ajoute le NOP sled.
3. On ajoute le shellcode.
4. On finit par un retour à la ligne.

Pour le « password » :

1. On met 80 octets de padding.
2. On ajoute l’adresse de retour (en little endian) qui pointe dans le NOP sled.
3. On finit par un retour à la ligne.

Après ça, les deux fichiers sont prêts à être utilisés comme input pour le programme.

## Lancement de l’exploit

On combine les deux fichiers et on garde le shell ouvert :

```bash
(cat /tmp/user01; cat /tmp/pass01; cat) | ./level01
```

Le programme lit d’abord `/tmp/user01` comme login, valide `"dat_wil"` et stocke le shellcode. Ensuite, il lit `/tmp/pass01` comme mot de passe, déclenche l’overflow et remplace l’adresse de retour.

Quand `main` retourne, ça part dans le shellcode, qui ouvre un shell `/bin/sh` avec les droits de `level02` grâce au setuid.

On peut vérifier avec `whoami` et `id`, puis naviguer dans le système.

## Récupération du mot de passe de level02

Depuis le shell obtenu, c’est simple :

```bash
cd /home/users/level02
ls -la
cat .pass
```

Le mot de passe du niveau suivant est :
`PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv`
