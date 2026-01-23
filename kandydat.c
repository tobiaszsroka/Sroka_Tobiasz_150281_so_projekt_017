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

    int szczegoly = (moj_id < 5) ? 1 : 0;

    // Weryfikacja matury
    if (ja->zdana_matura == 0) {
        printf("[Kandydat %d] Brak matury. Koncze.\n", moj_id);
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

        // Oczekiwanie na pytania
        if (rand() % 100 < 20) { 
             if (szczegoly) printf("    [Kandydat %d] Czekam na pytania (egzaminator sie spoznia)...\n", moj_id);
             usleep(500000);
        }

        //5 pytan
        int suma_pkt = 0;
        for (int i = 0; i < LICZBA_PYTAN_A; i++) {
            int id_egzaminatora = i;
            int id_tresci = rand() % 100;
            
            if (szczegoly) {
                drukuj_czas();
                printf("[Komisja A] Pytanie %d | Egzaminator %d | Zadaje tresc nr %d\n", i+1, id_egzaminatora, id_tresci);
            }

            usleep(100000 + rand()%200000); 

            int pkt = 0;
            if (rand() % 100 == 0) {
                if (szczegoly) {
                    drukuj_czas();
                    printf("[Komisja A] TIMEOUT! Dyskwalifikacja w pytaniu %d.\n", i+1);
                }
                pkt = 0;
            } else {
                pkt = rand() % 101;
                if (szczegoly) {
                    drukuj_czas();
                    printf("[Komisja A] Odpowiedz: OK | Egzaminator %d ocenia: %d pkt\n", id_egzaminatora, pkt);
                }
            }
            suma_pkt += pkt;
        }
        
        ja->ocena_teoria = suma_pkt / LICZBA_PYTAN_A;

        if (szczegoly) printf(" << [Kandydat %d] Wyszedlem z A. Srednia: %d%%\n", moj_id, ja->ocena_teoria);
        semafor_operacja(sem_id_global, SEM_SALA_A, 1);

        if (ja->ocena_teoria < 30) {
            ja->status = STATUS_OBLAL_TEORIE;
            zakoncz_proces(0);
        }
        ja->status = STATUS_ZDAL_TEORIE;
    }

    //Komisja B
    if (szczegoly) printf("[Kandydat %d] Czekam na wejscie do komisji B (Praktyka)...\n", moj_id);
    
    semafor_operacja(sem_id_global, SEM_SALA_B, -1);
    
    if (szczegoly) printf(" >> [Kandydat %d] Wszedlem do Sali B.\n", moj_id);

    //3 pytania 
    int suma_pkt = 0;
    for (int i = 0; i < LICZBA_PYTAN_B; i++) {
        int id_egzaminatora = i;
        int id_tresci = rand() % 200 + 500;
        
        if (szczegoly) {
            drukuj_czas();
            printf("[Komisja B] Pytanie %d | Egzaminator %d | Zadanie praktyczne nr %d\n", i+1, id_egzaminatora, id_tresci);
        }
        
        usleep(200000);

        int pkt = rand() % 101;
        if (szczegoly) {
            drukuj_czas();
            printf("[Komisja B] Wynik czastkowy: %d pkt\n", pkt);
        }
        suma_pkt += pkt;
    }

    ja->ocena_praktyka = suma_pkt / LICZBA_PYTAN_B;

    if (szczegoly) printf(" << [Kandydat %d] Koniec praktyki. Srednia: %d%%\n", moj_id, ja->ocena_praktyka);
    semafor_operacja(sem_id_global, SEM_SALA_B, 1);

    ja->status = STATUS_ZAKONCZYL;
    zakoncz_proces(0);

    return 0;
}