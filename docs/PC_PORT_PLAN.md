# Plan portu Parasite Eve na PC

Celem jest natywna wersja PC, która zachowuje fabułę, atmosferę, stałe ujęcia kamery i walkę ATB, a poprawia sterowanie, czytelność i wygodę. Najpierw powstaje grywalny port o zachowaniu zgodnym z oryginałem. Ulepszenia interfejsu, grafiki i balansu dochodzą po uzyskaniu stabilnej podstawy.

## Punkt startowy

Stan planu na 13 października 2026: README opisuje port jako niegrywalny. Główny `CMakeLists.txt` dołącza target w `pc/CMakeLists.txt`, który buduje `pc/main.c` i łączy PsyZ; kod gry nie jest jeszcze częścią tego programu. PsyZ jest istniejącą warstwą zastępującą środowisko PSY-Q i należy najpierw ocenić jej pokrycie potrzeb gry, zanim powstanie dodatkowy backend.

Repozytorium rozwijamy od tego miejsca jako niezależny projekt C11. Zgodność binarna, byte match, historyczne kompilatory i kontrole PS1 nie są wymagane. Źródła z `parasite-eve-decomp` są punktem startowym; ich kod można swobodnie zmieniać dla natywnego runtime. Zakres danych i działanie drugiego dysku wymagają osobnego potwierdzenia przed deklaracją pełnej grywalności.

## Priorytety i zależności

| Etap | Priorytet | Wynik | Zależność |
| --- | --- | --- | --- |
| 0. Audyt i kontrakt portu | P0 | Lista przeszkód, scenariusze porównawcze i zasady współdzielenia kodu | Brak |
| 1. Natywny runtime | P0 | Kod gry działa na hoście i wczytuje dane | Etap 0 |
| 2. Grywalny fragment | P0 | Eksploracja, walka, menu i zapis w jednej lokacji | Etap 1 |
| 3. Sterowanie i wybór celów | P1 | Klawiatura, mysz i pad obsługują całą grę | Etap 2 |
| 4. Ekwipunek i interfejs | P1 | Czytelne decyzje i mniej operacji w menu | Etap 3 |
| 5. Zapisy i stabilność | P1 | Trwałe zapisy, autosave i bezpieczne wznowienie | Podstawa z etapu 2 |
| 6. Płynność i oprawa | P2 | Wyższy FPS renderowania bez zmiany tempa gry | Etap 2 i audyt czasu |
| 7. Opcjonalne zmiany rozgrywki | P3 | Zweryfikowane ulepszenia tempa i balansu | Stabilna wersja bazowa |
| 8. Pełna gra i wydanie | P0 dla wydania | Przejście całej gry i powtarzalna paczka | Wszystkie wymagane elementy |

P0 oznacza warunek działania lub wydania; P1 podstawową wygodę PC; P2 poprawę oprawy; P3 eksperyment. Grafika i balans nie blokują pierwszej grywalnej wersji. Nie przypisujemy terminów przed ustaleniem zakresu problemów w etapie 0.

## Etap 0 Audyt i kontrakt portu

- Zbudować i uruchomić istniejący smoke test PsyZ; zapisać platformę, kompilator i wynik.
- Spisać używane API PSY-Q, BIOS, GPU, GTE, SPU, CD i kart pamięci. Dla każdej grupy wskazać obsługę przez PsyZ, adapter gry albo brak implementacji.
- Zinwentaryzować wskaźniki przechowywane w liczbach 32-bitowych, nakładające się symbole, stałe adresy, instrukcje MIPS i bariery wymagane tylko przez matching.
- Rozdzielić formaty plików od struktur runtime: pola zapisane jako 32-bitowe dane dysku nie stają się automatycznie wskaźnikami hosta. Określić endianowość, wyrównanie, signedness i konwersje.
- Rozwijać logikę gry w C11 i adaptery w `pc/`. Zmiany przyjmować na podstawie zachowania gry, przenośności i natywnego buildu; importy z upstream oceniać osobno, bez wymogu zgodności PS1.
- Przygotować bazę porównań: wejście do lokacji, zmiana kamery, walka, umiejętność PE, modyfikacja przedmiotu, zapis i odczyt. Ustalić kontrolę wejścia oraz losowości tam, gdzie umożliwia ją runtime.

