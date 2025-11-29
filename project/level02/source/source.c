#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _IO_FILE {
	int32_t e0;
};

struct _IO_FILE * g1 = NULL;
struct _IO_FILE * g2 = NULL;

int main(int argc, char ** argv) {
	int64_t str;
	__asm_rep_stosq_memset((char *)&str, 0, 12);
	bool v1;
	int64_t v2 = v1 ? -96 : 96;
	*(int32_t *)(v2 + (int64_t)&str) = 0;
	int64_t data;
	__asm_rep_stosq_memset((char *)&data, 0, 5);
	*(char *)((v1 ? -40 : 40) + (int64_t)&data) = 0;
	int64_t str2;
	__asm_rep_stosq_memset((char *)&str2, 0, 12);
	*(int32_t *)(v2 + (int64_t)&str2) = 0;
	struct _IO_FILE * file = fopen("/home/users/level03/.pass", "r");
	if (file == NULL) {
		fwrite((int64_t *)"ERROR: failed to open password file\n", 1, 36, g2);
		exit(1);
	}
	int64_t v3;
	int64_t v4 = &v3;
	int32_t items_read = fread(&data, 1, 41, file);
	*(char *)(v4 - 160 + (int64_t)strcspn((char *)&data, "\n")) = 0;
	if (items_read != 41) {
		fwrite((int64_t *)"ERROR: failed to read password file\n", 1, 36, g2);
		fwrite((int64_t *)"ERROR: failed to read password file\n", 1, 36, g2);
		exit(1);
	}
	fclose(file);
	puts("===== [ Secure Access System v1.0 ] =====");
	puts("/***************************************\\");
	puts("| You must login to access this system. |");
	puts("\\**************************************/");
	printf("--[ Username: ");
	fgets((char *)&str, 100, g1);
	*(char *)(v4 - 112 + (int64_t)strcspn((char *)&str, "\n")) = 0;
	printf("--[ Password: ");
	fgets((char *)&str2, 100, g1);
	*(char *)(v4 - 272 + (int64_t)strcspn((char *)&str2, "\n")) = 0;
	puts("*****************************************");
	if (strncmp((char *)&data, (char *)&str2, 41) == 0) {
		printf("Greetings, %s!\n", &str);
		system("/bin/sh");
		return 0;
	}
	printf((char *)&str);
	puts(" does not have access!");
	exit(1);
}
