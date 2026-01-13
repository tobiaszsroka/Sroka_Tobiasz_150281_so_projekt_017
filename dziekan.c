#include "common.h"
#include <time.h>

int id_pamieci = -1;
PamiecDzielona *wspolna_pamiec = NULL;

int losuj(int min, int max) {
    return min + rand() % (max - min + 1);
}

//Funkcja sprzatajaca(Ctrl+C, koniec programu)
void sprzatanie(int signal) {
    printf("[Dziekan] Rozpoczynam sprzatanie zasobow...\n");

    if (wspolna_pamiec != NULL) {
        shmdt(wspolna_pamiec);
	printf("[Dziekan] Pamiec odlaczona.\n");
    }

    if (id_pamieci != -1) {
        shmctl(id_pamieci, IPC_RMID, NULL);
        printf("[Dziekan] Pamiec usunieta.\n");
    }

    if (signal != 0) {
       printf("[Dziekan] Zakonczono przez sygnal %d.\n", signal);
       exit(0);
    }
}


int main() {
    srand(time(NULL));
    signal(SIGINT, sprzatanie);

    printf("[Dziekan] Tworzenie pamięci dzielonej...\n");
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    if (klucz == -1) report_error_and_exit("Blad ftok");

    id_pamieci = shmget(klucz, sizeof(PamiecDzielona), 0666 | IPC_CREAT);
    if (id_pamieci == -1) report_error_and_exit("Błąd shmget");

    wspolna_pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (wspolna_pamiec == (void*) -1) report_error_and_exit("Błąd shmat");

    //Weryfikacja matury
    wspolna_pamiec->liczba_kandydatow = MAX_KANDYDATOW;

    printf("[Dziekan] Weryfikacja dokumentow kandydatow...\n");
    for (int i = 0; i < MAX_KANDYDATOW; i++) {
        wspolna_pamiec->studenci[i].id_kandydata = i + 1;
	//2% szans na brak matury
        if (losuj(1, 100) <= 2) {
            wspolna_pamiec->studenci[i].zdana_matura = 0;
        } else {
            wspolna_pamiec->studenci[i].zdana_matura = 1;
        }
    }

    //Uruchomienie jednego testowego kandydata
    printf("[Dziekan] Uruchamiam proces testowy kandydata nr 0...\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        execl("./kandydat", "kandydat", "0", NULL);
        perror("Błąd execl");
        exit(1);
    }

    wait(NULL);

    printf("[Dziekan] Test zakończony. Sprzątam.\n");
    sprzatanie(0);
    return 0;
}
