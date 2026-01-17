#include "common.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("[Komisja] Brak typu komisji!\n");
		return 1;
	}

	char *typ_komisji = argv[1];

	srand(time(NULL) ^ getpid());

	//Inicjalizacja IPC
	key_t klucz = ftok(PATH_NAME, PROJECT_ID);
	int id_pamieci = shmget(klucz, 0, 0);
	if (id_pamieci == -1) {
		report_error_and_exit("Blad shmget(komisja)");
	}

	PamiecDzielona *pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
	if (pamiec == (void*) -1) {
		report_error_and_exit("Blad shmat(komisja)");
	}

	printf("[Komisja %s] (PID: %d) Otwieram sale. Czekam na %d kandydatow.\n", typ_komisji, getpid(), pamiec->liczba_kandydatow);

	//Praca komisji
	while (1) {
		if (pamiec->studenci_zakonczeni >= pamiec->liczba_kandydatow) {
			printf("[Komisja %s] Wszyscy obsluzeni. Koniec pracy.\n", typ_komisji);
			break;
		}
		if (pamiec->ewakuacja == 1) {
			printf("[Komisja %s] EWAKUACJA!\n", typ_komisji);
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
