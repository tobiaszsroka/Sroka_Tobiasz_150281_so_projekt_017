#include "common.h"

int czy_pracowac = 1;

void obsluga_ewakuacji(int sig) {
	if (sig == SIGUSR1) {
		printf("\n!!! [Komisja] OTRZYMANO SYGNAL EWAKUACJI (SIGUSR1) !!!\n");
		czy_pracowac = 0;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("[Komisja] Brak typu komisji!\n");
		return 1;
	}
	char *typ_komisji = argv[1];
	signal (SIGUSR1, obsluga_ewakuacji);
	signal(SIGTSTP, SIG_IGN);

	srand(time(NULL) ^ getpid());

	//Inicjalizacja IPC
	key_t klucz = ftok(PATH_NAME, PROJECT_ID);
	int id_pamieci = shmget(klucz, 0, 0);
	PamiecDzielona *pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);

	printf("[Komisja %s] (PID: %d) Czekam na kandydatów.\n", typ_komisji, getpid());

	//Praca komisji
	while (czy_pracowac) {
		if (pamiec->ewakuacja == 1) {
			printf("[Komisja %s] Zauważono flagę ewakuacji!\n", typ_komisji);
             break;
		}
		if (pamiec->studenci_zakonczeni >= pamiec->liczba_kandydatow) {
			printf("[Komisja %s] Wszyscy obsluzeni. Koniec pracy.\n", typ_komisji);
			break;
		}
		if (rand() % 100 < 2) {
			printf("[Komisja %s] Generuje nowe pytania...\n", typ_komisji);
		}

		sleep(1);
	}

	shmdt(pamiec);
	return 0;
}
