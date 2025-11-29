#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
	puts("***********************************");
	puts("* \t     -Level00 -\t\t  *");
	puts("***********************************");
	printf("Password:");
	int32_t v1;
	scanf("%d", &v1);
	int32_t result;
	if (v1 != 0x149c) {
		puts("\nInvalid Password!");
		result = 1;
	} else {
		puts("\nAuthenticated!");
		system("/bin/sh");
		result = 0;
	}
	return result;
}
