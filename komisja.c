#include "common.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("[Komisja] Brak typu komisji!\n");
		return 1;
	}
	printf("[Komisja %s] Gotowa do pracy (PID: %d)\n", argv[1], getpid());
	return 0;
}