**Ukończenie:** lista blokad ma właścicieli modułów, kolejność usuwania i sposób sprawdzania. Znany jest zakres pierwszego grywalnego fragmentu oraz platforma pierwszego wydania; proponowany cel to Windows x64, przy zachowaniu działającego środowiska deweloperskiego macOS.

## Etap 1 Natywny runtime

- Dodawać do targetu PC podsystemy gry w małych grupach, bez dołączania konsolowych bibliotek zastąpionych przez PsyZ.
- Usunąć truncation wskaźników i zależności od mapy RAM PS1. Zastąpić dostęp do scratchpadu i sprzętu jawnymi adapterami.
- Dostosować ordering tables i pakiety GPU do typów PsyZ. Przeprowadzić mały test renderowania tła, modelu, tekstu, przezroczystości i kolejności rysowania.
- Udostępnić archiwum `PE.IMG` przez odczyt plików lub obsługiwany obraz dysku. Zachować sektorowe znaczenie offsetów w danych; oddzielić je od rzeczywistych opóźnień napędu.
- Zastąpić ładowanie wykonywalnych overlayów natywnym mechanizmem rejestracji modułów. Sprawdzić kolizje nazw, inicjalizację i reset stanu przy zmianie lokacji. Dane overlayów przetwarzać niezależnie od kodu.
- Uruchomić audio AKAO i efekty przez dostępną warstwę PsyZ; sprawdzić synchronizację. Osobno zinwentaryzować odtwarzanie FMV.
- Wprowadzić krok symulacji zgodny z oryginałem, oddzielony od prezentacji. Sprawdzić wszystkie ścieżki VSync oraz pętle menu.

**Ukończenie:** program uruchamia właściwy kod gry, ładuje wybraną lokację, rysuje ją i przetwarza wejście. Braki audio lub FMV są jawnie opisane. Nie występują znane błędy wskaźników w tej ścieżce.

## Etap 2 Grywalny fragment

Wybrać krótki fragment zawierający eksplorację, przejście między ujęciami, interakcję, dialog, zwykłą walkę, PE, zdobycie przedmiotu i zapis. To pierwszy kamień milowy nadający się do oceny jakości portu.

- Uruchomić skrypty lokacji, aktorów, kolizje, przejścia i efekty.
- Zweryfikować ATB, ruch, zasięg, obrażenia, zużycie amunicji i PE oraz wyjście z walki.
- Udostępnić podstawowe menu i zapis; tymczasowy wygląd może odpowiadać PS1.
- Porównać zdarzenia i stan gry z oryginałem, korzystając ze stałych sekwencji wejścia i kontrolowanej losowości, gdzie jest to możliwe.

**Ukończenie:** fragment można przejść od początku do końca, zapisać, zamknąć program i kontynuować. Tempo ruchu i walki jest stałe przy różnych częstotliwościach prezentacji.

## Etap 3 Sterowanie i wybór celów

- Wprowadzić mapowanie akcji gry zamiast wiązania logiki z fizycznymi bitami pada. Zachować osobną obsługę przytrzymania, naciśnięcia i powtarzania w menu.
- Dodać WASD, przypisywanie klawiszy, bieg jako przytrzymanie lub przełącznik, martwą strefę analoga i automatyczne podpowiedzi dla aktywnego urządzenia.
- Sprawdzić utrzymanie kierunku przy zmianie kamery do puszczenia wejścia. Użytkownik może wybrać zachowanie oryginalne.
- Dodać wskazanie przeciwnika myszą, przełączanie klawiszem Tab i obsługę celów zasłoniętych lub nakładających się na ekranie. Mysz wybiera cel; zasady wykonania akcji pozostają zależne od ATB.
- Pokazywać zaznaczenie oraz stan „poza zasięgiem”. Wybór respektuje istniejącą listę legalnych celów, śmierć przeciwnika i ograniczenia umiejętności.
- Obsłużyć utratę fokusu, odłączenie pada i powrót do okna bez zablokowanych przycisków.

