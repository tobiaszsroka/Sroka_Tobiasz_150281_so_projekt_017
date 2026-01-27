#include "common.h"

int czy_pracowac = 1;
char *typ_komisji_global = "?"; 

void obsluga_ewakuacji(int sig) {
    if (sig == SIGUSR1) {
        printf("\n!!! [Komisja %s] OTRZYMANO SYGNAL EWAKUACJI (SIGUSR1) !!!\n", typ_komisji_global);
        czy_pracowac = 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Komisja] Błąd: Brak argumentu typu komisji!\n");
        return 1;
    }
    char *typ_komisji = argv[1];
    typ_komisji_global = typ_komisji;

    if (signal(SIGUSR1, obsluga_ewakuacji) == SIG_ERR) {
        perror("Blad signal SIGUSR1");
        exit(1);
    }
    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
        perror("Blad signal SIGTSTP");
        exit(1);
    }

    srand(time(NULL) ^ getpid());

    // Inicjalizacja IPC
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    if (klucz == -1) {
        perror("[Komisja] Blad ftok");
        exit(1);
    }

    int id_pamieci = shmget(klucz, 0, 0);
    if (id_pamieci == -1) {
        perror("[Komisja] Blad shmget (czy Dziekan dziala?)");
        exit(1);
    }

    PamiecDzielona *pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (pamiec == (void*) -1) {
        perror("[Komisja] Blad shmat");
        exit(1);
    }

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

        usleep(100000); 
    }

    if (shmdt(pamiec) == -1) {
        perror("[Komisja] Blad shmdt");
    }
    
    return 0;
}