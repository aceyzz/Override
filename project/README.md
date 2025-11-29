## Overview

La VM Override est installé sur un serveur distant etant donné le probleme de compatibilté avec ma machine host (MacOS ARM). Le serveur est configuré avec un VPN Tailscale, afin d'avoir accès en dehors de mon réseau local. Pour y accéder, j'ai télécharger l'iso depuis mon machine host et je l'ai transféré sur le serveur distant via SCP :
```bash
scp ./OverRide.iso xvi@100.97.200.81:/home/xvi/override/OverRide.iso
```

Ensuite, je me suis connecté en SSH sur le serveur distant pour lancer la VM Override avec la commande suivante :
```bash
nohup qemu-system-x86_64 \
  -enable-kvm \
  -cpu host \
  -m 2048 \
  -smp 2 \
  -drive file=override_disk.qcow2,if=virtio,format=qcow2 \
  -cdrom OverRide.iso \
  -boot d \
  -netdev user,id=net0,hostfwd=tcp::4242-:4242 \
  -device e1000,netdev=net0 \
  -display none \
  > vm.log 2>&1 &
```

Maintenant que la VM est lancé avec la redirection de port 4242, je peux y accéder sn SSH via ma machine host avec la commande suivante :
```bash
ssh -p 4242 level00@100.97.200.81
```

Tout les binaires je les decompile et desassemble avec RetDec ou Ghidra via [DogBolt](https://dogbolt.org/), les mets dans un fichier `source.c`, puis je fais un `objdump -d` du binaire, je les passes a l'IA pour avoir une version un peu plus lisible pour un humain, et je trace toute mon avancée dans des fichiers README.md dans chaque level.

<br>

## Table des matières

- [Level 00](./level00)
- [Level 01](./level01)
- [Level 02](./level02)
- [Level 03](./level03)
- [Level 04](./level04)
<!-- - [Level 05](./level05)
- [Level 06](./level06)
- [Level 07](./level07)
- [Level 08](./level08)
- [Level 09](./level09) (a voir si on le fait) -->

<br>

La partie peut commencer.
