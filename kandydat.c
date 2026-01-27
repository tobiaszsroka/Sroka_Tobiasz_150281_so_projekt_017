#include "common.h"
#include <string.h>
#include <sys/time.h>

PamiecDzielona *pamiec_global = NULL;
int sem_id_global = 0;
int moj_id_global = 0;

void drukuj_czas() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    char buffer[20];
    strftime(buffer, 20, "%H:%M:%S", tm_info);
    printf("[%s.%03ld] ", buffer, tv.tv_usec / 1000);
}

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
        char msg[128];
        snprintf(msg, sizeof(msg), "\n!!! [Kandydat %d] SLYSZE ALARM! UCIEKAM! !!!\n", moj_id_global);
        write(STDOUT_FILENO, msg, strlen(msg));
        
        if (pamiec_global != NULL) {
            if (shmdt(pamiec_global) == -1) {
                perror("[Kandydat] Blad shmdt przy ewakuacji");
            }
        }
        exit(0);
    }
    
    if (pamiec_global != NULL) {
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, -1);
        pamiec_global->studenci_zakonczeni++;
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, 1);
        if (shmdt(pamiec_global) == -1) perror("Blad shmdt");
    }

    exit(0);    
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Kandydat %d] Błąd krytyczny: Nie podano ID kandydata w argumentach!\n", moj_id_global);
        exit(1);
    }

    int moj_id = atoi(argv[1]);
    moj_id_global = moj_id;

    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) { perror("Signal error"); exit(1); }
    if (signal(SIGUSR1, zakoncz_proces) == SIG_ERR) { perror("Signal error"); exit(1); }

    srand(time(NULL) ^ getpid());

    //Pobranie zasobow
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    if (klucz == -1) report_error_and_exit("Blad ftok");

    int id_pamieci = shmget(klucz, 0, 0);
    if (id_pamieci == -1) report_error_and_exit("Blad shmget");

    sem_id_global = semget(klucz, 0, 0);
    if (sem_id_global == -1) report_error_and_exit("Blad semget");

    pamiec_global = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (pamiec_global == (void*) -1) report_error_and_exit("Blad shmat");

    KandydatDane *ja = &pamiec_global->studenci[moj_id];
    ja->pid = getpid();

    int szczegoly = 1; 

    if (ja->zdana_matura == 0) {
        zakoncz_proces(0);
    }

    //Komisja A
     if (ja->powtarza_egzamin == 1) {
        if (szczegoly) printf("[Kandydat %d] Powtarzam rok (teoria zaliczona). Ide do Sali B.\n", moj_id);
    } 
    else {
        if (szczegoly) printf("[Kandydat %d] Czekam na wejscie do komisji A (Teoria)...\n", moj_id);
        
        semafor_operacja(sem_id_global, SEM_SALA_A, -1);
        if (szczegoly) printf(" >> [Kandydat %d] Wszedlem do Sali A.\n", moj_id);

        if (rand() % 100 < 20) { 
             if (szczegoly) printf("    [Kandydat %d] Czekam az czlonkowie komisji przygotuja pytania (spoznienie)...\n", moj_id);
             usleep(2000); 
        }

        int otrzymane_pytania[LICZBA_PYTAN_A];
        for(int i=0; i<LICZBA_PYTAN_A; i++) {
            otrzymane_pytania[i] = 1 + rand() % 50; 
        }
        if (szczegoly) printf("    [Kandydat %d] Otrzymalem %d pytan. Rozpoczynam przygotowanie (Ti)...\n", moj_id, LICZBA_PYTAN_A);

        usleep(1000000); 

        if (szczegoly) printf("    [Kandydat %d] Gotowy. Czekam na zwolnienie miejsca przed komisja...\n", moj_id);

        semafor_operacja(sem_id_global, SEM_KRZESLO_A, -1);

        int suma_pkt = 0;
        
        for (int i = 0; i < LICZBA_PYTAN_A; i++) {
            if (rand() % 100 < 10) {
                 if (szczegoly) { drukuj_czas(); printf("    [Kandydat %d] Egzaminator szuka pytań... (Czekam)\n", moj_id); }
                 usleep(300000);
            }

            if (szczegoly) {
                drukuj_czas();
                printf("[Komisja A] [Kandydat %d] Egzaminator %d zadaje pytanie nr %d...\n", 
                       moj_id, i+1, otrzymane_pytania[i]);
            }

            usleep(1000); 
            int ocena = rand() % 101;
            suma_pkt += ocena;

            if (szczegoly) {
                drukuj_czas();
                printf("[Komisja A] [Kandydat %d] Członek nr %d ocenia odpowiedź na pyt. %d -> %d%%\n", 
                       moj_id, i+1, otrzymane_pytania[i], ocena);
            }
        } 
        
        ja->ocena_teoria = suma_pkt / LICZBA_PYTAN_A;
        
        if (szczegoly) printf("    [Komisja A] [Kandydat %d] Przewodniczący ustalił ocenę końcową: %d%%\n", moj_id, ja->ocena_teoria);

        semafor_operacja(sem_id_global, SEM_KRZESLO_A, 1);
        
        if (szczegoly) printf(" << [Kandydat %d] Opuszczam Salę A.\n", moj_id);
        semafor_operacja(sem_id_global, SEM_SALA_A, 1); 

        if (ja->ocena_teoria < 30) {
            ja->status = STATUS_OBLAL_TEORIE;
            zakoncz_proces(0);
        }
        ja->status = STATUS_ZDAL_TEORIE;
    }

    //Komisja B
    if (szczegoly) printf("[Kandydat %d] Czekam na wejscie do B...\n", moj_id);
    
    semafor_operacja(sem_id_global, SEM_SALA_B, -1);
    if (szczegoly) printf(" >> [Kandydat %d] Wszedlem do Sali B.\n", moj_id);

    if (rand() % 100 < 20) { 
         if (szczegoly) printf("    [Kandydat %d] Czekam na przygotowanie stanowiska (komisja B sie spoznia)...\n", moj_id);
         usleep(2000); 
    }
    
    int otrzymane_zadania[LICZBA_PYTAN_B];
    for(int i=0; i<LICZBA_PYTAN_B; i++) {
        otrzymane_zadania[i] = 100 + rand() % 50;
    }
    if (szczegoly) printf("    [Kandydat %d] Otrzymalem zadania. Przygotowuje sie (Ti)...\n", moj_id);

    usleep(1000000);

    if (szczegoly) printf("    [Kandydat %d] Czekam na weryfikacje zadan przez komisje...\n", moj_id);
    
    semafor_operacja(sem_id_global, SEM_KRZESLO_B, -1);

    int suma_pkt = 0;
    for (int i = 0; i < LICZBA_PYTAN_B; i++) {
        if (szczegoly) {
            drukuj_czas();
            printf("[Komisja B] [Kandydat %d] Egzaminator %d zleca zadanie praktyczne nr %d...\n", 
                   moj_id, i+1, otrzymane_zadania[i]);
        }

        usleep(300000); 
        int ocena = rand() % 101;
        suma_pkt += ocena;
        
        if (szczegoly) {
            drukuj_czas();
            printf("[Komisja B] [Kandydat %d] Członek nr %d ocenia zadanie %d -> %d%%\n", 
                   moj_id, i+1, otrzymane_zadania[i], ocena);
        }
    }

    ja->ocena_praktyka = suma_pkt / LICZBA_PYTAN_B;
    if (szczegoly) printf("    [Komisja B] [Kandydat %d] Przewodniczący ustalił ocenę końcową: %d%%\n", moj_id, ja->ocena_praktyka);

    semafor_operacja(sem_id_global, SEM_KRZESLO_B, 1);
    
    if (szczegoly) printf(" << [Kandydat %d] Koniec praktyki. Wychodzę.\n", moj_id);
    semafor_operacja(sem_id_global, SEM_SALA_B, 1);

    ja->status = STATUS_ZAKONCZYL;
    zakoncz_proces(0);

    return 0;
}