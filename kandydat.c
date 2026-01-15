#include "common.h"

void semafor_operacja(int sem_id, int sem_num, int op) {
    struct sembuf bufor_semafora;
    bufor_semafora.sem_num = sem_num;
    bufor_semafora.sem_op = op;
    bufor_semafora.sem_flg = 0;
    if (semop(sem_id, &bufor_semafora, 1) == -1) {
	if (errno != EINTR) perror("Blad semop");
	exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) exit(1);
    int moj_id = atoi(argv[1]);

    srand(time(NULL) ^ getpid());

    //Pobranie zasobow
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    int id_pamieci = shmget(klucz, 0, 0);
    int id_semaforow = semget(klucz, 0, 0);

    if (id_pamieci == -1 || id_semaforow == -1) {
	report_error_and_exit("Blad IPC w kandydat");
    }

    PamiecDzielona *pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (pamiec == (void*) -1) report_error_and_exit("Blad shmat kandydat");
    KandydatDane *ja = &pamiec->studenci[moj_id];
    ja->pid = getpid();

    //Weryfikacja matury
    if (ja->zdana_matura == 0) {
	printf("[Kandydat %d] Brak matury. Koncze.\n", moj_id);
	shmdt(pamiec);
	exit(0);
    }

    if (ja->powtarza_egzamin == 1) {
    	printf("[Kandydat %d] Powtarzam rok (teoria zaliczona). Ide od razu do Sali B.\n", moj_id);
    }
    else {
	printf("[Kandydat %d] Czekam na wejscie do komisji A (Teoria)...\n", moj_id);

	semafor_operacja(id_semaforow, SEM_SALA_A, -1);
	printf(" >> [Kandydat %d] Pisze egzamin teoretyczny...\n", moj_id);
	sleep(1 + rand() % 2);
	ja->ocena_teoria = rand() % 101;
 	printf(" <<  [Kandydat %d] Wyszedlem z Sali A. Wynik: %d%%\n", moj_id, ja->ocena_teoria);
	semafor_operacja(id_semaforow, SEM_SALA_A, 1);

	//Sprawdzenie czy zdany egzamin
	if (ja->ocena_teoria < 30) {
	    printf("[Kandydat %d] Oblalem teorie. Do widzenia!\n", moj_id);
	    ja->status = STATUS_OBLAL_TEORIE;
	    shmdt(pamiec);
	    exit(0);
	}
	ja->status = STATUS_ZDAL_TEORIE;
    }

    printf("[Kandydat %d] Czekam na wejscie do komisji B (Praktyka)...\n", moj_id);
    semafor_operacja(id_semaforow, SEM_SALA_B, -1);
    printf(" >> [Kandydat %d] Zdaje egzamin praktyczny...\n", moj_id);
    sleep(1 + rand() % 2);
    ja->ocena_praktyka = rand() % 101;
    printf(" << [Kandydat %d] Koniec praktyki. Wynik: %d%%\n", moj_id, ja->ocena_praktyka);
    semafor_operacja(id_semaforow, SEM_SALA_B, 1);

    ja->status = STATUS_ZAKONCZYL;


    shmdt(pamiec);
    return 0;
}
