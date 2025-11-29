#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int32_t decrypt(int32_t a1);
int32_t test(int32_t a1, int32_t a2);

int32_t g1;
int32_t g2;

void __stack_chk_fail(void);

int32_t decrypt(int32_t a1) {
	int32_t v1 = __readgsdword(20);
	int32_t v2 = 0x757c7d51;
	int32_t v3 = 0;
	int32_t v4 = &v2;
	bool v5;
	int32_t v6 = v5 ? -1 : 1;
	int32_t v7 = -1;
	int32_t v8 = v4;
	int32_t v9 = 0;
	while (v7 != 0) {
		int32_t v10 = v8;
		v7--;
		v8 = v10 + v6;
		v9 = v7;
		if (*(char *)v10 == 0) {
			break;
		}
		v9 = 0;
	}
	int32_t v11 = -2 - v9;
	if (v11 != 0) {
		int32_t v12 = 0;
		char * v13 = (char *)(v12 + v4);
		*v13 = *v13 ^ (char)a1;
		v12++;
		while (v12 != v11) {
			v13 = (char *)(v12 + v4);
			*v13 = *v13 ^ (char)a1;
			v12++;
		}
	}
	int32_t v14 = (int32_t)"Congratulations!";
	int32_t v15 = v4;
	int32_t v16 = 17;
	unsigned char v17 = *(char *)v15;
	char v18 = *(char *)v14;
	char v19 = v18;
	bool v20 = false;
	while (v17 == v18) {
		v16--;
		v14 += v6;
		v15 += v6;
		v19 = v17;
		v20 = true;
		if (v16 == 0) {
			break;
		}
		v17 = *(char *)v15;
		v18 = *(char *)v14;
		v19 = v18;
		v20 = false;
	}
	unsigned char v21 = v19;
	int32_t * v22 = (int32_t *)((int32_t)&v3 + 4);
	int32_t puts_rc;
	if ((v17 >= v21 && !v20) != v17 < v21) {
		*v22 = (int32_t)"\nInvalid Password";
		puts_rc = puts((char *)&g2);
	} else {
		*v22 = (int32_t)"/bin/sh";
		puts_rc = system((char *)&g2);
	}
	int32_t result = puts_rc;
	if (v1 != __readgsdword(20)) {
		__stack_chk_fail();
		result = &g2;
	}
	return result;
}

int32_t test(int32_t a1, int32_t a2) {
	int32_t v1 = a2 - a1;
	g1 = v1;
	int32_t result;
	switch (v1) {
		case 1: {
			result = decrypt(1);
			break;
		}
		case 2: {
			result = decrypt(2);
			break;
		}
		case 3: {
			result = decrypt(3);
			break;
		}
		case 4: {
			result = decrypt(4);
			break;
		}
		case 5: {
			result = decrypt(5);
			break;
		}
		case 6: {
			result = decrypt(6);
			break;
		}
		case 7: {
			result = decrypt(7);
			break;
		}
		case 8: {
			result = decrypt(8);
			break;
		}
		case 9: {
			result = decrypt(9);
			break;
		}
		case 16: {
			result = decrypt(16);
			break;
		}
		case 17: {
			result = decrypt(17);
			break;
		}
		case 18: {
			result = decrypt(18);
			break;
		}
		case 19: {
			result = decrypt(19);
			break;
		}
		case 20: {
			result = decrypt(20);
			break;
		}
		case 21: {
			result = decrypt(21);
			break;
		}
		default: {
			result = decrypt(rand());
			break;
		}
	}
	return result;
}

int main(int argc, char ** argv) {
	int32_t t;
	int32_t v1 = &t;
	int32_t * v2 = (int32_t *)(v1 + 4);
	*v2 = 0;
	*v2 = time((int32_t *)t);
	srand((int32_t)&g2);
	*v2 = (int32_t)"***********************************";
	puts((char *)&g2);
	*v2 = (int32_t)"*\t\tlevel03\t\t**";
	puts((char *)&g2);
	*v2 = (int32_t)"***********************************";
	puts((char *)&g2);
	*v2 = (int32_t)"Password:";
	printf((char *)&g2);
	int32_t v3 = v1 + 32;
	int32_t * v4 = (int32_t *)(v1 + 8);
	*v4 = v3;
	*v2 = (int32_t)"%d";
	scanf((char *)&g2);
	*v4 = 0x1337d00d;
	*v2 = *(int32_t *)v3;
	test((int32_t)&g2, (int32_t)&g2);
	return 0;
}
