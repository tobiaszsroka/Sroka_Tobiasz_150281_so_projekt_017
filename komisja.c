#include "common.h"

int czy_pracowac = 1;
char *typ_komisji_global = "?"; 
int id_kolejki = -1;
PamiecDzielona *pamiec = NULL;
int sem_id_global = -1;

// Funkcja do wysylania wiadomosci przez kolejke komunikatow
void wyslij_komunikat(int id_kolejki, long typ_adresata, int dane) {
    Komunikat msg;
    msg.mtype = typ_adresata;
    msg.nadawca_pid = getpid();
    msg.dane = dane;
    
    if (msgsnd(id_kolejki, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        if (errno != EINTR && errno != EIDRM) {
            perror("[Komisja] Blad msgsnd");
        }
    }
}

void obsluga_ewakuacji(int sig) {
    if (sig == SIGUSR1) {
        if (pamiec != NULL && pamiec->ewakuacja == 1) {
            printf("\n!!! [Komisja %s] OTRZYMANO SYGNAL EWAKUACJI (SIGUSR1) !!!\n", typ_komisji_global);
        } else {
            printf("[Komisja %s] Dziekan zarządził koniec egzaminów. Zamykam biuro.\n", typ_komisji_global);
        }
        czy_pracowac = 0;
        exit(0);
    }
}

int losuj(int min, int max) {
    return min + rand() % (max - min + 1);
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Komisja] Błąd: Brak argumentu typu komisji!\n");
        return 1;
    }

    char *typ_komisji = argv[1];
    typ_komisji_global = typ_komisji;

    //Konczy prace komisji
    if (signal(SIGUSR1, obsluga_ewakuacji) == SIG_ERR) {
        perror("Blad signal SIGUSR1");
        exit(1);
    }

    //(CTRL+Z) ignoruj
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

    //Pamiec dzielona
    int id_pamieci = shmget(klucz, sizeof(PamiecDzielona), 0600);
    if (id_pamieci == -1) {
        perror("[Komisja] Blad shmget (czy Dziekan dziala?)");
        exit(1);
    }

    PamiecDzielona *pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (pamiec == (void*) -1) {
        perror("[Komisja] Blad shmat");
        exit(1);
    }

    //Pobieramy ID semaforow
    sem_id_global = semget(klucz, 0, 0); 
    if (sem_id_global == -1) { 
        perror("[Komisja] Blad semget"); 
        exit(1); 
    }

    //Pobieramy ID kolejki komunikatow
    id_kolejki = msgget(klucz, 0600);
    if (id_kolejki == -1) {
        perror("[Komisja] Blad msgget - brak kolejki komunikatow");
        exit(1);
    }

    // Konfiguracja pod typ komisji (A lub B)
    long moj_kanal_nasluchu = (strcmp(typ_komisji, "A") == 0) ? MSG_TYP_KOMISJA_A : MSG_TYP_KOMISJA_B;
    int liczba_czlonkow = (strcmp(typ_komisji, "A") == 0) ? 5 : 3;

    printf("[Komisja %s] (PID: %d) Czekam na kandydatów.\n", typ_komisji, getpid());

    Komunikat msg_odebrana;

	//Praca komisji
    while (czy_pracowac) {
        if (pamiec->ewakuacja == 1) {
            printf("[Komisja %s] Zauważono flagę ewakuacji!\n", typ_komisji);
             break;
        }

        //Czekamy na wiadomosc od studenta
        if (msgrcv(id_kolejki, &msg_odebrana, sizeof(msg_odebrana) - sizeof(long), moj_kanal_nasluchu, 0) == -1) {
            if (errno == EINTR) continue;
            if (errno == EIDRM) break;   
            perror("[Komisja] Blad msgrcv");
            break;
        }

        int pid_studenta = msg_odebrana.nadawca_pid;
        int id_studenta = msg_odebrana.dane;

        //Logika egzaminu
        printf("[Komisja %s] Przygotowuje pytania dla [kandydata %d] (PID %d)...\n", typ_komisji, id_studenta + 1, pid_studenta);
        usleep(losuj(10000, 50000)); 

        wyslij_komunikat(id_kolejki, pid_studenta, ETAP_PYTANIA);

        //Czekamy na odpowiedzi studenta
        if (msgrcv(id_kolejki, &msg_odebrana, sizeof(msg_odebrana) - sizeof(long), moj_kanal_nasluchu, 0) == -1) {
            if (errno != EINTR) perror("[Komisja] Blad msgrcv (odpowiedz)");
            break;
        }

        int suma_ocen = 0;
        for(int k=0; k<liczba_czlonkow; k++) {
            suma_ocen += losuj(0, 100); 
        }
        int ocena_finalna = suma_ocen / liczba_czlonkow;

        //Zapis do pamieci przez komisje        
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, -1);
        
        //Modyfikacja pamieci dzielonej
        if (moj_kanal_nasluchu == MSG_TYP_KOMISJA_A) {
            pamiec->studenci[id_studenta].ocena_teoria = ocena_finalna;
            if (ocena_finalna < 30) 
                pamiec->studenci[id_studenta].status = STATUS_OBLAL_TEORIE;
            else 
                pamiec->studenci[id_studenta].status = STATUS_ZDAL_TEORIE;
        } else {
            pamiec->studenci[id_studenta].ocena_praktyka = ocena_finalna;
            pamiec->studenci[id_studenta].status = STATUS_ZAKONCZYL;
        }
        
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, 1);

        wyslij_komunikat(id_kolejki, pid_studenta, ocena_finalna);
        printf("[Komisja %s] [Kandydat %d] PID %d oceniony na: %d%%\n", typ_komisji, id_studenta + 1, pid_studenta, ocena_finalna);
    }

    //Sprzatanie
    if (shmdt(pamiec) == -1) {
        perror("[Komisja] Blad shmdt");
    }
    
    printf("[Komisja %s] Koniec pracy.\n", typ_komisji);
    return 0;
}