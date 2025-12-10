#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main()
{
    uint8_t buff[0x64]; // 100 octets
    uint32_t i = 0;
    
    fgets(buff, 0x64, stdin);  // Lit 100 chars max + copie ton input dans une zone locale sur la pile
    
    while (i < strlen(buff))
    {
        if (buff[i] > 64 && buff[i] <= 90) // Si c'est une MAJ (A-Z)
        {  
            buff[i] ^= 0x20;  // Convert en minuscule
        }
        i++;
    }
    
    printf(buff);  // VULNERABILITE ici
    exit(0);
}
