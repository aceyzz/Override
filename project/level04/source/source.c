#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

int32_t g1;

int32_t ptrace(int32_t a1, ...);

int main(int argc, char ** argv) {
	int32_t pid = fork();
	int32_t str;
	__asm_rep_stosd_memset((char *)&str, 0, 32);
	int32_t stat_loc = 0;
	if (pid == 0) {
		prctl(PR_SET_PDEATHSIG, 1, (int32_t)&g1, (int32_t)&g1, (int32_t)&g1);
		ptrace(0);
		puts("Give me some shellcode, k");
		gets((char *)&str);
		return 0;
	}
	wait((int32_t)&stat_loc);
	while (stat_loc % 128 != 0) {
		char v1 = (char)stat_loc % 128 + 1;
		if (v1 >= 2 && v1 >= 0) {
			break;
		}
		if (ptrace(3) == 11) {
			puts("no exec() for you");
			kill(pid, SIGKILL);
			return 0;
		}
		wait((int32_t)&stat_loc);
	}
	puts("child is exiting...");
	return 0;
}