**Ukończenie:** cały fragment jest dostępny samą klawiaturą i samym padem; mysz działa w menu i wyborze celów. Nowe sterowanie nie omija kosztów ani ograniczeń akcji.

## Etap 4 Ekwipunek i interfejs

- Dodać skalowanie UI i fontów, czytelne podpowiedzi oraz układ działający w oknie i na ekranach o różnym DPI.
- Pokazywać wyposażony i wskazany przedmiot jednocześnie, różnice statystyk i opisy właściwości. Wykorzystać istniejącą logikę porównań i zgodności wyposażenia.
- Dodać sortowanie i filtrowanie, zachowując poprawne identyfikatory przedmiotów oraz referencje wyposażenia po zmianie widoku.
- Wprowadzić podgląd wyniku modyfikacji: statystyki, przenoszone właściwości, zużywane narzędzia i przedmioty tracone przy zatwierdzeniu. Podgląd nie zmienia stanu gry; zatwierdzenie ponownie sprawdza warunki operacji.
- Dodać ulubione lub blokadę przed przypadkowym wyrzuceniem oraz szybsze przenoszenie do magazynu.
- Uczytelnić koszt PE, amunicję i statusy bez ujawniania informacji o przeciwniku, które oryginalna mechanika udostępnia dopiero po odpowiedniej akcji.

**Ukończenie:** porównanie, wyposażenie i modyfikacja wymagają mniej operacji niż w interfejsie bazowym. Wynik podglądu odpowiada zatwierdzonej zmianie; sortowanie nie zmienia wyposażenia. Pojemność ekwipunku pozostaje zgodna z wersją bazową.

## Etap 5 Zapisy i stabilność

- Zastąpić wybór portów kart pamięci listą zapisów z lokacją, dniem fabuły i czasem gry.
- Określić wersjonowany format zapisu i migracje; unikać zrzutów struktur zawierających wskaźniki hosta. Zgodność z zapisami PS1 ocenić osobno.
- Stosować zapis do pliku tymczasowego, atomową podmianę i zachowanie poprzedniej kopii. Zweryfikować niepełny lub uszkodzony plik i brak miejsca na dysku.
- Dodać rotacyjne autosave’y w bezpiecznych punktach, po zakończeniu przejścia lub zdarzenia. Nie zapisywać częściowo zmienionego stanu w trakcie ładowania.
- Dodać „zawieś i wyjdź” początkowo wyłącznie w stanach, które można w pełni odtworzyć. Zapis w dowolnej fazie walki wymaga odtworzenia kolejki akcji, skryptów i efektów.
- Zapisywać ustawienia sterowania, obrazu i audio niezależnie od postępu gry.

**Ukończenie:** odczyt przywraca ekwipunek, fabułę i stan lokacji. Nieudany zapis nie niszczy poprzedniego; autosave nie zastępuje zapisu ręcznego. Zakres wznowienia jest jasno pokazany w UI.

## Etap 6 Płynność i oprawa

- Zinwentaryzować liczniki animacji, ATB, skryptów, efektów i audio. Nie usuwać po prostu oczekiwania VSync.
- Renderować z interpolacją w 60/120 Hz przy zachowaniu oryginalnego kroku symulacji. Resetować interpolację przy zmianie pokoju, teleportacji i cięciu kamery.
- Dodać opcje skalowania teł, ostrego lub filtrowanego obrazu, rozdzielczości renderowania modeli i efektów. Ocenić spójność tła, maskowania i geometrii.
- Najpierw dopracować 4:3. Panoramę wdrożyć dopiero po audycie granic teł, kadrowania, masek zasłaniania, interakcji i scen fabularnych; pozwolić lokacjom wymagającym oryginalnego kadru pozostać w 4:3.
- Sprawdzić FMV pod kątem proporcji, synchronizacji i płynnego przejścia do gry.

**Ukończenie:** częstotliwość renderowania nie zmienia czasu ruchu, ATB, statusów ani skryptów. Nie występują smugi interpolacji przy cięciach; tła i modele pozostają poprawnie zasłaniane.

