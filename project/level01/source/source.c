#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct _IO_FILE {
	int32_t e0;
};

int32_t verify_user_name(void);
int32_t verify_user_pass(int32_t * a1);

struct _IO_FILE * g1 = NULL;
char * g2;

int32_t verify_user_name(void) {
	puts("verifying username....\n");
	bool v1;
	int32_t v2 = v1 ? -1 : 1;
	int32_t v3 = (int32_t)"dat_wil";
	int32_t v4 = (int32_t)&g2;
	int32_t v5 = 7;
	unsigned char v6 = *(char *)v4;
	char v7 = *(char *)v3;
	char v8 = v7;
	bool v9 = false;
	while (v6 == v7) {
		v5--;
		v3 += v2;
		v4 += v2;
		v8 = v6;
		v9 = true;
		if (v5 == 0) {
			break;
		}
		v6 = *(char *)v4;
		v7 = *(char *)v3;
		v8 = v7;
		v9 = false;
	}
	unsigned char v10 = v8;
	return (int32_t)!((v6 < v10 | v9)) - (int32_t)(v6 < v10);
}

int32_t verify_user_pass(int32_t * a1) {
	bool v1;
	int32_t v2 = v1 ? -1 : 1;
	int32_t v3 = (int32_t)"admin";
	int32_t v4 = (int32_t)a1;
	int32_t v5 = 5;
	unsigned char v6 = *(char *)v4;
	char v7 = *(char *)v3;
	char v8 = v7;
	bool v9 = false;
	while (v6 == v7) {
		v5--;
		v3 += v2;
		v4 += v2;
		v8 = v6;
		v9 = true;
		if (v5 == 0) {
			break;
		}
		v6 = *(char *)v4;
		v7 = *(char *)v3;
		v8 = v7;
		v9 = false;
	}
	unsigned char v10 = v8;
	return (int32_t)!((v6 < v10 | v9)) - (int32_t)(v6 < v10);
}

int main(int argc, char ** argv) {
	int32_t str;
	__asm_rep_stosd_memset((char *)&str, 0, 16);
	puts("********* ADMIN LOGIN PROMPT *********");
	printf("Enter Username: ");
	fgets((char *)&g2, 256, g1);
	if (verify_user_name() == 0) {
		puts("Enter Password: ");
		fgets((char *)&str, 100, g1);
		verify_user_pass(&str);
		puts("nope, incorrect password...\n");
	} else {
		puts("nope, incorrect username...\n");
	}
	return 1;
}
