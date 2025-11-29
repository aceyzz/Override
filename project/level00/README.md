Lors de la première connexion, nous voyons le message de bienvenue suivant :
```
	   ____                  ____  _     __
	  / __ \_   _____  _____/ __ \(_)___/ /__
	 / / / / | / / _ \/ ___/ /_/ / / __  / _ \
	/ /_/ /| |/ /  __/ /  / _, _/ / /_/ /  __/
	\____/ |___/\___/_/  /_/ |_/_/\__,_/\___/

                       Good luck & Have fun

   To start, ssh with level00/level00 on 10.0.2.15:4242
level00@100.97.200.81's password:

RELRO           STACK CANARY      NX            PIE             RPATH      RUNPATH      FILE
Partial RELRO   No canary found   NX enabled    No PIE          No RPATH   No RUNPATH   /home/users/level00/level00
```
> Le mot de passe est "level00", donné par le sujet.

Nous remarquons qu'un binaire nommé "level00" est présent dans le répertoire personnel de l'utilisateur. Nous pouvons vérifier ses permissions avec la commande `ls -l` :
```bash
ls -la level00
# resultat
-rwsr-s---+ 1 level01 users 7280 Sep 10  2016 level00
```

Tiens tiens, le binaire a le bit SUID activé (indiqué par le "s" dans les permissions). Cela signifie que lorsqu'il est exécuté, il s'exécute avec les privilèges de son propriétaire, qui est "level01". Nous allons donc exécuter ce binaire pour voir ce qu'il fait :
```bash
./level00
# resultat 
***********************************
* 	     -Level00 -		  *
***********************************
Password:
```

Le programme nous demande un mot de passe. En essayant plusieurs mots de passes (level00, admin, password, etc.), rien n'y fait. Nous devons analyser le binaire pour comprendre quel mot de passe il attend. Desassembler le binaire avec `objdump` ou utiliser un débogueur comme `gdb` peut nous aider à comprendre son fonctionnement.
```bash
objdump -d level00
# resultat (extrait)
level00:     file format elf32-i386
# [...]
 80484de:	e8 ed fe ff ff       	call   80483d0 <__isoc99_scanf@plt> # appel de scanf
 80484e3:	8b 44 24 1c          	mov    0x1c(%esp),%eax				# récupération de la valeur entrée
 80484e7:	3d 9c 14 00 00       	cmp    $0x149c,%eax					# comparaison avec 0x149c
```

En analysant la fonction `main`, nous remarquons qu'elle affiche plusieurs messages et demande un mot de passe via `scanf`. Le mot de passe attendu est comparé à la valeur hexadécimale `0x149c` (qui est 5276 en décimal). Et si nous essayions ce mot de passe ?
```bash
./level00
# resultat
***********************************
* 	     -Level00 -		  *
***********************************
Password:5276

Authenticated!
```
> Le mot de passe a fonctionné !

Un petit tour de propriétaire : 
```bash
$ whoami
level01
$ ls -la
total 13
dr-xr-x---+ 1 level01 level01   60 Sep 13  2016 .
dr-x--x--x  1 root    root     260 Oct  2  2016 ..
-rw-r--r--  1 level01 level01  220 Sep 10  2016 .bash_logout
lrwxrwxrwx  1 root    root       7 Sep 13  2016 .bash_profile -> .bashrc
-rw-r--r--  1 level00 level00 3533 Sep 10  2016 .bashrc
-rwsr-s---+ 1 level01 users   7280 Sep 10  2016 level00
-rw-r--r--  1 level01 level01  675 Sep 10  2016 .profile
$ pwd
/home/users/level00
```
> Nous sommes dans le home du level00.

Passons au home du level01 :
```bash
$ cd /home/users/level01
$ ls -la
total 17
dr-xr-x---+ 1 level01 level01   80 Sep 13  2016 .
dr-x--x--x  1 root    root     260 Oct  2  2016 ..
-rw-r--r--  1 level01 level01  220 Sep 10  2016 .bash_logout
lrwxrwxrwx  1 root    root       7 Sep 13  2016 .bash_profile -> .bashrc
-rw-r--r--  1 level01 level01 3533 Sep 10  2016 .bashrc
-rwsr-s---+ 1 level02 users   7360 Sep 10  2016 level01
-rw-r--r--+ 1 level01 level01   41 Oct 19  2016 .pass
-rw-r--r--  1 level01 level01  675 Sep 10  2016 .profile
```

Parfait,, un petit `cat` du fichier `.pass` nous donnera le mot de passe du level01 :
```bash
$ cat .pass
uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL
```

Trop facile : `uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL`.