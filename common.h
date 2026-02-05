#ifndef COMMON_H
#define COMMON_H
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <stdarg.h>

//Stale projektowe
#define PROJECT_ID 'E'
#define PATH_NAME "."

// KOLORY ANSI DO TERMINALA
#define KOLOR_RESET   "\033[0m"
#define KOLOR_CZERWONY "\033[1;31m" // Błędy / Odrzucenia
#define KOLOR_ZIELONY  "\033[1;32m" // Sukcesy
#define KOLOR_ZOLTY    "\033[1;33m" // Dziekan
#define KOLOR_NIEBIESKI "\033[1;34m" // Kandydat
#define KOLOR_MAGENTA  "\033[1;35m" // Komisja A
#define KOLOR_CYJAN    "\033[1;36m" // Komisja B

//Limity SYSTEMOWE
#define MAX_KANDYDATOW 1200
#define MIEJSCA_NA_UCZELNI 120
#define MAX_W_SALI_A 3
#define MAX_W_SALI_B 3
#define LICZBA_PYTAN_A 5
#define LICZBA_PYTAN_B 3

//Indeksy semaforow
#define SEM_DOSTEP_PAMIEC 0
#define SEM_SALA_A 1
#define SEM_SALA_B 2
#define SEM_KRZESLO_A 3
#define SEM_KRZESLO_B 4
#define SEM_STDOUT 5
#define LICZBA_SEMAFOROW 6

//Statusy kandydata
#define STATUS_NOWY 0
#define STATUS_ODRZUCONY_MATURA 1
#define STATUS_DOPUSCZONY 2
#define STATUS_OBLAL_TEORIE 3
#define STATUS_ZDAL_TEORIE 4
#define STATUS_ZAKONCZYL 5

//Typy komunikatow dla komisji
#define MSG_TYP_KOMISJA_A 1
#define MSG_TYP_KOMISJA_B 2
#define ETAP_PYTANIA 1
#define ETAP_OCENA 2

//Struktura kandydata w pamieci dzielonej
typedef struct {
    pid_t pid;
    int id_kandydata;
    bool zdana_matura;
    bool powtarza_egzamin;
    int ocena_teoria;
    int ocena_praktyka;
    int status;
} KandydatDane;

//Struktura pamieci dzielonej
typedef struct {
    KandydatDane studenci[MAX_KANDYDATOW];
    int liczba_kandydatow;
    int ewakuacja; // Flaga ewakuacji (dostępna dla wszystkich procesów)
    int studenci_zakonczeni; // Licznik atomowy (chroniony semaforem
} PamiecDzielona;

//Struktura komunikatów IPC
typedef struct {
    long mtype; //Adresat
    int nadawca_pid; //Nadawca
    int dane; //Tresc 
} Komunikat;

//---Funkcje pomocnicze---

//Funkcja do obslugi bledow
void report_error_and_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

//Wrapper na semop (Operacje P i V)
static void semafor_operacja(int sem_id, int sem_num, int op) {
    struct sembuf bufor;
    bufor.sem_num = sem_num;
    bufor.sem_op = op;
    bufor.sem_flg = SEM_UNDO; //Zapobiega blokadzie jesli proces padnie
    
    while (semop(sem_id, &bufor, 1) == -1) {
        if (errno != EINTR && errno != EIDRM && errno != EINVAL) {
             perror("Blad semop");
             break;
        }
        if (errno == EIDRM || errno == EINVAL) break;
    }
}

//Funkcja do synchronizacji i kolorowania wyjscia terminala
static void loguj(int sem_id, const char *kolor, const char *format, ...) {
    semafor_operacja(sem_id, SEM_STDOUT, -1);
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    struct tm *tm_info = localtime(&ts.tv_sec);
    char buffer[20];
    strftime(buffer, 20, "%H:%M:%S", tm_info);

    //Wypisz prefix (Czas + Kolor)
    printf("[%s.%03ld] %s", buffer, ts.tv_nsec / 1000000, kolor);

    //Obsluga zmiennej liczby argumentow
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s", KOLOR_RESET);

    semafor_operacja(sem_id, SEM_STDOUT, 1);
}

#endif
