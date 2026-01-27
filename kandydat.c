#include "common.h"
#include <sys/time.h>

PamiecDzielona *pamiec_global = NULL;
int sem_id_global = 0;
int msg_id_global = -1;
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
        if (errno != EINTR && errno != EIDRM) perror("Blad semop");
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

//Funkcja zdawania egzaminu
int zdawaj_egzamin(int typ_komisji_msg, int sem_sala, int sem_krzeslo, char* nazwa_sali, int liczba_pytan) {
    Komunikat msg_wysylana, msg_odebrana;
    pid_t moj_pid = getpid();
    int szczegoly = 1;

    if (szczegoly) printf("[Kandydat %d] Czekam na wejscie do %s...\n", moj_id_global, nazwa_sali);

    semafor_operacja(sem_id_global, sem_sala, -1);
    if (szczegoly) printf(" >> [Kandydat %d] Wszedlem do %s.\n", moj_id_global, nazwa_sali);

    if (szczegoly) printf("    [Kandydat %d] Czekam na wolne krzeslo przed komisja...\n", moj_id_global);
    semafor_operacja(sem_id_global, sem_krzeslo, -1);

    // Sprawdzenie ewakuacji
    if (pamiec_global->ewakuacja) {
        semafor_operacja(sem_id_global, sem_krzeslo, 1);
        semafor_operacja(sem_id_global, sem_sala, 1);
        return -1;
    }

    //Zgloszenie sie do komisji
    if (szczegoly) { 
        drukuj_czas(); 
        printf("[Kandydat %d] Zglaszam sie do komisji...\n", moj_id_global); }
    
    msg_wysylana.mtype = typ_komisji_msg;
    msg_wysylana.nadawca_pid = moj_pid;
    msg_wysylana.dane = moj_id_global;
    
    if (msgsnd(msg_id_global, &msg_wysylana, sizeof(msg_wysylana)-sizeof(long), 0) == -1) {
        semafor_operacja(sem_id_global, sem_krzeslo, 1);
        semafor_operacja(sem_id_global, sem_sala, 1);
        perror("Blad msgsnd start");
        return 0;
    }

    
    if (szczegoly) printf("    [Kandydat %d] Czekam az komisja przygotuje pytania...\n", moj_id_global);
    if (msgrcv(msg_id_global, &msg_odebrana, sizeof(msg_odebrana)-sizeof(long), moj_pid, 0) == -1) {
        if (errno != EINTR) perror("Blad msgrcv pytania");
        return -1;
    }

    int otrzymane_pytania[liczba_pytan];
    for(int i=0; i<liczba_pytan; i++) {
        otrzymane_pytania[i] = 1 + rand() % 50; 
    }
    if (szczegoly) printf("    [Kandydat %d] Otrzymalem %d pytan. Rozpoczynam przygotowanie (Ti)...\n", moj_id_global, liczba_pytan);

    usleep(1000000);

    for (int i = 0; i < liczba_pytan; i++) {
        if (rand() % 100 < 10) {
             if (szczegoly) { drukuj_czas(); printf("    [Kandydat %d] Egzaminator szuka pytań... (Czekam)\n", moj_id_global); }
             usleep(200000);
        }

        if (szczegoly) {
            drukuj_czas();
            printf("[%s] [Kandydat %d] Odpowiadam na pytanie nr %d...\n", nazwa_sali, moj_id_global, otrzymane_pytania[i]);
        }
        usleep(100000);
    }
    
    if (szczegoly) printf("    [Kandydat %d] Odpowiedzi zakonczone. Przekazuje komisji do oceny...\n", moj_id_global);
    
    msg_wysylana.mtype = typ_komisji_msg;
    msg_wysylana.nadawca_pid = moj_pid;
    msg_wysylana.dane = 0; 
    msgsnd(msg_id_global, &msg_wysylana, sizeof(msg_wysylana)-sizeof(long), 0);

    if (szczegoly) { drukuj_czas(); printf("[Kandydat %d] Czekam na werdykt...\n", moj_id_global); }

    if (msgrcv(msg_id_global, &msg_odebrana, sizeof(msg_odebrana)-sizeof(long), moj_pid, 0) == -1) {
         if (errno != EINTR) perror("Blad msgrcv wynik");
         return -1;
    }
    
    int ocena_oficjalna = msg_odebrana.dane;

    if (szczegoly) {
        drukuj_czas(); 
        printf("[%s] [Kandydat %d] Przewodniczący ustalił ocenę końcową: %d%%\n", 
               nazwa_sali, moj_id_global, ocena_oficjalna);
    }

    // Zwolnienie zasobow
    semafor_operacja(sem_id_global, sem_krzeslo, 1);
    if (szczegoly) printf(" << [Kandydat %d] Opuszczam %s.\n", moj_id_global, nazwa_sali);
    semafor_operacja(sem_id_global, sem_sala, 1);

    return ocena_oficjalna;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Kandydat %d] Błąd krytyczny: Nie podano ID kandydata w argumentach!\n", moj_id_global);
        exit(1);
    }

    int moj_id = atoi(argv[1]);
    moj_id_global = moj_id;

    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) { 
        perror("Signal error"); 
        exit(1); }

    if (signal(SIGUSR1, zakoncz_proces) == SIG_ERR) { 
        perror("Signal error"); 
        exit(1); }

    srand(time(NULL) ^ getpid());

    //Pobranie zasobow
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    if (klucz == -1) report_error_and_exit("Blad ftok");

    int id_pamieci = shmget(klucz, 0, 0);
    if (id_pamieci == -1) report_error_and_exit("Blad shmget");

    sem_id_global = semget(klucz, 0, 0);
    if (sem_id_global == -1) report_error_and_exit("Blad semget");

    msg_id_global = msgget(klucz, 0);
    if (msg_id_global == -1) { perror("Blad msgget"); exit(1); }

    pamiec_global = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (pamiec_global == (void*) -1) report_error_and_exit("Blad shmat");

    KandydatDane *ja = &pamiec_global->studenci[moj_id];
    ja->pid = getpid();
    int szczegoly = 1; 

   if (ja->zdana_matura == 0) {
        if(szczegoly) printf("[Kandydat %d] Brak matury. Wracam do domu.\n", moj_id);
        zakoncz_proces(0);
    }

    //Komisja A(Teoria)
     if (!ja->powtarza_egzamin) {
        int ocena = zdawaj_egzamin(MSG_TYP_KOMISJA_A, SEM_SALA_A, SEM_KRZESLO_A, "Sala A", LICZBA_PYTAN_A);
        
        if (ocena == -1) zakoncz_proces(SIGUSR1); //Ewakuacja

        // Zapis w pamieci
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, -1);
        ja->ocena_teoria = ocena;
        if (ocena < 30) ja->status = STATUS_OBLAL_TEORIE;
        else ja->status = STATUS_ZDAL_TEORIE;
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, 1);

        if (ja->status == STATUS_OBLAL_TEORIE) {
            if(szczegoly) printf("[Kandydat %d] Oblalem teorie (%d%%). Koniec.\n", moj_id, ocena);
            zakoncz_proces(0);
        }
    } else {
        if (szczegoly) printf("[Kandydat %d] Powtarzam rok (teoria zaliczona). Ide do Sali B.\n", moj_id);
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, -1);
        ja->status = STATUS_ZDAL_TEORIE;
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, 1);
    }

    //Komisja B(Praktyka)
    if (ja->status == STATUS_ZDAL_TEORIE) {
        int ocena = zdawaj_egzamin(MSG_TYP_KOMISJA_B, SEM_SALA_B, SEM_KRZESLO_B, "Sala B", LICZBA_PYTAN_B);
        
        if (ocena == -1) zakoncz_proces(SIGUSR1);

        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, -1);
        ja->ocena_praktyka = ocena;
        ja->status = STATUS_ZAKONCZYL;
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, 1);
        
        if(szczegoly) printf("[Kandydat %d] Koniec egzaminu (Praktyka: %d%%). Wychodze.\n", moj_id, ocena);
    }

    zakoncz_proces(0);
    return 0;
}