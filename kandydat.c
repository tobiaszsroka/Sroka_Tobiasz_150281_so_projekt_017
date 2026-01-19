#include "common.h"
#include <string.h>

PamiecDzielona *pamiec_global = NULL;
int sem_id_global = 0;
int moj_id_global = 0;

void semafor_operacja(int sem_id, int sem_num, int op) {
    struct sembuf bufor_semafora;
    bufor_semafora.sem_num = sem_num;
    bufor_semafora.sem_op = op;
    bufor_semafora.sem_flg = 0;
    if (semop(sem_id, &bufor_semafora, 1) == -1) {
        if (errno != EINTR) perror("Blad semop");
    }
}

void zakoncz_proces(int sig) {
    if (sig == SIGUSR1) {
	char *msg = "!!! [Kandydat %d] SLYSZE ALARM! UCIEKAM! !!!\n";
    write(STDOUT_FILENO, msg, 44);
    if (pamiec_global != NULL) {
        shmdt(pamiec_global);
    }
    exit(0);
    }
    
    if (pamiec_global != NULL) {
    semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, -1);
    pamiec_global->studenci_zakonczeni++;
    semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, 1);
    shmdt(pamiec_global);
    }

    exit(0);    
}

int main(int argc, char *argv[]) {
    if (argc < 2) exit(1);
    int moj_id = atoi(argv[1]);
    moj_id_global = moj_id;

    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, zakoncz_proces);

    srand(time(NULL) ^ getpid());

    // Pobranie zasobow
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    int id_pamieci = shmget(klucz, 0, 0);
    sem_id_global = semget(klucz, 0, 0);

    if (id_pamieci == -1 || sem_id_global == -1) {
        report_error_and_exit("Blad IPC w kandydat");
    }

    pamiec_global = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    KandydatDane *ja = &pamiec_global->studenci[moj_id];
    ja->pid = getpid();

    // Weryfikacja matury
    if (ja->zdana_matura == 0) {
        printf("[Kandydat %d] Brak matury. Koncze.\n", moj_id);
        zakoncz_proces(0);
    }

    if (ja->powtarza_egzamin == 1) {
        printf("[Kandydat %d] Powtarzam rok (teoria zaliczona). Ide od razu do Sali B.\n", moj_id);
    }
    else {
        printf("[Kandydat %d] Czekam na wejscie do komisji A (Teoria)...\n", moj_id);

        semafor_operacja(sem_id_global, SEM_SALA_A, -1);
        printf(" >> [Kandydat %d] Pisze egzamin teoretyczny...\n", moj_id);
        sleep(1 + rand() % 2);
        ja->ocena_teoria = rand() % 101;
        printf(" <<  [Kandydat %d] Wyszedlem z Sali A. Wynik: %d%%\n", moj_id, ja->ocena_teoria);
        semafor_operacja(sem_id_global, SEM_SALA_A, 1);

        // Sprawdzenie czy zdany egzamin
        if (ja->ocena_teoria < 30) {
            printf("[Kandydat %d] Oblalem teorie. Do widzenia!\n", moj_id);
            ja->status = STATUS_OBLAL_TEORIE;
            zakoncz_proces(0);
        }
        ja->status = STATUS_ZDAL_TEORIE;
    }

    printf("[Kandydat %d] Czekam na wejscie do komisji B (Praktyka)...\n", moj_id);
    semafor_operacja(sem_id_global, SEM_SALA_B, -1);
    printf(" >> [Kandydat %d] Zdaje egzamin praktyczny...\n", moj_id);
    sleep(1 + rand() % 2);
    ja->ocena_praktyka = rand() % 101;
    printf(" << [Kandydat %d] Koniec praktyki. Wynik: %d%%\n", moj_id, ja->ocena_praktyka);
    semafor_operacja(sem_id_global, SEM_SALA_B, 1);

    ja->status = STATUS_ZAKONCZYL;
    zakoncz_proces(0);

    return 0;
}
