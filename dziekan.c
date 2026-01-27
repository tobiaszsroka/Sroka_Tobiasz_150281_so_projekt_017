#include "common.h"
#include <time.h>

int id_pamieci = -1;
int id_semaforow = -1;
int id_kolejki = -1;
PamiecDzielona *wspolna_pamiec = NULL;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

//Funkcja sprzatajaca(Ctrl+C, koniec programu)
void sprzatanie(int signal) {
    printf("[Dziekan] Rozpoczynam sprzatanie zasobow...\n");

    if (wspolna_pamiec != NULL) {
        if (shmdt(wspolna_pamiec) == -1) {
            perror("Blad shmdt");
        } else {
            printf("[Dziekan] Pamiec odlaczona.\n");
        }
    }

    if (id_pamieci != -1) {
        if (shmctl(id_pamieci, IPC_RMID, NULL) == -1) {
            perror("Blad usuwania pamieci");
        } else {
            printf("[Dziekan] Pamiec usunieta.\n");
        }
    }

    if (id_semaforow != -1) {
        if (semctl(id_semaforow, 0, IPC_RMID) == -1) {
            perror("Blad usuwania semaforow");
        } else {
            printf("[Dziekan] Semafory usuniete.\n");
        }
    }

    //Usuwanie kolejki komunikatow
    if (id_kolejki != -1) {
        if (msgctl(id_kolejki, IPC_RMID, NULL) == -1) {
             perror("Blad usuwania kolejki komunikatow");
        } else {
             printf("[Dziekan] Kolejka komunikatow usunieta.\n");
        }
    }

    printf("[Dziekan] Zasoby zwolnione.\n");
    if (signal != 0) {
        printf("[Dziekan] Zakonczono przez sygnal %d.\n", signal);
        exit(0);
    }
}

int losuj(int min, int max) {
    return min + rand() % (max - min + 1);
}

void ustaw_semafor(int sem_id, int sem_num, int wartosc) {
    union semun arg;
    arg.val = wartosc;
    if (semctl(sem_id, sem_num, SETVAL, arg) == -1) {
        perror("Blad ustawiania semafora");
        exit(1);
    }
}

//Funkcja uruchamiajaca ewakuacje(Ctrl+Z)
void zaradz_ewakuacje(int sig) {
    printf("\n\n#####################################################\n");
    printf("[Dziekan] !!! ALARM !!! OGLASZAM EWAKUACJE !!! (Sygnal %d)\n", sig);
    printf("#####################################################\n\n");

    if (wspolna_pamiec != NULL)
        wspolna_pamiec->ewakuacja = 1;

    if (kill(0, SIGUSR1) == -1) {
        perror("Blad kill (ewakuacja)");
    }
}

//Funckja tworzaca ranking studentow
int porownaj_kandydatow(const void *a, const void *b) {
    KandydatDane *k1 = (KandydatDane *)a;
    KandydatDane *k2 = (KandydatDane *)b;

    if (k1->status != STATUS_ZAKONCZYL && k2->status == STATUS_ZAKONCZYL)
        return 1;
    if (k1->status == STATUS_ZAKONCZYL && k2->status != STATUS_ZAKONCZYL)
        return -1;
    
    int suma1 = k1->ocena_teoria + k1->ocena_praktyka;
    int suma2 = k2->ocena_teoria + k2->ocena_praktyka;
    return suma2 - suma1;
}


