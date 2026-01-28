#include "common.h"
#include <sys/time.h>

PamiecDzielona *pamiec_global = NULL;
int sem_id_global = 0;
int msg_id_global = -1;
int moj_id_global = 0;

// Struktura pomocnicza tylko dla kandydata, zeby pamietal kto mu zadal pytanie
typedef struct {
    int id_egzaminatora;
    int nr_pytania;
} PytanieSymulacja;

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

//Funkcja konczaca proces kandydata
void zakoncz_proces(int sig) {
    if (sig == SIGUSR1) {
        char msg[128];
        snprintf(msg, sizeof(msg), "\n!!! [Kandydat %d] SLYSZE ALARM! UCIEKAM! !!!\n", moj_id_global + 1);
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

        if (shmdt(pamiec_global) == -1) {
            perror("Blad shmdt");
        }
    }

    exit(0);    
}

//Funkcja do generowania unikalnych numerow pytan
int czy_byl_numer(int nr, int *tablica, int rozmiar) {
    for(int i=0; i<rozmiar; i++) {
        if(tablica[i] == nr) return 1;
    }
    return 0;
}

//Funkcja zdawania egzaminu(Komisja A i B)
int zdawaj_egzamin(int typ_komisji_msg, int sem_sala, int sem_krzeslo, char* nazwa_sali, int liczba_pytan) {
    Komunikat msg_wysylana, msg_odebrana;
    pid_t moj_pid = getpid();
    int szczegoly = 1;

    int liczba_egzaminatorow = (typ_komisji_msg == MSG_TYP_KOMISJA_A) ? 5 : 3;
    
    if (szczegoly) 
        printf("[Kandydat %d] Czekam na wejscie do %s...\n", moj_id_global + 1, nazwa_sali);

    semafor_operacja(sem_id_global, sem_sala, -1);

    if (szczegoly) 
        printf(" >> [Kandydat %d] Wszedlem do %s.\n", moj_id_global + 1, nazwa_sali);

    if (szczegoly) printf("    [Kandydat %d] Czekam na wolne krzeslo przed komisja...\n", moj_id_global + 1);

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
        printf("[Kandydat %d] Zglaszam sie do komisji...\n", moj_id_global + 1); }
    
    msg_wysylana.mtype = typ_komisji_msg;
    msg_wysylana.nadawca_pid = moj_pid;
    msg_wysylana.dane = moj_id_global;
    
    if (msgsnd(msg_id_global, &msg_wysylana, sizeof(msg_wysylana)-sizeof(long), 0) == -1) {
        semafor_operacja(sem_id_global, sem_krzeslo, 1);
        semafor_operacja(sem_id_global, sem_sala, 1);
        perror("Blad msgsnd start");
        return 0;
    }

    
    if (szczegoly) printf("    [Kandydat %d] Czekam az komisja przygotuje pytania...\n", moj_id_global + 1);

    if (msgrcv(msg_id_global, &msg_odebrana, sizeof(msg_odebrana)-sizeof(long), moj_pid, 0) == -1) {
        if (errno != EINTR) perror("Blad msgrcv pytania");
        return -1;
    }

    //Unikalni egzaminatorzy
    int dostepni_egzaminatorzy[liczba_egzaminatorow];
    for(int i=0; i<liczba_egzaminatorow; i++) dostepni_egzaminatorzy[i] = i;

    for (int i = 0; i < liczba_egzaminatorow; i++) {
        int r = rand() % liczba_egzaminatorow;
        int temp = dostepni_egzaminatorzy[i];
        dostepni_egzaminatorzy[i] = dostepni_egzaminatorzy[r];
        dostepni_egzaminatorzy[r] = temp;
    }

    //Struktura przechowujaca pytania
    PytanieSymulacja zestaw[liczba_pytan];
    int wylosowane_pytania[liczba_pytan];

    if (szczegoly) {
        drukuj_czas();
        printf("[%s] [Kandydat %d] Czekam na pytania...\n", nazwa_sali, moj_id_global + 1);
    }

    for (int i = 0; i < liczba_pytan; i++) {
        usleep(rand() % 40000 + 10000); 

        zestaw[i].id_egzaminatora = dostepni_egzaminatorzy[i];

        // Losujemy unikalny numer pytania
        int nr;
        do {
            nr = 1 + rand() % 50;
        } while(czy_byl_numer(nr, wylosowane_pytania, i));
        wylosowane_pytania[i] = nr;
        zestaw[i].nr_pytania = nr;

        if (szczegoly) {
            drukuj_czas();
            printf("[%s] Egzaminator %d zadał pytanie nr %d kandydatowi %d.\n", 
                   nazwa_sali, zestaw[i].id_egzaminatora + 1, zestaw[i].nr_pytania, moj_id_global + 1); 
        }
    }

    //Przygotowanie do odpowiedzi(Ti)
    if (szczegoly) {
        drukuj_czas();
        printf("[%s] [Kandydat %d] Otrzymałem wszystkie pytania. Przygotowuję się (czas Ti)...\n", nazwa_sali, moj_id_global + 1);
    }
    usleep(200000); 
    
    //Odpowiedz i ocena
    for (int i = 0; i < liczba_pytan; i++) {
        int ocena = rand() % 101;
        usleep(20000); 

        if (szczegoly) {
            drukuj_czas();
            printf("[%s] Egzaminator %d ocenil odpowiedz na pytanie nr %d kandydata %d na %d%%.\n", 
                   nazwa_sali, zestaw[i].id_egzaminatora + 1, zestaw[i].nr_pytania, moj_id_global + 1, ocena);
        }
    }

    if (szczegoly) printf("    [Kandydat %d] Odpowiedzi zakonczone. Przekazuje komisji do oceny...\n", moj_id_global + 1);
    
    msg_wysylana.mtype = typ_komisji_msg;
    msg_wysylana.nadawca_pid = moj_pid;
    msg_wysylana.dane = 0; 
    msgsnd(msg_id_global, &msg_wysylana, sizeof(msg_wysylana)-sizeof(long), 0);

    if (szczegoly) { drukuj_czas(); printf("[Kandydat %d] Czekam na werdykt...\n", moj_id_global + 1); }

    if (msgrcv(msg_id_global, &msg_odebrana, sizeof(msg_odebrana)-sizeof(long), moj_pid, 0) == -1) {
         if (errno != EINTR) perror("Blad msgrcv wynik");
         return -1;
    }
    
    int ocena_oficjalna = msg_odebrana.dane;

    if (szczegoly) {
        drukuj_czas(); 
        printf("[%s] [Kandydat %d] Przewodniczący ustalił ocenę końcową: %d%%\n", 
               nazwa_sali, moj_id_global + 1, ocena_oficjalna);
    }

    // Zwolnienie zasobow
    semafor_operacja(sem_id_global, sem_krzeslo, 1);
    if (szczegoly) printf(" << [Kandydat %d] Opuszczam %s.\n", moj_id_global + 1, nazwa_sali);
    semafor_operacja(sem_id_global, sem_sala, 1);

    return ocena_oficjalna;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Kandydat %d] Błąd krytyczny: Nie podano ID kandydata w argumentach!\n", moj_id_global + 1);
        exit(1);
    }

    //Konwersja i walidacja ID
    int moj_id = atoi(argv[1]);

    if (moj_id == 0 && strcmp(argv[1], "0") != 0) {
        fprintf(stderr, "[Kandydat] BLAD: Nieprawidlowe ID '%s'\n", argv[1]);
        fprintf(stderr, "ID musi byc liczba calkowita (0-%d)\n", MAX_KANDYDATOW - 1);
        exit(1);
    }

    if (moj_id < 0 || moj_id >= MAX_KANDYDATOW) {
        fprintf(stderr, "[Kandydat] BLAD: ID %d poza zakresem!\n", moj_id);
        fprintf(stderr, "Poprawny zakres: 0-%d\n", MAX_KANDYDATOW - 1);
        exit(1);
    }
    moj_id_global = moj_id;

    //SIGTSTP(CTRL+Z)
    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) { 
        perror("Signal error"); 
        exit(1); }

    // SIGUSR1-ewakuacja (wyslij przez dziekana)
    if (signal(SIGUSR1, zakoncz_proces) == SIG_ERR) { 
        perror("Signal error"); 
        exit(1); }

    srand(time(NULL) ^ getpid());

    //Podlaczenie do zasobow IPC
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    if (klucz == -1) 
        report_error_and_exit("Blad ftok");

    //Pamiec dzielona
    int id_pamieci = shmget(klucz, 0, 0);
    if (id_pamieci == -1) 
        report_error_and_exit("Blad shmget");

    //Semafory
    sem_id_global = semget(klucz, 0, 0);
    if (sem_id_global == -1) 
        report_error_and_exit("Blad semget");

    //Kolejka komunikatow
    msg_id_global = msgget(klucz, 0);
    if (msg_id_global == -1) 
        { perror("Blad msgget"); 
            exit(1); 
        }

    //Podlaczenie pamieci
    pamiec_global = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (pamiec_global == (void*) -1) 
        report_error_and_exit("Blad shmat");

    //Pobranie danych kandydata
    KandydatDane *ja = &pamiec_global->studenci[moj_id];
    ja->pid = getpid();
    int szczegoly = 1; 

    if (szczegoly) {
        drukuj_czas();
        printf("[Kandydat %d] Czekam przed wejściem. (Matura: %s)\n", moj_id + 1, ja->zdana_matura ? "TAK" : "NIE");
    }

    if (ja->zdana_matura == 0) {
        if(szczegoly) printf("[Kandydat %d] Brak matury. Wracam do domu.\n", moj_id + 1);
        zakoncz_proces(0);
    }

    //Komisja A(Teoria)
     if (!ja->powtarza_egzamin) {
        int ocena = zdawaj_egzamin(MSG_TYP_KOMISJA_A, SEM_SALA_A, SEM_KRZESLO_A, "Sala A", LICZBA_PYTAN_A);
        
        if (ocena == -1) zakoncz_proces(SIGUSR1); //Ewakuacja

        if (ja->status == STATUS_OBLAL_TEORIE) {
            if(szczegoly) printf("[Kandydat %d] Oblalem teorie (%d%%). Koniec.\n", moj_id + 1, ocena);
            zakoncz_proces(0);
        }
    } else {
        if (szczegoly) printf("[Kandydat %d] Powtarzam rok (teoria zaliczona). Ide do Sali B.\n", moj_id + 1);
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, -1);
        ja->status = STATUS_ZDAL_TEORIE;
        semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, 1);
    }

    //Komisja B(Praktyka)
    if (ja->status == STATUS_ZDAL_TEORIE) {
        int ocena = zdawaj_egzamin(MSG_TYP_KOMISJA_B, SEM_SALA_B, SEM_KRZESLO_B, "Sala B", LICZBA_PYTAN_B);
        
        if (ocena == -1) zakoncz_proces(SIGUSR1);
        
        if(szczegoly) printf("[Kandydat %d] Koniec egzaminu (Praktyka: %d%%). Wychodze.\n", moj_id + 1, ocena);
    }

    zakoncz_proces(0);
    return 0;
}