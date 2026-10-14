# Plan implementacji pełnego portu Parasite Eve na PC

Plan obejmuje całą grę, nie tylko boot i filmy. Celem jest ukończony natywny projekt C11: pełna kampania, oba dyski, wszystkie lokacje i sceny, eksploracja, walka ATB, PE, przeciwnicy, przedmioty, ekwipunek, menu, audio, zapis/odczyt, Tutorial oraz tryby i zawartość dodatkowa. Każda funkcja i każdy element zawartości mają otrzymać automatyczny test oraz mierzalny warunek odbioru.

„Perfekcyjny” oznacza pełny uzgodniony zakres, poprawne zachowanie, brak znanych defektów i komplet wymaganych dowodów automatycznych. Sam build, procent pokrycia kodu, ekran menu albo liczba przetestowanych plików nie dowodzą ukończenia gry. Nie można udowodnić braku błędów dla wszystkich możliwych kombinacji stanu i sprzętu; dlatego jawnie definiujemy katalog funkcji, niezmienniki, warianty, obsługiwane platformy i testy błędów. Żadna pozycja zakresu nie może zniknąć z raportu przez pominięcie testu.

Byte match, odbudowa EXE PS1, historyczne kompilatory, kolejność instrukcji i adresy kodu nie są wymagane. Oryginał jest wzorcem zachowania i źródłem danych; port ma normalne typy, wskaźniki, moduły i build C. Poprawiamy błędy pamięci i przenośności. Zmiany balansu i tempa pozostają osobnymi, opcjonalnymi funkcjami.

## 1. Stan rzeczywisty i zakres wydania

Aktualizacja audio: `akao-sequence` w CTest sprawdza maski i offsety kanałów oraz uszkodzone dane. `room-assets` kataloguje 600 rekordów muzycznych Disc 1: 599 pełnych sekwencji z poprawnymi granicami i jeden jawnie nierozpoznany krótki rekord ID 0. Nie wykonujemy jeszcze instrukcji sekwencera ani syntezy; ten nierozpoznany rekord nie jest uznany za obsłużoną zawartość. Łącznie CTest rejestruje obecnie 12 testów.

Punkt wyjściowy audytu: commit `83aa1c5f8`, 14 października 2026. Poniższy stan uwzględnia także wdrożony runner headless z WP01.

| Obszar | Co istnieje | Czego obecne dowody nie potwierdzają |
| --- | --- | --- |
| Boot | `pc/disc.c`, `movie.c`, `boot.c`, `title.c`; intro i oryginalne grafiki menu; wybór New Game/Tutorial przygotowuje natywny `PeScene`: dane M431/M353, VRAM, katalog skryptów i wspólny bank startowy SPU | New Game i Tutorial nie uruchamiają scen ani rozgrywki; Continue jest wyłączone; sekwencer/synteza AKAO oraz banki audio konkretnych pokojów pozostają do wdrożenia |
| Logika gry | 441 plików C pod `src/`, podsystemy gry i legacy SDK | Kod gry nie jest częścią natywnego targetu; same źródła nie dowodzą kompletności implementacji ani danych |
| Zawartość | Źródła współdzielone i 22 katalogi pod `src/overlays/`; notatki o pokojach | Katalog źródeł nie jest katalogiem wszystkich map, scen i wariantów obu dysków |
| Testy | `disc-reader`, `vram`, `script-format`, `script-runtime`, `script-scheduler`, `spu-bank`, `room-assets`, `automation`, `boot-assets`, `boot-replay`, `boot-newgame` w CTest; 414 niepustych / 450 slotów pokojów Disc 1; uploady tekstur; 2411 offsetów skryptów, 594 deskryptory aktorów i pierwsze komendy; bank startowy SPU; rollback/lifetimes `PeScene`; obie gałęzie zadania M431 entry 0; wszystkie 24 operatory; retry/yield/błędy; pula 72 zadań, współdzielony stan aktora, kolejność dzieci, deferred GC i błędne listy | Brak walidacji wszystkich payloadów/gałęzi skryptów, renderingu scen i pełnego wykonywania skryptów; aktywny aktor M431 zatrzymuje się w planisty na niezaimplementowanym AKAO opcode 234. Callbacki testowe nie dowodzą integracji audio/renderingu. Brak automatycznych kampanii, walk, zapisów, referencyjnego obrazu i audio |
| CI | Build i CTest na macOS; testy `disc-reader`, `vram`, `script-format`, `script-runtime`, `script-scheduler`, `spu-bank` i `automation` bez danych gry, w tym uszkodzone dane, banki operandów, ticki, arytmetyka, nieobsługiwane komendy, limity instrukcji i puli zadań | Cztery testy z danymi zwracają 77 bez `PE_TEST_DISC`, więc publiczny CI nie dowodzi działania gry z danymi; `PE_REQUIRE_GAME_ASSETS=1` zamienia brak danych w błąd |
| Test lokalny bootu | Zapisano wynik całego intro i podglądy w lokalnym `build/qa/` | To nie jest jeszcze wymagany, samodzielny test regresyjny obrazu, wyjścia audio i synchronizacji |

Założenie wydania 1.0: wersja USA obu dysków, zachowanie oryginalnej gry i pełna zawartość odblokowywana w normalnej rozgrywce. Windows x64 jest głównym celem wydania; macOS arm64/x64 i Linux x64 są pełnoprawnymi celami desktopowymi. Ich minimalne wersje systemów, GPU i konfiguracje audio ustalamy i zapisujemy w WP00. Wersje JP/PAL, mobile i web mają odrębne katalogi wymagań; nie deklarujemy ich działania na podstawie testów wersji USA.

CLI `--discs <Disc 1.cue/bin> <Disc 2.cue/bin>` montuje oba źródła przez cały boot; diagnostyczne `--disc` nadal przyjmuje jedno. `pc/assets.c` zachowuje pochodzenie i pozwala czytać oba źródła bez zmiany płyty, a `disc.c` obsługuje jednotrackowy CUE BINARY MODE2/2352 z relatywną ścieżką FILE i offsetem INDEX 01. Testy syntetyczne potwierdzają kolizje nazw, odczyty naprzemienne, offset pregapu i błędy otwierania; rzeczywisty CUE Disc 1 przechodzi testy bootu. Weryfikacja ról/wersji pary, pełny katalog i rzeczywiste dane Disc 2 pozostają do sprawdzenia w WP02/PR04. Własny fork udostępnia hi-res, ale integracja hi-res z pełnym rendererem Parasite Eve wymaga WP03.

