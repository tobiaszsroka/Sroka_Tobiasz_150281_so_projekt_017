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

//Funkcja sprzatajaca
void sprzatanie(int signal) {
    if (signal != 0) {
        printf("\n%s[Dziekan] Otrzymano sygnał kończący %d. Zarządzam natychmiastowe wyjście...%s\n", KOLOR_ZOLTY, signal, KOLOR_RESET);
        kill(0, SIGUSR1); 
    }
    //Oczekiwanie na wszystkie dzieci
    while (wait(NULL) > 0); 

    printf("\n%s[Dziekan] Wszyscy wyszli. Rozpoczynam sprzatanie zasobow...%s\n", KOLOR_ZOLTY, KOLOR_RESET);

    //Odlaczenie pamieci dzielonej
    if (wspolna_pamiec != NULL) {
        if (shmdt(wspolna_pamiec) == -1) {
            perror("Blad shmdt");
        } else {
            printf("%s[Dziekan] Pamiec odlaczona.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
        }
    }

    //Usuniecie segmentu pamieci dzielonej
    if (id_pamieci != -1) {
        if (shmctl(id_pamieci, IPC_RMID, NULL) == -1) {
            perror("Blad usuwania pamieci");
        } else {
            printf("%s[Dziekan] Pamiec usunieta.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
        }
    }

    //Usuniecie semaforow
    if (id_semaforow != -1) {
        if (semctl(id_semaforow, 0, IPC_RMID) == -1) {
            perror("Blad usuwania semaforow");
        } else {
            printf("%s[Dziekan] Semafory usuniete.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
        }
    }

    //Usuniecie kolejki komunikatow
    if (id_kolejki != -1) {
        if (msgctl(id_kolejki, IPC_RMID, NULL) == -1) {
             perror("Blad usuwania kolejki komunikatow");
        } else {
             printf("%s[Dziekan] Kolejka komunikatow usunieta.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
        }
    }

    printf("%s[Dziekan] Zasoby zwolnione.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
    //Zakoncz proces jesli wywolano przez sygnal
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

//Funkcja uruchamiajaca ewakuacje(Ctrl+C)
void zaradz_ewakuacje(int sig) {
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "%s\n!!! ALARM !!! OGLASZAM EWAKUACJE !!! (Sygnal %d)\n%s", KOLOR_CZERWONY, sig, KOLOR_RESET);
    write(STDOUT_FILENO, buf, len);

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


int main(int argc, char *argv[]) {
    srand(time(NULL));

    // Walidacja danych
    if (argc < 2) {
        fprintf(stderr, "Błąd: Nie podano liczby kandydatów!\n");
        exit(1);
    }
    int liczba_chetnych = atoi(argv[1]);

    if (liczba_chetnych <= 0 || liczba_chetnych > MAX_KANDYDATOW) {
        fprintf(stderr, "Błąd: Liczba kandydatów musi być z zakresu 1-%d\n", MAX_KANDYDATOW);
        exit(1);
    }

    //SIGINT(CTRL+C)
    if (signal(SIGINT, zaradz_ewakuacje) == SIG_ERR) { 
        perror("Signal error SIGINT");
        exit(1); 
    }
    
    //SIGUSR1(dziekan nie reaguje na wlasny sygnal ewakuacji)
    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) { 
        perror("Signal error");
        exit(1); 
    }
    
    //Generowanie klucza IPC
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    if (klucz == -1) report_error_and_exit("Blad ftok");

    //Tworzenie semaforow
    id_semaforow = semget(klucz, LICZBA_SEMAFOROW, 0600 | IPC_CREAT);
    if (id_semaforow == -1) report_error_and_exit("Blad semget");

    ustaw_semafor(id_semaforow, SEM_DOSTEP_PAMIEC, 1);
    ustaw_semafor(id_semaforow, SEM_SALA_A, MAX_W_SALI_A);
    ustaw_semafor(id_semaforow, SEM_SALA_B, MAX_W_SALI_B);
    ustaw_semafor(id_semaforow, SEM_KRZESLO_A, 1);
    ustaw_semafor(id_semaforow, SEM_KRZESLO_B, 1);
    ustaw_semafor(id_semaforow, SEM_STDOUT, 1); // Odblokowanie loggera(funkcja loguj)
    
    printf("=========================================\n");
    loguj(id_semaforow, KOLOR_ZOLTY, "[Dziekan] ROZPOCZYNAM EGZAMIN (Liczba miejsc: %d)\n", MIEJSCA_NA_UCZELNI);
    loguj(id_semaforow, KOLOR_ZOLTY, "[Dziekan] Liczba kandydatów: %d\n", liczba_chetnych);
    printf(">> Aby oglosic EWAKUACJE, wcisnij Ctrl+Z <<\n");
    printf("=========================================\n");

    loguj(id_semaforow, KOLOR_ZOLTY, "[Dziekan] Tworzenie pamięci dzielonej...\n");
    id_pamieci = shmget(klucz, sizeof(PamiecDzielona), 0600 | IPC_CREAT);
    if (id_pamieci == -1) report_error_and_exit("Błąd shmget");

    wspolna_pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (wspolna_pamiec == (void*) -1) report_error_and_exit("Błąd shmat");

    //Inicjalizacja pamieci
    wspolna_pamiec->liczba_kandydatow = liczba_chetnych;
    wspolna_pamiec->ewakuacja = 0;
    wspolna_pamiec->studenci_zakonczeni = 0;

    loguj(id_semaforow, KOLOR_ZOLTY, "[Dziekan] Rejestracja kandydatow w systemie...\n");
    for (int i = 0; i < liczba_chetnych; i++) {
        wspolna_pamiec->studenci[i].id_kandydata = i + 1;
        wspolna_pamiec->studenci[i].ocena_teoria = 0;
        wspolna_pamiec->studenci[i].ocena_praktyka = 0;
        wspolna_pamiec->studenci[i].pid = 0;
        wspolna_pamiec->studenci[i].status = STATUS_NOWY;
        
        //2% nie ma matury         
        if (losuj(1, 100) <= 2) {
            wspolna_pamiec->studenci[i].zdana_matura = 0;
            wspolna_pamiec->studenci[i].status = STATUS_ODRZUCONY_MATURA;
            loguj(id_semaforow, KOLOR_CZERWONY, " -> Kandydat %d: BRAK MATURY (Odrzucony)\n", i+1);
        } else {
            wspolna_pamiec->studenci[i].zdana_matura = 1;
        }

        //2% powtarza (tylko jeśli mają maturę)
        if (wspolna_pamiec->studenci[i].zdana_matura == 1 && losuj(1, 100) <= 2) {
            wspolna_pamiec->studenci[i].powtarza_egzamin = 1;
            wspolna_pamiec->studenci[i].ocena_teoria = 30 + losuj(0, 70);
            loguj(id_semaforow, KOLOR_CYJAN, " -> Kandydat %d: POWTARZA (Zaliczona teoria: %d%%)\n", i+1, wspolna_pamiec->studenci[i].ocena_teoria);
        } else {
            wspolna_pamiec->studenci[i].powtarza_egzamin = 0;
        }
    }

    //Tworzenie kolejki komunikatow
    loguj(id_semaforow, KOLOR_ZOLTY, "[Dziekan] Tworzenie kolejki komunikatow...\n");
    id_kolejki = msgget(klucz, 0600 | IPC_CREAT);
    if (id_kolejki == -1) 
        report_error_and_exit("Blad msgget");

    //Tworzenie procesow komisji
    pid_t pid_ka = fork();
    if (pid_ka == 0) { 
        execl("./komisja", "komisja", "A", NULL); 
        exit(1);
    }
    else if (pid_ka == -1) { 
        perror("Blad fork Komisja A");
        sprzatanie(1); 
    }

    pid_t pid_kb = fork();
    if (pid_kb == 0) { 
        execl("./komisja", "komisja", "B", NULL);
        exit(1); 
    }
    else if (pid_kb == -1) { 
        perror("Blad fork Komisja B");
        sprzatanie(1); 
    }

    loguj(id_semaforow, KOLOR_ZOLTY, "[Dziekan] Otwieram drzwi uczelni dla %d kandydatow...\n", liczba_chetnych);
    printf("ID   | PID   | STATUS WEJSCIA\n");
    printf("-----|-------|---------------\n");

    //Tworzenie procesow potomnych(kandydatow)
    for (int i = 0; i < liczba_chetnych; i++) {
        if (wspolna_pamiec->ewakuacja == 1) {
            printf(KOLOR_CZERWONY "[Dziekan] Ewakuacja w toku! Przestaję wpuszczać nowych studentów.\n" KOLOR_RESET);
            break; 
        }
        //Klonowanie dziekana
        pid_t pid = fork();
        if (pid < 0) {
            perror("Blad fork");
            exit(1);
        }

        if (pid == 0) {
            char id_str[10];
            sprintf(id_str, "%d", i);
            execl("./kandydat", "kandydat", id_str, NULL);
            perror("Blad execl kandydat");
            exit(1);
        } else {
            wspolna_pamiec->studenci[i].pid = pid;
            
            semafor_operacja(id_semaforow, SEM_STDOUT, -1);
            printf("%s", KOLOR_RESET); 
            if (wspolna_pamiec->studenci[i].zdana_matura == 0) {
                printf("%04d | %5d | ODRZUCONY (Brak matury)\n", i+1, pid);
            } else {
                printf("%04d | %5d | WPUSZCZONY NA EGZAMIN\n", i+1, pid);
            }
            semafor_operacja(id_semaforow, SEM_STDOUT, 1);
        }

        if (i % 10 == 0) {}
            usleep(250000);
    }

    int studenci_obsluzeni = 0;
    while (studenci_obsluzeni < liczba_chetnych) {
        pid_t w = wait(NULL);
        if (w == -1) {
            if (errno == ECHILD) break;
            if (errno == EINTR) continue; 
            perror("Wait error");
            break;
        }

        if (w != pid_ka && w != pid_kb) {
            studenci_obsluzeni++;
        } 
    }

    loguj(id_semaforow, KOLOR_ZOLTY, "\n[Dziekan] Wszyscy studenci zakonczyli egzaminy. Zamykam komisje...\n");

    kill(pid_ka, SIGUSR1);
    kill(pid_kb, SIGUSR1);

    wait(NULL);
    wait(NULL);

    loguj(id_semaforow, KOLOR_ZOLTY, "\n[Dziekan] Egzaminy zakonczone. Generuje raport i zapisuje do pliku...\n");

    //Sortowanie rankingu studentow(od najelspzego do najgorszego wyniku)
    qsort(wspolna_pamiec->studenci, liczba_chetnych, sizeof(KandydatDane), porownaj_kandydatow);

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
    for (int i = 0; i < liczba_chetnych; i++) {
        KandydatDane *k = &wspolna_pamiec->studenci[i];
        int suma = k->ocena_teoria + k->ocena_praktyka;
        char status_str[40];

        //Okreslenie statusu kandydata na podstawie danych z pamieci dzielonej
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

        //Zapis do pliku i wyswietlanie na ekran
        if (plik != NULL) {
            fprintf(plik, "| #%03d | %04d | %-3s |  %3d%%  |  %3d%%  |  %3d | %-17s |\n",
                i + 1, k->id_kandydata, k->zdana_matura ? "TAK" : "NIE", 
                k->ocena_teoria, 
                k->ocena_praktyka, suma, status_str);
        }

        printf("| #%03d | %04d |  %-4s  |  %3d%%  |  %3d%%  |    %3d    | %-27s |\n",
            i + 1, k->id_kandydata, k->zdana_matura ? "TAK" : "NIE", 
            k->ocena_teoria, k->ocena_praktyka, suma, status_str);
    }

    printf("====================================================================\n");
    printf(KOLOR_ZIELONY "ILOSC KANDYDATOW: %d\n" KOLOR_RESET, wspolna_pamiec->studenci_zakonczeni);
    loguj(id_semaforow, KOLOR_ZIELONY, "STATYSTYKA: Miejsc: %d, Przyjęto: %d.\n", MIEJSCA_NA_UCZELNI, licznik_przyjetych);
    loguj(id_semaforow, KOLOR_ZOLTY, "Raport zapisano w pliku 'wyniki.txt'.\n");

    if (plik != NULL) {
        fprintf(plik, "====================================================================\n");
        fprintf(plik, "ILOSC KANDYDATOW: %d\n", wspolna_pamiec->studenci_zakonczeni);
        fprintf(plik, "STATYSTYKA: Miejsc: %d, Przyjęto: %d.\n", MIEJSCA_NA_UCZELNI, licznik_przyjetych);
        if (fclose(plik) == EOF) {
            perror("Blad fclose(wyniki.txt)");
        }
    }

    sprzatanie(0);
    return 0;
}