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

    printf("=========================================\n");
    printf("[Dziekan] PRZYGOTOWANIE DANYCH\n");
    printf("=========================================\n");

    printf("[Dziekan] Tworzenie pamięci dzielonej...\n");
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    if (klucz == -1) report_error_and_exit("Blad ftok");

    id_pamieci = shmget(klucz, sizeof(PamiecDzielona), 0666 | IPC_CREAT);
    if (id_pamieci == -1) report_error_and_exit("Błąd shmget");

    wspolna_pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (wspolna_pamiec == (void*) -1) report_error_and_exit("Błąd shmat");

    //Generowanie listy
    wspolna_pamiec->liczba_kandydatow = MAX_KANDYDATOW;
    wspolna_pamiec->ewakuacja = 0;

    printf("[Dziekan] Generowanie bazy kandydatów...\n"); 
    for (int i = 0; i < MAX_KANDYDATOW; i++) {
        wspolna_pamiec->studenci[i].id_kandydata = i + 1;
        wspolna_pamiec->studenci[i].ocena_teoria = 0;
        wspolna_pamiec->studenci[i].ocena_praktyka = 0;
        wspolna_pamiec->studenci[i].pid = 0;

        //2% nie ma matury
        if (losuj(1, 100) <= 2) {
            wspolna_pamiec->studenci[i].zdana_matura = 0;
            wspolna_pamiec->studenci[i].status = STATUS_ODRZUCONY_MATURA;
            printf(" -> Kandydat %d: BRAK MATURY\n", i+1);
        } else {
            wspolna_pamiec->studenci[i].zdana_matura = 1;
            wspolna_pamiec->studenci[i].status = STATUS_NOWY;
        }

        //2% powtarza (tylko jeśli mają maturę)
        if (wspolna_pamiec->studenci[i].zdana_matura == 1 && losuj(1, 100) <= 2) {
            wspolna_pamiec->studenci[i].powtarza_egzamin = 1;
            wspolna_pamiec->studenci[i].ocena_teoria = 30 + losuj(0, 70);
            printf(" -> Kandydat %d: POWTARZA (Zaliczona teoria: %d%%)\n", i+1, wspolna_pamiec->studenci[i].ocena_teoria);
        } else {
            wspolna_pamiec->studenci[i].powtarza_egzamin = 0;
        }
    }

    printf("[Dziekan] Baza danych gotowa. Testuję na próbce procesów...\n");

    //Uruchomienie kilku procesów testowych
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char id_str[10];
            sprintf(id_str, "%d", i);
            execl("./kandydat", "kandydat", id_str, NULL);
            exit(1);
        }
    }

    for(int i=0; i<3; i++) wait(NULL);

    printf("[Dziekan] Testy OK. Sprzątam.\n");
    sprzatanie(0);
    return 0;
}