Materiały gry pozostają lokalne/prywatne. Publiczne repo i paczka zawierają kod oraz wygenerowane syntetyczne fixture’y, bez obrazów dysków i wyodrębnionych zasobów.

## 1.1. Przyjęte zasady portowania

**Wierność rozgrywce jest ważniejsza od zgodności z API PS1.** Zachowujemy reguły, progresję, kolizje, AI, animacyjne zdarzenia, koszty i kolejność istotnych zdarzeń. API PS1 można usuwać, scalać, upraszczać i zastępować zwykłym C; nie trzeba utrzymywać kompatybilnej fasady PSY-Q, BIOS, przerwań, DMA ani CD, jeśli nie jest potrzebna do zachowania gry. Testy sprawdzają rezultat domenowy, nie odtworzenie mechaniki sprzętu.

**Oba CUE/BIN tworzą jeden logiczny zestaw danych.** Docelowa konfiguracja wymaga wskazania i zweryfikowania obu obrazów przed rozpoczęciem pełnej gry. Warstwa zasobów automatycznie wybiera źródło; gracz nie zmienia płyt i nie ogląda komunikatu o włożeniu CD2. To logiczne scalenie katalogów, nie obowiązek tworzenia dużego nowego BIN ani ładowania całych obrazów do RAM. Nazwy i LBA mogą kolidować między obrazami: katalog musi zachować pochodzenie, wersję i jawne reguły wyboru zasobu. Audyt obu dysków w WP00 potwierdza mapowanie i różnice zawartości, a WP02 wdraża ten model.

**Nie obsługujemy fizycznego napędu CD.** Pozostają odczyt plików, parser CUE/tracków i sektorów BIN, dane XA/STR oraz kontrolowana obsługa uszkodzonego lub niedostępnego pliku. Nie implementujemy otwierania pokrywy, wysuwania tacki, prędkości/seek napędu, sprzętowych komend CD ani ich kolejek. Stare testy zmiany fizycznej płyty zastępują testy kompletności zestawu i płynnego przejścia między zawartością obrazów. Skrócenie ładowania nie może zmienić progresji ani pominąć zdarzenia skryptowego.

**Overlaye są statycznymi modułami C.** Kod linkujemy raz do programu, wspólne procedury współdzielimy, a pokoje/sceny wybieramy rejestrem. Nie trzeba ładować/wykonywać kodu z obrazu, relokować overlayów ani odtwarzać okien RAM PS1. Dane nadal są ładowane według potrzeb. Każdy moduł ma jawny stan, wejście, wyjście i reset; statyczne linkowanie nie może przypadkiem zachować stanu, który znikał przy wymianie overlayu.

**Scratchpad zastępujemy zwykłą pamięcią hosta.** Używamy lokalnych zmiennych, buforów kontekstu lub kontrolowanego obszaru roboczego; nie odtwarzamy adresu scratchpadu ani przełączania stosu. Zachowujemy potrzebne rozmiary danych, lifetime i znaczenie współdzielenia, a testy wykrywają stare dane oraz aliasowanie wpływające na gameplay.

**Hi-res od początku, hi-FPS później.** Korzystamy z istniejącego skalowania wewnętrznej rozdzielczości PsyZ (`Psyz_VideoSetInternalResolution` w `psyz/video.h`, obecnie mnożniki 1–8). WP03 integruje je z rendererem gry i testuje tła, maski, modele, efekty, UI oraz niezmieniony stan symulacji. Samo powiększenie okna nie wystarcza. Pierwsze pełne wydanie zachowuje właściwe kadencje oryginału; wysokie FPS i interpolacja są osobnym WP13, który nie blokuje pełnej gry z hi-res. Od początku oddzielamy zegar symulacji od prezentacji, aby późniejszy hi-FPS nie wymagał zmiany reguł gry.

