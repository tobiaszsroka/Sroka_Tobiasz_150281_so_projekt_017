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

    printf("[Kandydat %d] Czekam w kolejce do sali A...\n", moj_id);
    semafor_operacja(id_semaforow, SEM_SALA_A, -1);
    printf(" >> [Kandydat %d] WSZEDLEM do SALI A! (Pisze egzamin...)\n", moj_id);
    sleep(1 + rand() % 3);
    printf(" << [Kandydat %d] WYCHODZE z Sali A.\n", moj_id);
    semafor_operacja(id_semaforow, SEM_SALA_A, 1);

    shmdt(pamiec);
    return 0;
}
