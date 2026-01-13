#include "common.h"

int main(int argc, char *argv[]) {
    if (argc < 2) exit(1);
    int moj_id = atoi(argv[1]);

    printf("    [Kandydat %d] Uruchomiony. Odczytuje pamiec ...\n", moj_id);

    //Podlaczenie do pamieci
    key_t klucz = ftok(PATH_NAME, PROJECT_ID);
    int id_pamieci = shmget(klucz, 0, 0);
    if (id_pamieci == -1) report_error_and_exit("Blad shmget kandydat");

    PamiecDzielona *pamiec = (PamiecDzielona*) shmat(id_pamieci, NULL, 0);
    if (pamiec == (void*) -1) report_error_and_exit("Blad shmat kandydat");

    //Odczyt danych przygotowanych przez Dziekana
    int czy_matura = pamiec->studenci[moj_id].zdana_matura;
    
    if (czy_matura) {
        printf("   [Kandydat %d] Hura! Mam zdana mature.\n", moj_id);
    } else {
        printf("   [Kandydat %d] Brak matury. Koncze udzial.\n", moj_id);
    }

    shmdt(pamiec);
    return 0;
}
