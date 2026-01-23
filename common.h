#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

//Stale projektowe
#define PROJECT_ID 'E'
#define PATH_NAME "."

//Limity
#define MAX_KANDYDATOW 30
#define MIEJSCA_NA_UCZELNI 10
#define MAX_W_SALI_A 3
#define MAX_W_SALI_B 3
#define LICZBA_PYTAN_A 5
#define LICZBA_PYTAN_B 3

//Statusy semaforow
#define SEM_DOSTEP_PAMIEC 0
#define SEM_SALA_A 1
#define SEM_SALA_B 2
#define SEM_KRZESLO_A 3
#define SEM_KRZESLO_B 4
#define LICZBA_SEMAFOROW 5

//Statusy kandydata
#define STATUS_NOWY 0
#define STATUS_ODRZUCONY_MATURA 1
#define STATUS_DOPUSCZONY 2
#define STATUS_OBLAL_TEORIE 3
#define STATUS_ZDAL_TEORIE 4
#define STATUS_ZAKONCZYL 5

//Struktura kandydata
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
    int ewakuacja;
    int studenci_zakonczeni;
} PamiecDzielona;

//Funkcja do obslugi bledow
void report_error_and_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

#endif