int main() {
    srand(time(NULL));

    if (signal(SIGINT, sprzatanie) == SIG_ERR) { perror("Signal error"); exit(1); }
    if (signal(SIGTSTP, zaradz_ewakuacje) == SIG_ERR) { perror("Signal error"); exit(1); }
    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) { perror("Signal error"); exit(1); }

    printf("=========================================\n");
    printf("[Dziekan] ROZPOCZYNAM EGZAMIN (Miejsc: %d)\n", MIEJSCA_NA_UCZELNI);
    printf("[Dziekan] Liczba kandydatów (z common.h): %d\n", MAX_KANDYDATOW);
    printf(">> Aby oglosic EWAKUACJE, wcisnij Ctrl+Z <<\n");
    printf("=========================================\n");

    printf("[Dziekan] Tworzenie pamięci dzielonej...\n");
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    if (klucz == -1) report_error_and_exit("Blad ftok");

    id_pamieci = shmget(klucz, sizeof(PamiecDzielona), 0600 | IPC_CREAT);
    if (id_pamieci == -1) report_error_and_exit("Błąd shmget");
    wspolna_pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (wspolna_pamiec == (void*) -1) report_error_and_exit("Błąd shmat");

    //Inicjalizacja pamieci
    wspolna_pamiec->liczba_kandydatow = MAX_KANDYDATOW;
    wspolna_pamiec->ewakuacja = 0;
    wspolna_pamiec->studenci_zakonczeni = 0;

    printf("[Dziekan] Rejestracja kandydatow...\n"); 
    for (int i = 0; i < MAX_KANDYDATOW; i++) {
        wspolna_pamiec->studenci[i].id_kandydata = i + 1;
        wspolna_pamiec->studenci[i].ocena_teoria = 0;
        wspolna_pamiec->studenci[i].ocena_praktyka = 0;
        wspolna_pamiec->studenci[i].pid = 0;
        wspolna_pamiec->studenci[i].status = STATUS_NOWY;

        //2% nie ma matury
        if (losuj(1, 100) <= 2) {
            wspolna_pamiec->studenci[i].zdana_matura = 0;
            wspolna_pamiec->studenci[i].status = STATUS_ODRZUCONY_MATURA;
            printf(" -> Kandydat %d: BRAK MATURY\n", i+1);
        } else {
            wspolna_pamiec->studenci[i].zdana_matura = 1;
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

    //Tworzenie semaforow
    printf("[Dziekan] Tworzenie semaforow...\n");
    id_semaforow = semget(klucz, LICZBA_SEMAFOROW, 0600 | IPC_CREAT);
    if (id_semaforow == -1) report_error_and_exit("Blad semget");

    ustaw_semafor(id_semaforow, SEM_DOSTEP_PAMIEC, 1);
    ustaw_semafor(id_semaforow, SEM_SALA_A, MAX_W_SALI_A);
    ustaw_semafor(id_semaforow, SEM_SALA_B, MAX_W_SALI_B);
    ustaw_semafor(id_semaforow, SEM_KRZESLO_A, 1);
    ustaw_semafor(id_semaforow, SEM_KRZESLO_B, 1);

    //Tworzenie kolejki komunikatow
    printf("[Dziekan] Tworzenie kolejki komunikatow...\n");
    id_kolejki = msgget(klucz, 0600 | IPC_CREAT);
    if (id_kolejki == -1) report_error_and_exit("Blad msgget");

    pid_t pid_ka = fork();
    if (pid_ka == 0) { execl("./komisja", "komisja", "A", NULL); exit(1); }
    else if (pid_ka == -1) { perror("Blad fork Komisja A"); sprzatanie(1); }

    pid_t pid_kb = fork();
    if (pid_kb == 0) { execl("./komisja", "komisja", "B", NULL); exit(1); }
    else if (pid_kb == -1) { perror("Blad fork Komisja B"); sprzatanie(1); }

    //Uruchamianie procesow
    printf("[Dziekan] Otwieram drzwi uczelni dla %d kandydatow...\n", MAX_KANDYDATOW);
    
    printf("ID   | PID   | STATUS WEJSCIA\n");
    printf("-----|-------|---------------\n");


     for (int i = 0; i < MAX_KANDYDATOW; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Blad fork");
            exit(1);
        }
        if (pid == 0) {
            char id_str[10];
            sprintf(id_str, "%d", i);
            if (sprintf(id_str, "%d", i) < 0) {
                perror("Blad sprintf");
                exit(1);
            }
            execl("./kandydat", "kandydat", id_str, NULL);
            perror("Blad execl kandydat");
            exit(1);
        } else {
            wspolna_pamiec->studenci[i].pid = pid;
            
            if (wspolna_pamiec->studenci[i].zdana_matura == 0) {
                printf("%04d | %5d | ODRZUCONY (Brak matury)\n", i+1, pid);
            } else {
                printf("%04d | %5d | WPUSZCZONY NA EGZAMIN\n", i+1, pid);
            }
        }

        if (i % 10 == 0) 
            usleep(250000);
    }

    while (1) {
        pid_t w = wait(NULL);
        if (w == -1) {
            if (errno == ECHILD) break; 
            if (errno == EINTR) continue; 
            perror("Wait error");
            break;
        }
    }

    printf("\n[Dziekan] Egzaminy zakonczone. Generuje raport i zapisuje do pliku...\n");

    //Sortowanie rankingu studentow(od najelspzego do najgorszego wyniku)
    qsort(wspolna_pamiec->studenci, MAX_KANDYDATOW, sizeof(KandydatDane), porownaj_kandydatow);

    FILE *plik = fopen("wyniki.txt", "w");
    if (plik == NULL) {
        perror("Blad otwarcia pliku raportu");
    } else {
        fprintf(plik, "RAPORT Z EGZAMINU WSTEPNEGO\n");
        fprintf(plik, "========================================================================================\n");
        fprintf(plik, "| POZ  |  ID  | MATURA | TEORIA | PRAKT. | SUMA(T+P) | STATUS                      |\n");
        fprintf(plik, "========================================================================================\n");
    }

    printf("\n========================================================================================\n");
    printf("| POZ  |  ID  | MATURA | TEORIA | PRAKT. | SUMA(T+P) | STATUS                      |\n");
    printf("========================================================================================\n");


    int licznik_przyjetych = 0;
    for (int i = 0; i < MAX_KANDYDATOW; i++) {
        KandydatDane *k = &wspolna_pamiec->studenci[i];
        int suma = k->ocena_teoria + k->ocena_praktyka;
        char status_str[40];

        if (wspolna_pamiec->ewakuacja == 1 && k->status != STATUS_ZAKONCZYL) {
            strcpy(status_str, "EWAKUACJA");
        }else if (k->zdana_matura == 0) {
            strcpy(status_str, "BRAK MATURY");
        } else if (k->status == STATUS_OBLAL_TEORIE) {
            strcpy(status_str, "OBLANY (Teoria)");
        } else if (k->status == STATUS_ZAKONCZYL && k->ocena_praktyka < 30) {
             strcpy(status_str, "OBLANY (Praktyka)");
        } else if (k->status == STATUS_ZAKONCZYL) {
            if (licznik_przyjetych < MIEJSCA_NA_UCZELNI) {
                strcpy(status_str, "PRZYJĘTY");
                licznik_przyjetych++;
            } else {
                strcpy(status_str, "NIEPRZYJĘTY (BRAK MIEJSC)");
            }
        } else {
             strcpy(status_str, "NIEUKOŃCZONY");
        }

        if (plik != NULL) {
            fprintf(plik, "| #%03d | %04d | %-3s |  %3d%%  |  %3d%%  |  %3d | %-17s |\n",
                i + 1, k->id_kandydata, k->zdana_matura ? "TAK" : "NIE", 
                k->ocena_teoria, k->ocena_praktyka, suma, status_str);
        }

        printf("| #%03d | %04d |  %-4s  |  %3d%%  |  %3d%%  |    %3d    | %-27s |\n",
            i + 1, k->id_kandydata, k->zdana_matura ? "TAK" : "NIE", 
            k->ocena_teoria, k->ocena_praktyka, suma, status_str);
    }

    printf("====================================================================\n");
    printf("STATYSTYKA: Miejsc: %d, Przyjęto: %d.\n", MIEJSCA_NA_UCZELNI, licznik_przyjetych);
    printf("Raport zapisano w pliku 'wyniki.txt'.\n");

    if (plik != NULL) {
        fprintf(plik, "====================================================================\n");
        fprintf(plik, "STATYSTYKA: Miejsc: %d, Przyjęto: %d.\n", MIEJSCA_NA_UCZELNI, licznik_przyjetych);
        if (fclose(plik) == EOF) {
            perror("Blad fclose(wyniki.txt)");
        }
    }

    sprzatanie(0);
    return 0;
}
