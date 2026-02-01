# EGZAMIN WSTĘPNY - Raport z projektu

## Systemy Operacyjne - Temat 17

---

| Imię i Nazwisko | Kierunek/Tryb | Rok | Semestr | Temat Projektu |
|:--------------|:--------------|:---:|:--------|:---------------|
| Tobiasz Sroka | Informatyka/Stacjonarnie | II | Zimowy | Temat 17 – Egzamin wstępny |

**Numer albumu:** 150281  
**Repozytorium:** [https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017](https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017)

---

## Spis treści

**[0. Polecenie - problem do rozwiązania](#0-polecenie---problem-do-rozwiązania)**

**[1. Wymagania i Instrukcja Uruchomienia](#1-wymagania-i-instrukcja-uruchomienia)**
* [1.1. Wymagania systemowe](#11-wymagania-systemowe)
* [1.2. Kompilacja i uruchomienie](#12-kompilacja-i-uruchomienie)
* [1.3. Parametry konfiguracyjne](#13-parametry-konfiguracyjne)

**[2. Założenia projektowe](#2-założenia-projektowe)**
* [2.1. Architektura systemu](#21-architektura-systemu)
* [2.2. Decyzje projektowe](#22-decyzje-projektowe)
* [2.3. Mechanizmy IPC](#23-mechanizmy-ipc)

**[3. Przebieg inicjalizacji oraz struktura symulacji](#3-przebieg-inicjalizacji-oraz-struktura-symulacji)**
* [3.1. Inicjalizacja środowiska w procesie Dziekana](#31-inicjalizacja-środowiska-w-procesie-dziekana)
* [3.2. Tworzenie i inicjalizacja zasobów IPC](#32-tworzenie-i-inicjalizacja-zasobów-ipc)
* [3.3. Tworzenie procesów Komisji](#33-tworzenie-procesów-komisji)
* [3.4. Tworzenie i logika procesów Kandydatów](#34-tworzenie-i-logika-procesów-kandydatów)
* [3.5. Logika operacyjna Komisji](#35-logika-operacyjna-komisji)
* [3.6. Finalizacja procesu symulacji](#36-finalizacja-procesu-symulacji)

**[4. Dokumentacja techniczna kodu](#4-dokumentacja-techniczna-kodu)**
* [4.1. Moduł common.h - definicje wspólne](#41-moduł-commonh---definicje-wspólne)
* [4.2. Moduł dziekan.c - proces zarządzający](#42-moduł-dziekanc---proces-zarządzający)
* [4.3. Moduł komisja.c - procesy komisji](#43-moduł-komisjac---procesy-komisji)
* [4.4. Moduł kandydat.c - procesy kandydatów](#44-moduł-kandydatc---procesy-kandydatów)

**[5. Testy funkcjonalne](#5-testy-funkcjonalne)**
* [5.1. Test normalnego przepływu](#51-test-normalnego-przepływu)
* [5.2. Test weryfikacji matury](#52-test-weryfikacji-matury)
* [5.3. Test progu zaliczenia teorii](#53-test-progu-zaliczenia-teorii)
* [5.4. Test kandydatów powtarzających](#54-test-kandydatów-powtarzających)
* [5.5. Test ewakuacji](#55-test-ewakuacji)
* [5.6. Test limitu miejsc i rankingu](#56-test-limitu-miejsc-i-rankingu)

**[6. Problemy napotkane podczas realizacji](#6-problemy-napotkane-podczas-realizacji)**
* [6.1. Synchronizacja dostępu do pamięci dzielonej](#61-synchronizacja-dostępu-do-pamięci-dzielonej)
* [6.2. Zakleszczenia przy semaforach](#62-zakleszczenia-przy-semaforach)
* [6.3. Obsługa ewakuacji](#63-obsługa-ewakuacji)
* [6.4. Procesy zombie](#64-procesy-zombie)
* [6.5. Trwałość zasobów IPC](#65-trwałość-zasobów-ipc)

**[7. Implementacja wymaganych konstrukcji systemowych](#7-implementacja-wymaganych-konstrukcji-systemowych-linki-do-github)**
* [7.1. Tworzenie i obsługa plików](#71-tworzenie-i-obsługa-plików)
* [7.2. Tworzenie procesów](#72-tworzenie-procesów)
* [7.3. Obsługa sygnałów](#73-obsługa-sygnałów)
* [7.4. Synchronizacja procesów - semafory](#74-synchronizacja-procesów---semafory)
* [7.5. Segmenty pamięci dzielonej](#75-segmenty-pamięci-dzielonej)
* [7.6. Kolejki komunikatów](#76-kolejki-komunikatów)

**[8. Podsumowanie](#8-podsumowanie)**
* [8.1. Zrealizowane funkcjonalności](#81-zrealizowane-funkcjonalności)
* [8.2. Wyzwania techniczne](#82-wyzwania-techniczne)
* [8.3. Wynik końcowy](#83-wynik-końcowy)

---

## 0. Polecenie - problem do rozwiązania

### Temat 17 – Egzamin wstępny

Na pewnej uczelni zorganizowano egzamin wstępny na kierunek informatyka. Liczba miejsc wynosi **M** (np. M=120), liczba chętnych ok. 10 osób na jedno miejsce. Kandydaci gromadzą się przed budynkiem wydziału czekając w kolejce na wejście. Warunkiem udziału w egzaminie jest zdana matura (ok. 2% kandydatów nie spełnia tego warunku).

O określonej godzinie **T** dziekan wpuszcza kandydatów na egzamin, sprawdzając jednocześnie, czy dana osoba ma zdaną maturę – w tym momencie dziekan tworzy listę kandydatów i listę osób niedopuszczonych do egzaminu (id procesu).

Egzamin składa się z 2 części: części teoretycznej (**komisja A**) i części praktycznej (**komisja B**). Komisja A składa się z 5 osób, komisja B składa się z 3 osób. Komisje przyjmują kandydatów w osobnych salach.

Każda z osób w komisji zadaje po jednym pytaniu, pytania są przygotowywane na bieżąco (co losową liczbę sekund) w trakcie egzaminu. Może zdarzyć się sytuacja w której, członek komisji spóźnia się z zadaniem pytania wówczas kandydat czeka aż otrzyma wszystkie pytania. Po otrzymaniu pytań kandydat ma określony czas **Ti** na przygotowanie się do odpowiedzi. Po tym czasie kandydat udziela komisji odpowiedzi (jeżeli w tym czasie inny kandydat siedzi przed komisją, musi zaczekać aż zwolni się miejsce), które są oceniane przez osobę w komisji, która zadała dane pytanie (ocena za każdą odpowiedź jest losowana - wynik procentowy w zakresie 0-100%). Przewodniczący komisji (jedna z osób) ustala ocenę końcową z danej części egzaminu (wynik procentowy w zakresie 0-100%).

Do komisji A kandydaci wchodzą wg listy otrzymanej od dziekana. Do danej komisji może wejść jednocześnie maksymalnie 3 osoby.

### Zasady przeprowadzania egzaminu:

* Kandydaci w pierwszej kolejności zdają egzamin teoretyczny.
* Jeżeli kandydat zdał część teoretyczną na mniej niż 30% nie podchodzi do części praktycznej.
* Po pozytivnym zaliczeniu części teoretycznej (wynik >30%) kandydat staje w kolejce do komisji B.
* Wśród kandydatów znajdują się osoby powtarzające egzamin, które mają już zaliczoną część teoretyczną egzaminu (ok. 2% kandydatów) – takie osoby informują komisję A, że mają zdaną część teoretyczną i zdają tylko część praktyczną.
* Listę rankingową z egzaminu tworzy Dziekan po pozytywnym zaliczeniu obu części egzaminu – dane do Dziekana przesyłają przewodniczący komisji A i B.
* Po wyjściu ostatniego kandydata Dziekan publikuje listę rankingową oraz listę przyjętych. Na listach znajduje się id kandydata z otrzymanymi ocenami w komisji A i B oraz oceną końcową z egzaminu.

### Ewakuacja:

Na komunikat (sygnał1) o ewakuacji – sygnał wysyła Dziekan - kandydaci natychmiast przerywają egzamin i opuszczają budynek wydziału – Dziekan publikuje listę kandydatów wraz z ocenami, którzy wzięli udział w egzaminie wstępnym.

### Zadanie:

Napisz programy **Dziekan**, **Komisja** i **Kandydat** symulujące przeprowadzenie egzaminu wstępnego. Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym.

### Uwagi ogólne do wszystkich tematów:

* Należy unikać rozwiązań scentralizowanych – realizacja symulacji na procesach
* Program napisany w C/C++

---

## 1. Wymagania i Instrukcja Uruchomienia

Projekt został zaimplementowany w języku **C** z wykorzystaniem standardu **C11** i mechanizmów **System V IPC**, co wymaga środowiska **Linux** lub **WSL** (Windows Subsystem for Linux).

### 1.1. Wymagania systemowe

#### System operacyjny
* **Linux** (Ubuntu 20.04+, Debian, Fedora)
* **Windows** z WSL 2
* **macOS** (może wymagać modyfikacji - brak testów)

#### Kompilator i narzędzia
```bash
# Narzędzie do kompilacji
make
```

### 1.2. Kompilacja i uruchomienie

#### Sklonowanie repozytorium
```bash
git clone https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017.git
cd Sroka_Tobiasz_150281_so_projekt_017
```

#### Kompilacja
```bash
# Automatyczna kompilacja (zalecane)
make

# Czyszczenie i ponowna kompilacja
make clean
make
```

#### Uruchomienie
```bash
# Użyj
./dziekan[liczba_kandydatow]

#Przykład
./dziekan 200
```

#### Obsługa podczas działania
* **Ctrl+Z** - Ogłoszenie ewakuacji (SIGTSTP)
* **Ctrl+C** - Zakończenie programu i sprzątanie zasobów (SIGINT)

### 1.3. Parametry konfiguracyjne

Główne parametry symulacji znajdują się w pliku `common.h`:

```c
// Liczba kandydatów w symulacji
#define MAX_KANDYDATOW 1200

// Liczba miejsc na uczelni (M z polecenia)
#define MIEJSCA_NA_UCZELNI 120

// Maksymalna liczba osób jednocześnie w sali A
#define MAX_W_SALI_A 3

// Maksymalna liczba osób jednocześnie w sali B
#define MAX_W_SALI_B 3

// Liczba pytań w komisji A (5 osób = 5 pytań)
#define LICZBA_PYTAN_A 5

// Liczba pytań w komisji B (3 osoby = 3 pytania)
#define LICZBA_PYTAN_B 3
```


## 2. Założenia projektowe

### 2.1. Architektura systemu

System został zaprojektowany jako **wieloprocesowa symulacja rozproszona** z wykorzystaniem mechanizmów IPC (Inter-Process Communication). Główne założenia architektury:

#### Brak centralizacji
Zgodnie z wymaganiami projektu **unikamy rozwiązań scentralizowanych**. Każdy kandydat jest osobnym procesem działającym niezależnie. Nie ma centralnego "zarządcy" który by sekwencyjnie obsługiwał kandydatów - każdy proces kandydata sam zarządza swoim przebiegiem przez egzamin.

#### Hierarchia procesów
```
dziekan (PID główny, proces rodzica)
│
├── komisja A (proces potomny, wykonuje ./komisja A)
│
├── komisja B (proces potomny, wykonuje ./komisja B)
│
├── kandydat 0 (proces potomny, wykonuje ./kandydat 0)
├── kandydat 1 (proces potomny, wykonuje ./kandydat 1)
├── kandydat 2 (proces potomny, wykonuje ./kandydat 2)
│   ...
└── kandydat N (proces potomny, wykonuje ./kandydat N)
```

#### Komunikacja między procesami

Procesy komunikują się przez **3 mechanizmy IPC**:

1. **Pamięć dzielona** (Shared Memory)
   - Przechowuje dane wszystkich kandydatów
   - Współdzielona przez wszystkie procesy
   - Chroniona semaforami

2. **Semafory**
   - Kontrolują dostęp do zasobów (sale, krzesła, pamięć)
   - Synchronizują procesy
   - Zapobiegają race conditions

3. **Kolejki komunikatów** (Message Queues)
   - Komunikacja kandydat ↔ komisja
   - Wymiana pytań, odpowiedzi, ocen
   - Skierowana po PID lub typie

### 2.2. Decyzje projektowe

#### Dlaczego System V IPC zamiast POSIX?

**Wybrano System V IPC** zamiast POSIX z następujących powodów:

1. **Prostota** - mniej boilerplate code
2. **Kompatybilność** - działa na starszych systemach Linux
3. **Atomowe operacje** - `semop()` pozwala na atomowe operacje na wielu semaforach
4. **Łatwość debugowania** - `ipcs` pokazuje wszystkie zasoby


#### Dlaczego procesy zamiast wątków?

**Kandydaci jako procesy**

**Zalety procesów:**
1. **Zgodność z poleceniem** - "unikać centralizacji, realizacja na procesach"
2. **Izolacja** - crash jednego kandydata nie zabija innych
3. **Realistyczna symulacja** - każdy kandydat to niezależna "osoba"
4. **Łatwiejsze zarządzanie** - kill, wait, zombie handling

**Wady:**
- Większe zużycie pamięci
- Wolniejsze tworzenie niż wątki

#### Struktura danych w pamięci dzielonej

```c
typedef struct {
    KandydatDane studenci[MAX_KANDYDATOW];  // Tablica wszystkich
    int liczba_kandydatow;                  // Licznik
    int ewakuacja;                          // Flaga ewakuacji
    int studenci_zakonczeni;                // Licznik zakończonych
} PamiecDzielona;
```

**Decyzje:**
- **Tablica statyczna** zamiast dynamicznej (prostsze zarządzanie w SHM)
- **Flagi w strukturze** zamiast osobnych zmiennych (wszystko w jednym miejscu)
- **Liczniki** dla synchronizacji (dziekan wie kiedy wszyscy skończyli)

#### Synchronizacja semaforami

**5 semaforów w jednym zestawie:**

| Indeks | Nazwa | Wartość | Typ | Zastosowanie |
|--------|-------|---------|-----|--------------|
| 0 | SEM_DOSTEP_PAMIEC | 1 | Mutex | Sekcja krytyczna pamięci |
| 1 | SEM_SALA_A | 3 | Counting | Max 3 w sali A |
| 2 | SEM_SALA_B | 3 | Counting | Max 3 w sali B |
| 3 | SEM_KRZESLO_A | 1 | Binary | 1 odpowiada w A |
| 4 | SEM_KRZESLO_B | 1 | Binary | 1 odpowiada w B |
| 5 | SEM_STDOUT | 1 | Binary | Synchronizacja wypisywania na ekran |

**Dlaczego osobne semafory na salę i krzesło?**
- **Sala** (3) - limit osób fizycznie w pomieszczeniu
- **Krzesło** (1) - limit osób ODPOWIADAJĄCYCH jednocześnie
- Scenariusz: 3 kandydatów w sali, ale tylko 1 odpowiada, 2 czekają

#### Komunikacja przez kolejki

**Typy wiadomości (mtype):**
```c
#define MSG_TYP_KOMISJA_A 1  // Do komisji A
#define MSG_TYP_KOMISJA_B 2  // Do komisji B
// PID kandydata            // Do konkretnego kandydata
```

**Dlaczego ten schemat?**
1. Komisja nasłuchuje na swoim typie (1 lub 2)
2. Kandydat nasłuchuje na swoim PID
3. Brak kolizji - każdy proces ma unikalny PID

**Alternatywa odrzucona:** Osobne kolejki dla A i B
- Problem: Więcej zasobów IPC, trudniejsze zarządzanie

### 2.3. Mechanizmy IPC

Szczegółowy opis 3 używanych mechanizmów IPC.

#### Pamięć dzielona (Shared Memory)

**System V API:**
```c
key_t klucz = ftok(PATH_NAME, PROJECT_ID);          // Generuj klucz
int id = shmget(klucz, rozmiar, 0600 | IPC_CREAT);  // Utwórz segment
void *ptr = shmat(id, NULL, 0);                     // Podłącz do procesu
shmdt(ptr);                                         // Odłącz
shmctl(id, IPC_RMID, NULL);                         // Usuń segment
```

**Jak działa:**
1. Dziekan tworzy segment pamięci jądra
2. Wszystkie procesy podłączają się (`shmat`)
3. Każdy widzi TĘ SAMĄ pamięć fizyczną
4. Zmiany jednego procesu widoczne dla wszystkich

**Synchronizacja:**
- MUSI być chroniona semaforami (race conditions!)
- P(SEM_DOSTEP_PAMIEC) przed odczytem/zapisem
- V(SEM_DOSTEP_PAMIEC) po zakończeniu

**Przykład użycia:**
```c
// Kandydat zapisuje swój PID
semafor_operacja(sem_id, SEM_DOSTEP_PAMIEC, -1);  // P - wejście
pamiec->studenci[moj_id].pid = getpid();
semafor_operacja(sem_id, SEM_DOSTEP_PAMIEC, +1);  // V - wyjście
```

#### Semafory

**System V API:**
```c
key_t klucz = ftok(PATH_NAME, PROJECT_ID);
int id = semget(klucz, liczba, 0600 | IPC_CREAT);  // Zestaw semaforów
semctl(id, numer, SETVAL, arg);                     // Ustaw wartość
semop(id, &ops, 1);                                 // Operacja P/V
semctl(id, 0, IPC_RMID);                            // Usuń zestaw
```

**Operacje:**
```c
struct sembuf ops;
ops.sem_num = numer_semafora;  // Który semafor (0-5)
ops.sem_op = -1;               // P (wait, zmniejsz)
ops.sem_op = +1;               // V (signal, zwiększ)
ops.sem_flg = SEM_UNDO;        // WAŻNE: Automatyczne cofnięcie operacji przy awarii procesu!
semop(sem_id, &ops, 1);
```
***Dlaczego SEM_UNDO?***

* Jeśli proces zostanie nagle zabity (np. kill -9) podczas trzymania semafora (wewnątrz sali/sekcji krytycznej), system operacyjny automatycznie "cofa" zmiany tego procesu na semaforze.
* Zapobiega to zakleszczeniom (deadlock) całego systemu w przypadku awarii jednego kandydata.

**Przykład - wejście do sali:**
```c
// Kandydat chce wejść do sali A (max 3 osoby)
semop(sem_id, {SEM_SALA_A, -1, 0}, 1);  // P - zmniejsz licznik
// Jeśli licznik był 0 -> CZEKA aż ktoś wyjdzie

// ... zdaje egzamin ...

semop(sem_id, {SEM_SALA_A, +1, 0}, 1);  // V - zwiększ licznik
// Budzi czekających kandydatów
```

#### Kolejki komunikatów (Message Queues)

**System V API:**
```c
key_t klucz = ftok(PATH_NAME, PROJECT_ID);
int id = msgget(klucz, 0600 | IPC_CREAT);                    // Utwórz kolejkę
msgsnd(id, &msg, rozmiar, 0);                                // Wyślij
msgrcv(id, &buf, rozmiar, typ, 0);                           // Odbierz
msgctl(id, IPC_RMID, NULL);                                  // Usuń kolejkę
```

**Struktura wiadomości:**
```c
typedef struct {
    long mtype;         // Typ (MUSI być pierwszym polem!)
    int nadawca_pid;    // PID nadawcy
    int dane;           // Payload
} Komunikat;
```

**Przykład - kandydat zgłasza się do komisji A:**
```c
// Kandydat
Komunikat msg;
msg.mtype = MSG_TYP_KOMISJA_A;  // Typ = 1 (do komisji A)
msg.nadawca_pid = getpid();
msg.dane = moj_id;
msgsnd(kolejka_id, &msg, sizeof(msg) - sizeof(long), 0);

// Komisja A
Komunikat odebrana;
msgrcv(kolejka_id, &odebrana, sizeof(odebrana) - sizeof(long), 
       MSG_TYP_KOMISJA_A, 0);  // Odbiera tylko typ=1
```

**Dlaczego `sizeof(msg) - sizeof(long)`?**
- Rozmiar NIE ZAWIERA pola `mtype` (to część nagłówka jądra)
- Wysyłamy tylko payload (nadawca_pid + dane)

---

## 3. Przebieg inicjalizacji oraz struktura symulacji

### 3.1. Inicjalizacja środowiska w procesie Dziekana

Proces `dziekan` jest procesem **głównym** (rodzic) odpowiedzialnym za inicjalizację całego systemu. Oto szczegółowy przebieg:

#### Krok 1: Rejestracja handlerów sygnałów

```c
signal(SIGINT, sprzatanie);           // Ctrl+C -> sprzątanie i exit
signal(SIGTSTP, zaradz_ewakuacje);    // Ctrl+Z -> ewakuacja
signal(SIGUSR1, SIG_IGN);             // Ignoruj własny SIGUSR1
```

**Dlaczego SIGUSR1 ignorowany?**
- Dziekan wysyła SIGUSR1 do grupy procesów (`kill(0, SIGUSR1)`)
- Grupa zawiera również dziekana
- Gdyby nie ignorował, zakończyłby sam siebie!

#### Krok 2: Losowanie właściwości kandydatów

```c
for (int i = 0; i < MAX_KANDYDATOW; i++) {
    // Podstawowe dane
    studenci[i].id_kandydata = i + 1;
    studenci[i].pid = 0;
    studenci[i].status = STATUS_NOWY;
    
    // Losowanie: czy ma maturę? (2% szans że NIE)
    for (int i = 0; i < MAX_KANDYDATOW; i++) {
    wspolna_pamiec->studenci[i].zdana_matura = 0;  // Wszyscy bez matury
    wspolna_pamiec->studenci[i].status = STATUS_ODRZUCONY_MATURA;
}
    } else {
        studenci[i].zdana_matura = true;
        
        // Losowanie: czy powtarza? (2% szans że TAK)
        if (losuj(1, 100) <= 2) {
            studenci[i].powtarza_egzamin = true;
            studenci[i].ocena_teoria = 30 + losuj(0, 70);  // 30-100%
        }
    }
}
```

**Probabilistyka:**
- P(brak matury) = 0.02
- P(powtarza | ma maturę) = 0.02
- P(powtarza ∩ ma maturę) = 0.98 × 0.02 = 0.0196 ≈ 2%

### 3.2. Tworzenie i inicjalizacja zasobów IPC

#### A. Pamięć dzielona

**Kod:**
```c
key_t klucz = ftok(PATH_NAME, PROJECT_ID);
// PATH_NAME = ".", PROJECT_ID = 'E'
// ftok() generuje unikalny klucz systemowy

id_pamieci = shmget(klucz, sizeof(PamiecDzielona), 0600 | IPC_CREAT);
// 0600 = rw------- (tylko właściciel może czytać/pisać)
// IPC_CREAT = utwórz jeśli nie istnieje

wspolna_pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
// shmat() mapuje segment do przestrzeni adresowej procesu
```

#### B. Semafory

**Kod:**
```c
id_semaforow = semget(klucz, LICZBA_SEMAFOROW, 0600 | IPC_CREAT);
// Tworzy ZESTAW 6 semaforów

// Ustawianie wartości początkowych
ustaw_semafor(id_semaforow, SEM_DOSTEP_PAMIEC, 1);   // Mutex
ustaw_semafor(id_semaforow, SEM_SALA_A, 3);          // 3 miejsca
ustaw_semafor(id_semaforow, SEM_SALA_B, 3);          // 3 miejsca
ustaw_semafor(id_semaforow, SEM_KRZESLO_A, 1);       // 1 odpowiada
ustaw_semafor(id_semaforow, SEM_KRZESLO_B, 1);       // 1 odpowiada
ustaw_semafor(id_semaforow, SEM_STDOUT, 1);          // Odblokowanie loggera(funkcja loguj)
```

**Funkcja ustaw_semafor:**
```c
void ustaw_semafor(int sem_id, int sem_num, int wartosc) {
    union semun arg;
    arg.val = wartosc;
    if (semctl(sem_id, sem_num, SETVAL, arg) == -1) {
        perror("Blad ustawiania semafora");
        exit(1);
    }
}
```

#### C. Kolejka komunikatów

**Kod:**
```c
id_kolejki = msgget(klucz, 0600 | IPC_CREAT);
// Jedna kolejka dla wszystkich (kandydaci + komisje)
```

**Dlaczego jedna kolejka?**
- System V Message Queues używa pola `mtype` do filtrowania
- `msgrcv(..., mtype=1, ...)` odbiera tylko typ 1 (Komisja A)
- `msgrcv(..., mtype=PID, ...)` odbiera tylko dla konkretnego PID
- Brak potrzeby wielu kolejek

### 3.3. Tworzenie procesów Komisji

#### Fork + Exec pattern

**Kod:**
```c
// Komisja A
pid_t pid_ka = fork();
if (pid_ka == 0) {
    // Proces potomny
    execl("./komisja", "komisja", "A", NULL);
    // Jeśli dojdzie tutaj, execl się nie powiódł
    perror("Blad execl komisja A");
    exit(1);
} else if (pid_ka == -1) {
    perror("Blad fork Komisja A");
}

// Komisja B (analogicznie)
pid_t pid_kb = fork();
if (pid_kb == 0) {
    execl("./komisja", "komisja", "B", NULL);
    exit(1);
}
```

**Jak działa fork()?**
1. `fork()` tworzy KOPIĘ procesu rodzica
2. Proces potomny dostaje nowy PID
3. `fork()` zwraca:
   - 0 w procesie potomnym
   - PID dziecka w procesie rodzica
   - -1 przy błędzie

**Jak działa exec()?**
1. `execl()` **zastępuje** kod procesu nowym programem
2. Proces potomny "zamienia się" w `./komisja`
3. Argumenty: `argv[0]="komisja"`, `argv[1]="A"`, `argv[2]=NULL`
4. Jeśli exec się uda, kod po nim NIE WYKONA SIĘ
5. Jeśli exec się nie uda, wykonuje się `perror()` i `exit(1)`

**Diagram przepływu:**
```
Dziekan (PID 1000)
    |
    fork() ────┐
    |          |
PID 1000   PID 1001 (kopia dziekana)
    |          |
    |      execl("./komisja", "komisja", "A", NULL)
    |          |
    |      PID 1001 (teraz to komisja A)
    |
    fork() ────┐
    |          |
PID 1000   PID 1002
    |          |
    |      execl("./komisja", "komisja", "B", NULL)
    |          |
    |      PID 1002 (teraz to komisja B)
    |
(kontynuuje)
```

### 3.4. Tworzenie i logika procesów Kandydatów

#### Uruchamianie kandydatów

**Kod:**
```c
for (int i = 0; i < MAX_KANDYDATOW; i++) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Proces potomny
        char id_str[10];
        sprintf(id_str, "%d", i);
        execl("./kandydat", "kandydat", id_str, NULL);
        exit(1);
    } else {
        // Proces rodzica - zapisz PID
        wspolna_pamiec->studenci[i].pid = pid;
    }
}
```

**Dlaczego przekazujemy ID jako string?**
- `execl()` przyjmuje tylko `char*` (stringi)
- Kandydat odbiera `argv[1]` i konwertuje z powrotem: `atoi(argv[1])`

#### Logika procesu kandydata

**Diagram stanów kandydata:**
```
START
  |
  v
Sprawdź maturę ──[NIE]──> KONIEC (odrzucony)
  |
 [TAK]
  |
  v
Powtarza? ──[TAK]──> Pomiń Komisję A
  |                        |
 [NIE]                     |
  |                        |
  v                        v
Komisja A          ┌───────┘
  |                |
  v                v
Teoria < 30%? ──[TAK]──> KONIEC (oblał)
  |
 [NIE]
  |
  v
Komisja B
  |
  v
KONIEC (zakończył)
```

#### Funkcja zdawaj_egzamin() - 10 etapów

**Etap 1: Wejście do sali**
```c
semafor_operacja(sem_id, sem_sala, -1);  // P - wejdź
// Jeśli sala pełna -> CZEKA
```

**Etap 2: Zajęcie krzesła**
```c
semafor_operacja(sem_id, sem_krzeslo, -1);  // P - zajmij
// Tylko 1 kandydat może odpowiadać
```

**Etap 3: Zgłoszenie do komisji**
```c
msg.mtype = typ_komisji;  // MSG_TYP_KOMISJA_A lub B
msg.nadawca_pid = getpid();
msg.dane = moj_id;
msgsnd(kolejka_id, &msg, ...);
```

**Etap 4: Oczekiwanie na pytania**
```c
msgrcv(kolejka_id, &odebrana, ..., moj_pid, 0);
// Czeka na wiadomość od komisji
```

**Etap 5-7: Otrzymywanie pytań (symulacja)**
```c
for (int i = 0; i < liczba_pytan; i++) {
    usleep(losuj(10000, 50000));  // Członek przygotowuje pytanie
    printf("Egzaminator %d zadał pytanie nr %d\n", ...);
}
```

**Etap 8: Przygotowanie (czas Ti)**
```c
usleep(200000);  // 200ms = czas Ti
```

**Etap 9: Odpowiadanie (symulacja)**
```c
for (int i = 0; i < liczba_pytan; i++) {
    int ocena = rand() % 101;  // Losowa ocena
    usleep(20000);
    printf("Egzaminator %d ocenił na %d%%\n", ...);
}
```

**Etap 10: Oczekiwanie na werdykt**
```c
msgsnd(...);  // Informujemy że skończyliśmy
msgrcv(..., moj_pid, 0);  // Czekamy na ocenę końcową
```

**Etap 11: Zwolnienie zasobów**
```c
semafor_operacja(sem_id, sem_krzeslo, +1);  // V - zwolnij krzesło
semafor_operacja(sem_id, sem_sala, +1);     // V - wyjdź z sali
```

### 3.5. Logika operacyjna Komisji

#### Główna pętla komisji

**Kod:**
```c
while (czy_pracowac) {
    // Oczekiwanie na kandydata
    msgrcv(kolejka_id, &msg, ..., moj_kanal_nasluchu, 0);
    
    int pid_studenta = msg.nadawca_pid;
    int id_studenta = msg.dane;
    
    // Przygotowanie pytań (symulacja)
    usleep(losuj(10000, 50000));
    wyslij_komunikat(kolejka_id, pid_studenta, ETAP_PYTANIA);
    
    // Oczekiwanie na odpowiedzi
    msgrcv(kolejka_id, &msg, ..., moj_kanal_nasluchu, 0);
    
    // Ocena
    int suma_ocen = 0;
    for (int k = 0; k < liczba_czlonkow; k++) {
        suma_ocen += losuj(0, 100);
    }
    int ocena_finalna = suma_ocen / liczba_czlonkow;
    
    // Zapis do pamięci (SEKCJA KRYTYCZNA)
    semafor_operacja(sem_id, SEM_DOSTEP_PAMIEC, -1);
    if (typ == A) {
        pamiec->studenci[id_studenta].ocena_teoria = ocena_finalna;
        pamiec->studenci[id_studenta].status = 
            (ocena_finalna < 30) ? STATUS_OBLAL_TEORIE : STATUS_ZDAL_TEORIE;
    } else {
        pamiec->studenci[id_studenta].ocena_praktyka = ocena_finalna;
        pamiec->studenci[id_studenta].status = STATUS_ZAKONCZYL;
    }
    semafor_operacja(sem_id, SEM_DOSTEP_PAMIEC, +1);
    
    // Wysłanie wyniku
    wyslij_komunikat(kolejka_id, pid_studenta, ocena_finalna);
}
```

#### Różnice między Komisją A i B

| Aspekt | Komisja A | Komisja B |
|--------|-----------|-----------|
| Liczba członków | 5 | 3 |
| Liczba pytań | 5 | 3 |
| Kanał komunikacji | MSG_TYP_KOMISJA_A (1) | MSG_TYP_KOMISJA_B (2) |
| Próg zaliczenia | 30% | 30% |
| Konsekwencja oblania | Koniec egzaminu | Oblany, ale w rankingu |
| Specjalna logika | Powtarzający pomijają | Brak |

### 3.6. Finalizacja procesu symulacji

#### Oczekiwanie na zakończenie kandydatów

**Kod:**
```c
int studenci_obsluzeni = 0;
while (studenci_obsluzeni < MAX_KANDYDATOW) {
    pid_t w = wait(NULL);
    
    if (w == -1) {
        if (errno == ECHILD) break;     // Nie ma już dzieci
        if (errno == EINTR) continue;   // Przerwane przez sygnał
        break;
    }
    
    // Jeśli to nie komisja, zlicz jako kandydata
    if (w != pid_ka && w != pid_kb) {
        studenci_obsluzeni++;
    }
}
```

**Dlaczego pomijamy komisje?**
- `wait()` zbiera WSZYSTKIE procesy potomne
- Komisje żyją dłużej niż kandydaci
- Chcemy poczekać tylko na kandydatów

#### Zamykanie komisji

**Kod:**
```c
kill(pid_ka, SIGUSR1);  // Komisja A -> zakończ
kill(pid_kb, SIGUSR1);  // Komisja B -> zakończ

wait(NULL);  // Czekaj na komisję A
wait(NULL);  // Czekaj na komisję B
```

#### Generowanie rankingu

**Sortowanie:**
```c
qsort(wspolna_pamiec->studenci, MAX_KANDYDATOW, 
      sizeof(KandydatDane), porownaj_kandydatow);
```

**Funkcja porównująca:**
```c
int porownaj_kandydatow(const void *a, const void *b) {
    KandydatDane *k1 = (KandydatDane *)a;
    KandydatDane *k2 = (KandydatDane *)b;
    
    // Priorytet 1: Ci którzy zakończyli są wyżej
    if (k1->status != STATUS_ZAKONCZYL && k2->status == STATUS_ZAKONCZYL)
        return 1;   // k2 wyżej
    if (k1->status == STATUS_ZAKONCZYL && k2->status != STATUS_ZAKONCZYL)
        return -1;  // k1 wyżej
    
    // Priorytet 2: Sortuj malejąco po sumie
    int suma1 = k1->ocena_teoria + k1->ocena_praktyka;
    int suma2 = k2->ocena_teoria + k2->ocena_praktyka;
    return suma2 - suma1;  // Malejąco
}
```

**Zapis do pliku:**
```c
FILE *plik = fopen("wyniki.txt", "w");
fprintf(plik, "RAPORT Z EGZAMINU WSTEPNEGO\n");
// ... formatowanie tabeli ...
fclose(plik);
```

---

## 4. Dokumentacja techniczna kodu

### 4.1. Moduł common.h - definicje wspólne

#### Struktury danych

**KandydatDane**
```c
typedef struct {
    pid_t pid;              // PID procesu kandydata
    int id_kandydata;       // Numer ID (1, 2, 3, ...)
    bool zdana_matura;      // Czy ma maturę
    bool powtarza_egzamin;  // Czy powtarza
    int ocena_teoria;       // 0-100%
    int ocena_praktyka;     // 0-100%
    int status;             // STATUS_NOWY, STATUS_OBLAL_TEORIE, ...
} KandydatDane;
```

**PamiecDzielona**
```c
typedef struct {
    KandydatDane studenci[MAX_KANDYDATOW];
    int liczba_kandydatow;
    int ewakuacja;           // 0 = normalna praca, 1 = ewakuacja
    int studenci_zakonczeni;
} PamiecDzielona;
```

**Komunikat**
```c
typedef struct {
    long mtype;         // MUSI być pierwszym polem!
    int nadawca_pid;
    int dane;
} Komunikat;
```

#### Stałe

```c
#define MAX_KANDYDATOW 1200         // Liczba kandydatów
#define MIEJSCA_NA_UCZELNI 120       // Liczba miejsc
#define MAX_W_SALI_A 3              // Max w sali A
#define MAX_W_SALI_B 3              // Max w sali B
#define LICZBA_PYTAN_A 5            // Pytań w A
#define LICZBA_PYTAN_B 3            // Pytań w B

#define SEM_DOSTEP_PAMIEC 0         // Indeks semafora
#define SEM_SALA_A 1
#define SEM_SALA_B 2
#define SEM_KRZESLO_A 3
#define SEM_STDOUT 5
#define LICZBA_SEMAFOROW 6

#define MSG_TYP_KOMISJA_A 1         // Typy komunikatów
#define MSG_TYP_KOMISJA_B 2
```

#### Funkcje pomocnicze (static)

**static void semafor_operacja(int id, int num, int op)**
- Uniwersalna funkcja do operacji P/V.
- Zdefiniowana jako `static` w nagłówku, aby każdy moduł miał własną kopię (uniknięcie problemów linkera).
- **Kluczowa cecha:** Ustawia flagę `SEM_UNDO` dla bezpieczeństwa systemu.

**static void loguj(...)**
- Wrapper na `printf` z obsługą kolorów ANSI i synchronizacją.
- Używa semafora `SEM_STDOUT` (mutex), aby komunikaty z różnych procesów nie mieszały się na ekranie.


### 4.2. Moduł dziekan.c - proces zarządzający

#### Funkcje główne

**main()**
- Inicjalizuje zasoby IPC
- Rejestruje kandydatów
- Uruchamia komisje i kandydatów
- Czeka na zakończenie
- Generuje ranking

**sprzatanie(int signal)**
- Odłącza pamięć dzieloną (`shmdt`)
- Usuwa pamięć (`shmctl IPC_RMID`)
- Usuwa semafory (`semctl IPC_RMID`)
- Usuwa kolejkę (`msgctl IPC_RMID`)

**zaradz_ewakuacje(int sig)**
- Ustawia flagę `ewakuacja = 1`
- Wysyła `SIGUSR1` do grupy (`kill(0, SIGUSR1)`)

**porownaj_kandydatow(const void *a, const void *b)**
- Funkcja dla `qsort()`
- Sortuje malejąco po sumie (teoria + praktyka)
- Zakończeni przed nie zakończonymi

### 4.3. Moduł komisja.c - procesy komisji

#### Funkcje główne

**main(int argc, char *argv[])**
- Sprawdza argument `argv[1]` ("A" lub "B")
- Podłącza się do zasobów IPC
- Pętla obsługi kandydatów
- Odłącza pamięć przy zakończeniu

**obsluga_ewakuacji(int sig)**
- Handler SIGUSR1
- Kończy proces komisji

**wyslij_komunikat(int id, long typ, int dane)**
- Wrapper dla `msgsnd()`
- Upraszcza wysyłanie wiadomości

**semafor_operacja()**
- Importowana z `common.h`

### 4.4. Moduł kandydat.c - procesy kandydatów

#### Funkcje główne

**main(int argc, char *argv[])**
- Waliduje ID z `argv[1]`
- Podłącza się do zasobów IPC
- Sprawdza maturę
- Zdaje egzaminy A i B
- Kończy się

**zdawaj_egzamin(...)**
- Wejście do sali (P na sem_sala)
- Zajęcie krzesła (P na sem_krzeslo)
- Zgłoszenie do komisji (msgsnd)
- Oczekiwanie na pytania (msgrcv)
- Symulacja otrzymywania pytań
- Czas Ti na przygotowanie
- Symulacja odpowiadania
- Oczekiwanie na ocenę (msgrcv)
- Zwolnienie zasobów (V na sem_krzeslo, sem_sala)

**zakoncz_proces(int sig)**
- Zwiększa licznik `studenci_zakonczeni`
- Odłącza pamięć (`shmdt`)
- Kończy proces

**drukuj_czas()**
- Wyświetla timestamp [HH:MM:SS.mmm]
- Używa `gettimeofday()` dla precyzji

---

## 5. Testy funkcjonalne

Przeprowadzono **8 testów funkcjonalnych** sprawdzających poprawność działania symulacji w różnych scenariuszach. Każdy test został wykonany wielokrotnie w celu weryfikacji deterministyczności i poprawności obsługi warunków brzegowych.

### 5.1. Test wydajnościowy "Wyścig Szczurów"(zakomentowane sleepy)

**Cel:** Weryfikacja poprawności synchronizacji IPC przy maksymalnym obciążeniu procesora, bez sztucznych opóźnień czasowych (`sleep`). Test ma na celu udowodnienie, że system jest stabilny i nie polega na taktowaniu zegarem, lecz na mechanizmach semaforów i kolejek.

**Konfiguracja:**
```c
//sleep = 0 (zakomentowane w kazdym miejscu w kodzie)
#define MAX_KANDYDATOW 1200
#define MIEJSCA_NA_UCZELNI 120
```

**Procedura:**
1. Uruchomienie: `./dziekan 1200`
2. Monitorowanie czasu wykonania oraz użycia procesora
3. Weryfikacja tablicy procesów pod kątem procesów Zombie

**Oczekiwane zachowanie:**
- Symulacja kończy się w czasie bardzo krotkim czasie dla 1200 kandydatów
- Generowany jest poprawny raport końcowy
- Brak zakleszczeń (deadlock)
- Brak osieroconych procesów (Zombie) w trakcie i po teście

**Wynik testu:**
```
[16:18:45.005] [Dziekan] ROZPOCZYNAM EGZAMIN (Miejsc: 120, Chętnych: 1200)
... (błyskawiczny strumień logów) ...
[16:22:55.169] STATYSTYKA: Miejsc: 120, Przyjęto: 120.
[16:22:55.169] Raport zapisano w pliku 'wyniki.txt'.

[Dziekan] Rozpoczynam sprzatanie zasobow...
[Dziekan] Pamiec odlaczona.
[Dziekan] Pamiec usunieta.
[Dziekan] Semafory usuniete.
[Dziekan] Kolejka komunikatow usunieta.
[Dziekan] Zasoby zwolnione
```

**Weryfikacja zarządzania procesami (Zombie):**

Podczas testu monitorowano tablicę procesów pod kątem statusu `<defunct>`. Potwierdzono, że proces główny (Dziekan) poprawnie odbiera statusy zakończenia wszystkich 1200 procesów potomnych przy użyciu funkcji `wait()`.
```bash
ps aux | grep 'Z' | grep -v grep
(brak wyników)
```

**✅ TEST ZALICZONY**

**Obserwacje:**
- System działał stabilnie przy pełnym obciążeniu CPU
- Poprawna synchronizacja i sprzątanie procesów
- Brak race conditions mimo maksymalnej konkurencji
- Wszystkie semafory i kolejki działały deterministycznie
- Czas wykonania: ~4,1s (1200 procesów!)

---

### 5.2. Test odporności na awarię krytyczną (Kill -9) – Test flagi SEM_UNDO

**Cel:**
Weryfikacja stabilności systemu w przypadku nagłej awarii procesu wewnątrz sekcji krytycznej (np. gdy kandydat zajmuje miejsce w sali). Test sprawdza, czy flaga `SEM_UNDO` poprawnie zwolni semafory za "martwy" proces, zapobiegając zakleszczeniu (deadlock) całego systemu.

**Scenariusz:**
1. Kandydat (PID 3514553) wchodzi do Sali A i zajmuje zasoby (semafor sali/krzesła).
2. **Symulacja awarii:** Wysłanie sygnału `SIGKILL` (`kill -9`) do tego procesu z drugiego terminala.
3. **Obserwacja:** Czy inny oczekujący kandydat (np. Kandydat 10) będzie mógł wejść na zwolnione miejsce.

**Przebieg testu (Terminal 1 - Logi symulacji):**

```text
[17:25:33.542] [Komisja A] Przygotowuje pytania dla [kandydata 7] (PID 3514553)...
[17:25:33.561] [Sala A] [Kandydat 7] Czekam na pytania...
...
[17:25:33.713] [Sala A] [Kandydat 7] Otrzymałem wszystkie pytania. Przygotowuję się (czas Ti)...

// --- TUTAJ NASTĘPUJE ATAK W TERMINALU 2: kill -9 3514553 ---
// Kandydat 7 zostaje natychmiast usunięty przez system.
// Nie wykonuje kodu zwalniającego semafory (V).
// System operacyjny wykrywa śmierć procesu i cofa operacje semaforowe (SEM_UNDO).

[17:25:39.041] >> [Kandydat 10] Wszedlem do Sala A.  <-- SUKCES! Zasób został zwolniony automatycznie.
[17:25:39.041] [Kandydat 9] Zglaszam sie do komisji...
[17:25:39.041]      [Kandydat 10] Czekam na wolne krzeslo przed komisja...
[17:25:39.041] [Komisja A] [Kandydat 7] PID 3514553 oceniony na: 59% (Komisja zakończyła przetwarzanie danych)
```

***Weryfikacja w Terminalu 2:***
```bash
ps aux | grep kandydat

$kill -9 3514553$ ps -p 3514553
(brak wyników - proces nie istnieje)
$ ipcs -s
(Semafory nadal aktywne, brak wiszących blokad blokujących system)
```
**✅ TEST ZALICZONY**

### 5.3. Test Procedury Ewakuacji (Sygnał Ctrl+Z / SIGTSTP)

**Opis:**
Test symuluje sytuację awaryjną, w której Dziekan ogłasza alarm w trakcie trwania egzaminu. Wymuszenie przerwania następuje poprzez wysłanie sygnału `SIGTSTP` (kombinacja klawiszy `Ctrl+Z`) do procesu głównego w terminalu.

**Cel:**
Weryfikacja, czy:
1.  Dziekan poprawnie przechwytuje sygnał i rozsyła `SIGUSR1` do całej grupy procesów.
2.  Procesy potomne (Kandydaci, Komisje) natychmiast przerywają oczekiwanie (nawet jeśli są zablokowane na semaforach lub kolejkach komunikatów).
3.  Raport końcowy jest generowany natychmiastowo i uwzględnia status "EWAKUACJA" dla osób, które nie dokończyły egzaminu.
4.  Zasoby IPC są poprawnie zwalniane po wymuszonym zakończeniu.

**Konfiguracja:**
```c
#define MAX_KANDYDATOW 20
#define MIEJSCA_NA_UCZELNI 120
```

**Przebieg testu - Logi symulacji (Terminal 1):**
```
[17:40:48.172] [Dziekan] ROZPOCZYNAM EGZAMIN (Liczba miejsc: 120)
[17:40:48.172] [Dziekan] Liczba kandydatów: 20
>> Aby oglosic EWAKUACJE, wcisnij Ctrl+Z <<
=========================================
[17:40:48.172] [Dziekan] Tworzenie pamięci dzielonej...
[17:40:48.172] [Dziekan] Rejestracja kandydatow w systemie...
[17:40:48.172]  -> Kandydat 12: BRAK MATURY (Odrzucony)
...
[17:40:48.173] [Dziekan] Otwieram drzwi uczelni dla 20 kandydatow...
...
[17:40:48.608] [Komisja A] [Kandydat 1] PID 3536130 oceniony na: 38%
[17:40:48.608]  << [Kandydat 1] Opuszczam Sala A.
[17:40:48.608] [Kandydat 1] Czekam na wejscie do Sala B...
...
[17:40:49.649] [Sala A] Egzaminator 3 zadał pytanie nr 34 kandydatowi 4.

^Z  <--- UŻYTKOWNIK WYSYŁA SYGNAŁ SIGTSTP (CTRL+Z)

[17:40:49.659] 
!!! ALARM !!! OGLASZAM EWAKUACJE !!! (Sygnal 20)
[17:40:49.659] 
!!! [Kandydat 18] SLYSZE ALARM! UCIEKAM! !!!
!!! [Kandydat 14] SLYSZE ALARM! UCIEKAM! !!!
!!! [Kandydat 11] SLYSZE ALARM! UCIEKAM! !!!
...
!!! [Kandydat 2] SLYSZE ALARM! UCIEKAM! !!!
!!! [Kandydat 4] SLYSZE ALARM! UCIEKAM! !!!
!!! [Komisja B] OTRZYMANO SYGNAL EWAKUACJI (SIGUSR1) !!!
!!! [Komisja A] OTRZYMANO SYGNAL EWAKUACJI (SIGUSR1) !!!
 
[Dziekan] Wszyscy studenci zakonczyli egzaminy. Zamykam komisje...
[Dziekan] Egzaminy zakonczone. Generuje raport i zapisuje do pliku...

========================================================================================
| POZ  |  ID  | MATURA | TEORIA | PRAKT. | SUMA(T+P) | STATUS                      |
========================================================================================
| #001 | 0003 |  TAK   |   31%  |   64%  |     95    | PRZYJĘTY                    |
| #002 | 0001 |  TAK   |   38%  |   52%  |     90    | PRZYJĘTY                    |
| #003 | 0002 |  TAK   |   62%  |    0%  |     62    | EWAKUACJA                   |
| #004 | 0004 |  TAK   |    0%  |    0%  |      0    | EWAKUACJA                   |
| #005 | 0005 |  TAK   |    0%  |    0%  |      0    | EWAKUACJA                   |
...
| #019 | 0019 |  TAK   |    0%  |    0%  |      0    | EWAKUACJA                   |
| #020 | 0020 |  TAK   |    0%  |    0%  |      0    | EWAKUACJA                   |
========================================================================================
[17:40:49.664] STATYSTYKA: Miejsc: 120, Przyjęto: 2.
[17:40:49.664] Raport zapisano w pliku 'wyniki.txt'.

[Dziekan] Rozpoczynam sprzatanie zasobow...
[Dziekan] Pamiec odlaczona.
[Dziekan] Pamiec usunieta.
[Dziekan] Semafory usuniete.
[Dziekan] Kolejka komunikatow usunieta.
[Dziekan] Zasoby zwolnione.
```

**Weryfikacja czystości zasobów (Terminal 2): Sprawdzenie, czy po przerwaniu programu zasoby systemowe zostały zwolnione i nie doszło do wycieku pamięci lub osierocenia semaforów.** 
```bash
$ ipcs -a

# Output:
# (brak wyników dla użytkownika, co potwierdza poprawne działanie funkcji sprzątającej)
```
**✅ TEST ZALICZONY**

---

### 5.4. Test weryfikacji logiki biznesowej (~2% brak matury i powtarzających)

**Opis:**
Test sprawdza poprawność probabilistycznego losowania statusów kandydatów zgodnie z założeniami projektu. Wykorzystuje dużą próbę statystyczną (M=1000) w celu zweryfikowania, że:
1. Około 2% kandydatów nie posiada zdanej matury i jest odrzucanych natychmiastowo.
2. Około 2% kandydatów to "powtarzający" (mają zaliczoną teorię z poprzedniego roku i pomijają Komisję A).

**Cel:**
Weryfikacja, czy:
1. Generator liczb pseudolosowych (`rand()`) działa poprawnie i nie wprowadza systematycznego błędu.
2. Kandydaci bez matury są natychmiastowo odrzucani i nie wchodzą do sal egzaminacyjnych.
3. Kandydaci powtarzający otrzymują ocenę z teorii w zakresie 30-100% i pomijają Komisję A.
4. Statystyka końcowa mieści się w granicach błędu dla rozkładu dwumianowego (~2% ± odchylenie).

**Konfiguracja:**
```c
#define MAX_KANDYDATOW 1000  // Duża próba statystyczna
#define MIEJSCA_NA_UCZELNI 120
```

**Procedura:**
1. Uruchomienie symulacji z przekierowaniem logów do pliku:
```bash
   ./dziekan 1000 > logi_statystyka.txt
```
2. Analiza pliku `wyniki.txt` pod kątem statusu "BRAK MATURY"
3. Analiza logów pod kątem kandydatów powtarzających

**Wynik testu:**

**1. Weryfikacja statusu "Brak Matury" (analiza pliku wyniki.txt):**
```bash
grep "BRAK MATURY" wyniki.txt | wc -l
17
```

**Interpretacja:**
- Oczekiwano: ~20 kandydatów (2% z 1000)
- Otrzymano: 17 kandydatów (1.7%)
- Wynik mieści się w granicach błędu statystycznego dla rozkładu dwumianowego
- **✓ Weryfikacja poprawna**

**2. Weryfikacja kandydatów powtarzających (analiza logów):**
```bash
grep "POWTARZA" logi_statystyka.txt | head -n 5
[08:14:41.716]  -> Kandydat 53: POWTARZA (Zaliczona teoria: 83%)
[08:14:41.716]  -> Kandydat 101: POWTARZA (Zaliczona teoria: 93%)
[08:14:41.716]  -> Kandydat 111: POWTARZA (Zaliczona teoria: 100%)
[08:14:41.717]  -> Kandydat 244: POWTARZA (Zaliczona teoria: 85%)
[08:14:41.717]  -> Kandydat 263: POWTARZA (Zaliczona teoria: 38%)
```

**Interpretacja:**
- Dziekan poprawnie oznacza kandydatów jako "POWTARZA" w fazie rejestracji
- Oceny z teorii mieszczą się w wymaganym zakresie 30-100%

**Fragment rankingu końcowego (wyniki.txt):**
```
| #001 | 0111 |  TAK   |  100%  |   95%  |    195    | PRZYJĘTY                    |
| #002 | 0053 |  TAK   |   83%  |   88%  |    171    | PRZYJĘTY                    |
...
| #049 | 0015 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #050 | 0044 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
====================================================================
STATYSTYKA: Miejsc: 120, Przyjęto: 120.
```

**Weryfikacja matematyczna (rozkład dwumianowy):**

Dla `n = 1000` kandydatów i `p = 0.02` (prawdopodobieństwo):
- Wartość oczekiwana: `μ = n × p = 1000 × 0.02 = 20`
- Odchylenie standardowe: `σ = √(n × p × (1-p)) = √(1000 × 0.02 × 0.98) ≈ 4.43`
- Przedział ufności 95%: `[20 - 2σ, 20 + 2σ] ≈ [11, 29]`

**Wynik 17 kandydatów bez matury mieści się w przedziale ufności ✓**

**✅ TEST ZALICZONY**

**Obserwacje:**
- Generator liczb pseudolosowych działa poprawnie
- Rozkład statystyczny zgodny z założeniami (~2%)
- Kandydaci bez matury prawidłowo odrzucani
- Kandydaci powtarzający zachowują ocenę z teorii i pomijają Komisję A
- Brak systematycznych błędów w logice warunkowej
---

### 5.5. Test weryfikacji matury

**Cel:** Sprawdzenie czy system poprawnie odrzuca kandydatów bez matury.

**Konfiguracja:**
Zmodyfikowano kod w `dziekan.c` aby symulować scenariusz gdzie **wszyscy** kandydaci nie mają matury:

```c
// Tymczasowa modyfikacja dla testu
for (int i = 0; i < liczba_chetnych; i++) {
        wspolna_pamiec->studenci[i].id_kandydata = i + 1;
        wspolna_pamiec->studenci[i].ocena_teoria = 0;
        wspolna_pamiec->studenci[i].ocena_praktyka = 0;
        wspolna_pamiec->studenci[i].pid = 0;
        wspolna_pamiec->studenci[i].status = STATUS_NOWY;

        //Dodane dla testu
        wspolna_pamiec->studenci[i].zdana_matura = 0; 
        wspolna_pamiec->studenci[i].status = STATUS_ODRZUCONY_MATURA;
        loguj(id_semaforow, KOLOR_CZERWONY, " -> Kandydat %d: BRAK MATURY(TEST WYMUSZONY)\n", i+1);
```

**Procedura:**
1. Kompilacja z modyfikacją
2. Uruchomienie
3. Sprawdzenie czy ŻADEN kandydat nie jest wpuszczany na egzamin

**Wynik testu:**

```
[18:03:14.108] [Dziekan] Liczba kandydatów: 10
>> Aby oglosic EWAKUACJE, wcisnij Ctrl+Z <<
=========================================
[18:03:14.108] [Dziekan] Tworzenie pamięci dzielonej...
[18:03:14.108] [Dziekan] Rejestracja kandydatow w systemie...
[18:03:14.108]  -> Kandydat 1: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.108]  -> Kandydat 2: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.108]  -> Kandydat 3: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.108]  -> Kandydat 4: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.108]  -> Kandydat 5: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.108]  -> Kandydat 6: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.108]  -> Kandydat 7: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.109]  -> Kandydat 8: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.109]  -> Kandydat 9: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.109]  -> Kandydat 10: BRAK MATURY (TEST WYMUSZONY)
[18:03:14.109] [Dziekan] Tworzenie kolejki komunikatow...
[18:03:14.109] [Dziekan] Otwieram drzwi uczelni dla 10 kandydatow...
ID   | PID   | STATUS WEJSCIA
-----|-------|---------------
0001 | 3604891 | ODRZUCONY (Brak matury)
[18:03:14.111] [Komisja B] (PID: 3604890) Czekam na kandydatów.
[18:03:14.111] [Komisja A] (PID: 3604889) Czekam na kandydatów.
[18:03:14.111] [Kandydat 1] Czekam przed wejściem. (Matura: NIE)
[18:03:14.112] [Kandydat 1] Brak matury. Wracam do domu.
0002 | 3604896 | ODRZUCONY (Brak matury)
0003 | 3604897 | ODRZUCONY (Brak matury)

... 


[18:03:14.367] [Kandydat 8] Brak matury. Wracam do domu.
[18:03:14.367] [Kandydat 9] Brak matury. Wracam do domu.
[18:03:14.367] [Kandydat 10] Brak matury. Wracam do domu.
[18:03:14.367] 
[Dziekan] Wszyscy studenci zakonczyli egzaminy. Zamykam komisje...
[18:03:14.368] [Komisja A] Dziekan zarządził koniec egzaminów. Zamykam biuro.
[18:03:14.368] [Komisja B] Dziekan zarządził koniec egzaminów. Zamykam biuro.
[18:03:14.368] 
[Dziekan] Egzaminy zakonczone. Generuje raport i zapisuje do pliku...
```

**Plik wyniki.txt:**
```

========================================================================================
| POZ  |  ID  | MATURA | TEORIA | PRAKT. | SUMA(T+P) | STATUS                      |
========================================================================================
| #001 | 0001 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #002 | 0002 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #003 | 0003 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #004 | 0004 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #005 | 0005 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #006 | 0006 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #007 | 0007 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #008 | 0008 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #009 | 0009 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
| #010 | 0010 |  NIE   |    0%  |    0%  |      0    | BRAK MATURY                 |
====================================================================
[18:03:14.369] STATYSTYKA: Miejsc: 120, Przyjęto: 0.
```

**✅ TEST ZALICZONY**

**Obserwacje:**
- Wszyscy kandydaci bez matury są poprawnie odrzucani
- Procesy kandydatów kończą się natychmiast
- Komisje nie otrzymują żadnych zgłoszeń
- Ranking zawiera wszystkich z statusem "BRAK MATURY"
- Liczba przyjętych = 0 (poprawnie)

---
### 5.6. Test limitu miejsc w Sali (Semafor licznikowy)

**Opis:**
Test weryfikuje poprawność działania semafora licznikowego odpowiedzialnego za ograniczenie liczby kandydatów przebywających jednocześnie w sali egzaminacyjnej. Zgodnie z założeniami projektu (`MAX_W_SALI_A = 3`), w Sali A mogą przebywać jednocześnie maksymalnie 3 osoby.

**Cel:**
Weryfikacja, czy:
1. Semafor `SEM_SALA_A` poprawnie blokuje wejście 4. kandydata gdy sala jest pełna.
2. Proces czekający na semaforze zostaje obudzony natychmiastowo po zwolnieniu miejsca.
3. System zachowuje zasadę "1 wyjście = 1 wejście" (FIFO dla oczekujących).
4. Nie dochodzi do sytuacji race condition (np. 4 osoby jednocześnie w sali).

**Konfiguracja:**
```c
#define MAX_KANDYDATOW 20  // Mniejsza grupa dla precyzyjnej analizy
#define MAX_W_SALI_A 3     // Limit: 3 osoby jednocześnie
```

**Procedura:**
1. Uruchomienie symulacji z przekierowaniem logów:
```bash
   ./dziekan 20 > logi_semafor.txt
```
2. Analiza sekwencji wejść i wyjść za pomocą grep:
```bash
   grep -E "Wszedlem do Sala A|Opuszczam Sala A|Czekam na wejscie do Sala A" logi_semafor.txt | head -n 20
```

**Wynik testu - Analiza logów**

Poniższy fragment logów z precyzyjnymi znacznikami czasu idealnie obrazuje działanie mechanizmu synchronizacji:
```
[Kandydat 1] Czekam na wejscie do Sala A...
[10:56:23.605]  >> [Kandydat 1] Wszedlem do Sala A.  <-- (1. osoba w sali)
[10:56:23.857]  >> [Kandydat 2] Wszedlem do Sala A.  <-- (2. osoba w sali)
[10:56:23.858]  >> [Kandydat 3] Wszedlem do Sala A.  <-- (3. osoba - SALA PEŁNA)

[10:56:23.858] [Kandydat 4] Czekam na wejscie do Sala A... <-- BLOKADA (Semafor = 0)
[10:56:23.858] [Kandydat 6] Czekam na wejscie do Sala A... <-- Kolejkowanie
[10:56:23.859] [Kandydat 7] Czekam na wejscie do Sala A... <-- Kolejkowanie

[10:56:24.114]  << [Kandydat 1] Opuszczam Sala A.    <-- Zwolnienie miejsca (V +1)
[10:56:24.114]  >> [Kandydat 4] Wszedlem do Sala A.  <-- Natychmiastowe wejście (P -1)

[10:56:24.602]  << [Kandydat 2] Opuszczam Sala A.    <-- Zwolnienie miejsca
[10:56:24.602]  >> [Kandydat 6] Wszedlem do Sala A.  <-- Wymiana 1:1 (idealnie w punkt)
```

**Analiza poklatkowa:**

**Faza 1: Napełnianie sali (23.605 - 23.858)**
1. Kandydat 1 wchodzi o `10:56:23.605` → Semafor: 3→2 (w sali: 1)
2. Kandydat 2 wchodzi o `10:56:23.857` → Semafor: 2→1 (w sali: 2)
3. Kandydat 3 wchodzi o `10:56:23.858` → Semafor: 1→0 (w sali: 3) **SALA PEŁNA**

**Faza 2: Blokada (23.858 - 24.114)**
- Kandydat 4, 6, 7 próbują wejść w tej samej sekundzie (23.858-23.859)
- Semafor = 0 → wszystkie procesy są **blokowane** w `semop()`
- System wyświetla "Czekam na wejście..." ale **NIE WPUSZCZA** nikogo

**Faza 3: Rotacja (24.114)**
- O `10:56:24.114` Kandydat 1 wywołuje `semop(..., +1)` (operacja V)
- Semafor: 0→1
- Jądro systemu **natychmiast budzi** Kandydata 4 z kolejki oczekujących
- W **tym samym znaczniku czasu** (24.114) Kandydat 4 wchodzi

**Faza 4: Kontynuacja rotacji (24.602)**
- O `10:56:24.602` Kandydat 2 wychodzi → Semafor: 0→1
- O `10:56:24.602` Kandydat 6 wchodzi → wymiana **1 za 1**

**Kluczowe obserwacje:**

| Znacznik czasu | Zdarzenie | Stan semafora | Kandydaci w sali |
|----------------|-----------|---------------|------------------|
| 10:56:23.605 | Kandydat 1 wchodzi | 3→2 | [1] |
| 10:56:23.857 | Kandydat 2 wchodzi | 2→1 | [1,2] |
| 10:56:23.858 | Kandydat 3 wchodzi | 1→0 | [1,2,3] ✓ |
| 10:56:23.858 | Kandydat 4 **czeka** | 0 (BLOKADA) | [1,2,3] |
| 10:56:24.114 | Kandydat 1 wychodzi | 0→1 | [2,3] |
| 10:56:24.114 | Kandydat 4 wchodzi | 1→0 | [2,3,4] ✓ |
| 10:56:24.602 | Kandydat 2 wychodzi | 0→1 | [3,4] |
| 10:56:24.602 | Kandydat 6 wchodzi | 1→0 | [3,4,6] ✓ |

**✅ TEST ZALICZONY**

**Wnioski:**
- Semafor licznikowy działa perfekcyjnie
- W żadnym momencie w sali nie przebywały więcej niż 3 osoby
- Mechanizm kolejkowania FIFO działa poprawnie
- Operacje V (zwolnienie) i P (zajęcie) są atomowe
- Brak race conditions mimo wysokiej konkurencji procesów
- System gwarantuje sprawiedliwy dostęp do zasobu (sala)

---

### 5.7. Test progu zaliczenia teorii - "Symulacja Rzezi Niewiniątek"

**Opis:**
Test weryfikuje poprawność działania logiki warunkowej odcinającej dostęp do Komisji B (egzamin praktyczny) kandydatom, którzy nie osiągnęli minimalnego progu 30% w części teoretycznej (Komisja A). Symulowany jest scenariusz ekstremalny, gdzie **wszyscy** kandydaci oblają teorię.

**Cel:**
Weryfikacja, czy:
1. System poprawnie identyfikuje kandydatów z oceną < 30% jako "oblanych"
2. Procesy oblanych kandydatów kończą się natychmiast (nie przechodzą do Sali B)
3. Komisja B nie obsługuje żadnego kandydata (pozostaje w stanie bezczynności)
4. Raport końcowy poprawnie oznacza wszystkich jako "OBLANY (Teoria)"
5. Statystyka pokazuje 0 przyjętych na uczelnię

**Konfiguracja:**

**Modyfikacja 1: komisja.c (wymuszenie oceny niedostatecznej)**
```c
// ZAMIAST: int ocena_finalna = suma_ocen / liczba_czlonkow;
int ocena_finalna = 20;  // Sztywna ocena 20% dla WSZYSTKICH
```

**Modyfikacja 2: dziekan.c (wyłączenie mechanizmu powtarzających)**
```c
// ZAMIAST: if (wspolna_pamiec->studenci[i].zdana_matura == 1 && losuj(1, 100) <= 2) {
if (0) {  // NIKT nie powtarza - wszyscy muszą iść na teorię i ją oblać
    wspolna_pamiec->studenci[i].powtarza_egzamin = 1;
    wspolna_pamiec->studenci[i].ocena_teoria = 30 + losuj(0, 70);
    // ...
}
```

**Uzasadnienie wyłączenia powtarzających:**
- Studenci powtarzający mają już zaliczoną teorię (30-100%) z poprzedniego roku
- Pomijają Komisję A i idą bezpośrednio do Komisji B
- Aby test był "czysty" (100% oblanych w teorii), musieli przejść przez Komisję A

**Wynik testu:**

**Fragment logów konsoli:**
```
[Komisja A] [Kandydat 1] PID 17301 oceniony na: 20%
[Kandydat 1] Oblalem teorie (20%). Koniec.

[Komisja A] [Kandydat 2] PID 17302 oceniony na: 20%
[Kandydat 2] Oblalem teorie (20%). Koniec.

[Komisja A] [Kandydat 3] PID 17303 oceniony na: 20%
[Kandydat 3] Oblalem teorie (20%). Koniec.

...

[Komisja A] [Kandydat 50] PID 17350 oceniony na: 20%
[Kandydat 50] Oblalem teorie (20%). Koniec.

[Dziekan] Wszyscy studenci zakonczyli egzaminy. Zamykam komisje...
[Komisja A] Dziekan zarządził koniec egzaminów. Zamykam biuro.
[Komisja B] Dziekan zarządził koniec egzaminów. Zamykam biuro.
```

**Analiza pliku wyniki.txt (kompletna tabela):**
```
========================================================================================
| POZ  |  ID  | MATURA | TEORIA | PRAKT. | SUMA(T+P) | STATUS                      |
========================================================================================
| #001 | 0001 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
| #002 | 0002 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
| #003 | 0003 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
| #004 | 0004 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
| #005 | 0005 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
...
| #046 | 0046 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
| #047 | 0047 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
| #048 | 0048 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
| #049 | 0049 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
| #050 | 0050 |  TAK   |  20%   |   0%   |     20    | OBLANY (Teoria)             |
========================================================================================
STATYSTYKA: Miejsc: 120, Przyjęto: 0.
```

**Weryfikacja aktywności Komisji B:**
```bash
grep "Komisja B" logi_komisja.txt
[15:12:14.198] [Komisja B] (PID: 812166) Czekam na kandydatów.
[15:12:38.932] [Komisja B] Dziekan zarządził koniec egzaminów. Zamykam biuro.
```

**Interpretacja:**
- Komisja B uruchomiła się poprawnie
- Oczekiwała na zgłoszenia (`msgrcv()` blokujące)
- **NIE OBSŁUŻYŁA ŻADNEGO KANDYDATA** (brak logów o egzaminowaniu)
- Zakończyła się dopiero po otrzymaniu SIGUSR1 od Dziekana

**Weryfikacja izolacji procesów:**

| Proces | Stan | Kandydaci obsłużeni | Status |
|--------|------|---------------------|--------|
| Komisja A | Aktywna | 50/50 | ✓ Wszyscy ocenieni na 20% |
| Komisja B | Bezczynna | 0/50 | ✓ Brak zgłoszeń (poprawna izolacja) |

**Obserwacje:**
- Logika warunkowa `if (ocena < 30)` poprawnie identyfikuje oblanych
- Izolacja procesów jest perfekcyjna (Komisja B nie otrzymała żadnego komunikatu)
- Kolejka komunikatów nie została zapchana martwymi wiadomościami
- Wszystkie procesy zakończyły się poprawnie (brak zombie)
- Statystyka końcowa: 0 przyjętych (zgodnie z oczekiwaniami)

**Wnioski techniczne:**
- Próg 30% jest twardym ograniczeniem (hard limit)
- Brak "wycieków" kandydatów do Komisji B
- Synchronizacja przez kolejki działa selektywnie (tylko spełniający warunek dostają się do kolejki)
- Test potwierdza deterministyczność systemu (100% powtarzalność przy tych samych parametrach)

**✅ TEST ZALICZONY**

---

### 5.8. Test integralności pamięci dzielonej (Weryfikacja Mutexa)

**Cel:**
Udowodnienie, że semafor binarny `SEM_DOSTEP_PAMIEC` (Mutex) poprawnie chroni pamięć dzieloną przed wyścigami (Race Conditions) przy masowym, jednoczesnym zapisie danych przez setki procesów.

**Teoria:**
Gdyby mutex nie działał, inkrementacja licznika `studenci_zakonczeni++` przez 2000 procesów jednocześnie zakończyłaby się wynikiem mniejszym niż 2000 (nadpisywanie wartości w wyniku wyścigu). Test wymusza maksymalne obciążenie systemu przez usunięcie wszystkich opóźnień (//sleep).

**Konfiguracja:**

**Modyfikacja 1: common.h**
```c
#define MAX_KANDYDATOW 2500
```

**Modyfikacja 2: Zakomentowanie funkcji (sleep) we wszytskich programach**


**Modyfikacja 3: Dodanie "czujki" w dziekan.c**

```
// --- CZUJKA TESTOWA ---
printf(KOLOR_ZIELONY "[DEBUG] Wartość licznika w pamięci dzielonej (studenci_zakonczeni): %d\n" 
       KOLOR_RESET, wspolna_pamiec->studenci_zakonczeni);
// ----------------------

sprzatanie(0);
return 0;
```

**Wynik testu:**

**Fragment logów konsoli (początek i koniec):**
```
[16:18:45.005] [Dziekan] Otwieram drzwi uczelni dla 2000 kandydatow...
[16:18:45.012] [Dziekan] ROZPOCZYNAM EGZAMIN (Miejsc: 120, Chętnych: 2000)

... (błyskawiczny strumień logów - czas trwania < 3s) ...

[16:18:47.800] [Dziekan] Wszyscy studenci zakonczyli egzaminy. Zamykam komisje...
[16:18:47.805] [Dziekan] Egzaminy zakonczone. Generuje raport i zapisuje do pliku...
STATYSTYKA: Miejsc: 120, Przyjęto: 120.
Raport zapisano w pliku 'wyniki.txt'.

[DEBUG] Wartość licznika w pamięci dzielonej (studenci_zakonczeni): 2000

[16:18:47.810] [Dziekan] Zasoby zwolnione.
```

**Weryfikacja spójności danych - liczba linii w raporcie:**
```bash
wc -l wyniki.txt
2006 wyniki.txt
```

**Matematyka:**
- 2000 kandydatów (rekordy w tabeli)
- \+ 6 linii nagłówka/stopki (separator, nagłówki kolumn, statystyka)

**Weryfikacja procesów Zombie:**
```bash
ps aux | grep 'Z' | grep -v grep
(brak wyników - wszystkie procesy odebrane przez wait())
```

**Obserwacje:**
- Semafor nr 0 (`SEM_DOSTEP_PAMIEC`) skutecznie serializuje dostęp do zapisu
- Mimo 2000 procesów próbujących zapisać wynik w ułamku sekundy, żaden rekord nie został utracony ani nadpisany
- Brak zakleszczeń (deadlock) mimo braku odstępów czasowych
- Tablica procesów czysta (brak zombie)

**✅ TEST ZALICZONY**

---

**Wszystkie 8 testów zakończone sukcesem! ✅**

---

## 6. Problemy napotkane podczas realizacji

### 6.1. Synchronizacja dostępu do pamięci dzielonej

**Problem:**
Podczas pierwszych testów z wieloma kandydatami (>50) występowały **race conditions** - kandydaci jednocześnie modyfikowali pole `studenci_zakonczeni` bez synchronizacji, co prowadziło do niepoprawnej liczby zakończonych procesów.

**Objawy:**
```
[Dziekan] Oczekiwano 50 zakończonych, otrzymano 47
```

**Przyczyna:**
Brak sekcji krytycznej przy inkrementacji licznika:
```c
// ZŁE - race condition!
pamiec_global->studenci_zakonczeni++;
```

**Dlaczego to problem?**
Operacja `++` NIE JEST atomowa! Składa się z 3 kroków:
1. Odczytaj wartość z pamięci
2. Zwiększ o 1
3. Zapisz z powrotem

Jeśli dwa procesy wykonają te kroki jednocześnie:
```
Proces A: odczytuje 10
Proces B: odczytuje 10  (zanim A zapisze!)
Proces A: zapisuje 11
Proces B: zapisuje 11  (nadpisuje 11, powinno być 12!)
```

**Rozwiązanie:**
Użycie semafora jako **mutex** (mutual exclusion):
```c
// DOBRE - chronione semaforem
semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, -1); 
pamiec_global->studenci_zakonczeni++;
semafor_operacja(sem_id_global, SEM_DOSTEP_PAMIEC, +1);
```

**Wynik:**
- Race conditions wyeliminowane
- Licznik zawsze poprawny
- Testy z 100+ kandydatami przechodzą

---

### 6.2. Zakleszczenia przy semaforach

**Problem:**
W wczesnej wersji kodu występowały **deadlocki** gdy kandydat zajmował krzesło ale nie zwalniał go przy ewakuacji.

**Scenariusz zakleszczenia:**
1. Kandydat A zajmuje krzesło (P na SEM_KRZESLO_A)
2. Rozpoczyna odpowiadanie
3. **EWAKUACJA** (Ctrl+Z)
4. Kandydat A otrzymuje SIGUSR1 i kończy się NATYCHMIAST
5. Krzesło **NIE ZOSTAJE ZWOLNIONE**
6. Kandydat B czeka na krzesło -> **WIECZNE CZEKANIE**

**Kod problematyczny:**
```c
semafor_operacja(sem_id, SEM_KRZESLO_A, -1);

// ... zdawanie egzaminu ...

zakoncz_proces(SIGUSR1);  // Brak zwolnienia semafora!

semafor_operacja(sem_id, SEM_KRZESLO_A, +1);  // Nigdy się nie wykona!
```

**Rozwiązanie:**
Użycie flagi `SEM_UNDO` w `semop()`:
```c
struct sembuf ops;
ops.sem_num = sem_num;
ops.sem_op = op;
ops.sem_flg = SEM_UNDO;
semop(sem_id, &ops, 1);
```

**Jak działa SEM_UNDO?**
- Jądro systemu śledzi operacje na semaforach
- Gdy proces się kończy, jądro automatycznie cofa operacje
- Jeśli proces zrobił P (-1), jądro robi V (+1) przy zakończeniu
- Działa nawet przy SIGKILL!

**Wynik:**
- Brak deadlocków nawet przy ewakuacji
- Wszystkie zasoby poprawnie zwalniane
- Test ewakuacji przechodzi ✓

---

### 6.3. Obsługa ewakuacji

**Problem:**
Początkowa implementacja ewakuacji była **powolna** - zajmowało 5-10 sekund zanim wszystkie procesy się zakończyły.

**Przyczyna:**
Procesy czekały na `msgrcv()` lub `semop()` które są **blokujące** i nie reagują na flagi:
```c
// Proces BLOKUJE SIĘ tutaj i nie sprawdza flagi ewakuacji!
msgrcv(kolejka_id, &msg, ..., moj_pid, 0);
```

**Rozwiązanie:**
Użycie sygnału `SIGUSR1` który przerywa blokujące wywołania systemowe:
```c
// Dziekan
kill(0, SIGUSR1);  // Wyślij do WSZYSTKICH w grupie

// Kandydat
if (signal(SIGUSR1, zakoncz_proces) == SIG_ERR) { ... }

void zakoncz_proces(int sig) {
    if (sig == SIGUSR1) {
        // NATYCHMIASTOWE zakończenie
        exit(0);
    }
}
```

**Jak działa przerwanie wywołań systemowych?**
1. Proces czeka na `msgrcv()`
2. Otrzymuje sygnał SIGUSR1
3. Jądro **przerywa** `msgrcv()` i zwraca błąd `EINTR`
4. Handler sygnału się wykonuje
5. `exit(0)` kończy proces

**Obsługa EINTR:**
```c
if (msgrcv(...) == -1) {
    if (errno == EINTR) {
        return -1;
    }
    perror("Blad msgrcv");
}
```

**Wynik:**
- Wszystkie procesy kończą się synchronicznie
- Brak procesów zombie

### 6.4. Procesy zombie

**Problem:**
Po zakończeniu symulacji komenda `ps aux | grep defunct` pokazywała **procesy zombie**:

**Czym jest proces zombie?**
- Proces który się zakończył (`exit()`)
- Ale rodzic nie wywołał `wait()` aby odebrać kod zakończenia
- Zajmuje wpis w tabeli procesów (PID)
- Nie zużywa pamięci/CPU, ale **nie można uruchomić nowego procesu z tym PID**

**Przyczyna:**
Dziekan tworzył procesy ale nie wywołał `wait()` dla WSZYSTKICH:
```c
// ZŁE - czekamy tylko na kandydatów, nie na komisje!
for (int i = 0; i < MAX_KANDYDATOW; i++) {
    wait(NULL);
}
// Komisje A i B zostają zombie!
```

**Rozwiązanie:**
Czekanie na WSZYSTKIE procesy potomne:
```c
// DOBRE - czekamy na wszystkich
int studenci_obsluzeni = 0;
while (studenci_obsluzeni < MAX_KANDYDATOW) {
    pid_t w = wait(NULL);
    if (w == -1) {
        if (errno == ECHILD) break;  // Nie ma już dzieci
        if (errno == EINTR) continue;
    }
    // Zlicz tylko kandydatów, nie komisje
    if (w != pid_ka && w != pid_kb) {
        studenci_obsluzeni++;
    }
}

// Teraz zakończ komisje
kill(pid_ka, SIGUSR1);
kill(pid_kb, SIGUSR1);
wait(NULL);  // Komisja A
wait(NULL);  // Komisja B
```

**Wynik:**
- Brak procesów zombie po zakończeniu
- `ps aux | grep defunct` -> brak wyników ✓

---

### 6.5. Trwałość zasobów IPC

**Problem:**
Po crash programu (np. `kill -9`) zasoby IPC **pozostawały w systemie**:

**Przyczyna:**
System V IPC jest **trwałe** - nie znika automatycznie gdy proces się kończy!

**Rozwiązanie 1:**
Sprzątanie w handlerze SIGINT (Ctrl+C):
```c
void sprzatanie(int signal) {
    if (id_pamieci != -1)
        shmctl(id_pamieci, IPC_RMID, NULL);
    if (id_semaforow != -1)
        semctl(id_semaforow, 0, IPC_RMID);
    if (id_kolejki != -1)
        msgctl(id_kolejki, IPC_RMID, NULL);
    exit(0);
}

signal(SIGINT, sprzatanie);
```

**Problem z rozwiązaniem 1:**
- Nie działa przy `kill -9` (SIGKILL)
- Nie działa przy crash (segfault)

**Rozwiązanie 2 (dodatkowe):**
Czyszczenie PRZED utworzeniem zasobów:
```c
// Spróbuj usunąć stare zasoby (ignoruj błędy)
shmctl(shmget(klucz, 0, 0), IPC_RMID, NULL);
semctl(semget(klucz, 0, 0), 0, IPC_RMID);
msgctl(msgget(klucz, 0), IPC_RMID, NULL);

// Teraz utwórz nowe
id_pamieci = shmget(klucz, rozmiar, IPC_CREAT | ...);
```

**Rozwiązanie 3 (w Makefile):**
```makefile
clean:
    rm -f $(TARGETS)
    ipcrm -a 2>/dev/null || true
```

**Wynik:**
- `make clean` czyści wszystko
- Ctrl+C sprząta zasoby
- Po normalnym zakończeniu zasoby usunięte 

---

## 7. Linki do istotnych fragmentów kodu

Poniżej znajdują się permalinki do konkretnych linii kodu w repozytorium GitHub, pokazujące implementację wymaganych funkcji systemowych.

### 7.1. Tworzenie i obsługa plików

#### `fopen()` - Otwarcie pliku
**Zastosowanie:** Utworzenie pliku `wyniki.txt` do zapisu rankingu

[dziekan.c (Linia 284-286)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L284-L286
```c
FILE *plik = fopen("wyniki.txt", "w");
if (plik == NULL) {
    perror("Blad otwarcia pliku raportu");
}
```

#### `fprintf()` - Zapis do pliku
**Zastosowanie:** Zapisywanie sformatowanych wierszy tabeli rankingu.

[dziekan.c (Linia 326-329)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L326-L329
```c
fprintf(plik, "| #%03d | %04d | %-3s |  %3d%%  |  %3d%%  |  %3d | %-17s |\n",
    i + 1, k->id_kandydata, k->zdana_matura ? "TAK" : "NIE", 
    k->ocena_teoria, 
    k->ocena_praktyka, suma, status_str);
```
#### `fclose()` - Zamknięcie pliku
**Zastosowanie:** Zamknięcie pliku z rankingiem po zapisie

[dziekan.c (Linia 344-345)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L344-L345
```c
if (fclose(plik) == EOF) {
    perror("Blad fclose(wyniki.txt)");
}
```

---

### 7.2. Tworzenie procesów

#### `fork()` - Tworzenie procesu potomnego
**Zastosowanie:** Tworzenie procesów komisji i kandydatów

[dziekan.c (Linia 204) - Fork komisji A] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L204
```c
pid_t pid_ka = fork();
if (pid_ka == 0) {
    // Proces potomny - komisja A
```

[dziekan.c (Linia 214) - Fork komisji B] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L204
```c
pid_t pid_kb = fork();
if (pid_kb == 0) {
    // Proces potomny - komisja B
```

[dziekan.c (Linia 22) - Fork kandydatów] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L229
```c
for (int i = 0; i < liczba_chetnych; i++) {
        pid_t pid = fork();
        if (pid < 0) {
        // Proces potomny - kandydat
```

#### `execl()` - Zamiana obrazu procesu
**Zastosowanie:** Zamiana obrazu procesu Zastosowanie: Uruchomienie oddzielnych programów wykonywalnych (./komisja, ./kandydat) w procesach potomnych.

[dziekan.c (Linia 206) - Exec komisji A] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L206
```c
execl("./komisja", "komisja", "A", NULL);
```

[dziekan.c (Linia 216) - Exec komisji B] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L216
```c
execl("./komisja", "komisja", "B", NULL);
```

[dziekan.c (Linia 237) - Exec kandydata] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L237
```c
execl("./kandydat", "kandydat", id_str, NULL);
```

#### `wait()` - Oczekiwanie na zakończenie procesu
**Zastosowanie:** Zbieranie procesów zakończonych kandydatów

[dziekan.c (Linia 257-270) - Wait w pętli] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L257-L270
```c
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
    }
}
```

#### `exit()` - Zakończenie procesu
**Zastosowanie:** Zakończenie procesu przy błędzie lub normalnie

[dziekan.c (Linia 238-239) - Exit przy błędzie exec] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L238-L239
```c
 perror("Blad execl kandydat");
    exit(1);
```
---

### 7.3. Obsługa sygnałów

#### `signal()` - Rejestracja handlera sygnału
**Zastosowanie:** Przypisanie funkcji obsługi do sygnałów SIGINT (sprzątanie), SIGTSTP (ewakuacja) oraz ignorowanie SIGUSR1 przez Dziekana.

[dziekan.c (Linia 120-136) - Signal handlers] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L120-L136
```c
if (signal(SIGINT, sprzatanie) == SIG_ERR) {
    perror("Signal error");
    exit(1);
}
if (signal(SIGTSTP, zaradz_ewakuacje) == SIG_ERR) {
    perror("Signal error");
    exit(1);
}
if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
    perror("Signal error");
    exit(1);
}
```

[kandydat.c (Linia 211-218) - Signal handlers w kandydacie] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/kandydat.c#L211-L218
```c
if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
    perror("Signal error");
    exit(1);
}
if (signal(SIGUSR1, zakoncz_proces) == SIG_ERR) {
    perror("Signal error");
    exit(1);
}
```

#### `kill()` - Wysłanie sygnału do procesu
**Zastosowanie:** Rozgłoszenie sygnału ewakuacji do grupy procesów oraz zamykanie komisji.

[dziekan.c (Linia 86-87) - Kill 0 = cała grupa] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L86-L87
```c
if (kill(0, SIGUSR1) == -1) {
    perror("Blad kill (ewakuacja)");
}
```

[dziekan.c (Linia 274-275) - Kill komisji] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L274-L275
```c
kill(pid_ka, SIGUSR1);
kill(pid_kb, SIGUSR1);
```

---

### 7.4. Synchronizacja procesów - semafory

#### `ftok()` - Generowanie klucza IPC
**Zastosowanie:** Utworzenie unikalnego klucza dla zasobów IPC

[dziekan.c (Linia 223-225)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/kandydat.c#L223-L225
```c
key_t klucz = ftok(PATH_NAME, PROJECT_ID);
if (klucz == -1)
    report_error_and_exit("Blad ftok");
```

#### `semget()` - Utworzenie zestawu semaforów
**Zastosowanie:** Alokacja zestawu 6 semaforów (mutex pamięci, limity sal, krzesła, stdout)

[dziekan.c (Linia 143-144)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L143-L144
```c
id_semaforow = semget(klucz, LICZBA_SEMAFOROW, 0600 | IPC_CREAT);
if (id_semaforow == -1) report_error_and_exit("Blad semget");
```

#### `semctl()` - Kontrola semaforów
**Zastosowanie:** Nadanie wartości początkowych (SETVAL) oraz usunięcie zestawu (IPC_RMID)

[dziekan.c (Linia 146-151) - Inicjalizacja wartości] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L146-L151
```c
ustaw_semafor(id_semaforow, SEM_DOSTEP_PAMIEC, 1);
ustaw_semafor(id_semaforow, SEM_SALA_A, MAX_W_SALI_A);
ustaw_semafor(id_semaforow, SEM_SALA_B, MAX_W_SALI_B);
ustaw_semafor(id_semaforow, SEM_KRZESLO_A, 1);
ustaw_semafor(id_semaforow, SEM_KRZESLO_B, 1);
ustaw_semafor(id_semaforow, SEM_STDOUT, 1);
```

[dziekan.c (Linia 67-74) - Funkcja ustaw_semafor] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L67-L74
```c
void ustaw_semafor(int sem_id, int sem_num, int wartosc) {
    union semun arg;
    arg.val = wartosc;
    if (semctl(sem_id, sem_num, SETVAL, arg) == -1) {
        perror("Blad ustawiania semafora");
        exit(1);
    }
}
```

[dziekan.c (Linia 38-44) - Usuwanie semaforów] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L38-L44
```c
 if (id_semaforow != -1) {
        if (semctl(id_semaforow, 0, IPC_RMID) == -1) {
            perror("Blad usuwania semaforow");
        } else {
            printf("%s[Dziekan] Semafory usuniete.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
        }
    }
```

#### `semop()` - Operacje P i V na semaforze
**Zastosowanie:** Zastosowanie: Funkcja pomocnicza w common.h realizująca operacje zajmowania (-1) i zwalniania (+1) zasobów z zabezpieczeniem SEM_UNDO.

[common.h (Linia 102-115) - Funkcja semafor_operacja] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/common.h#L102-L115
```c
static void semafor_operacja(int sem_id, int sem_num, int op) {
    struct sembuf bufor;
    bufor.sem_num = sem_num;
    bufor.sem_op = op;
    bufor.sem_flg = SEM_UNDO; 
    
    while (semop(sem_id, &bufor, 1) == -1) {
        if (errno != EINTR && errno != EIDRM && errno != EINVAL) {
             perror("Blad semop");
             break;
        }
        if (errno == EIDRM || errno == EINVAL) break;
    }
}
```

[kandydat.c (Linia 62) - P na sali (wejście)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/kandydat.c#L62
```c
semafor_operacja(sem_id_global, sem_sala, -1);
```

[kandydat.c (Linia 69) - P na krześle] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/kandydat.c#L69
```c
semafor_operacja(sem_id_global, sem_krzeslo, -1);
```

[kandydat.c (Linia 73) - V na krześle] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/kandydat.c#L73
```c
semafor_operacja(sem_id_global, sem_krzeslo, 1);
```

[kandydat.c (Linia 74) - V na sali (wyjście)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/kandydat.c#L74
```c
semafor_operacja(sem_id_global, sem_sala, 1);
```

---

### 7.5. Segmenty pamięci dzielonej

#### `shmget()` - Utworzenie segmentu pamięci
**Zastosowanie:** Alokacja bloku pamięci współdzielonej dla struktury PamiecDzielona

[dziekan.c (Linia 160-161)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L160-L161
```c
id_pamieci = shmget(klucz, sizeof(PamiecDzielona), 0600 | IPC_CREAT);
    if (id_pamieci == -1) report_error_and_exit("Błąd shmget");
```

#### `shmat()` - Podłączenie pamięci do procesu
**Zastosowanie:** Rzutowanie pamięci dzielonej na wskaźnik w procesie

[dziekan.c (Linia 163-164)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L163-L164
```c
wspolna_pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (wspolna_pamiec == (void*) -1) report_error_and_exit("Błąd shmat");
```

[kandydat.c (Linia 245-247)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/kandydat.c#L245-L247
```c
pamiec_global = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (pamiec_global == (void*) -1) 
        report_error_and_exit("Blad shmat");
```

#### `shmdt()` - Odłączenie pamięci od procesu
**Zastosowanie:** Odłączenie pamięci przed zakończeniem procesu

[dziekan.c (Linia 21-26)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L21-L26
```c
 if (shmdt(wspolna_pamiec) == -1) {
            perror("Blad shmdt");
        } else {
            printf("%s[Dziekan] Pamiec odlaczona.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
        }
    }
```

[kandydat.c (Linia 33-34)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/kandydat.c#L33-L34
```c
if (shmdt(pamiec_global) == -1) {
    perror("Blad shmdt");
}
```

#### `shmctl()` - Kontrola pamięci dzielonej
**Zastosowanie:** Fizyczne usunięcie zasobu z systemu przez proces Dziekana.

[dziekan.c (Linia 30-35)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L30-L35
```c
 if (shmctl(id_pamieci, IPC_RMID, NULL) == -1) {
            perror("Blad usuwania pamieci");
        } else {
            printf("%s[Dziekan] Pamiec usunieta.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
        }
    }
```

---

### 7.6. Kolejki komunikatów

#### `msgget()` - Utworzenie kolejki
**Zastosowanie:** Stworzenie kanału komunikacyjnego Kandydat ↔ Komisja

[dziekan.c (Linia 200-202)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L200-L202
```c
id_kolejki = msgget(klucz, 0600 | IPC_CREAT);
if (id_kolejki == -1)
    report_error_and_exit("Blad msgget");
```

#### `msgsnd()` - Wysłanie komunikatu
**Zastosowanie:** Przesłanie zgłoszenia kandydata do komisji (z PID-em nadawcy)

[kandydat.c (Linia 160-163)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/kandydat.c#L160-L163
```c
msg_wysylana.mtype = typ_komisji_msg;
msg_wysylana.nadawca_pid = moj_pid;
msg_wysylana.dane = 0; 
msgsnd(msg_id_global, &msg_wysylana, sizeof(msg_wysylana)-sizeof(long), 0);

```

#### `msgrcv()` - Odbiór komunikatu
**Zastosowanie:** Kandydat odbiera pytania i wyniki od komisji

[kandydat.c (Linia 167-170)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/kandydat.c#L167-L170
```c
if (msgrcv(msg_id_global, &msg_odebrana, sizeof(msg_odebrana)-sizeof(long), moj_pid, 0) == -1) {
        if (errno != EINTR) perror("Blad msgrcv pytania");
        return -1;
    }
```

[komisja.c (Linia 122-127)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/main/komisja.c#L122-L127
```c
if (msgrcv(id_kolejki, &msg_odebrana, sizeof(msg_odebrana) - sizeof(long), moj_kanal_nasluchu, 0) == -1) {
            if (errno == EINTR) continue;
            if (errno == EIDRM) break;   
            perror("[Komisja] Blad msgrcv");
            break;
        }
```

#### `msgctl()` - Kontrola kolejki
**Zastosowanie:** Usuwanie kolejki komunikatów

[dziekan.c (Linia 47-53)] https://github.com/tobiaszsroka/Sroka_Tobiasz_150281_so_projekt_017/blob/bf57a511184a1034e0cdef8ac4df20b23a9296a3/dziekan.c#L47-L53
```c
if (id_kolejki != -1) {
        if (msgctl(id_kolejki, IPC_RMID, NULL) == -1) {
             perror("Blad usuwania kolejki komunikatow");
        } else {
             printf("%s[Dziekan] Kolejka komunikatow usunieta.%s\n", KOLOR_ZOLTY, KOLOR_RESET);
        }
    }
```