## Etap 7 Opcjonalne zmiany rozgrywki

Wprowadzać pojedynczo, z możliwością przywrócenia zachowania bazowego:

- Skróty do ulubionych umiejętności i przedmiotów, respektujące ATB i koszty.
- Historię dialogów i pomijanie obejrzanych scen tylko w zweryfikowanych punktach, z zachowaniem flag fabuły i skutków skryptów.
- Skracanie powtarzalnych animacji bez pomijania trafień, kosztów i zdarzeń.
- Opcjonalne ułatwienia czytelności: dodatkowe oznaczenia celu, wielkość tekstu i podpowiedzi niezależne od koloru.
- Eksperymenty z balansem broni, tempem ATB, pojemnością ekwipunku i częstotliwością potyczek dopiero po pomiarach oraz ograniu wersji bazowej.

**Ukończenie:** każda zmiana ma opis wpływu, konfigurację i porównanie z wersją bazową. Balans wymaga osobnej decyzji o zakresie; nie jest domyślnym skutkiem portowania.

## Etap 8 Pełna gra i wydanie

- Sprawdzić wszystkie dostępne lokacje, bossów, specjalne walki, efekty, menu i FMV, w tym różne ścieżki fabularne.
- Potwierdzić obsługę obu dysków, zmiany dysku, końca gry i trybów dodatkowych w zakresie planowanego wydania.
- Przejść grę od początku do końca na docelowej platformie, regularnie zapisując i wznawiając. Wykonać dłuższe sesje oraz przejścia między wieloma lokacjami, obserwując pamięć i zasoby.
- Przygotować powtarzalny build, paczkę bez danych gry, instrukcję wskazania własnych danych i diagnostykę brakujących lub niezgodnych plików.
- Sprawdzić uruchomienie paczki na czystym systemie oraz ścieżki zawierające spacje i znaki narodowe.

**Ukończenie:** pełne przejście nie wymaga narzędzi deweloperskich ani ręcznej naprawy stanu. Lista znanych ograniczeń odpowiada rzeczywistemu zakresowi wydania.

## Najważniejsze ryzyka

| Ryzyko | Sposób ograniczenia |
| --- | --- |
| 64-bitowe wskaźniki i formaty PS1 | Jawne typy, konwersje na granicy danych, ostrzeżenia kompilatora i sanitizery dla wspieranych ścieżek |
| Overlaye i nakładające się symbole | Audyt nazw oraz osobny model rejestracji i inicjalizacji modułów |
| Niepełne pokrycie API przez PsyZ | Małe scenariusze GPU, GTE, audio i odczytu danych przed pełną integracją |
| Czas gry zależny od klatek | Stały krok symulacji oraz porównanie stanu po takim samym czasie symulacji |
| Szybsze ładowanie zmienia kolejność zdarzeń | Zachować kontrakt zakończenia operacji i przejść skryptów |
| Zapis w niespójnym stanie | Jawne bezpieczne punkty zapisu i test odtworzenia po ponownym uruchomieniu |
| Panorama odsłania brakujące tła | Audyt lokacji i możliwość zachowania oryginalnego kadru |
| Regresje przy zmianach kodu gry | Natywne testy zachowania i powtarzalne scenariusze rozgrywki; selektywne importy poprawek upstream |

## Pierwsze zadania do realizacji

1. Uruchomić smoke test z obecnego README i ustalić pierwszą platformę docelową.
2. Sporządzić mapę API oraz problemów wskaźników, adresów i overlayów.
3. Wybrać jeden pokój i jego wymagane podsystemy jako pierwszy pionowy fragment.
4. Dołączyć minimalny kod gry do targetu PC i uruchomić odczyt jego danych.
5. Doprowadzić fragment do eksploracji, jednej walki oraz zapisu i odczytu.
6. Na działającej podstawie wdrożyć sterowanie PC i porównywanie wyposażenia.

Pierwszy grywalny fragment jest punktem ponownej oceny zakresu i terminów. Dostarcza też podstawy do oceny, czy proponowane ulepszenia rzeczywiście poprawiają grę.
