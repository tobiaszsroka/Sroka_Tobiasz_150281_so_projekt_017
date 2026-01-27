# Projekt Systemy Operacyjne: Symulacja Egzaminu Wstępnego

## Autor
**Imię i Nazwisko:** Tobiasz Sroka  
**Temat:** 17 - Egzamin wstępny na informatykę

## Opis projektu
Projekt symuluje proces przeprowadzania egzaminu wstępnego z wykorzystaniem mechanizmów systemowych Linuxa (IPC). System składa się z zarządcy (Dziekan), komisji egzaminacyjnych oraz kandydatów.

## Wykorzystane mechanizmy
- **Procesy (`fork`, `exec`)**: Każdy kandydat i komisja to osobny proces.
- **Pamięć dzielona (`shmget`, `shmat`)**: Przechowywanie listy wyników i stanu symulacji.
- **Semafory (`semget`, `semop`)**: Synchronizacja dostępu do sal egzaminacyjnych (kolejkowanie) oraz sekcji krytycznych pamięci.
- **Sygnały (`signal`, `kill`)**: Obsługa nagłego przerwania symulacji (Ewakuacja).

## Jak uruchomić?
1. **Kompilacja:**
   Wpisz w terminalu:
   ```bash
   make
   ./dziekan

2. **Symulacja Ewakuacji**
   Podczas trwania egzaminu wcisnij kombinacje klawiszy (CTRL+Z) aby oglosic alarm i przerwac egzamin.