**Utrzymujemy własny PsyZ.** Źródłem submodułu jest [khasinski/parasite-eve-psyz](https://github.com/khasinski/parasite-eve-psyz), z `Xeeynamo/psyz` jako upstream. Fork zachowuje historię kodu i jest osobnym repozytorium GitHub; nie współdzieli repozytorium Rage Racer. Możemy dopisywać brakujące możliwości oraz upraszczać integrację. Generyczne GPU/GTE/audio/runtime trafiają do forka z testami; reguły Parasite Eve i dekodowanie jej danych pozostają w projekcie gry. Nie rozbudowujemy PS1 API tylko po to, by zachować stare nazwy wywołań.

## 1.2. Samodzielne testowanie i widoczność

Agent musi móc wykonać i ocenić każdy wymagany test bez działania użytkownika.
Headless jest domyślnym trybem testowania: brak widocznego okna i syntetyczne
urządzenia wejścia/wyjścia. Sprzętowe testy backendów także mają działać bez
ręcznego klikania, przez kontrolowany proces i capture, nie przez prośbę o ocenę
obrazu lub dźwięku. Żaden test nie czeka bezterminowo na input.

Runner ma pełne sterowanie akcjami i stanem: inicjalizacja sesji, seed RNG,
czas/ticki, hold/release i akcje urządzeń, fixture/checkpoint stanu, zapis/odczyt,
I/O i wstrzyknięte błędy. Pełne kampanie nadal używają zwykłej rozgrywki, nie
bezpośredniego przestawiania flag. Obserwacje obejmują kanoniczny stan gry,
zdarzenia/skrypty, aktorów, input, render, audio i kolejki. Oczekiwanie na stan,
asercje, trace i timeout są częścią interfejsu programu, nie automatyzacją pulpitu.

Obecny frontend ma `--headless`, `--replay`, `--trace` i konfigurowany timeout;
`pc/automation.h` publikuje rzeczywiste stany prezentacji i pozwala sterować
wejściem testu. To początek WP01, nie dowód pełnego sterowania niepodłączoną
jeszcze logiką gry. Stan domenowy i wirtualny zegar zostaną dołączone przy WP02.

`save-editor/` to osobny projekt prowadzony przez Gemini. Nie jest zależnością
buildu ani częścią prac nad portem. Gdy jego interfejs będzie dostępny, można
wykorzystać go do generowania fixture’ów zapisów o zadanych parametrach;
fixture’y i ich oczekiwany wynik trzeba niezależnie zweryfikować. Nie zastępują
one kampanii od New Game ani testów serializacji w rzeczywistym runtime.

## 2. Architektura docelowa

| Warstwa | Odpowiedzialność i kontrakt |
| --- | --- |
| `game_core` | Stan gry, reguły, skrypty, aktorzy, kolizje, ATB, PE, przedmioty i progresja; bez SDL, FFmpeg, ściennego zegara i adresów PS1 |
| `game_content` | Statycznie linkowane moduły map/scen/efektów i adaptery danych; identyfikatory mapowane na natywne entry pointy i jawny stan/reset |
| `asset_io` | Jeden logiczny katalog obu CUE/BIN, ISO9660, PE.IMG i dekodery; jawne pochodzenie zasobu/offsetu, bez napędu CD i ręcznej zmiany płyt |
| `render` | Tła, maski, modele, animacja, tekst, UI, efekty i hi-res własnego PsyZ; jeden właściciel okna, logiczne współrzędne niezależne od rozdzielczości |
| `audio` | AKAO, próbki, efekty, muzyka, SPU/DSP i dźwięk filmów; własny zegar i miksowanie, kontrolowane przejścia oraz wyciszanie |
| `platform` | SDL/PsyZ, urządzenia, okno, wejście, zegary, pliki użytkownika; jawne interfejsy dające się zastąpić w testach |
| `persistence` | Wersjonowany zapis i ustawienia; serializacja stanu domenowego, bez zrzutów struktur i wskaźników |
| `test_runner` | Akcje wejścia na tickach, seed RNG, wirtualny czas, kontrolowane I/O, checkpointy, asercje zdarzeń/stanu/obrazu/audio i powtarzalny replay |

Kod pod `src/main/psyq/` służy do porównania funkcjonalności z PsyZ. Nie linkujemy całego legacy SDK obok jego natywnego odpowiednika. Każde używane API otrzymuje decyzję: usunąć po zmianie call-site, uprościć/scalić, zastąpić C, użyć PsyZ albo rozszerzyć własny fork. Testujemy zachowanie wymagane przez grę; nie portujemy niepotrzebnych kontraktów sprzętowych.

Formaty dysku zachowują szerokości pól i offsety. Stan runtime używa wskaźników hosta, `size_t`, uchwytów i typowanych struktur. Niezdefiniowane przepełnienia, przesunięcia i aliasowanie zastępujemy jawnymi operacjami o udokumentowanej semantyce. GTE może realizować renderowanie, lecz wynik kolizji i reguł gry nie może zależeć od GPU ani tempa prezentacji.

Symulacja korzysta ze stałego kroku i jawnych kadencji podsystemów, potwierdzonych audytem oryginału. Nie zakładamy, że każda stara pętla wykonuje się 60 razy na sekundę. Rozdzielczość renderowania, film, czas odczytu pliku i pauza nie zmieniają liczby kroków logiki. WP13 dopiero rozszerza prezentację do wysokich FPS, z testami 60/120/144/240 Hz; nie jest to wymaganie gotowości pierwszej pełnej wersji. Losowość ma kontrolowany stan. Zapis i replay odtwarzają ten stan.

## 3. Katalog wymagań i dowodów

Pierwszym artefaktem implementacji jest maszynowo czytelny katalog, a nie procent „postępu dekompilacji”. Docelowe pliki `tests/catalog/features.json`, `content.json`, `cases.json` i `platforms.json` jeszcze nie istnieją; tworzy je WP00/WP01.

Każda pozycja zawiera: stabilne ID, opis zachowania, podsystem i właściciela zadania, zależności, źródło ustalenia, pozytywne/negatywne testy, oczekiwane wyniki, obsługiwane warianty, status implementacji i ostatni wynik dla konkretnego commita. Statusy to: zinwentaryzowane, opisane, zaimplementowane, automatycznie zweryfikowane. Wyłącznie ostatni status oznacza gotowość.

Katalog zawartości powstaje z obu dysków, tabel gry, skryptów i źródeł. Łączy mapy z rejestrem modułów, przejściami, scenami, bankami audio, filmami, przeciwnikami i obiektami. Musi rozróżniać wspólne pliki od różnych wariantów fabularnych tej samej lokacji. Liczby końcowe ustalamy z danych; brakującego modułu nie zastępuje „udany” pusty stub.

Raport pokazuje osobno: pełną kampanię, zawartość dodatkową, każdy podsystem, błędy/awarie, obraz, audio i platformy. Brak testu, fixture’a, danych dysku, wyniku albo nieznana pozycja daje wynik **niezweryfikowane**, a na bramce wydania **FAIL**.

## 4. Kolejność implementacji

| Pakiet | Zależności | Wynik odbioru | Główne testy |
| --- | --- | --- | --- |
| WP00 — katalog i kontrakty | Brak | Pełny zakres USA/2 dyski, mapa zależności/API, docelowe platformy, referencje | Spójność katalogów i kompletność mapowania zawartości |
| WP01 — automatyczny runner | WP00 | Deterministyczny replay, wymagane testy z danymi, raporty i diagnostyka | Obecny boot jako pierwszy przypadek; celowo uszkodzone wyniki muszą być wykryte |
| WP02 — runtime i dane | WP00, WP01 | Przenośny stan gry, zasoby, moduły, zegar, RNG, inicjalizacja | Parsery, lifetimes, błędy I/O, formaty, powtarzalność stanu |
| WP03 — scena i prezentacja | WP02 | Pokój z tłem, maską, modelem, kamerą, animacją, audio i hi-res PsyZ | Referencyjne sceny w różnych rozdzielczościach, AKAO/SPU i reset przy przejściu |
| WP04 — eksploracja i skrypty | WP03 | New Game prowadzi do sterowanej lokacji; działają interakcje i zdarzenia | Skrypty, kolizje, aktorzy, przejścia, dialogi i puzzle |
| WP05 — pełna walka | WP04 | ATB, PE, AI, statusy, nagrody, zwykłe i specjalne walki | Macierz mechanik i deterministyczne scenariusze bojowe |
| WP06 — menu, przedmioty i progresja | WP04, WP05 | Wszystkie menu, wyposażenie, rozwój i transakcje | Reguły, granice, podgląd/commit, wejście i UI |
| WP07 — trwały zapis i odczyt | WP02; pełny odbiór po WP06 | Continue, trwałe ustawienia, atomowy zapis, wznowienie procesu | Round-trip, przerwanie zapisu, migracje, uszkodzenia |
| WP08 — pierwszy pełny fragment | WP04–WP07 | Eksploracja → dialog → walka → nagroda → zapis → restart → dalsza gra | Replay użytkownika bez ustawiania flag fabularnych przez test |
| WP09 — cała zawartość | WP08 | Cała kampania, drugi dysk, finał, Tutorial i zawartość dodatkowa | Wszystkie lokacje/sceny/walki/filmy; kampanie od początku do końca |
| WP10 — jakość PC i warianty | WP08; pełny odbiór po WP09 | Sterowanie, UI, ustawienia, hi-res i opcjonalne ulepszenia | Macierz urządzeń, rozdzielczości/DPI, pauza/fokus, warianty ustawień |
| WP11 — stabilność i pakowanie | Podstawy od WP01; pełny odbiór po WP09/WP10 | Wydajne, powtarzalne paczki dla wszystkich celów | Sanitizery, fuzzing, soak, GPU/audio, instalacja i aktualizacja |
| WP12 — bramka pełnego wydania | WP00–WP11 | Cała gra z hi-res ma świeży dowód dla tej samej paczki/commita | Pełna macierz zakresu bazowego bez braków i pominięć |
| WP13 — hi-FPS, późniejszy etap | Stabilny WP09–WP12 | Wyższe FPS i interpolacja przy niezmienionej symulacji | Replaye przy 60/120/144/240 Hz, czas zdarzeń i obraz na cięciach |

Zapis, testy i audio rozwijamy od fundamentów; nie odkładamy ich do końca kampanii. Pierwszy fragment służy do sprawdzenia architektury, nie zmniejsza zakresu pełnego portu. Pakiety otrzymują osobne PR-y o małym zakresie; do każdego dochodzą testy jego zachowania. Nie wyznaczamy dat wydania bez pomiaru kompletności danych, zależności i kosztu integracji w WP00–WP03.

## 5. WP00/WP01 — audyt i testowanie od pierwszego dnia

1. Zinwentaryzować każdy używany symbol/API, dane globalne bez definicji, realny MIPS/GTE, adresy sprzętu, wskaźniki w liczbach i nakładające się symbole. Wynik ma wskazać usunięcie/uproszczenie albo potrzebną implementację i test zachowania gry, nie samą listę tekstowych wystąpień.
2. Ustalić pełny katalog obu dysków i dostępność materiałów do prywatnych testów. Wybrać pierwszą lokację na podstawie pokrycia eksploracji/skryptów/walki, a nie łatwości zrobienia pustego renderu.
3. Rozdzielić publiczne fixture’y syntetyczne od prywatnych danych i referencji oryginału. Dokumentować wersję danych i sposób uzyskania każdego oczekiwanego wyniku.
4. Dodać runner z akcjami typu `start_new_game`, `move`, `interact`, `choose_target`, `use_pe`, `save`, `reload`, wykonywanymi przez normalne interfejsy gry. Sterowanie bazuje na tickach/stanie, bez `SDL_Delay(100)` jako dowodu gotowości.
5. Dodać checkpointy stanu domenowego: lokacja, progresja, aktorzy, zasoby, skrypty, HP/PE/ATB, przedmioty, wyposażenie, RNG. Usunąć z porównań adresy hosta, padding i dane diagnostyczne.
6. Dodać rozdzielne tryby: przyspieszona symulacja bez prezentacji, deterministyczny render programowy, render na prawdziwym GPU i realne urządzenie/wyjście audio. Tryb dummy nie jest dowodem poprawności pozostałych trybów.
7. Każda porażka zapisuje replay, seed, ostatni poprawny tick, oczekiwany/uzyskany stan, stderr, obraz różnic, fragment PCM, profil i konfigurację platformy. Timeout oraz zawieszenie są błędami.
8. Wymagany job z zasobami kończy się błędem, gdy `PE_TEST_DISC`/drugi dysk są nieobecne albo test zwraca SKIP. Zwykły publiczny build może nadal testować tylko syntetyczne dane, ale jego nazwa i status nie mogą oznaczać „pełna gra działa”.
9. Wzmocnić istniejący boot: pełne przejście intro → menu, wszystkie klawisze i ścieżki wyjścia, uszkodzony dysk, brak pamięci/dekodera/urządzenia, anulowanie przy pustej/pełnej kolejce, obrazy i rzeczywisty sygnał audio. Nie skupiać dalszego portowania wyłącznie na tym module.

**Odbiór:** ten sam replay jest powtarzalny; kontrolowane zepsucie obrazu, audio, stanu, kolejki i wyniku kampanii powoduje FAIL. Brak danych nie daje pozornego PASS. Wszystkie kolejne pakiety używają tego samego systemu dowodów.

## 6. WP02/WP03 — fundament pełnej gry

**Runtime i dane:** podział stanu na sesję, pokój, aktorów i zadania; jawna inicjalizacja/reset; uchwyty zamiast adresów PS1; brak globalnych aliasów pamięci między pokojami. Statyczne linkowanie i rejestr natywnych entry pointów zastępują wykonywalne overlaye; stan każdego pokoju resetujemy zgodnie z kontraktem gry. Scratchpad i specjalny stos zastępują normalne bufory i stos hosta. Portujemy semantykę `Boot_InitGameState`, inicjalizacji podsystemów i `Boot_RunFrame`, bez konsolowego start-upu i zmiany scratchpad-stack.

**Ładowanie:** parser CUE/BIN i jeden katalog zasobów obu obrazów, PE.IMG, rekordy pokojów/scen, TIM, geometria/model/animacja, skrypty, AKAO i próbki. Przy starcie weryfikujemy oba źródła, ich role/wersje oraz kompletność; oryginalne numery sektorów odnoszą się do właściwego obrazu, nigdy do jednego globalnego LBA bez pochodzenia. Przenosimy wymagane stałe/tabele przez kontrolowane dekodowanie danych, nie przez zależność od PS1 linkera. Odczyt plików obsługuje zamknięcie, anulowanie, ponowienie i reset żądania. Brak/uszkodzenie pliku obrazu nie może zostawić częściowo gotowej sceny. Komendy, opóźnienia i kolejki fizycznego CD usuwamy; kolejność zdarzeń wymagana przez gameplay zostaje jawna.

**Grafika:** ordering tables i pakiety mają typy PsyZ/hosta; tła, maski zasłaniania, modele, skóra/kości, tekstury, CLUT, tekst, transparencja, mieszanie, cienie i efekty dostają mikrotesty oraz referencyjne sceny. Oddzielamy współrzędne logiczne od renderowania i DPI; od razu używamy hi-res własnego PsyZ. Testy przy każdym wspieranym mnożniku sprawdzają dopasowanie tła/masek/modeli i niezmieniony stan gry, zamiast powiększać logikę wraz z pikselami. Nie używamy zgodności zrzutu pochodzącego z tego samego wadliwego renderera jako jedynego wzorca.

**Audio:** cała ścieżka AKAO/SPU, nie tylko XA. Sekwencer, banki, wysokość dźwięku, obwiednie, panning, reverb, muzyka i efekty, dialog/FMV tam gdzie występują, przejścia i nakładanie kanałów. Synchronizacja ma wspólny kontrakt czasu; brak urządzenia, zmiana formatu i restart wyjścia są kontrolowanymi stanami.

**Odbiór:** wybrany rzeczywisty pokój ładuje wszystkie potrzebne dane i pokazuje oryginalną scenę z aktorami oraz audio. Kolejne wejście po wyjściu nie przenosi nieprawidłowego stanu. Testy wykrywają brakujące dane/moduły, błędy masek, zgubione kanały i niewłaściwą kolejność zdarzeń.

## 7. WP04 — eksploracja, fabuła i skrypty

Portujemy podsystemy `scene`, `field`, `entity`, `anim`, `script`, `pm`, `text` i ich wymagane zależności. Każda instrukcja skryptowa ma test argumentów, efektu, warunku zakończenia i błędnych danych. System tekstu dekoduje oryginalne banki, podstawienia i kody sterujące; testy sprawdzają pełną treść, wybory, zawijanie, czas wyświetlania i granice buforów. Zadania, callbacki i skrypty muszą wracać do oczekiwania/zakończenia w kontrolowany sposób, zamiast wisieć na pętli konsolowej.

Testy obejmują: ruch i bieg, kolizje i krawędzie, schody/wysokości, kamerę i przejście jej granicy, interakcje, rozmowy, wybory dialogowe, przedmioty/klucze, drzwi, flagi fabuły, puzzle, cutsceny, pojawianie/usuwanie aktorów, zdarzenia czasowe oraz powrót do już odwiedzonych lokacji. Katalog wylicza wszystkie odmiany pokojów wynikające z progresji.

**Odbiór:** New Game przez normalny boot uruchamia sterowaną grę. Replay doprowadza do określonego zdarzenia fabularnego, a checkpointy potwierdzają stan, widocznych aktorów i poprawne odblokowanie przejść. Test nie ustawia bezpośrednio flag, które ma wytworzyć rozgrywka.

## 8. WP05 — wszystkie reguły walki

Portujemy `battle`, `aya`, bojowe `item`, AI aktorów, interakcje z animacją, kolizją, skryptami i efektami. Model walki ma jawne stany wejścia, ATB, wyboru, kosztu, wykonania, trafienia, reakcji, nagrody i wyjścia.

Macierz testów obejmuje każdą klasę ataku/broni, celowanie/zasięg, amunicję/przeładowanie, obrażenia i obronę, trafienie/chybienie/krytyczne trafienie, statusy i odporności, umiejętności PE i koszty, przedmioty, leczenie, śmierć/wskrzeszenie gdzie mechanika na to pozwala, ucieczkę, game over, doświadczenie/poziomy/BP i nagrody. Spis mechanik pochodzi z kodu i danych; nie kończy się na „jednej zwykłej walce”.

Każdy przeciwnik i boss ma test sekwencji zachowania: warunki wyboru AI, fazy, przejścia, ataki, skutki i zakończenie. Specjalne/starcia fabularne mają scenariusze z oryginalnymi ograniczeniami. Testujemy konfiguracje graniczne: zero/jeden/maksimum zasobu, ostatni przeciwnik, jednoczesne zdarzenia, zmiana statusu w trakcie akcji i przerwanie animacji.

**Odbiór:** wyniki liczbowe i kolejność zdarzeń zgadzają się z udokumentowanymi regułami/referencją; każdy element katalogu walk jest wykonywany automatycznie. Identyczne akcje i seed dają ten sam wynik przy różnych rozdzielczościach i tempie hosta; po wdrożeniu WP13 również przy różnych FPS renderowania.

## 9. WP06/WP07 — pełne menu, progresja i trwałość

**Menu i przedmioty:** wszystkie ekrany, opis/przewijanie tekstu, wyposażenie, właściwości broni/pancerza, użycie/wyrzucenie/przenoszenie, magazyn, klucze, parametry i rozwój, operacje narzędziami oraz modyfikacje. Utrzymujemy spójne identyfikatory i referencje po zmianach listy. Każda transakcja ma walidację warunków, koszt i jednoznaczny wynik; anulowanie nie zmienia stanu.

**Zapis:** wersjonowany format stanu domenowego i osobny format ustawień. Zapis do pliku tymczasowego, trwałe zakończenie zapisu i atomowa podmiana z kopią poprzedniej wersji. Testy zabijają osobny proces na każdym etapie zapisu, następnie rzeczywisty restart ma odczytać ostatni kompletny stan. Bez miejsca/bez uprawnień/błędna ścieżka/uszkodzony plik/nieznana wersja dają kontrolowany wynik.

**Wznowienie:** Continue z procesu uruchomionego od zera; stan pokoju, progresji, ekwipunku, wyposażenia, czasu i RNG zgodny z kontraktem. Migracje testowane fixture’ami wszystkich opublikowanych wersji. Import kart PS1 może być osobną opcjonalną funkcją z własnymi testami; nie wymaga emulacji API kart, portów ani ich ekranów. Wymagany zapis PC nie jest zrzutem karty ani RAM.

**Odbiór:** każda operacja menu ma pozytywny test, test odrzucenia i anulowania. Zapis/odczyt odtwarza wszystkie pola kontraktu oraz pozwala kontynuować kampanię bez resetu progresji. Nieudany zapis nie niszczy poprzedniego. Tutorial i Continue uruchamiają właściwe ścieżki zamiast kończyć aplikację.

## 10. WP08/WP09 — cała gra, oba dyski, zawartość dodatkowa

Najpierw zamykamy krótki kompletny fragment: boot → New Game → eksploracja → fabuła/dialog → walka/PE → nagroda/przedmiot → menu → zapis → zamknięcie procesu → Continue → następna lokacja. To test całego przepływu i serializacji, nie demo renderera.

Następnie integrujemy zawartość według grafu zależności fabuły, pokojów i zasobów. Każda mapa/scena/odmiana z katalogu dostaje: wejście, obraz i dźwięk, wykonanie zdarzenia/skryptu, wyjście oraz reset. Dotyczy to także lokacji opcjonalnych i ponownie odwiedzanych. Każdy film i każdy bank/sekwencja muzyczna ma automatyczny test poprawności oraz test osadzenia w odpowiednim zdarzeniu gry.

Wymagane rodziny kampanii:

| Trasa | Dowód końcowy |
| --- | --- |
| New Game → wszystkie dni fabuły → zakończenie | Prawidłowa progresja, bossowie, przejście obu dysków, zakończenie/napisy i stan końcowy |
| Kampania z zapisami i restartami | Osobne procesy, prawdziwe Continue, ten sam dalszy przebieg |
| Warianty wyborów, opcjonalnych lokacji i przedmiotów | Wszystkie zinwentaryzowane gałęzie mają scenariusz; nie tylko jedna optymalna ścieżka |
| Tutorial | Pełne ukończenie i powrót do właściwego stanu |
| New Game+/EX Game i zawartość dodatkowa, w tym Chrysler Building | Odblokowanie przez normalną progresję, pełne przejście i właściwe zakończenie |
| Śmierć/game over/ponowienie | Właściwe menu i czysty reset; nieprzenoszenie stanu poprzedniej próby |
| Jeden zestaw obu obrazów | Weryfikacja obu CUE/BIN przy starcie; automatyczne przejście między zawartością bez promptu o zmianie CD; błędna para i brak/uszkodzenie pliku mają testy |

Bazowa kampania zaczyna się z czystych danych użytkownika i używa prawdziwych akcji przez cały przebieg. Testy wariantów mogą startować z wcześniej wygenerowanych checkpointów, których pochodzenie i wersja są zapisane; same checkpointy nie zastępują testu kampanii od początku. Przyspieszone przejście nie może omijać warunków skryptów, kosztów akcji ani flag. Osobno sprawdzamy nieprzyspieszone przejścia czasowe i prezentację.

**Odbiór:** pełny katalog zawartości jest domknięty i automatycznie wykonywany. Obydwa dyski, kampanie i tryby dodatkowe przechodzą od początku do końca. Żadne „nieznane ID” ani brak natywnej implementacji nie wracają sukcesem.

## 11. WP10 — jakość PC i opcjonalne ulepszenia

Mapowanie akcji zamiast bitów pada; klawiatura/mysz/pad, przypisania klawiszy, deadzone, repeat, hotplug, fokus, podpowiedzi urządzenia. Mysz wybiera wyłącznie legalne cele i respektuje ATB/koszty. Obsługujemy pauzę, zmianę rozmiaru, fullscreen, DPI, wiele monitorów i utratę urządzenia audio.

Interfejs: skalowanie tekstu/UI, porównanie wyposażenia, podgląd skutków modyfikacji, sortowanie/filtry, blokada ważnych przedmiotów i szybsze przenoszenie. Automatyczne testy sprawdzają zarówno nawigację, jak i skutek domenowy; obraz ładnego menu nie wystarcza.

Autosave w bezpiecznych punktach, rotacja kopii, suspend/resume ze zdefiniowanego stanu, historia dialogów i pomijanie już obejrzanych scen mają osobne wymagania oraz testy wszystkich skutków ubocznych. Historia/skip nie pomijają progresji fabuły. Odtwarzanie zapisów po zmianie konfiguracji pozostaje poprawne.

Hi-res należy do bazowej wersji: używamy istniejącego skalowania PsyZ, testujemy wszystkie wspierane mnożniki i oddzielamy je od FPS. Hi-res nie tworzy dodatkowych szczegółów w oryginalnych tłach/FMV; ich filtrowanie i ewentualne ulepszenie jest osobną opcją. Najpierw pełne 4:3. Panorama wymaga testów wszystkich teł, masek i scen; lokacje bez danych poza oryginalnym kadrem zachowują poprawny kadr. Ulepszenia obrazu, tempa i balansu są konfiguracją, a bazowy tryb pozostaje w pełni testowany.

**Odbiór:** wszystkie funkcje bazowe i hi-res działają na całej zawartości, a wyłączone ulepszenia przywracają zachowanie bazowe. Hi-FPS nie blokuje WP10/WP12; jego osobny odbiór obejmuje wszystkie zadeklarowane później częstotliwości, interpolację i reset przy teleportacji/cięciu kamery. Testy obejmują każdą opcję i konflikty pomiędzy opcjami; kombinacje niezależnych ustawień pokrywa macierz par, a kombinacje wpływające na reguły gry mają pełne scenariusze.

## 12. Macierz automatycznych testów

Każdy prefiks poniżej jest rodziną wymagań. W WP00 nadajemy ID konkretnym instancjom: instrukcji, przedmiotowi, bossowi, pokojowi, filmowi i operacji. Ta tabela jest planem, nie deklaracją istniejącego pokrycia.

| ID | Zakres | Automatyczny dowód |
| --- | --- | --- |
| DATA | Jeden zestaw obu CUE/BIN, ISO9660, PE.IMG i dekodowane formaty | Pochodzenie/kolizje ID, pary obrazów, tracki/sektory, granice, endianowość, obcięcia i złe wersje; bez testów fizycznego napędu |
| CORE | Inicjalizacja, RNG, zegar, zwykłe bufory, uchwyty i statyczne moduły | Powtarzalność, reset po zmianie pokoju, brak starego scratchpad-stanu, błędne ID i lifetime |
| WORLD | Każda lokacja, wariant i przejście | Replay wejście/zdarzenie/wyjście; stan aktorów, kolizji, kamery, obrazu i audio |
| SCRIPT | Każda instrukcja, proces i zdarzenie fabularne | Efekt, kolejność, oczekiwanie, zakończenie, błąd; brak zawieszeń |
| COMBAT | Każda mechanika, PE, klasa broni, status, AI i boss | Wyliczenia, stany graniczne, kolejność akcji, nagrody i wyjście z walki |
| ITEM | Każda kategoria przedmiotów i operacja | Zachowanie identyfikatorów, koszty, pojemność, warunki, commit/anulowanie |
| TEXT | Wszystkie banki tekstowe, glyphy, podstawienia i kody sterujące | Dekodowanie i layout, pełne dialogi/wybory, brak ucięcia lub przekroczenia bufora |
| UI | Wszystkie menu, dialogi, Tutorial i wybór celu | Akcje wejścia → stan domenowy; zrzuty/maski referencyjne i czytelne skalowanie |
| SAVE | Każdy stan zapisywany i wersja formatu | Round-trip i realny restart; przerwania, migracje i uszkodzenia |
| AUDIO | AKAO, SPU, każdy bank/utwór/efekt i XA | Zdarzenia muzyczne, PCM, obwiednia, kanały, panning, miks i ciągłość |
| VIDEO | Każdy FMV, skala i przejście | Klatki, kolejność, paleta/range, proporcje, PCM, zegary, koniec/skip/anulowanie |
| INPUT | Wszystkie akcje i urządzenia | Zmapowane wejścia, repeat, hold/release, fokus, hotplug i utrata urządzenia |
| FLOW | Wszystkie kampanie i warianty progresji | Replay od startu, checkpointy po każdym istotnym zdarzeniu, prawidłowe zakończenie |
| FAULT | Błędy I/O, pamięci, dekodera i urządzeń | Wstrzykiwanie błędu na każdej granicy; kontrolowany wynik i zachowana sesja/zapis |
| PLATFORM | Każda wspierana kombinacja OS/backend/sprzęt | Prawdziwy render/wyjście audio, urządzenia, paczka, instalacja i aktualizacja |
| SOAK | Długie sesje, powroty, wielokrotne boot/zmiana pokoju | Limity pamięci/uchwytów/kolejek, brak wycieków, deadlocków i narastającego opóźnienia |

## 13. Wyrocznie i mierzalne kryteria

Testy nie mogą opierać się wyłącznie na wyniku wygenerowanym przez testowaną implementację. Reguły liczbowe i formaty mają niezależne fixture’y i ręcznie opisane wartości graniczne. Referencje obrazu/audio/zdarzeń oryginału zbieramy kontrolowanym replayem z własnych danych gry; przechowujemy metadane urządzenia, konfiguracji, wejść i czasu. PS1/emulator może być źródłem referencji offline, nigdy runtime portu.

| Obszar | Warunek odbioru |
| --- | --- |
| Stan i logika | Dokładny wynik dla deterministycznej arytmetyki, zdarzeń i ID; identyczny kanoniczny stan po tym samym replayu przy różnych rozdzielczościach i tempie hosta; dodatkowo przy różnych FPS po wdrożeniu WP13 |
| Obraz programowy | Referencyjne sceny i assety; dokładny wynik, gdy pipeline jest deterministyczny; błędy maski/kolejności/obiektu zawsze FAIL |
| Obraz GPU/hi-res | Wszystkie wspierane mnożniki PsyZ i niezmieniony stan gameplay; progi per scena/obszar skalibrowane wobec referencji przed zmianą renderera; raport różnic, nie jeden globalny średni próg ukrywający brak obiektu |
| Audio | Poprawna lista zdarzeń, kanały, brak utraconych bloków/underrunów; porównanie wyrównanego PCM i cech dla ustalonego DSP/resamplera; cisza lub zgubiona ścieżka zawsze FAIL |
| Synchronizacja FMV | Różnica zegarów audio/wideo na całym przebiegu najwyżej jedna źródłowa klatka; sama długość plików nie jest testem synchronizacji |
| Pamięć/wątki | Zero raportów własnego kodu ASan/UBSan i TSan w obsługiwanych konfiguracjach; osobne uruchomienia TSan oraz ASan |
| Powtarzalność | 100 kolejnych identycznych replayów runnera bez zmiany wyniku i bez automatycznego retry zamieniającego porażkę w PASS |
| Pokrycie | 100% pozycji katalogu ma test i świeży wynik; wszystkie osiągalne gałęzie własnego rdzenia/serializacji/parserów testowane; kod vendorów nie inflatuje wyniku |
| Skuteczność testów | Mutacje kluczowych reguł, flag, kosztów, zapisu i renderowania muszą być wykryte; samo wykonanie linii nie dowodzi asercji |
| Stabilność | 1000 cykli ładowania/wyjścia/odczytu zapisu i minimum 8 h soak na referencyjnych konfiguracjach; brak wzrostu żywych zasobów po rozgrzaniu cache |
| Wydajność | Docelowy FPS i budżet p99 czasu klatki/I/O/pamięci przypisane do jawnej konfiguracji w WP00/WP03; brak zmiany symulacji przy spadku FPS |

Progi jakości obrazu i audio zapisujemy w katalogu. Zmiana progu musi opisywać rzeczywisty powód i wpływ; nie jest sposobem naprawienia czerwonego testu. Zależności i baseline’y są wersjonowane, a aktualizacja SDL/własnego PsyZ/FFmpeg uruchamia pełne testy obszaru i paczki. Zmiany we własnym PsyZ mają najpierw testy biblioteki, potem regresję Parasite Eve i dopiero nowy pin submodułu; współdzielonych zmian nie uznajemy za poprawne tylko dlatego, że jeden mikrotest przeszedł.

Fuzzing obejmuje parsery, save, skrypty, offsety, rekordy animacji/modeli i sekwencje operacji menu. Wygenerowany crash staje się stałym testem regresyjnym. Testy własności/modelowe sprawdzają zachowanie zasobów, dozwolone przejścia i transakcje dla wielu seedów. Analiza statyczna oraz ostrzeżenia kompilatorów uzupełniają testy wykonywalne.

## 14. CI, sprzęt i wydanie

| Poziom | Co uruchamia | Polityka wyniku |
| --- | --- | --- |
| Publiczny PR | Windows/macOS/Linux build, unit, parsery syntetyczne, analiza i wybrane sanitizery | Potwierdza tylko zakres bez danych gry; zero ukrytych błędów i ostrzeżeń własnego kodu |
| Wymagany prywatny job dla zmiany | Dane obu dysków, scenariusze dotkniętych funkcji, pierwsza kompletna trasa i boot | Brak danych/runnera, timeout, SKIP lub zaległy wynik oznacza brak gotowości |
| Pełny nightly | Wszystkie katalogi, gałęzie kampanii, sceny, audio, fuzz/property i soak rotacyjnie | Wynik przypisany do commita; awarie tworzą konkretny replay do naprawy |
| Realne GPU/audio | Każdy wspierany backend i reprezentatywne klasy sprzętu z `platforms.json` | Dummy nie zastępuje GPU; PCM przed urządzeniem nie zastępuje testu wyjścia/loopback |
| Kandydat wydania | Cała macierz dla dokładnie tej paczki i kodu, bez pominięć; instalacja na czystych systemach | Wydanie blokowane do czasu pełnego PASS |

Stan projektu nie może nazywać się „wszystko działa”, gdy ostatni pełny job jest czerwony albo nie obejmuje bieżącego commita. Dla PR można wybierać testy według jawnej mapy zależności; nieznany wpływ wymaga rozszerzenia zakresu. Aktualizacje wspólnego stanu, I/O, zapisu, czasu i rejestru modułów wymagają kampanii, nie tylko unit tests.

Na maszynach sprzętowych automatyzujemy start procesu, akcje urządzeń, capture obrazu/loopback audio, resize/fullscreen, odłączenie i powrót kontrolera/audio, suspend/resume systemu i raport. Artefakty prywatne zawierające dane gry nie są publikowane. Ręczne ogranie może dostarczać nowej informacji, ale nie zastępuje żadnego obowiązkowego automatycznego testu.

Paczka nie potrzebuje kompilatora, Pythona, repo ani narzędzi deweloperskich. Testy sprawdzają zależności SDL/FFmpeg, ścieżki ze spacjami/Unicode, wybór i trwałą konfigurację obu CUE/BIN jako jednego zestawu, pierwsze uruchomienie, katalog użytkownika, read-only instalację, aktualizację i zgodność zapisów. Każda platforma otrzymuje własny artefakt i dowód z czystego systemu.

## 15. Najbliższe PR-y

| Kolejność | Konkretna zmiana | Dowód przed przejściem dalej |
| --- | --- | --- |
| PR01 | Katalog pełnej gry/2 dysków, mapa brakujących danych/API i zakres platform | Każda znana funkcja ma ID i zaplanowany test; nieznane obszary widoczne jako blokady |
| PR02 | Rozszerzenie headless runnera o ticki/RNG i checkpointy stanu gry oraz wymagane testy prywatne | Boot powtarzalny bez sleeps; brak danych i celowe regresje dają FAIL |
| PR03 | Rozdział aplikacji, game_core i platformy; statyczne moduły, normalne bufory i usunięcie zbędnego PS1 API | New Game/Tutorial prowadzą do rzeczywistego dispatchera, reset ma testy |
| PR04 | Oba CUE/BIN jako jeden zestaw, asset manager i loader pierwszego pokoju | Poprawne pochodzenie/wybór danych bez zmiany CD; błędne pary, rekordy i anulowanie mają testy |
| PR05 | Renderer pierwszej sceny z hi-res własnego PsyZ oraz AKAO/SPU | Tło + maski + model + kamera + animacja + audio zgodne z referencją; rozdzielczość nie zmienia gameplay |
| PR06 | Skrypty, eksploracja, interakcje i pierwsze zdarzenia fabularne | Replay ruchu/dialogu/przejścia bez bezpośredniej manipulacji progresją |
| PR07 | Pierwsza kompletna walka i menu przedmiotów | ATB, koszt, trafienie, PE, nagroda i wyjście potwierdzone stanem |
| PR08 | Zapis i Continue; pierwszy fragment od startu do restartu | Drugi proces kontynuuje prawidłowy stan; fault tests zapisu przechodzą |

To początek pełnego planu WP00–WP12, nie zakres końcowego wydania. WP13 dodaje hi-FPS później i ma osobną bramkę jakości. Kolejne PR-y zamykają resztę mechanik i katalogów, całą kampanię, drugi dysk oraz dodatkowe tryby.

## 16. Ostateczna definicja ukończenia

1. New Game, Continue i Tutorial uruchamiają i kończą właściwe ścieżki; pełna kampania z danych obu obrazów oraz zawartość dodatkowa są grywalne od początku do końca bez ręcznej zmiany CD.
2. Wszystkie lokacje, sceny, przeciwnicy/bossowie, mechaniki, przedmioty, menu, filmy i audio mają działającą implementację i automatyczny scenariusz z asercjami.
3. Zapis/odczyt i ustawienia są trwałe, wersjonowane i odporne na testowane przerwania; migracje przechodzą macierz; ewentualny opcjonalny import PS1 ma własny odbiór.
4. Sterowanie i prezentacja, w tym hi-res PsyZ, są poprawne na wszystkich zadeklarowanych platformach i urządzeniach, przy pełnej macierzy ustawień. Hi-FPS jest późniejszym rozszerzeniem; jego odroczenie nie odracza żadnej części kampanii ani funkcji rozgrywki.
5. Pełne replaye mają prawidłowe wyniki i progresję; testy obrazu/audio mierzą rzeczywisty rezultat, nie tylko długość plików i wyjście procesu.
6. Cały wymagany CI, testy z danymi, hardware, sanitizer/fuzz/soak oraz testy paczek przechodzą dla tego samego kandydata. Zero pominiętych wymaganych testów, brakujących ID i znanych blokujących defektów.
7. Raport i dokumentacja odpowiadają faktycznemu zakresowi; żaden kod tylko deklarujący sukces nie zastępuje funkcji gry.

Ukończenie tego dokumentu oznacza gotowy plan implementacji. Nie oznacza, że pełny port albo opisane testy już istnieją.

## Źródła techniczne do implementacji bramek

Audyt bieżącego projektu: `pc/CMakeLists.txt`, `pc/tests/test_disc.c`, `pc/tests/test_boot_assets.c`, `.github/workflows/pc.yml`, `docs/ARCHITECTURE.md`, `docs/ROOMS.md` i źródła `src/`.

[CTest SKIP_RETURN_CODE](https://cmake.org/cmake/help/latest/prop_test/SKIP_RETURN_CODE.html) opisuje istniejące pomijanie testu kodem 77. Docelowa bramka wymaganego zakresu musi je traktować jako brak dowodu. [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) służy do wykrywania błędów pamięci, a [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) do wykrywania wyścigów; wybieramy obsługiwane przez narzędzie konfiguracje zamiast deklarować jeden identyczny tryb na wszystkich systemach.
