# Level06

### étape 1: On teste le binaire:
```bash
level06@OverRide:~$  ./level06
***********************************
*		level06		  *
***********************************
-> Enter Login: cduffaut
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 12345
level06@OverRide:~$
```
- Le programme se termine simplement sans message d'erreur quand le serial est incorrect.

#### étape 2: Analyser avec GDB:
```bash
gdb level06
```
- On liste les fonctions

```bash
info functions
```
### étape 3: Comprendre le code:
On voit clairement trois fonctions importantes :

	```bash
	0x08048748  auth - probablement la fonction qui vérifie notre login/serial
	0x08048879  main - le point d'entrée
	0x080485f0  ptrace - utilisé pour détecter le débogage
	```

# étape 4: Étude du main:
	```bash
	disas main
	```
- Ce que fait main:
	```bash
		Ligne +110 : call fgets - Lit le login (stocké à esp+0x2c)
		Ligne +180 : call scanf - Lit le serial (stocké à esp+0x28)
		Ligne +200 : call auth - Appelle la fonction auth avec ces deux valeurs
		Ligne +205 : test %eax,%eax - Teste le retour de auth
		Ligne +207 : jne 0x8048969 - Si auth retourne NON-ZÉRO, saute à +240 (échec)
		Ligne +228 : call system - Si auth retourne ZÉRO, lance /bin/sh
	```

- Donc notre objectif : faire en sorte que auth retourne 0.

### étape 5: désassemblons la fonction auth pour comprendre comment elle calcule le hash:
```bash
disas auth
```
- Structure de la fonction auth :
```bash
	Ligne +50 : mov %eax,-0xc(%ebp) - Stocke la longueur du login
	Ligne +62 : cmpl $0x5,-0xc(%ebp) - Vérifie si longueur > 5
	Ligne +109 : call ptrace - PROBLÈME : détecte le débogage
	Ligne +114 : cmp $0xffffffff,%eax - Si ptrace retourne -1, le programme détecte GDB
	Ligne +117 : jne 0x80487ed - Si pas de débogage détecté, continue normalement
	Lignes +165 à +275 : Calcul du hash (boucle complexe)
	Ligne +286 : cmp -0x10(%ebp),%eax - Compare le hash calculé avec notre serial
	Ligne +289 : je 0x8048872 - Si égaux, va à +298
	Ligne +298 : mov $0x0,%eax - Retourne 0 (succès)
```

### étape 6: Premier test
- Stratégie:
	Mettre un breakpoint juste après l'appel à ptrace (ligne +114)
	Contourner ptrace en modifiant eax
	Mettre un breakpoint à la comparaison finale (ligne +286)
	Lire la valeur du hash calculé

	```bash
	gdb level06
	b *0x080487ba
	```
- Puis on lance le programme:
	```bash
	r
	```
### étape 6: Comprendre le résultat
	```bash
	Starting program: /home/users/level06/level06
	***********************************
	*		level06		  *
	***********************************
	-> Enter Login: cduffaut
	***********************************
	***** NEW ACCOUNT DETECTED ********
	***********************************
	-> Enter Serial: 12345

	Breakpoint 1, 0x080487ba in auth ()
	(gdb)
	```
- Le programme s'est arrêté juste après l'appel à ptrace.
- Vérifions d'abord la valeur de eax (qui contient le retour de ptrace):
info registers eax
	
	```bash
		(gdb) info registers eax
		eax            0xffffffff	-1
	```
- On voit eax = 0xffffffff (c'est-à-dire -1), ce qui signifie que ptrace a détecté GDB
- Le programme va comparer eax à -1. Si c'est égal, il quitte. 

### Étape 7: Contourner ptrace avec jump

Le problème : modifier `eax` après l'appel à ptrace ne suffit pas car le programme continue son exécution normale et vérifie quand même la valeur.

**Solution** : Utiliser `jump` pour sauter directement après la vérification de ptrace.

- Plaçons les deux breakpoints nécessaires:
```bash
(gdb) b *0x080487ba    # Juste après l'appel à ptrace
(gdb) b *0x08048866    # À la comparaison finale du hash
```

- On lance le programme:
```bash
(gdb) r
# Entre un login de plus de 5 caractères
-> Enter Login: uffaut
# Entre n'importe quel serial
-> Enter Serial: 12345
```

- Le programme s'arrête au Breakpoint 1. On va à l'adresse `0x080487ed` (ligne +165 dans le désassemblage): c'est là où le programme continue normalement quand ptrace ne détecte pas de débogage :
```bash
Breakpoint 1, 0x080487ba in auth ()
(gdb) jump *0x080487ed
Continuing at 0x80487ed.
```

### Étape 8: Lire le hash calculé

- Le programme s'arrête maintenant au Breakpoint 2, juste avant de comparer le hash avec notre serial:
```bash
Breakpoint 2, 0x08048866 in auth ()
```

- En lisant l'assembleur ligne par ligne, on voit que -0x10(%ebp) est l'adresse où le hash est calculé et stocké, puis utilisée pour la comparaison finale.

**Avant de continuer**, lisons la valeur du hash qui a été calculé. Le hash est stocké à `ebp-0x10` :
```bash
(gdb) x/d $ebp-0x10 # La commande `x` dans GDB signifie "examine" (la mémoire) (x/[format] [adresse])
0xffffd5f8:	6232820
```

- Le hash pour le login "uffaut" est **6232820**.

### Étape 9: Tester avec le bon serial

- On quitte GDB et on test avec le vrai programme :
```bash
(gdb) quit
level06@OverRide:~$ ./level06
***********************************
*		level06		  *
***********************************
-> Enter Login: uffaut
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 6232820
Authenticated!
$
```

- Le prompt `$` apparaît, c'est le shell qui s'est lancé. On peut maintenant exécuter des commandes:
```bash
$ cat /home/users/level07/.pass
GbcPDRgsFK77LNnnuh7QyFYA2942Gp8yKj9KrWD8
$ exit
```

### Récapitulatif de la solution

1. **Analyser** les fonctions avec GDB (`info functions`, `disas main`, `disas auth`)
2. **Identifier** le point clé : la fonction `auth` calcule un hash et le compare au serial
3. **Contourner** ptrace avec `jump *0x080487ed` pour éviter la détection de débogage
4. **Extraire** le hash calculé avec `x/d $ebp-0x10` au breakpoint de comparaison
5. **Utiliser** le hash extrait comme serial avec le même login pour obtenir le shell

### Flag
```bash
GbcPDRgsFK77LNnnuh7QyFYA2942Gp8yKj9KrWD8
```