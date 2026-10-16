# Plan implementacji pełnego portu Parasite Eve na PC

## Aktualny potwierdzony stan — 16 października 2026

M075: zwykły input uruchamia walkę (typ5/75HP, defense18; pomocniczy
aktor4 pojawia się po Script_SpawnEntityAt niedźwiedzia).
M16 pokonuje przeciwników, wynik HP8/107. Ekran nagród: Ammo+15, Obtain All,
End przy pełnych18 slotach; następnie Heal1 podnosi HP8→38. Telefon, Save,
Slot1, pusty slot6, oryginalny zapis G:8192 bajty,
SHA256 1cd948c5ced01f8cbd42d52ec587e611e3b768204e1355c3f098e550726e2463.
B–F niezmienione. To rzeczywisty zapis przez UI, bez pe-save i modyfikacji
stanu gry. Pełny replay od Continue F/cap111681 PASS normalnie oraz
ASan/UBSan/function: stan, VRAM i cały G identyczne. Zakończenie testu jest
na liście plików po komunikacie File saved; menu zamknięto w dalszym inputcie.
Zimny Continue G/cap24000 także PASS w obu wersjach: M075, etap248,
HP38/107,18 slotów, broń9/pancerz14, lock0, bez menu.
Artefakty /tmp/pe-m075-phone-{normal,san}.log/.vram,
/tmp/pe-m075-g-continue-{normal,san}.log/.vram,
/tmp/pe-m075-phone-verify.py i proof.json; karty prywatne
/tmp/pe-m075-phone-checkpoint/bu00. Użytkowe bu00/ i bu10/ nietknięte.
Nie jest to dowód wszystkich ataków, klatek efektów ani wszystkich dropów.

Dalszy żywy input przechodzi M075→M074→M076 (185 trójkątów)→M077
(162 trójkąty), nadal HP38 i ten sam ekwipunek. Oba obrazy obejrzane.
Próba M077→M080 na poprzednim buildzie zatrzymuje się na slot75/80194048.
Cały moduł22116 bajtów w M080 i M082 jest identyczny z M075, łącznie
z oboma descriptorami. Loader wiąże teraz także te dwie lokacje i zeruje
ich BSS przy ładowaniu. Rozszerzony original_motion_assets sprawdza cały
moduł, listy, program, relokowaną klasęE i33-elementową tablicę Configure
każdego z trzech pokojów; fixture --assets PASS normalnie i pod sanitizerami
(/tmp/pe-motion-variants-final-{san-,}assets.log). Zwykły pełny replay do cap119194 przechodzi wejście M080 (most),
HP38/107, etap248, lock0, ekwipunek zachowany; obraz obejrzany.
Pełny odpowiednik z ASan/UBSan/function PASS: cały JSON i1MiB VRAM
identyczne, SHA256 eb0e0ed1416d6469c4ef6719869f50478120e630659cc4bed308dc3767da16d2.
Zapis G identyczny, B–F niezmienione. Artefakty
/tmp/pe-m080-entry-{normal,san}.log/.vram, args.json, verify.py i proof.json.
Walka M080 jest teraz ukończona: typ5/75HP i typ6/35HP, M16,
końcowe HP16, poziom10, maxHP115, pojemność19. Obtain All zabiera Ammo+15
i jeden Junk do nowego slotu18; drugi Junk pozostaje. Heal1 po walce
podnosi HP16→46. Pełny input zawiera także poprzednią porażkę, Game Over,
Continue G przez UI, Heal1 przed ponowną walką i zwycięstwo. Normalny
oraz ASan/UBSan/function replay do cap136265 PASS: cały JSON i VRAM
identyczne, SHA256 f46e3ec214b12ca3165fe53f01b269cd954a2fd9c6bfd345f991359457e94b5c.
B–F niezmienione, cały G identyczny. Artefakty
/tmp/pe-m080-battle-{normal,san}.log/.vram, args.json, verify.py i proof.json.
To nie dowodzi wszystkich pośrednich klatek ani wszystkich ataków.

Dalszy żywy input: skrzynia M080, pełne19 slotów, oryginalna wymiana
Junk ze slotu12 na Sv Vest1 (nowy rekord263). Przejście do M078,
walka typ5/32HP i typ4/44HP, zwycięstwo HP31. Nagrody: wymiana kolejnego
Junk ze slotu11 na Medicine1 oraz Ammo+6. Medicine1 użyta przez menu:
HP31→76, slot11 staje się pusty. M078 ma osobną skrzynię, której ten
przebieg nie otwiera. W M079 przejście południową alejką, bez otwierania
obiektu typu5 w środku. M081: cztery typ3 po35HP, zwycięstwo HP40;
wybrano Medicine1, następnie użyto jej przez UI: HP40→85, slot11 pusty.
Pozostałe Junk z nagród pozostają. Wejście M082/cap149915 działa,
obraz mostu obejrzany, HP85/115, etap248, lock0, broń9/pancerz14,
pojemność19, Sv Vest1 w slocie12. Pełne replaye od F normalnie i pod
ASan/UBSan/function PASS, cały JSON i1MiB VRAM identyczne:
SHA256 8d5c1f78c9a41fb75b35c7a4e735ab1fd225ee753f344ae219598d0fa222ecca.
Cały G identyczny, prywatneB–F niezmienione. Artefakty
/tmp/pe-m082-entry-{normal,san}.log/.vram, args.json, verify.py i proof.json.
To potwierdza wymienione walki, wymianę i leczenie do wejścia M082,
a nie wszystkie ataki ani wszystkie klatki pośrednie.

Próby walki M082 (typ5/75HP, dwa typ3/44HP) kończyły się Game Over
po wejściu przy granicy i zatruciu. To nie jest dowód błędu kolizji ani
ukończona walka. Krótsza gałąź od rzeczywistego zimnego Continue G
obejmuje zwycięstwa M080/M078/M081, wydanie100BP na M16 przez oryginalne
menu (125→25BP, attack35→36), wymianę pełnego ekwipunku w skrzyni M080,
leczenie i wejście M082 z HP81/115. Próba użycia PE wybierała Scan7,
choć pomocnik sterowania zamierzał wybrać Slow8. Potwierdzono to w LLDB;
pomocnik /tmp wybiera teraz według rzeczywistej listy i pozycji kursora,
zamiast zakładać liczbę naciśnięć. Nie zaliczamy tego jako użycia Slow.

Scan ujawnił brak resident task101 (command394/status-8). Włączono cały
istniejący fieldfx/emitters.c, zachowując w nim wszystkie29 funkcji.
Callbacki oraz dwie instancje spinning_model.inc mają natywne wspólne ABI.
Oryginalne programy101/102, rozmiary stanów, skrypty i cztery ścieżki
kolorów potwierdzono bajtowo w dostarczonym EXE USA. Podpięto tylko te
zweryfikowane programy; kompilacja reszty modułu nie oznacza pełnego
sprawdzenia pozostałych efektów. DiamondOutline rezerwuje teraz cztery
sizeof(RenderLinePacket), zamiast64 bajtów PS1 nakładających pakiety PC.
Pełny sanitizer replay ujawnił ujemny odziedziczony color_time=-406:
SampleColorTrack używa teraz jawnego 32-bitowego przesunięcia MIPS,
bez undefined behavior C i bez zerowania oryginalnego stanu cząstki.

Fixture --assets PASS normalnie i z ASan/UBSan/function: dodatkowe208
niehookowanych stanów CPU/BIOS Scan/Confuse,14 przypadków interpolacji
koloru z oryginalnymi argumentami na granicy GTE (ujemne i INT32 limits),
cztery autorskie skrypty92/93/101/102 po100 pełnych klatek PM, obie
buforowane listy pakietów, zagnieżdżone pule, wygasanie i guards.
Osobny test trzech kolejnych konturów na obu buforach pilnuje rezerwacji
pakietów linii i draw-mode. Artefakty /tmp/pe-resident-color-fixture{,-san}.log,
/tmp/pe-resident-emitters-cpu.py i /tmp/pe-resident-color-cpu.py.

Pełny replay tej krótszej gałęzi od rzeczywistego G do cap49900 PASS
normalnie i pod ASan/UBSan/function: cały JSON i1MiB VRAM identyczne,
SHA256 231d263b5fd299543c74be70fbd379a793d2f8036075030ac3f9f2b51d584822.
Scan został rzeczywiście wywołany (LLDB: DiamondEmitter mode0/age0,
g_MenuSelectedSpell=7), efekt kończy się i gra wraca do aktywnej walki
M082, HP26/115, bez menu. Nie jest to zwycięstwo ani dowód wszystkich
klatek efektu. Wszystkie prywatne pliki B–G niezmienione względem kopii
G. Artefakty /tmp/pe-resident-color-{normal,san}-replay.log i katalogi
z frame.vram, /tmp/pe-resident-color-verify.py i proof.json, wspólny input
/tmp/pe-resident-scan-args.json; obserwacja callbacka
/tmp/pe-resident-scan-callback-replay.log.

Regresja zgłoszonego wejścia do pierwszej walki z Melissą:
original-game-melissa-encounter PASS na aktualnym buildzie, wymagane
CUE i PE_REQUIRE_GAME_ASSETS=1,126.50s, bez SKIP. Log
/tmp/pe-resident-color-melissa.log.

Korekta nazewnictwa efektu: task102 jest Confuse10, nie Slow8.
Oryginalna BattleActionEffectIds wybiera dla Slow task99. Testy208 stanów
i programów101/102 powyżej dotyczą Scan/Confuse; nazwa w poprzednim
opisie była błędna. Oryginalne dwukolumnowe menu PE wymaga po Down także
Right, aby wybrać Slow8, a nie Scan7. Pomocnik /tmp odczytuje teraz
rzeczywistą szerokość siatki i packed listę oraz sprawdza selected spell.

Właściwy Slow odtworzył brak task99 (command395/status-8). Naprawiono
ABI wszystkich35 istniejących callbacków sprite_effects.c w miejscu;
żadnego nie przeniesiono ani nie zastąpiono. Dodano oryginalny deskryptor99,
18-halfword script i anchor BSS w tym samym module, z dwoma slotami8/8
potwierdzonymi w EXE. Porównanie280 niehookowanych stanów MIPS/BIOS
obejmuje dwa emitery, opadającą cząstkę i obie fazy obracającego modelu,
RNG, nieaktywne pola i pełną logiczną zawartość24 slotów puli. Fixture
--assets PASS normalnie i z ASan/UBSan/function, pięć programów92/93/99/
101/102 po100 pełnych klatek PM z modelem, zagnieżdżonymi pulami,
rysowaniem, oboma buforami, wygasaniem i guards. Wszystkie35 funkcji
modułu zachowano; pozostałych rodzin nie uznajemy za w pełni sprawdzone.
Artefakty /tmp/pe-slow99-fixture{,-san}.log i /tmp/pe-slow99-cpu.py.

Pełny rzeczywisty replay od Continue G do cap49520 PASS normalnie i pod
ASan/UBSan/function: wybrany Slow8, efekt99 zakończony, ruch odblokowany
(lock4), M082, HP81/115. LLDB potwierdza coreFlags1048577 spowolnionego
przeciwnika i spadek PE6651842→1366457 (koszt i dalsza regeneracja).
Cały JSON i1MiB VRAM identyczne, SHA256
9d42871183e9b64025ca89397f647f0372ebca7ae606f1021a9c80b622b2af3e.
Wszystkie prywatne pliki karty B–G niezmienione. Artefakty
/tmp/pe-slow99-complete-{normal,san}.log i katalogi z frame.vram,
/tmp/pe-slow99-complete-args.json, verify.py i proof.json.
To potwierdza jedno rzeczywiste użycie Slow, nie zwycięstwo M082 ani
wszystkie klatki efektu. Żywy input trwa w osobnym /tmp/pe-park-slow99,
log /tmp/pe-park-slow99-lldb.log, trace /tmp/pe-park-slow99-input.jsonl.

Dalsza próba po Slow kończy się Game Over/cap50888. Cure-P użyte przy
17HP przez prawdziwe menu zużywa slot10 i usuwa poison (stateFlags
270532608 po efekcie); potem Aya ginie przed następną turą Heal1. To nie
jest regresja ani zwycięstwo. Zakończono ten debugger po Game Over.

M082 jest teraz ukończone inną gałęzią inputu od prawdziwego Continue G.
Heal1 przed wejściem HP81→111; druga próba Heal1 przy zbyt małym PE nic
nie leczy. Na początku walki użyto Cure-P przez menu, stan otrzymuje
poison immunity3. Input biegnie dolnym i górnym brzegiem mostu, zamiast
pozostawać przy lewym przewężeniu. Dwa typ3/44HP pokonane, HP91. Seria
M16 spoza zasięgu (2930 przy1587) nie rani niedźwiedzia; po podejściu
do1353 dwie serie kończą typ5/75HP na-13, HP65. Pomocniczy typ4/10HP
nie jest osobnym celem. Oryginalne ekrany wyniku i awansu: poziom11
(zapisane level10), maxHP123, capacity20. Obtain All zabiera Ammo+6,
Cure-P i Ammo+15; wszystkie3 wpisy pending list są wyczyszczone, nowy
Cure-P wraca do zwolnionego slotu10, slot11 i nowy19 pozostają puste.
Po End dwa Heal1 przez oryginalne UI podnoszą HP65→95→123; menu zamknięte.

Pełny replay tej gałęzi od G do cap53727 PASS normalnie i pod
ASan/UBSan/function: cały JSON i1MiB VRAM identyczne, SHA256
a1954a568fc9e58a4f7471cb46aca32fdf0aadf4222db04e0e84bfb65b82b5aa.
M082, etap248, lock0, brak menu/żywych wrogów, HP123/123, capacity20,
broń9/pancerz14, bonus M16 nadal2, ekwipunek poza amunicją i odnowionym
Cure-P zachowany. Wszystkie prywatne pliki B–G niezmienione; użytkowe
bu00/ i bu10/ nietknięte. Artefakty /tmp/pe-m082-victory-{normal,san}.log,
katalogi z frame.vram, /tmp/pe-m082-victory-args.json, verify.py i proof.json.
Nie jest to dowód wszystkich ataków, dropów ani klatek pośrednich.

Żywy input idzie na zachód mostu, uruchamia oryginalny skrypt wyjścia,
i wchodzi do M083/cap54603 (A80081C8): fontanna,49 trójkątów, HP123,
etap248, lock0. Obraz /tmp/pe-m083-live.png obejrzany. Ruch alejką
uruchamia walkę dwóch typ3/57HP; po czekaniu do55335 HP94, ATB9000,
bez poison. Wtedy walka nie była jeszcze ukończona.
Po48 klatkach uniku i otwarciu menu/cap55399 nadal HP94.
Ten przebieg używał zamrożonego /tmp/pe-park-slow99-game-debug,
prywatnego /tmp/pe-m082-prepared, args.json, log
/tmp/pe-m082-prepared-lldb.log i ciągłego trace
/tmp/pe-m082-prepared-input.jsonl (od47891). Wyłącznie input i limity
hosta; nie zmieniano HP, pozycji, AI, flag ani inventory przez debugger.
Pomocnik /tmp/pe_park_combat_control.py ma dodatkowy pe-ready-range,
który fizycznie zbliża Ayę na zasięg M16, zanim zatwierdzi cel.

M083: Cure-P użyte przez menu przy94HP nadaje immunity3 i zwalnia
slot10. Dwa typ3/57HP pokonane M16, wynik HP59/123. W tej gałęzi nagrody
to dwa Ammo+6, bez Medicine/Cure-P. Obtain All wyczyściło oba pending
wpisy, End zamyka ekran. Dwa Heal1 leczą HP59→89→119. Trzecie użycie
nie leczy (po dwóch pozostało27PE, koszt60); menu zamknięte. Próba
pomocnika single z Circle cofa wybór w oryginalnym reticle i nie kończy
pojedynczego strzału; dopiero dalsze zatwierdzone cele wykonują serię.
Nie traktujemy samego pojedynczego Cross/Circle jako trafienia.

Pełny replay od G do cap59047 PASS normalnie i z ASan/UBSan/function:
cały JSON i1MiB VRAM identyczne, SHA256
a2bfd4f1d83990d3399c963607c84f7a9dc55252e034302cd575e9ac79a461f0.
M083, etap248, HP119/123, capacity20, lock0, brak menu/żywych wrogów,
slot10 pusty po Cure-P, broń9/pancerz14, bonus M16 nadal2. Wszystkie
prywatne pliki B–G niezmienione; użytkowe bu00/ i bu10/ nietknięte.
Artefakty /tmp/pe-m083-victory-{normal,san}.log, katalogi z frame.vram,
/tmp/pe-m083-victory-args.json, verify.py i proof.json. Obraz końcowy
frame.png obejrzany. To nie dowodzi wszystkich ataków, dropów ani
wszystkich wariantów walk M083.

Dalszy ruch do schodów daje script lock przycap59582 i prowadzi do M085,
nie M084. Pierwszy loader zatrzymuje się jawnie: slot10/80190024,
Scene frame startup failed:-2, przed zamierzonym cap59702. Proces
zakończył się; debugger zamknięto. Zachowany ciągły input:
/tmp/pe-m085-entry-failure-args.json i /tmp/pe-m082-prepared-input.jsonl.
Bieżący kandydat naprawia cały istniejący room_lib/three_particles.c
w miejscu: zachowuje wszystkie13 funkcji (5 metod klasy i8 callbacków),
a CMake dołącza ten oryginalny plik. Nie dodano plików produkcyjnych.
Loader wiąże wyłącznie potwierdzone M065/M085 slot10/80190024; inne
warianty pokojów pozostają niezweryfikowane. Oba pełne moduły mają4276
bajtów i identyczny SHA256
5f38b43c1a2a1c16f29c5e9f8b5e27641e93f0b77b12571d214dbf17b010c035.

Poprawiono ABI wszystkich metod/callbacków, rzeczywisty natywny
FieldActor i jego macierze zamiast offset238h, wskaźnik w RoomDebrisClock
(48→56 bajtów) i rozmiary alokacji. RoomDebrisTriplet ma3 wektory velocity,
zgodnie z oryginalnym B4h, a nie dawnym przypuszczeniem velocity[4].
Odtworzono wszystkie listy,14-word script, layout i16-bajtową rampę.
Alpha jest aliasem pola depth głównego packetu, zgodnie z SH8018F984,
nie osobną zmienną. Oba szablony packetów zerują się przy ładowaniu.
Jawna arytmetyka unsigned zachowuje32-bitowy wrap pozycji i promienia.
W DrawGlowSprite mnożenie przez16 zachowuje podpisaną arytmetykę UV
bez niezdefiniowanego przesunięcia ujemnej wartości (wykryte przez UBSan
w poprawnym wariancie page1). Nie zmieniono algorytmów rozgrywki.

Świeży test wymagający rzeczywistego CUE PASS normalnie i z
ASan/UBSan/function, z instrumentowanymi PsyZ/SDL. Zestaw porównuje
pełne180-bajtowe stany i RNG z wykonaniem oryginalnego MIPS, rzeczywistego
ROM BIOS i COP2/GTE:288 inicjalizacji (stos0/A5 daje identyczny wynik),
384 aktualizacje,648 stanów timera/kontaktu i448 rysowań z łańcuchami OT.
Zerowanie scratch macierzy zmienia tylko odrzucone translacje, nie wynik
cząstek ani RNG. Sprawdzono oba bufory, presety stron, kamery, maski i
ujemny wiek, alias jasności, pełny oryginalny skrypt PM, oba przebiegi
cząstek/cieni, natywny actor/matrix/TIM, szerokie vars i zakończenie zadań.
Hashe FNV: init234309fa30aaa62d, updateff1932b53e5636f9,
timerdcf26f313369619d, draw51e1deb8f951b445.
Artefakty /tmp/pe-three-particle-{cpu,timer-cpu,draw-cpu}.py i
/tmp/pe-three-particle-final-fixture{,-san}.log. Wcześniejsze kandydaty
v1/uv nie zaliczają sanitizera; powyższy PASS dotyczy final.

Pełny replay od Continue G do M085/cap59702 na final PASS normalnie
oraz ASan/UBSan/function: cały JSON i1MiB VRAM identyczne, SHA256
f55ffdfb94feeed3ab6729af5893e9c1fdf242b2d822e838b094d9da86293b23.
Etap248, HP119/123, capacity20, broń9/pancerz14, lock0, brak menu i
żywych wrogów. Wszystkie prywatne pliki B–G identyczne ze źródłem.
Obraz frame.png obejrzany. Artefakty /tmp/pe-three-final-m085-entry-
{normal,san}.log, katalogi z frame.vram, /tmp/pe-three-final-m085-verify.py
i proof.json. To dowód wejścia i ładowania modułu, nie wszystkich walk
ani ataków w tej lokacji. Dalszy rzeczywisty input uruchamia przy59828
skrypt walki, a przy60248 są trzy typ3/60HP; walka jeszcze niezaliczona.
Świeży original-game-melissa-encounter PASS127.04s na tym samym buildzie,
wymagane CUE, bez SKIP/abortów opcode206 i82; log
/tmp/pe-three-final-melissa.log.
Dalsza walka M085: M16 obniża pierwszego typ3 z60 do16HP, druga seria
go pokonuje. Pozostałe dwa nadal mają60HP. HP119→102→53; Heal1 przez
oryginalne menu podnosi HP, dalsze ataki/trucizna dają67, potem51.
Próba Detox (spell3) ujawnia następny jawny brak: Cannot start original
PE effect100 (command390,status−8), abort w Battle_AdvancePhase.
Nie jest to zwycięstwo ani PASS Detox. Proces zatrzymany na SIGABRT;
trace /tmp/pe-three-final-m085-input.jsonl jest ciągły od59702 do62024,
pełne args /tmp/pe-m085-detox100-failure-args.json, backtrace w
/tmp/pe-three-final-m085-live-lldb.log.
Audyt oryginalnego resident bloku: task100 descriptor800E1C64, callbacki
800DACA4/800DB0D0 odpowiadają istniejącym FieldFx_CosineEmitter i
FieldFx_SineEmitter w kompletnym już przenośnym emitters.c; rozmiary12/12,
script800E1BE0 ma18 halfwordów. W chwili pierwszego audytu brakowało
powiązania i oryginalnych danych:
color tracks800E1C04/40 i800E1C2C/56, tints800C22E8/800C22EC,
anchor800E221C. Ten audyt wyznaczył naprawę danych w tym samym module,
a następnie porównania CPU/GTE i ponowienie pełnego inputu opisane niżej.
Detox100: bieżąca naprawa uzupełnia wyłącznie oryginalne dane w istniejącym
emitters.c i wpis w task_list.c; wszystkie29 funkcji tego modułu pozostają
w miejscu. Nie dodano plików ani nowej implementacji zaklęcia. Odtworzono
pełny descriptor,18 halfwordów skryptu, oba color tracks, tints i anchor.
Test odczytuje je bezpośrednio z wymaganego CUE i porównuje z natywnymi.
224 porównania CPU/ROM BIOS stanów czterech oryginalnych callbacków,
RNG i wszystkich slotów pul dają FNV e7f3d5cc149b7a78; oracle
/tmp/pe-detox100-cpu.py. Pierwszy oracle błędnie założył nagłówek listy16
zamiast oryginalnego12 i prefix slotu8 zamiast4; poprawiony oracle zgadza
się z natywnym wynikiem. Nie zmieniano implementacji, żeby dopasować hash.
Sześć oryginalnych programów PM (w tym Detox) x100 klatek: rysowanie,
pule, łańcuchy OT, expiry i guards PASS. Pełny wymagany asset fixture
PASS zwykle i z ASan/UBSan/function; logi
/tmp/pe-detox100-verified-fixture{,-san}.log.
Zwykły replay od Continue G do62200 przechodzi dawny abort: M085,
HP51/123, dwa żywe typ3, menu zamknięte. LLDB przy62200 odczytuje
stateFlags10200000, poison mask wyzerowany. Dalsze ataki mogą ponownie
zatruć; Detox nie daje odporności. Pełny replay z ASan/UBSan/function do62200 PASS, bez diagnostyki błędów:
cały JSON i1MiB VRAM identyczne ze zwykłym, SHA256
71d9f71c8e17ea179f87b56b7ab6b2b6d11577d3ca193686f52635e8fde1eabf.
Prywatne B–G niezmienione, obraz frame.png obejrzany. Logi
/tmp/pe-detox100-{normal,san}.log i katalogi z frame.vram, args.json,
/tmp/pe-detox100-verify.py oraz proof.json.
Dalsza próba walki nie jest PASS: po ponownym poison HP51→26→18→0
przy63124. Dwa polecenia pe-ready-range nie były zarejestrowane w nowej
sesji debuggera, więc nie wykonały żadnego inputu; to błąd sterownika testu,
nie gry. Po rejestracji i ruchu do zasięgu Aya ginie. Po600 dalszych klatkach cap63724: scenaA9400048/M5000I, flags0, lock0,
battle−1, rgb24, odtworzony film otwierający z logo SQUARE (zrzut
/tmp/pe-detox100-loss.vram/png obejrzany). To potwierdza powrót po śmierci
w tym debug przebiegu; nie jest pełnym testem śmierci z sanitizerami.
Ciągły trace /tmp/pe-detox100-input.jsonl od62200 do63724. Następny
przebieg powinien przygotować zapas leczenia/Cure-P lub poprawić ruch
w walce; śmierć przy braku zasobów nie dowodzi błędu oryginalnej walki.
Nowa gałąź od rzeczywistego Continue G oszczędza Cure-P w M083.
Po cap54735 czeka180 zamiast600 klatek: walka zaczyna się z123HP.
Dwa typ3/57 pokonane przy91HP, oba końcowe cele13HP wybrane przez
oryginalny reticle; nagrody Ammo+6 dwa razy, bez Medicine/Cure-P.
Jawny wybór Heal1 przez menu daje121HP. Cure-P nadal w slocie10.
Wejście M085/cap57400: HP121, bez menu/locka. W walce Cure-P zużyte
przez Items/slot10, stateFlags200003 potwierdza odporność3. Trzy typ3/60
pokonane M16; podczas walki HP48→78 po jawnym Heal1. Pomocnik focus
wybiera rzeczywisty cel z najmniejszym HP w zasięgu, wyłącznie przez input.
Końcowe HP46, nagrody trzy Ammo+6 (bez Medicine/Cure-P), Obtain All/End.
Dwa Heal1 dają106HP, zostaje27PE; menu zamknięte, bez kolejnej próby
zaklęcia za60PE. Poprzedni pe-heal po Detox pamiętał wybór zaklęcia3,
więc nie wolno uznawać tamtych prób za Heal1; nowy pe-cast wybiera
zaklęcie według packed list i aktualnej siatki kursora.

Pełny replay G→M085 zwycięstwo/cap62734 PASS normalnie oraz
ASan/UBSan/function na zamrożonym kandydacie Detox100: cały JSON i1MiB
VRAM identyczne, SHA256
2326e05c1566a8170f20c2250cb7b90d2256ddd5b2b06b191473145c1138c0bb.
M085, etap248, HP106/123, capacity20, broń9/pancerz14, live0, mode9,
lock0, brak menu; slot10 pusty po Cure-P, inventory zachowane. B–G
niezmienione, bu10 puste; użytkowe karty nietknięte. Obraz frame.png
obejrzany. Artefakty /tmp/pe-m085-victory-{normal,san}.log i katalogi,
/tmp/pe-m085-victory-args.json, verify.py i proof.json. Nie jest to dowód
wszystkich wariantów obrazu/ataków/dropów tej walki.

Dalszy input wychodzi z areny na z3000/cap63709, potem przy z6832
uruchamia wyjście/cap64049. Wejście M087 zatrzymało się na brakującym
slocie10/80193518, startup−2; proces zakończył się1. Pełny reproducer
/tmp/pe-m087-entry-failure-args.json zamierza cap64289, ciągły trace
/tmp/pe-m083-cure-reserve-input.jsonl, log lldb o tej samej nazwie.
Audyt M087: cały moduł18004 bajty, FNV0d8472ecf5bd2d50. Oba wymagane
recordy są slot10/80193518 oraz75/801935B4. Wszystkie1023 instrukcje
całej klasy three_particles różnią się od M085 tylko30 relokacjami:
kod+1D8, dane+34F4, work packets+35A0. Seed, cały skrypt, rampa i
rozmiary PS1 są identyczne; Alpha nadal aliasuje depth. Pełna klasa E
AA4 bajtów i33-way Configure są identyczne z już zweryfikowanym M075
po relokacji−A34, wskaźnik tabeliF08C→F07C. Do loadera dodano tylko te
dwa dokładne powiązania i reset obu szablonów przy slocie10. Test assets
sprawdza cały moduł i każdy wymieniony blok; pełny fixture --assets PASS
normalnie i z ASan/UBSan/function (/tmp/pe-m087-bind-fixture{,-san}.log).
Pełny replay G→M087/cap64289 na kandydacie bind PASS normalnie i z
sanitizerami: cały JSON i1MiB VRAM identyczne, SHA256
72bdf63c0e1d2b5ef7726f9bf2ffba003179e91c416df9c3d735c4c5adb44360.
M087/A80083C8, HP106/123, etap248, capacity20, broń9/pancerz14,
lock0/mode9/live0, brak menu, prywatne B–G niezmienione, bu10 puste.
Obraz frame.png obejrzany (alejka i telefon). Artefakty
/tmp/pe-m087-bind-{normal,san}.log, katalogi z frame.vram, args.json,
/tmp/pe-m087-bind-verify.py i proof.json. To dowód wejścia, nie całej walki.
Dalszy rzeczywisty input przy64477 uruchamia walkę; przy64657 jeden
rat typ3/60 i dwa typ4/35. Oba nietoperze pokonane M16, przy65537 HP66,
zostaje typ3/60. Sterownik używa wyłącznie inputu, bez zmian stanu gry.
M087: oba typ4/35 oraz typ3/60 pokonane M16. Pod koniec HP9; nagrody
Junk/Junk/Ammo+6 (Obtain All dodało Junk do10 i11), bez Medicine/Cure-P.
Dwa jawne Heal1 podnoszą9→39→69HP; zostaje27PE, menu zamknięte przy67493.
Pełny replay G→M087 zwycięstwo/cap67493 PASS normalnie oraz
ASan/UBSan/function: cały JSON i1MiB VRAM identyczne, SHA256
8b3bcd354f7c098a4c85efe6043541afd24cc76027ee776acccf9e488fdd5bbd.
M087/A80083C8, etap248, HP69/133 (maxHP wzrosło po walce), mode9/live0,
lock0, brak menu, capacity20, broń9/pancerz14, nowe Junk10/11. Wszystkie
prywatne B–G niezmienione, bu10 puste. Obraz frame.png obejrzany.
Artefakty /tmp/pe-m087-victory-{normal,san}.log i katalogi,
/tmp/pe-m087-victory-args.json, verify.py i proof.json.
Próba dojścia do telefonu kończy się przejściem: przy67642 skrypt blokuje
ruch, x1790,z−77, a następnie M086 startup−2, brak native descriptor
slot121/80190B3C. Proces zakończony1. Telefon nie został uruchomiony,
nie utworzono H i nie zaliczono nowego zapisu. Reproducer
/tmp/pe-m086-entry-failure-args.json zamierza67826, trace
/tmp/pe-m087-bind-input.jsonl od64289, backtrace/log
/tmp/pe-m087-bind-live-lldb.log. To jawny brak portu, nie zawieszenie walki.

Następny cały oryginalny moduł: istniejący
src/overlays/room_m086_central_park/m086.c, wszystkie6 funkcji w miejscu.
Rzeczywisty blok ma7080 bajtów, SHA256
802a5f596dc71c68dcdd355f01d5971713843388249cb085a420e5767459772a,
FNV20b7a2d44a6de28b. Task120/80190AEC: callback8018F734,
size28, init80190A44, script80190ACC (16 halfwordów). Task121/80190B3C:
callback80190574, size8, init80190A90, script80190B24 (12 halfwordów).
To SeekerController i BounceGlint oraz dwa callbacki dzieci i dwa
inicjalizatory parametrów. W chwili pierwszego audytu źródło nie było natywne: stare widoki
aktor/pool/bone, callbacki2/3arg, PS1 void** indeksy, int argumenty zamiast
intptr_t, brak kompletnych danych/descriptorów i jawnej arytmetyki wrap.
Audyt /tmp/pe-m086-module-audit.json, pełne dane /tmp/pe-m086-room.bin.
Audyt wyznaczył naprawę całego pliku i istniejących nagłówków, danych
i powiązań120/121 oraz testy CPU/GTE/PM i wejścia opisane niżej.
Po błędnym podejściu do telefonu ponowiono od prawdziwego cap67493.
Pomocnik projekcji musi używać camera.screenOffsetX/Y, nie wartości
GTE odczytanych po Render_ResetGteScreenOffset (160/112). Współrzędne
telefonu wyznaczono z rzeczywistego obrazu, macierzy, projekcji i wysokości
podłoża: okolice x200,z637, podejście z przechodniego x200,z500.
Zwykły Cross uruchamia oryginalny dialog6 "Will you save your progress?".
Yes→Slot1→lista plików (domyślnieG6)→Down do pustegoH7→Save. Przy70028
nowy H ma8192 bajty, SHA256
3536b725f8f40be24e18bb274a6137e748e9f0b00a246ab012b67940cfc9b176.
B–G porównane bajtowo i niezmienione; prywatna kopia checkpointu
/tmp/pe-m087-phone-checkpoint/bu00. Po potwierdzeniu File saved przy70232
lista plików pokazujeH7. Ten H jest autentycznym zapisem przez UI,
bez pe-save/Save_BuildCardFile wywoływanego przez debugger. Zimne ContinueH/cap24000 PASS normalnie i z ASan/UBSan/function:
cały JSON i1MiB VRAM identyczne, SHA256
85b4a434ee41116505895452fd76770a01b332180ba0bb5b95a44855c7297694.
M087, etap248, HP69/133, level12 (stored11 odczytane w LLDB), pełny
sprzęt i inventory zachowane, lock0, brak menu/żywych wrogów. Oryginalny
spawn telefonu to [13762560,−262144,31260672], yaw2048; obraz obejrzany.
H i B–G niezmienione po Continue. Artefakty
/tmp/pe-m087-phone-continue-{normal,san}.log i katalogi,
/tmp/pe-m087-phone-continue-proof.json. Pełny replay G→utworzenie H/cap70232 PASS również normalnie oraz z
ASan/UBSan/function: całe JSON,1MiB VRAM i wszystkie8192 bajty H są
identyczne (także z autentycznym zapisem z debug przebiegu). VRAM SHA256
f88f59d4078c4a95abbe6182a7897d4f2f8730848d7ac81f192b828c603c1cde.
Koniec na liście plików Save/H7 po File saved, bez nadpisania B–G.
Artefakty /tmp/pe-m087-phone-save-{normal,san}.log i katalogi z frame.vram,
/tmp/pe-m087-phone-save-args.json, /tmp/pe-m087-phone-verify.py i
/tmp/pe-m087-phone-proof.json. Oba obrazy Save/Continue obejrzane.
Menu w dalszym debug inputcie zamknięte przy70440 (lock0, brak menu),
a proces zakończono. Wszystkie cztery procesy replay zakończyły się0.
Checkpoint H jest gotowy do następnych niezależnych testów modułu M086;
nie oznacza ukończenia pełnej kampanii ani wszystkich wariantów zapisów.
Root bu00/bu10 nietknięte.
M086: wszystkie6 oryginalnych funkcji naprawiono w tym samym m086.c,
a CMake dołącza cały plik. Callbacki mają natywne int/void*/intptr_t ABI,
initializatory intptr_t, owner/actor/matrices/action_state i context.flags
są rzeczywistymi natywnymi polami. Usunięto stare fałszywe widoki struktur
PS1 z istniejącego nagłówka, bez nowych plików. BounceGlint/mode0 jawnie
zwraca rozmiar natywnej listy (stary C kończył się bez return, podczas gdy
kod MIPS zachowywał wynik InitCallbackList). Odtworzono oba pełne programy,
wektory (0,0,−30)/(0,0,−41), globals i reset BSS przy ładowaniu. Loader
wiąże wyłącznie M086120/80190AEC i121/80190B3C.

192 porównania czterech callbacków z rzeczywistym CPU/ROM BIOS/GTE,
pełne stany, RNG, wszystkie24 sloty puli, contact/owner flags i guards:
FNV3c2b77bfca9f9902, /tmp/pe-m086-cpu.py. Native initializer sprawdza
pełne wskaźniki i parametry. Assets test czyta cały oryginalny moduł,
oba deskryptory, wszystkie słowa skryptów, work-zero i oba wektory.
Oba rzeczywiste programy PM przechodzą pełny start/send/update/draw/expiry
z modelami z tego samego wymaganego CUE, oboma buforami GPU, zagnieżdżonymi
pulami, łańcuchami OT i guardami. Pełny --assets PASS normalnie i z
ASan/UBSan/function: /tmp/pe-m086-pm-fixture.log i
/tmp/pe-m086-final-fixture-san.log. Nie jest to pełna weryfikacja każdego
piksela ani wszystkich granicznych parametrów modeli.

Cold ContinueH→M086/cap24464 na final PASS normalnie i z sanitizerami:
cały JSON i1MiB VRAM identyczne, SHA256
c72b2102bfb00b57f2cc5f4c9b5d76ac15ceca77deb351c8808eb4157502bdd8.
M086/A8008348, etap248, HP69/133, capacity20, broń9/pancerz14, flags0,
lock0, brak menu/żywych wrogów. Wszystkie prywatne B–H identyczne,
bu10 puste; użytkowe karty nietknięte. Obraz M086 obejrzany.
Artefakty /tmp/pe-m086-final-entry-{normal,san}.log i katalogi,
/tmp/pe-m086-entry-h-args.json, /tmp/pe-m086-final-entry-verify.py i proof.json.
Dalszy input na debug v1 (przed samymi castami char* i rozszerzeniem
fixture) uruchamia walkę przy24680: cztery typ3/120HP, defense13.
To jeszcze nie jest zwycięstwo ani pełny dowód przebiegu tej walki.
Dalszy rzeczywisty input w M086 otwiera battle root (Items/PE/Weapon/
Armor/Escape). Pierwsze wejście do Armor było błędem sterowania, wrócono
Circle; później dynamiczny pomocnik odczytuje bieżący kursor zamiast
zakładać jego wiersz. Cztery próby Escape nie kończą walki; HP69→49→19,
przy26780 wszystkie4 typ3 nadal żyją, escape counter4. Odczyt rzeczywistych
core.statusFlags2 każdego wroga daje00552555, w tym bit00040000:
Battle_RollEscapeChance zwraca−1 dla zakazu ucieczki przed losowaniem.
To oryginalne ograniczenie tej walki, nie powód do zmiany prawdopodobieństwa
ani flag wroga. Późna próba Heal1 nie zwiększyła HP przed śmiercią przy27356;
przy27436 HP0/mode3. Nie zaliczamy zwycięstwa ani poprawnego ukończenia
leczenia. Pełny input /tmp/pe-m086-first-loss-args.json i
/tmp/pe-m086-v1-input.jsonl, log /tmp/pe-m086-v1-live-lldb.log; proces zabity
po diagnostyce, bez zmiany zapisów.

Cold H→cztery odmowy Escape/cap26780 na final PASS normalnie i z
ASan/UBSan/function: cały JSON i1MiB VRAM identyczne, SHA256
85f621a32c9ad6ea5d0e865747d989627af567e22ee3cde497909bd31fb4fb9d.
M086, HP19/133, live4/mode0, flags130/lock4, menu zamknięte. Wszystkie
prywatne B–H niezmienione. Obraz obejrzany. To zintegrowany przebieg ataków
wrogów i odmów ucieczki, nie zwycięstwo ani dowód wszystkich wariantów
ich efektów. Artefakty /tmp/pe-m086-escape-four-{normal,san}.log i katalogi,
/tmp/pe-m086-escape-four-args.json, verify.py i proof.json. Debug final
/tmp/pe-m086-escape-four-live-lldb.log odtwarza26780 i potwierdza bit blokady.
Kolejna gałąź od zimnego Continue H używa wyłącznie inputu: wcześniejsze
Heal1 i serie M16. Przy cap30812 pozostają trzy żywe typ3, Aya HP69/133,
flags130/lock4, battle0, bez menu; broń9/pancerz14 i ekwipunek bez zmian.
Pełny replay tego inputu PASS normalnie i z ASan/UBSan/function: cały JSON
i1MiB VRAM identyczne, SHA256
2b8de8980e088ead4f8def85c4757c1b2a9f35b038cf99d0198bf95e07fecd29.
Wszystkie prywatne B–H niezmienione, bu10 puste, obraz obejrzany.
Artefakty /tmp/pe-m086-first-kill-{normal,san}.log i katalogi,
/tmp/pe-m086-first-kill-args.json, verify.py i proof.json. To dowód
wyeliminowania jednego przeciwnika, nie zwycięstwa ani wszystkich klatek.
Dalszy input tej gałęzi kończy się HP0/battle3 przy32524; pełny zapis
/tmp/pe-m086-second-loss-args.json, trace /tmp/pe-m086-heal2-input.jsonl,
log /tmp/pe-m086-heal2-live-lldb.log. Nie zmieniano HP, PE ani pozycji.

Pomocnik /tmp wcześniej pomijał żywych wrogów z flagą4000 przy planowaniu
ruchu; poprawiono wyłącznie obserwację sterownika. Wybór zaklęcia nie
potwierdza jego wykonania: g_MenuSelectedSpell może zawierać poprzednią
wartość, gdy menu PE nadal odrzuca użycie. Pomocnik wymaga teraz także
opuszczenia listy PE. Resident tabela oraz oryginalna Battle_ApplySpellEffect
potwierdzają koszt Heal1=60PE/leczenie30HP, Heal2=120PE/leczenie60HP.
W osobnym Continue H stanie bez ruchu przez1800 klatek w M087 zostawia
PE27 i HP69; Heal1 zostaje odrzucone, menu zamknięto Circle. To nie jest
przygotowanie zasobów przed walką. Prywatne karty tej próby także niezmienione.
Debug /tmp/pe-m086-prepared-live-lldb.log zatrzymany przy26076 w M087,
bez menu, flags0/lock0, HP69/133. Nadal potrzebny skuteczniejszy rzeczywisty
przebieg walki lub przygotowanie przedmiotów przez oryginalne menu/skrzynie.

Dalsza zwykła próba od H (sterownik uwzględnia już niedostępność celu i
30 klatek logiki względem60 VBlanków) kończy się HP0 przy31792, nadal bez
ukończenia M086. Pełny input /tmp/pe-m086-third-loss-args.json, trace
/tmp/pe-m086-prepared-input.jsonl, log /tmp/pe-m086-prepared-live-lldb.log.
Nie uznajemy tego za błąd oryginalnej walki. Poprawiono także pomocnik PE:
odrzucenie zaklęcia z pozostającym menu8 jest jawnym błędem sterowania;
stara wartość g_MenuSelectedSpell nie wystarcza do zaliczenia użycia.

Osobny przebieg DIAGNOSTYCZNY odnawia HP przez debugger na prywatnych
kartach. /tmp/pe-m089-coverage-case.json zawiera pełny prefix od ContinueH,
1189 odcinków inputu26756→40458 i15 jawnych wymuszeń HP. To fixture
pokrycia modułów, NIE dowód zwykłego zwycięstwa ani pełnej kampanii.
Nie zmieniano poziomu, PE, ekwipunku, flag skryptów ani HP przeciwników.
Oryginalne ataki kończą M086 i uruchamiają ekran nagród; Obtain All bierze
Ammo+15 oraz dwa Defense+1 (sloty19/20), End zamyka nagrody. Dalszy input
przechodzi M088, rozmowę przy powozie i wejście M089. Poprzedni build
zatrzymywał startup na M089 slot75/8019405c. Wszystkie B–H w tej kopii
pozostały identyczne z rzeczywistym checkpointem; nie zapisano tej postaci
jako nowego kanonicznego checkpointu.

M089: naprawiono cały istniejący src/overlays/room_m089_central_park/m089.c,
zachowując wszystkie6 oryginalnych funkcji. Pięć callbacków używa wspólnego
natywnego ABI, inicjalizator zwraca intptr_t. Usunięto stare widoki struktur
z offsetami PS1 w istniejącym nagłówku; kod używa prawdziwych FieldActor,
FieldFxTaskContext.flags, activeTaskSlot.end i flag stanu gracza/właściciela.
Przesunięcia ze znakiem zapisano z jawnym zawijaniem MIPS, bez UB C;
rysowanie modeli i działanie całego oryginalnego skryptu pozostają w module.
Odtworzono task119/80194128, wszystkie22 półsłowa skryptu, trzy rozmiary
stanów8/4/12, cztery kolory, cały48-bajtowy color track i zerowany anchor/
modele. Loader wiąże wyłącznie dwa potwierdzone deskryptory M089 oraz
resetuje ich dane robocze. Nie dodano plików repozytorium ani kodu gry
zastępującego istniejące funkcje.

Audyt całego modułu20864 bajty: SHA256
2abdde7b00da24b3255f76f36599d6a4436f22899ad99d9afcedbffadebe2d3a,
FNV3f2c0352d3a3caa9. Całe0xAA4 bajty klasyE zgadzają się z M059 po30
jawnych relokacjach, wszystkie33 cele Configure także potwierdzone.
/tmp/pe-m089-cpu.py wykonuje oryginalne instrukcje i ROM BIOS bez
hookowania funkcji:240 stanów aktualizacji pięciu callbacków, pełne bufory
stanu,24 sloty cząstek, RNG i obie flagi kontaktu dają FNV3345e804cde1fcfa.
Istniejący fixture original_actor_animation sprawdza ten wynik, guardy,
natywny inicjalizator, pełne dane modułu oraz rzeczywisty PM/task119 z
oryginalnymi modelami M089. Wszystkie trzy callbacki główne i obie pule
zagnieżdżone działają do naturalnego wygaśnięcia w obu buforach GPU;
sprawdzane są natywne linki OT oraz granice aren i pakietów.
Pełny fixture --assets PASS normalnie i pod ASan/UBSan/function:
/tmp/pe-m089-final-assets.log, /tmp/pe-m089-v1-assets-san.log. To nie jest
porównanie wszystkich pośrednich klatek z konsolą ani dowód całej kampanii.

Nowy debug build odtwarza wszystkie1189 odcinków inputu i15 wymuszeń HP
ze starego przypadku diagnostycznego, przechodzi dawny błąd i osiąga
M089/A80084C8/cap40458, dialog60, HP181, bez menu. Obraz obejrzany,
VRAM SHA2565218fac66ae858e79d887885405e2b8a4c8db1901a86171e3b4a2661b01410c2.
Dowód /tmp/pe-m089-v1-coverage-proof.json jawnie oznacza coverage_only=true,
ordinary_campaign_proof=false; karty niezmienione. Log
/tmp/pe-m089-v1-coverage-live-lldb.log, obraz /tmp/pe-m089-v1-entry.png,
pełne dane/audyt /tmp/pe-m089-room.bin i /tmp/pe-m089-module-audit.json.
Dalsze dialogi60–63 prowadzą przy41374 do battle0, flags2178/lock4,
M089, HP181. Debugger pozostaje zatrzymany w Boot_PollHost, bez okna;
nie dołożono dalszych wymuszeń HP. Następne kroki: zbadać tę walkę i
kolejne oryginalne moduły, a niezależnie uzyskać zwykłe przejście M086 odH
oraz pełny replay kampanii bez wymuszeń. Nadal nie ma dowodu całej gry.

Następny krok: przejść tę obowiązkową walkę od prawdziwego H, z wcześniejszym
leczeniem/przygotowaniem i skuteczniejszym unikaniem pocisków, bez zmiany
stanu gry przez debugger. Nadal nie ma dowodu pełnej kampanii.
Następnie kontynuować kampanię. Pełna kampania i pełny zakres celu
pozostają nieukończone. Użytkowe bu00/ i bu10/ nietknięte.

Pełny oryginalny input po etapie240 przechodzi M069→M072, ucieczkę Eve
(etap244), M070 z dialogami96/45/97, odblokowane wyjście, powrót do M072
(etap248) i M068. Replay od Continue F do M068/cap103462 PASS normalnie
oraz ASan/UBSan/function z instrumentowanymi PsyZ/SDL: cały JSON,512
zmiennych, timery, wyposażenie i1MiB VRAM identyczne (SHA
ea070568e5c3f5e59b0c24039cf618ccb9f930090d19b36237e860ac2bc36b49).
Ekwipunek, cała tabela4096 bajtów i aktywne sloty są identyczne jak po
scenie etapu240: HP39/107, pojemność18, broń9/pancerz14, ZooKey zachowany,
lock0, bez menu i żywych przeciwników. PrywatneB–F niezmienione.
Artefakty /tmp/pe-post-eve-field-{normal,san}.log/.vram, input
/tmp/pe-post-eve-field-args.json, kontrola
/tmp/pe-post-eve-field-verify.py i proof.json. Nie jest to dowód wszystkich
klatek pośrednich ani niezależne przejście całości na PS1.

Dalszy żywy input dochodzi M068→M074 (dolna alejka przy altanie;
obraz obejrzany, to nadal park). Schody i bieganie po mesh działają,
HP39 i ekwipunek zachowane. Opcja run pomocniczego pe-path w /tmp
używa zwykłego przycisku Circle; zachowuje stop przy skrypcie/menu/walce.
Wyjście M074 wcześniej zatrzymywało program na brakującym descriptorze
M075 slot75/80194048 (oraz slot12/80193F74). Reproducer
/tmp/pe-m075-entry-args.json obejmuje3290 ciągłych odcinków inputu
od24000 do cap105807; stan przed wyjściem M074/cap105687 ma lock1 i HP39.

Cały oryginalny moduł M075 jest teraz kompilowany i podpięty: wszystkie15
funkcji pozostają w motion_particles.c, motion_particles_2.c i
motion_particles_3.c. Poprawiono natywne wskaźniki, rzeczywiste macierze
FieldActor, rekordy pierścieni, wyrównanie pamięci FieldFx i sygnatury
callbacków. Siedem wspólnych metod korzysta z istniejącego PE_OVERLAY=motion,
oryginalnych plików, lokalnych tabel i30-słowowego programu. Przy ponownym
ładowaniu M075 zerowane są oryginalne dane BSS efektów. Nie dodano plików.

Testy normalne oraz ASan/UBSan/function z instrumentowanymi PsyZ/SDL PASS:
2192 stany względem CPU USA (aktualizacja, seed, fade, timer, inicjalizacja
z macierzami aktora i pierścieniem) oraz prawdziwy cykl PM/FieldFx,
wszystkie pięć typów obiektów, oba bufory rysowania, wyrównanie wskaźników
i granice pakietów. Fixture --assets sprawdza cały moduł22116 bajtów,
descriptory, listy callbacków, program i całą relokowaną klasęE wraz
z33-elementową tablicą Configure. Logi /tmp/pe-motion-final-assets.log
oraz /tmp/pe-motion-final-san-assets.log. Testy nie są dowodem zgodności
każdego piksela ani ukończenia walki w M075. Zwykły replay do cap105807
przechodzi wejście M075: HP39/107, ekwipunek zachowany, obraz obejrzany.
Ten sam pełny replay PASS pod ASan/UBSan/function: cały JSON i1MiB VRAM
identyczne z normalnym kandydatem, SHA256 VRAM
769a9adb3a573f3af4867cd154720037d01cd3327738dc4c07efa58add93f787.
PrywatneB–F niezmienione. Artefakty /tmp/pe-m075-motion-{final,san}.log/.vram,
/tmp/pe-m075-motion-verify.py i proof.json. Nadal nie jest to ukończona
walka w M075; nie rozszerzać bindingu na niezweryfikowane warianty.
Ponowny test CTest
original-game-melissa-encounter z PE_REQUIRE_GAME_ASSETS=1 i rzeczywistym
CUE PASS (127,51s), bez abortów opcode206/82.

Harness LLDB dla długiego inputu używa teraz SBLaunchInfo.SetArguments
z JSON zamiast tysięcy argumentów drivera: poprzedni start zablokował się
w SBDebugger.SetInputString/write przed utworzeniem procesu gry.
Ścieżka VRAM musi być ustawiona od początku w JSON; próba jej zmiany
przez debugger na tymczasowy literał przerwała wcześniejszy test statusem2
(Cannot write VRAM capture). Ten przebieg nie jest PASS; nowy, poprawny
replay podany wyżej kończy się statusem0. Bazowy VRAM Continue F zachował
hash142ac576eb17678b541f74eec957299937b7f480618a4d56f54fed8305fced41.
W tamtym przebiegu nie zmieniono produkcyjnego kodu gry ani nie dodano plików.

Oryginalny input przechodzi M358→M067 i kolejną walkę: dwóch typu3
po35HP i jeden typu4 z44HP. M16 pokonuje wszystkich; Heal1 w walce
zwiększa HP7→37 (po kolejnym trafieniu33), końcowe HP9/107.
Przy pełnym ekwipunku18/18 wybrano Obtain All, następnie End; dwa nowe
Junk pozostawiono, poprzednie przedmioty i wyposażenie zachowano.
Po ekranie wyników wróciło sterowanie. Heal1 w polu podniósł HP9→39.
Zoo Key/ID202 uruchamia otwarcie następnej bramy M067; po tekście107
Aya przechodzi do M068. Dalej M068→M072→M069 (amfiteatr), podejście
uruchamia tekst36 i scenę Eve w M071, po dialogach37/38 wraca M069,
etap fabuły240, tekst108 i ponownie sterowanie.

Pełny replay od prywatnego Continue F do M069/cap92236, obejmujący
wszystkie wcześniejsze łupy, Game Over i ponowne Continue, walki M062,
M358 i M067, obie bramy oraz scenę Eve, PASS na tym samym kandydacie
normalnie i ASan/UBSan/function z instrumentowanymi PsyZ/SDL. Cały JSON,
512 zmiennych, timery, ekwipunek i1MiB VRAM identyczne (SHA
381525db6c5273e33dd096c359c071e1d5947bceea494ec9c7a7499075b467a4).
M069, etap240, HP39/107, pojemność18, broń9/pancerz14, lock0, bez menu
ani żywych przeciwników. PrywatneB–F niezmienione. Artefakty
/tmp/pe-eve-stage240-{normal,san}.log/.vram, input
/tmp/pe-eve-stage240-args.json, kontrola /tmp/pe-eve-stage240-verify.py
i proof.json. Obrazy M067/M068/M072/M069/M071 obejrzane headless.
Nie wykonano jeszcze niezależnego przejścia całego odcinka na PS1;
identyczność końcowego VRAM nie dowodzi wszystkich klatek pośrednich FMV.
Nie zmieniano kodu rozgrywki ani nie dodawano plików.

Wcześniejszy osobny replay do M068/cap82412 ukończył zwykły build,
lecz sanitizowany proces zakończył się statusem143 przed raportem —
nie jest liczony jako PASS. Zakończył się też stary debugger. Zapisany
input pozwolił odtworzyć cap92236 w nowym debuggerze; jego VRAM jest
identyczny z ukończonym zwykłym replayem. Późniejszy pełny sanitizowany
przebieg cap92236 wymieniony powyżej zakończył się statusem0.
Dalszy oryginalny input wraca M069→M072, uruchamia ucieczkę Eve i etap244,
przechodzi do M070 i kończy dialogi96/45/97, przywracając sterowanie.
Po odejściu Eve odblokowane trójkąty pozwalają dojść do wyjścia M070;
powrót do M072 ustawia etap248. Żywy debugger: cap102038, HP39/107,
bez menu/locka/przeciwników, M072, pozycja[189857792,?,158793728], yaw76.
Ten odcinek po cap92236 nie jest jeszcze objęty porównaniem obu buildów.
Nowa sesja LLDB87162/PID14980 używa zamrożonego
/tmp/pe-glint-native-m358-live-debug; launcher /tmp/pe-eve240-live-command.sh,
log /tmp/pe-eve240-debug-console.log (stdout debuggera przekierowany,
stan wymaga odczytu logu), wejście nadal /tmp/pe-zoo-checkpoint-input.jsonl.
Nie nadpisywać tego pliku wykonywalnego, dopóki debugger żyje.

Skrzynia Medicine2 w M062 przeszła również oryginalną ścieżkę pełnego
ekwipunku (17/17): komunikat „Your inventory is full”, wymianę
przedmiotu256 na Medicine2/ID7, użycie Medicine2 przez menu (HP35→101),
powrót do skrzyni i odzyskanie przedmiotu256. Po zamknięciu odbioru
lock0, bez menu, ekwipunek jak po walce, klucz i wyposażenie zachowane;
zmienna84 ma268568592 (flaga skrzyni268435456), zmienna128 wraca do0.
Cały replay Continue F do cap59442 PASS normalnie i pod ASan/UBSan/function
z instrumentowanymi PsyZ/SDL: pełny JSON,512 zmiennych, timery, tabela
wyposażenia i VRAM identyczne (SHA
d3ff64abd3ff245689d4db9efdb0e3a330ded48a8950ca1bbe2f4f8e7f821714).
PrywatneB–F niezmienione. Artefakty /tmp/pe-zoo-medicine-{normal,san}.log
oraz .vram, input /tmp/pe-zoo-medicine-args.json, kontrola
/tmp/pe-zoo-medicine-verify.py i proof.json. Pozostałe dwie skrzynie
M062 nie były jeszcze otwarte.

Po powrocie M062→M063 dalsze północne wyjście zatrzymało starą wersję:
„native descriptor missing for M358 slot74 (PS1 80193378)”.
Oryginalne M358/M384/M387 mają identyczny kompletny moduł0x43a4 bajtów
(SHA40b20ebed8367de7e228cf0722adb22362254961ad555f21fb6b664bb4e66ecc).
Ich klasa okna ma te same wszystkie instrukcje co M063/M083 po
relokacji0x34f4, lecz tablica skoków jest przeniesiona przed klasy aktorów;
klasaE ma tę samą pełną procedurę po relokacji0x60. Test --assets
porównuje wszystkie instrukcje obu klas,24 wpisy tablicy skoków,
obie tablice po7 metod i oba deskryptory dla wszystkich trzech lokacji.
Loader wiąże wyłącznie slot74/80193378 i slot75/80193308 z istniejącymi
oryginalnymi funkcjami; inne warianty nie są podpinane przez podobieństwo.
CPU MIPS M358 potwierdza2304 aktualizacjeE (ab6562b990f5c842),648 cykli
okna (d06378940ffec935) i192 stanów lotu (b938fce337189cd5, mostek COP2
PsyZ, nie niezależny audyt sprzętowego GTE). Nie dodano nowych plików
ani nie przeniesiono funkcji. Pełny replay od Continue F, przez wszystkie wcześniejsze interakcje, walkę
M062, Medicine2 i wyjście M063, do M358/cap65219 PASS normalnie oraz
ASan/UBSan/function z instrumentowanymi PsyZ/SDL. Cały JSON,512 zmiennych,
timery, ekwipunek i VRAM identyczne (SHA
b6cb866a0952e3ec8b9f7ee4bbdf6d3fe124ae48671713d0e77955bac15f9493).
M358, HP101/101, lock0, bez menu i żywych przeciwników. PrywatneB–F
niezmienione. Artefakty /tmp/pe-m358-entry-{normal,san}.log/.vram,
input /tmp/pe-m358-entry-args.json, kontrola /tmp/pe-m358-entry-verify.py
i proof.json. Pełny fixture --assets PASS normalnie oraz sanitizowany
(/tmp/pe-m358-assets.log, /tmp/pe-m358-fixture-san-assets.log).
Obraz wejścia obejrzany headless. Dalszy marsz uruchamia walkę z dwoma
typu6 (35HP) i jednym typu4 (57HP). Oryginalny input pokonuje wszystkich,
korzystając z M16 i Medicine1 przez menu (HP18→63). Po ekranach EXP,
awansu i odbioru dwóch Junk wraca sterowanie: M358/cap70450, HP14/107,
pojemność18, lock0, bez menu i żywych przeciwników, battle9, broń9,
pancerz14, ZooKey i wszystkie wcześniejsze przedmioty zachowane poza
zużytym Medicine1; Junk w slotach16/17. Pełny replay tego samego kandydata
PASS normalnie i ASan/UBSan/function z instrumentowanymi PsyZ/SDL:
cały JSON,512 zmiennych, timery, wyposażenie i VRAM identyczne (SHA
95bc7a91277eade6a25947e5f00795d639e95bf6e418b3e13489617d1866c57b).
PrywatneB–F niezmienione. Artefakty /tmp/pe-m358-battle-{normal,san}.log
oraz .vram, input /tmp/pe-m358-battle-args.json, kontrola
/tmp/pe-m358-battle-verify.py i proof.json. Brak niezależnego przejścia
całego spotkania na PS1; M384/M387 mają dowód modułu/danych, nie pełnej
rozgrywki tych lokacji. Późniejsze M067 i amfiteatr opisano powyżej.
Poprzedni debugger zatrzymano po dodatkowym Heal1 w polu (HP14→44),
cap70830, bez menu/locka; ten późniejszy odcinek nie jest częścią
porównanego replayu cap70450.

Po założeniu SP Vest1 oryginalny input wraca z M064 przez M063 do M062.
Podejście do bramy z Zoo Key/ID202 uruchamia oryginalny skrypt bez
potrzeby dodatkowego przycisku; po animacji wraca sterowanie i Aya
przechodzi na drugą stronę. Pełny replay od Continue F do cap50587
zwykłego i sanitizowanego kandydata ma identyczny cały JSON, wszystkie512
zmiennych, timery, wyposażenie oraz1MiB VRAM (SHA
73fe52820f617edc144a559a27cf3987ec5ab6d9c31960c1045e374d992fc673).
ASan/UBSan/function z instrumentowanymi PsyZ/SDL bez błędów.
W sanitizowanym replayu debugger odczytał flagę trójkąta16:128 przed
otwarciem (cap48730),0 po przejściu (cap50587). Zwykły kandydat nie
ma informacji linii init.c, dlatego jego dodatkowy odczyt po przejściu
nie zadziałał; nie jest liczony jako osobny dowód flagi. Końcowy raport
i VRAM zwykłego procesu są zgodne z sanitizowanym. PrywatneB–F zachowały
hashe. Artefakty /tmp/pe-zoo-gate-{normal,san}.log/.vram,
input /tmp/pe-zoo-gate-args.json i /tmp/pe-zoo-gate-proof.json.
Nie jest to jeszcze niezależne porównanie całego przejścia z PS1 ani
ukończenie zoo. Nie zmieniono kodu rozgrywki ani nie dodano plików.

Dalsze podejście za bramą uruchamia walkę z dwoma przeciwnikami typu6,
po32HP każdy. Oryginalny Heal1 zwiększa HP17→47; dwa ataki M16 po dwa
strzały pokonują obydwu. Zamknięcie ekranów EXP/przedmiotów przywraca
pole z HP35/101, lock0, bez menu i żywych przeciwników, battle9.
Nagroda ID6 zajmuje slot16; klucz i oba przedmioty z M064 zachowane,
aktywny SP Vest1. Pełny replay Continue F→obie serie łupów i Game Over→
pancerz→brama→ta walka/cap54211 PASS normalnie oraz pod
ASan/UBSan/function z instrumentowanymi PsyZ/SDL. Cały JSON ze wszystkimi
512 zmiennymi i wyposażeniem, timery i VRAM są identyczne (SHA
089b810e682973cc76d119d979c2482c31ea8179d9ca81193cacd78206f302bc).
PrywatneB–F niezmienione. Artefakty /tmp/pe-zoo-gate-battle-
{normal,san}.log/.vram, input /tmp/pe-zoo-gate-battle-args.json,
kontrola /tmp/pe-zoo-gate-battle-verify.py i proof.json. Medicine2 sprawdzono w nowszym przebiegu powyżej; dwie pozostałe
skrzynie M062 i dalsza fabuła pozostają do przejścia.

Świeży Continue F przechodzi interakcje M064: otwarcie wejścia do
budynku, rozmowę przy ciele (tekst94), szafkę z Zoo Key/ID202,
odbiór SP Vest1/ID160, otwarcie drzwi do kolejnej szafki i odbiór
M203/ID131 po sprawdzeniu klucza. Po zamknięciu ekranów odbioru
wraca sterowanie; HP21/101, etap232. Klucz zajmuje slot13, nowe
przedmioty sloty14/15 (referencje261/262), rekordy wyposażenia5/6.
Pierwsze pięć i wszystkie pozostałe rekordy zachowane; M16 nadal4
naboje i2 sloty. Zmienna skryptowa54 wynosi126. Pełny replay
Continue F→łupy/cap29102 PASS normalnie i sanitizowany: cały JSON,
timery,4096 bajtów wyposażenia i1MiB VRAM identyczne
(SHA8b89a7dc4cb244ecc4130ef9aceaba97b9d8aa3d34421e61165291ba1ce60ed5).
Obrazy interakcji i odbioru przedmiotów obejrzano headless.
Artefakty /tmp/pe-zoo-key-{normal,san}.log/.vram,
input /tmp/pe-zoo-key-route-args.json.

Powrót przez M063 uruchomił drugą walkę z dwoma aktorami3:31.
Ten input zakończył się porażką (HP0); następnie oryginalny Game Over
wrócił do ekranu tytułowego. Widoczne New Game/Continue/Tutorial,
bez EX Game. Continue F w tym samym procesie ponownie wczytał M064,
HP21/101, pierwotny ekwipunek F i oddał sterowanie. Pełny replay
tej sekwencji do cap39129 PASS normalnie i sanitizowany: cały JSON
i VRAM identyczne (SHA4dccd2451dac5b0fac4d3c0a9642dc4fd9ef3c371f100ecd3db014d2ade33b1a).
Nie traktuje się porażki jako ukończenia walki ani przejścia do M062.
Pomocniczy skrypt nawigacji w /tmp zatrzymuje teraz marsz po rozpoczęciu
walki; ruch w walce wymaga jawnego argumentu combat.

Po tym Continue ponowiono wszystkie interakcje M064 i założono
SP Vest1 przez oryginalne menu. W istniejącym Boot_PollHost dodano
aktywny weapon_slot/armor_slot oraz wszystkie512 script_variables
w raporcie diagnostycznym. Pozwala to automatycznie sprawdzać
aktywny pancerz i flagi skryptów po ponownym użyciu stanu gry;
nie zmieniono logiki rozgrywki ani nie dodano plików.

Końcowy kandydat z tym raportem powtarza całą sekwencję od Continue F,
przez pierwsze łupy, porażkę i ekran tytułowy, do ponownego Continue,
ponownego zdobycia przedmiotów i założenia pancerza/cap44566.
Oba pełne replaye PASS (zwykły oraz ASan/UBSan/function z instrumentowanymi
PsyZ/SDL): cały JSON, wszystkie512 zmiennych, timery i1MiB VRAM identyczne
(SHAf9ce49ca1f9e83a07be2ad1977a81e3cd80f92ec20b0a05b9f693b4b87148a2f).
M064, etap232, HP21/101, lock0, bez menu/przeciwników, aktywna broń9,
pancerz14 i zmienna54=126. Cała tabela4096 bajtów zgadza się między
buildami; względem F zmieniły się tylko dwa dodane rekordy5/6.
Raport po usunięciu nowych pól oraz VRAM zgadzają się też z kandydatem
sprzed zmiany diagnostycznej. CTest original-game-actor-animation PASS.

PrywatneB/C/D/E/F we wszystkich powyższych przebiegach pozostały
niezmienione. Artefakty /tmp/pe-zoo-state-final-{normal,san}.log/.vram,
input /tmp/pe-zoo-recovery-equip-args.json, kontrola
/tmp/pe-zoo-state-final-verify.py i proof.json. Jest to natywny dowód
interakcji i ponownego użycia stanu po Game Over; nie wykonano jeszcze
niezależnego przejścia tych interakcji na PS1. Otwarcie bramy M062
po zdobyciu klucza zweryfikowano poniżej; dalsza kampania pozostaje otwarta.


Dalsza eksploracja dociera do M064 i telefonu. Zapis przez oryginalne
menu odtworzył fortify/strcpy abort w Save_BuildCardFile: tytuł Central Park
ma dokładnie64 bajty Shift-JIS, a strcpy dopisywało terminator poza
64-bajtowym polem title. W istniejącym save_flow.c zastąpiono to kopiowaniem
pełnego64-bajtowego pola z wyzerowanego68-bajtowego bufora; sąsiedni
reserved pozostaje wyzerowany, więc wynikowy plik zachowuje oryginalny
nagłówek. Nie skrócono nazwy i nie dodano plików ani nowej warstwy zapisu.

Nowy korpus obejmuje2700 tytułów: wszystkie450 numerów lokacji,
oba style i trzy czasy. Kod USA daje hash89861520334f7fe0,
maksimum64 bajty i36 lokacji z pełnym polem. Testuje się również
cały256-bajtowy nagłówek M064 (8864e135c46b03c4 przy czasie4930).
CPU wykonuje oryginalny kod; jego wywołania libc bzero/memcpy/strcpy
są zwykłymi operacjami pamięci w referencji. Pełny --assets PASS
normalnie i pod ASan/UBSan/function z instrumentowanymi PsyZ/SDL,
włącznie z wcześniejszymi24 pełnymi obrazami8192 bajtów.

Świeży replay Continue E→walki zoo→M062→M063→M064→telefon→nowy slotF,
cap85092, PASS normalnie i sanitizowany. Cały JSON, cztery timery i1MiB
VRAM są identyczne (SHAc7789c829bdb99d9125c44b43f8329af6bc595ad655aaba9e6943db6a34f9951).
Oba przebiegi utworzyły identyczny8192-bajtowy F (SHA
b006ff41571866b85a58ab1f0329d6ef4bdb210cd26801772ecfc28ab5ca5ae0),
z poprawnym CRC. Pełny nagłówek rzeczywistego F porównano osobno
z kodem USA przy jego zapisanym czasie4920; jest identyczny.
Zapis odbywa się przez UI telefonu, bez wywołania sztucznego checkpointu.
PrywatneB/C/D/E nie zostały nadpisane; karty użytkownika nietknięte.

Zimny Continue F/cap24000 również PASS normalnie i sanitizowany:
cały JSON i1MiB VRAM identyczne (SHA
142ac576eb17678b541f74eec957299937b7f480618a4d56f54fed8305fced41).
M064, etap232, HP21/101, brak menu/locka/przeciwników; pozycja
[49545216,−65536,−14745600], yaw3104 odpowiada oryginalnemu punktowi
powrotu od telefonu. Wszystkie sloty i całe4096 bajtów wyposażenia
zgadzają się ze stanem przed zapisem (M16 nadal4 naboje i2 sloty).
Siedem prywatnych zestawówB/C/D/E/F zachowało swoje hashe.
Legitymny checkpoint zachowano w /tmp/pe-zoo-phone-checkpoint/bu00,
manifest /tmp/pe-zoo-phone-card-hashes.json. Replaye: /tmp/pe-save-title-
route-args.json i continue-args.json, wyniki route-{normal,san}.log/.vram
oraz continue-{normal,san}.log/.vram, podsumowanie phone-proof.json.
Dalsze interakcje M064 i zdobycie Zoo Key zweryfikowano w nowszym
przebiegu opisanym powyżej; bramę M062 sprawdzono po zdobyciu klucza.


Dalsze przejście M063 uruchamia walkę z aktorem3:31 i dołączonym
obiektem4:31. Pierwotny replay kończył się abortem przy VBlank71997:
opcode197, script_byte48. USA tabela800910A0 wskazuje80019170,
czyli istniejące Entity_CopyAttachmentData w system_opcodes.c.
Podpięto tę oryginalną funkcję bez zmiany logiki i dodano deklarację
w istniejącym nagłówku; nie dodano plików ani implementacji zastępczej.

Nowy fixture porównuje240 stanów z niehookowanym CPU USA i jego
oryginalnym Render_SetObjectAnim (hash4b22e715ef4e6670): gracz/brak gracza,
pusta lista, dead/subtype/type filtering, pełne wartości identyfikatorów,
pięć wariantów modelu i signed16 numer animacji. Sprawdza też pozostałe
bajty całego aktora oraz240 programów rzeczywistego interpretera.
Pełny --assets przechodzi normalnie i pod ASan/UBSan/function
z instrumentowanymi PsyZ/SDL.

Świeży pełny replay Continue E do wejścia w tę walkę/cap72553 przechodzi
normalnie i sanitizowany: cały JSON i1MiB VRAM identyczne
(SHA59cfca91f35e98e85dcfde136dd53b8fa20edc4c74edffb63a1640d856072408).
Kolejny pełny replay obejmuje ukończenie spotkania i M062/cap77670:
Heal1 przez oryginalne menu PE zwiększa HP37→67, kolejne ataki zadają
obrażenia, M16 pokonuje przeciwnika HP57, ekran wyników zamyka się
i sterowanie wraca do pola. Stan końcowy: M062, etap232, HP21/101,
lock0, bez menu i żywych przeciwników, Cure-P i dwa wcześniejsze Junk
zachowane. Załadowana amunicja M16 ma4: względem E tylko bajt138
tabeli wyposażenia zmienił16→4, pozostałe4095 bajtów bez zmian.
W pełnym replayu normalnym i sanitizowanym cały JSON, cztery timery,
wyposażenie i1MiB VRAM są identyczne (SHA
f9ffd39f4170b2663aa6c64a37763098cc3514ad7608daf99ba7a891eee700bb).
Wszystkie prywatneB/C/D/E bez zmian. Obrazy walki i wejścia M062
obejrzane headless. Artefakty /tmp/pe-attachment-complete-{normal,san}.log
oraz .vram, input /tmp/pe-attachment-complete-args.json,
pełna kontrola /tmp/pe-attachment-complete-verify.py i proof.json.
Nie jest to jeszcze niezależne przejście tej walki na PS1 ani dowód
ukończenia zoo. M062 ustawia barierę trójkąta16; skrypt pobliskiej bramy
sprawdza Zoo Key/ID202, którego bieżący ekwipunek nie zawiera.


Dalszy oryginalny input przechodzi M061: losowa walka z dwoma aktorami5,
odebranie EXP i dwóch Junk (ID20, sloty11/12), powrót do pola oraz
zachodnie wyjście do M063. Przy wejściu brakowało deskryptora
slot74/USA8018FDDC. Włączono istniejący cały room_lib/window_class.c
oraz window_class_table.c, zachowując wszystkie10 funkcji w miejscu.
Poprawiono natywne widoki aktora i stanu, pełne sygnatury PM/callbacków,
argumenty wskaźnikowe, lokalną pamięć zamiast scratchpada i zawijanie
32-bitowej arytmetyki. Loader wiąże M063/M083 po dokładnym slocie i adresie;
nie dodano plików ani zastępczej logiki rozgrywki.

Fixture z wymaganym Disc1 sprawdza cały identyczny moduł0xE08 bajtów
obu pokoi, wszystkie7 metod i oba deskryptory.648 stanów arm/arrival/release
zgadza się z niehookowanym CPU USA (d06378940ffec935).192 stany lotu,
początku ruchu, signed przesunięć, obrotów, skali, targetowania i dojścia
zgadzają się z oryginalnym kodem CPU przy obsłudze COP2 przez PsyZ
(b938fce337189cd5); to nie jest niezależny audyt sprzętowego GTE.
Test obejmuje też natywny wskaźnik wyniku komendy19. Pełny --assets
przechodzi normalnie i pod ASan/UBSan/function z instrumentowanymi PsyZ/SDL.

Pełny świeży replay Continue E→M059/Heal1/nagrody→Medicine1→M060→M061/
walka/nagrody→M063, cap68462, przechodzi normalnie i sanitizowany:
cały JSON, cztery timery,4096 bajtów wyposażenia i1MiB VRAM identyczne
(SHA892e77947e7d67cd6ddffcb38102d9ffc68701ca0a3e5bc3585f927d715a4ce5).
M063: etap232, HP37/101, poziom zapisu7 (wyświetlany8), pojemność17,
lock0, bez menu i żywych przeciwników; obraz wejścia obejrzany headless.
M16 ma7 naboi i nadal2 sloty: względem E tylko bajt138 zmienił16→7,
pozostałe4095 bajtów zachowane. PrywatneB/C/D/E bez zmian.
Artefakty: /tmp/pe-window-{final-route,route-san}.log i .vram;
input /tmp/pe-window-route-args.json. Dalsza eksploracja M063, rozgrywka
M083 oraz warianty window w M358/M384/M387 pozostają niezweryfikowane.


Biblioteka actor_classes.c jest teraz powiązana we wszystkich29 pokojach
Disc1, których kompletny moduł0x35A0 bajtów i tabela są identyczne
z M022. Obsługiwane48 deskryptorów obejmuje17 klasyD (slot70),26 klasyE
(slot75) i5 klasyC (slot73). Loader nadal sprawdza oryginalny adres
każdego deskryptora i używa istniejących pełnych klas; wariant M028
pozostaje osobno. Nie dodano nowych plików ani implementacji zastępczej.

Rozszerzony fixture wczytuje wszystkie29 modułów z CUE/BIN, porównuje
cały kod/dane i wszystkie42 metody oraz sprawdza komplet48 deskryptorów,
klasy, sloty i brak duplikatów. Pełny --assets przechodzi normalnie
oraz pod ASan/UBSan/function z instrumentowanymi PsyZ/SDL. Potwierdza to
zgodność powiązań i danych; nie oznacza przejścia wszystkich29 lokacji.

Nowy zwykły replay Continue E→walka M059/Heal1/nagrody→Medicine1→M060
przechodzi także do M061/cap60580, gdzie starszy kandydat kończył się
brakiem slot75/USA80192520. Etap232, HP61/92, Cure-P w slocie10,
zużyte Medicine1 ze slotu11, lock0, bez menu i żywych przeciwników.
M061 wczytuje obraz oraz oryginalne skrypty wejścia. Pełny replay
sanitizowany tego samego kandydata również PASS: cały JSON, timery,
4096 bajtów wyposażenia i1MiB VRAM identyczne (SHA b07f487c2249974f47f65e7b6bec8c7c2ae529275e6458e8b4bc892794ed55a4).
M16 ma11 naboi po pięciu strzałach i nadal2 sloty; wszystkie inne bajty
wyposażenia zachowane. PrywatneB/C/D/E pozostały niezmienione.
Dalszej eksploracji M061 i pozostałych26 nowych lokacji nie uznaje się
jeszcze za zweryfikowaną rozgrywkę.
Niezależny PS1 w rzeczywistej walce M059 potwierdził też Heal1 przez
oryginalne menu PE: HP17→47 (dokładnie30), broń9/pancerz1 zachowane.
Nie wyrównywano HP ani upływu czasu między przebiegami.


Oryginalny input na kandydacie z powiększonym buforem tła kończy losową
walkę M059: M16 pokonuje aktorów5:31 i6:3, Heal1 przez menu PE zwiększa
HP6→36. Po odbiorze EXP i wszystkich przedmiotów sterowanie wraca do
pola, brak żywych przeciwników, flags2 wyczyszczone, lock0, HP16/92.
Cure-P zostaje w slocie10; nagroda Medicine1 (ID6) trafia do11.
EXP wynosi93; nie wyprowadzano przyrostu z pierwszej klatki animowanego
licznika. W całych4096 bajtach wyposażenia jedyna różnica względem E
to załadowana amunicja M16: offset138 tabeli (rekord4+10) zmienia16→11,
zgodnie z pięcioma rzeczywiście oddanymi strzałami. Dwa sloty broni
po Mod Permit i wszystkie pozostałe bajty są zachowane.
Pełny świeży replay Continue E→Cure-P→walka→Heal1→nagrody/cap53694
przechodzi normalnie i pod ASan/UBSan/function (oba PASS): cały JSON,
cztery timery,4096 bajtów wyposażenia i1MiB VRAM identyczne (SHA
45453224baf684a4891bd5370d10aa13de57e97933cfc7dd92c763bffb401cfb). PrywatneB/C/D/E
bez zmian. Jest to natywne ukończenie jednego spotkania; PS1 niezależnie
potwierdził jego wejście i identyczne typy/sub-ID obu przeciwników,
lecz nie ukończono jeszcze na nim tej samej walki z porównywalnym inputem.

Dalsza automatyzacja inputu tego samego kandydata używa Medicine1
z menu pola: HP16→61, slot11 wyzerowany, Cure-P zachowany. Menu zamyka
się i przejście M059→M060 kończy przy cap57664, HP61/92, lock0 i braku
menu. Ten dalszy fragment nie ma jeszcze świeżego pełnego replayu
sanitizowanego. Główne wejście do zoo M060 ma14 trójkątów podłoża;
kampania nie jest jeszcze ukończona ani zweryfikowana jako całość.


Dalszy realny input w M059 otwiera skrzynię Cure-P i zamyka jej menu.
Natywne50 slotów i zmienna84=133136 zgadzają się z PS1: przedmiot13
trafia do wolnego slotu10, a bit2048 oznacza otwartą skrzynię. Całe4096
bajtów wyposażenia pozostaje zgodne ze stanem po Mod Permit. Pozycje
po interakcji różnią się nieznacznie przez analogowe sterowanie PS1.
Dane59 trójkątów podłoża i wszystkie ich połączenia również są zgodne.

Na dalszej trasie zaczyna się losowa walka z aktorami5:31 i6:3,
potwierdzona niezależnie na PS1. W natywnym wejściu wystąpiło uszkodzenie
OT i awaria GPU_Enqueue przy VBlank50201. Przyczyną był pierwotny limit
scene_object_work=7320 bajtów: model tła walki potrzebuje8972 bajtów
po poszerzeniu nagłówków pakietów, więc nadpisywał początek OT.
W istniejących Boot_InitMemoryLayout i Draw_InitDisplay powiększono
rezerwację do14640 bajtów na64-bit. Nie zmieniano logiki walki ani SDK
w celu pomijania uszkodzonego pakietu; nie dodano plików.

Nowy test przed poprawką odtwarza brak pamięci dla banku24 (9068>7320).
Po poprawce uruchamia oryginalny Render_SetupEntityPrims dla16 banków
Disc1 o indeksach23–38, sprawdza ich rzeczywisty koniec i cały obszar OT
wypełniony znacznikami. Maksimum9068/14640 bajtów; wszystkie znaczniki
pozostają nienaruszone. Pełny fixture --assets przechodzi normalnie i pod
ASan/UBSan/function z instrumentowanymi PsyZ/SDL.
Zwykły replay Continue E→Cure-P→walka/cap50764 przechodzi bez awarii:
M059, etap232, dwóch żywych przeciwników, stan0, HP83/92; Cure-P
pozostaje w slocie10, całe wyposażenie niezmienione. Nie porównuje się
HP/pozycji z późniejszym stanem PS1 po innym przebiegu inputu i nie
uznaje się jeszcze tej walki za ukończoną. Pełny replay sanitizowany
tego samego punktu również PASS: cały JSON i1MiB VRAM identyczne
ze zwykłym buildem (SHA 6c2600ea9ce6353ddc90bd1f09249756af407acb40efaee1000f663f9c3cb3ba), prywatneB/C/D/E bez zmian.


Dalsza eksploracja M374 potwierdziła wyjście na mapę świata i ponowne
wejście przy etapie232. Ponowne wejście stawia dwóch dodatkowych
policjantów: przeszkody przyx283,z390 ix289,z1009 są kolizjami NPC,
nie potwierdzonym błędem ruchu. Przejście do parku biegnie zachodnim
wyjściemx<−1280,z≈550; północna droga wraca na mapę świata.

W M059 brakowało wiązania slot75/USA80192520. Całe0x35A0 bajtów kodu
i tabel z tej lokacji jest identyczne z M022 (SHA
ba882d6c2ff9f160796d490043d58f387b77a06e3ecde233f26de9516952f464).
Podpięto istniejącą klasę E, trzeci wiersz g_RoomActorClassTable;
nie dodano implementacji zastępczej ani pliku. Fixture z Disc 1 sprawdza
oba deskryptory i identyczność całej biblioteki, nie tylko siedmiu metod.

Nowy korpus klasy E wykazał UBSan „left shift of negative value” w ruchu
w ujemnym kierunku. W istniejącym HandlerE poprawiono arytmetykę słów
przy wyznaczaniu celu, delty, czasu i ruchu liniowego/z easingiem, zachowując
32-bitowe zawijanie.2304 stany już rozpoczętego ruchu zgadzają się z
niehookowanym CPU USA (ab6562b990f5c842), włącznie z fazami signed,
lockY, kopiowaniem pozycji i końcem kontrolera.576 pierwszych klatek
(100939fc059de009) porównuje ponadto czas automatyczny, signed lokalne
przesunięcia i transformację; CPU i jego wywołania wykonuje kod USA,
ale COP2 obsługuje PsyZ, więc ten drugi korpus nie jest niezależnym
audytem sprzętowego GTE. Oba korpusy przechodzą normalnie; sprawdzają
szerokie natywne wskaźniki oraz granice oryginalnego slotu PM.
Pełny fixture --assets z instrumentowanymi PsyZ/SDL przechodzi
ASan/UBSan/function również z oboma nowymi korpusami klasy E.
Świeży zwykły replay Continue E/cap45174 na poprawionej arytmetyce
wczytuje M059 i oddaje sterowanie, etap232, HP92/92, broń9/pancerz1,
pozycja[−271974400,0,20381696], obrót3952 zgodne z oryginalnym PS1.
Cały JSON i1MiB VRAM są identyczne z kandydatem sprzed poprawki
arytmetyki (1f74fda52e8a450a2a268087ea6a5ae3f4c138908df26bd54d5aaa0ee1e44aa1).
PrywatneB/C/D/E pozostają niezmienione. Ten sam pełny replay
ASan/UBSan/function zakończył się PASS: cały JSON i1MiB VRAM identyczne
ze zwykłym buildem, wszystkie cztery prywatne zapisy bez zmian.
To świeży dowód przejścia do M059, nie całego parku ani dnia2.


Świeże testy z wymaganym Disc 1 potwierdzają wejście do walki z Melissą
(100,95 s) i pełny scenariusz spotkania (129,22 s), oba PASS bez SKIP.
Komendy82 i206 pozostają podpięte do oryginalnych Scene_SetGlobalFlags
i Entity_SetFieldByTypeId; nie zastępuje się ich pustym pomijaniem.

Dalszy rzeczywisty input po Continue E przechodzi posterunek, FMV7,
scenę samochodu M039 i wejście do Central Parku M374. Znaleziony tam
błąd brakującego deskryptora slot14/USA80192BCC wynikał z pominięcia
istniejącego całego room_lib/effect_sequence.c w buildzie. Moduł zachowuje
wszystkie25 funkcji; poprawiono natywne widoki wskaźników, typy GTE i ABI
callbacków, dołączono oryginalne tabele oraz istniejące class_nop0.c
i class_plant_script.c. Loader wiąże wyłącznie potwierdzone pięć pokoi
M049/M050/M090/M344/M374 i zeruje ich pierwotne dane przy przeładowaniu.
Stan kontrolera ma48 bajtów na64-bit, z wyrównaniem wskaźnika; pozostałe
sześć stanów zachowuje56 bajtów. Nie wydzielono funkcji ani nowych plików.

Fixture z Disc 1 porównuje identyczny kod/dane pięciu modułów, siedem
metod PM,21 wpisów callbacków, rozmiary, skrypt i obie rampy kolorów.
3136 stanów czterech inicjalizatorów zgadza się z niehookowanym kodem
CPU USA, także przy zawężaniu signed rejestrów i losowości BIOS
(hash a5f62be2508a87fd); sprawdzany jest cały512-bajtowy bufor.
Dodatkowe2592 stany trzech aktualizacji sprite'ów zgadzają się z
niehookowanym CPU USA (f7d6259fe0896699): cztery ziarna RNG, cztery
wypełnienia całego bufora, signed wiek slotu, indeksy macierzy0/13 oraz
współrzędne−32768/32767/40000. Fixture sprawdza także cały kontroler;
na PC używa rzeczywistego szerokiego wskaźnika właściciela. Nowy korpus
przechodzi normalnie i pod ASan/UBSan/function. Nie obejmuje aktualizacji
używającej GTE ani rysowania wszystkich wariantów.
Zwykły replay Continue E/cap38000 kończy dialogi przy bramie Central
Parku, oddaje sterowanie, ma etap232 i HP92/92. Nie oznacza to jeszcze
przejścia całego parku ani weryfikacji obrazu wszystkich wariantów efektu.
Niezależny PS1 przeszedł tę samą sekwencję dialogów12–16 i98–106,
oddając sterowanie w M374 przy etapie232, HP92/92, broń9/pancerz1.
Pozycja Ayi[39452672,5439488,38076416] i obrót1466 zgadzają się dokładnie
z PC; nie porównywano jeszcze pełnego obrazu tej sceny z PS1.
Pełny fixture --assets przechodzi ASan/UBSan/function z instrumentowanymi
PsyZ/SDL, w tym nowe porównania modułu i3136 inicjalizacji.
Pełny replay Continue E→M374/cap38000 przechodzi normalnie i pod
ASan/UBSan/function (oba PASS): cały JSON i1MiB VRAM identyczne,
SHA VRAM 841c0cabac22e23dae77d6270f82539e1fccbad1ce4ecfc5713c43f77a3660ab.
Wszystkie prywatneB/C/D/E pozostają niezmienione. Obejrzano także oba
obrazy M374 po zakończeniu rozmowy; nie jest to wyczerpujący audyt pikseli.


Dalszy rzeczywisty input po Continue D przechodzi M042/M043/M051,
ponownie wchodzi M052 i kończy scenę Wayne/Torres143–147. Właściwe
wejście do zbrojowni ma w M051 regionx−510..−360,z−3792..−3645;
drzwi przyx−250,z180 wymagają klucza i nie są tym wejściem.
Oryginalny skrypt przy ladzie Torresa używa osobnego regionux226..479,
z−121..29 i kierunku na ladę; zwykłe zbliżanie się do samego NPC za
ladą nie jest właściwą metodą interakcji.

Przez oryginalne menu wybrano nowy Mod Permit ze slotu10 i M16A1,
zatwierdzono Add slot, zamknięto wynik i dialogi52–53. Permit znika,
broń pozostaje w slocie9; liczba jej slotów rośnie1→2. Całe4096 bajtów
128 rekordów wyposażenia przed i po operacji jest zgodne z niezależnym
wykonaniem na PS1. Jedyna różnica: bajt148 całej tabeli (offset20 rekordu4)
zmienia1→2. SHA przed542fa7d52be2bf2d019ac7389a69fcad248f3f2d9e7224652872ca0a8156cd62,
po f0d59b453fe521729cbe6b91f6cca68f5e31695c443ac9828c9d58425a509e1d.
Oba przebiegi zachowują etap224, HP92/92, broń9/pancerz1 i pozostałe
50 slotów; sterowanie wraca. Pozycje różnią się przez ręcznie sterowany
input przy ladzie, więc nie ogłasza się zgodności obrazu między PS1 a PC.

Istniejący raport Boot_PollHost na limicie headless zawiera teraz także
story_stage, cztery pełne timery i całe4096 bajtów equipment_records jako
hex. Pozwala to sprawdzać wynik ulepszenia i tykanie czasu ze świeżego
procesu bez pobierania stanu debuggerem. Nie dodano pliku, mechaniki gry
ani nowej warstwy rozgrywki. Zwykły pełny replay D→Mod Permit/cap41602
przeszedł na nowym raporcie; wszystkie4096 bajtów zgadzają się z wynikiem
PS1, slot10 wyzerowany, timery0/1/2 aktywne. VRAM zgadza się z przebiegiem
sprzed rozszerzenia raportu (ad1b4bf9579efcb7e7a29bbbdf2bf9f82c94dfef0daa36a1b737ad259e384628).
Ten sam pełny replay przeszedł pod ASan/UBSan/function (oba PASS).
Cały JSON, w tym wszystkie128 rekordów i cztery timery, oraz cały1MiB
VRAM są identyczne. PrywatneB/C/D niezmienione. Jest to świeży dowód
użycia permitu i dodania slotu do M16A1, nie całego Tune-Up, Tool,
pozostałych modyfikacji ani pełnego dnia2. Nie uruchomiono pełnego50/50.


Telefon po ulepszeniu tworzy prywatny zapisE8192 bajty,
83182cd2dc22bda1ca6c0d37302bc52dcb1742e95c861fa69977143a41b2222a.
B/C/D pozostały niezmienione. Świeże Continue E/cap24000 na tym samym
kandydacie przeszło normalnie i pod ASan/UBSan/function (oba PASS).
Pełny JSON, wszystkie rekordy wyposażenia, timery i1MiB VRAM są identyczne
(2265a2819edfc2968c231059ef7a3d1c511b8ff2a01ed2a34e2f63bcfa123a23).
Rekordy zgadzają się ze stanem po ulepszeniu na PS1; M16 ma2 sloty,
permit ze slotu10 zużyty. Zapisany czas rzeczywiście wzrósł: po Continue
zegary0/2=237197, flags1, limit0=21600000. Każdy replay miał osobny prywatny
katalog, wszystkie cztery plikiB/C/D/E bez zmian. Oryginalny PS1 przyjąłE
przez własny Continue po zwykłym soft resecie i odtworzył M042, etap224,
pozycję[39714816,-131072,-51904512], obrót0, HP92/92 i broń9/pancerz1.
Wszystkie50 slotów i całe4096 bajtów wyposażenia zgadzają się z native;
zegary0/2=231882, flags1 i limit0=21600000. To dowód trwałości ulepszenia
i rosnącego czasu przez zapis/wczytanie, nie pełnej kampanii.

Naprawiono inicjalizację zegarów przy rzeczywistym rozruchu. Istniejący
src/main/psyq/libsnd/ssinit_c.c był pominięty w buildzie, a mylna nazwa
SsInit wiązała wywołanie Boot_InitSubsystems z inicjalizacją dźwięku SDK.
Zegary0/1/2 miały flags0 i limit0, więc czas nie rósł po Continue.
Oryginalny USA80036DC8 wywołuje wyłącznie trzy inicjalizatory zegarów.
Dołączono cały istniejący plik, nazwano jego funkcję GameTime_InitCounters
i wywołano ją z pierwotnego miejsca w boot/init.c; nie dodano wrappera
ani pliku.12 pełnych stanów banku niehookowanego CPU zgadza się z C
(28838f78c52e3a0c), także z nienaruszonym skryptowym zegarem3.

Rzeczywisty Continue D na nowym kandydacie uruchamia zegary0/1/2 z flags1,
limit0=21600000.120 VBlanków zwykłego oczekiwania zwiększa każdy z trzech
liczników dokładnie o120; niezależny oryginalny PS1 daje ten sam przyrost.
Natywny punkt przed/po: cap24000→24120, zegar0/2=205997→206117,
zegar1=15699→15819, M042 i etap224 zachowane. Nie nadpisywano czasu,
pozycji ani stanu fabuły. Należy odróżnić historyczną nazwę
g_PlayTimeFrameCounter (zmienna fabuły74) od rzeczywistego licznika2.

Nowe Continue D/cap24000 normalnie i pod ASan/UBSan/function: PASS,
pełny JSON i1MiB VRAM identyczne (2265a2819edfc2968c231059ef7a3d1c511b8ff2a01ed2a34e2f63bcfa123a23),
wszystkie prywatneB/C/D bez zmian. Pełny fixture --assets z instrumentowanymi
PsyZ/SDL przechodzi ASan/UBSan/function. Siedem wymagających dysku CTestów,
w tym świeża pierwsza walka z Melissą:7/7 PASS141.35s (Melissa127.36s),
bez SKIP. Nie jest to pełne50/50 ani pełny replay muzeum po naprawie zegarów;
wcześniejsze replaye nie potwierdzały tykania czasu i pozostają dowodami
wyłącznie opisanych tam stanów. Pełna kampania nadal wymaga weryfikacji.


Domknięto podpięcie istniejących poleceń sterowania geometrią122/187/188.
Nie przenoszono funkcji: Script_GetObjEntryField6, Script_SetEntryScrolled
i Script_SetEntryMirrored pozostają w system_opcodes.c. Ich pomocnicze
funkcje używają wspólnych deklaracji w geom_state.h; usunięto sprzeczne
lokalne deklaracje void dla funkcji zwracających int i przyjmujących
bez znaku współrzędne przewijania. Pozostaje18 niezerowych poleceń
oryginalnej tabeli bez natywnego podpięcia; oryginalne puste sloty pozostają puste.

Niezależne wykonanie USA8001994C/8001998C z ich prawdziwymi helperami
obejmuje36864 kombinacje dwóch wpisów, wszystkich256 bajtów flag,
trzech wartości włączenia i12 par współrzędnych z granicami/ujemnymi
bitami (hash334ad29225047b25). Odczyt USA80018C10 ma120 przypadków
podpisanych półsłów i aliasów wyjścia, także do samego indeksu oraz pól
obu kontrolek (eac2f716824c248d). Istniejący fixture sprawdza cały dispatcher,
nienaruszenie sąsiednich wpisów, pozostałych pól i prawdziwych64-bitowych
wskaźników pakietów. Sześć powiązanych CTestów z wymaganym dyskiem:
PASS12.57s, bez SKIP. Cały fixture --assets przeszedł też ASan/UBSan/function
z instrumentowanymi bibliotekami PsyZ/SDL (exit0). Pełny build przeszedł;
nie uruchomiono pełnego50/50.


Dalsza kontrola fabuły: oryginalny PS1 i natywny port kończą konferencję
prasową i rozmowę u Bakera, wracając do M042 z etapem184, pozycją
[-28049408,-131072,71827456], obrotem3104, HP92/92 oraz bronią9/pancerzem1.
Warunkiem uruchomienia jest podejście do biurka Bakera; sama wcześniejsza
wizyta w pustej M047 przy etapie164 nie uruchamia konferencji.
Porównanie dotyczy stanu rozgrywki; nie dowodzi identycznego renderingu PS1.
Świeże replaye Continue C → konferencja → M042/cap72988 na aktualnym
kandydacie przeszły normalnie i pod ASan/UBSan/function (oba exit0).
Pełny końcowy JSON i1048576 bajtów VRAM są identyczne; VRAM zgadza się
również z przebiegiem sprzed zmian zegarów:
50a629fabc56b70ceed81730637a240918a82b8a931eecd6a1f82e89b19f2d19.
B iC pozostały niezmienione. Nie jest to dowód pełnego dnia2 ani całej gry.

Kontynuacja C → konferencja → mapa → Museum → M038/M191/cap83542
przeszła świeżo normalnie i pod ASan/UBSan/function (exit0), bez zmiany B/C.
Pełny JSON i1048576 bajtów VRAM są identyczne; VRAM zgadza się także
z kandydatem sprzed zmian zegarów (109f3985097dfd2cafa93c76d3f091751bf2479643c33ad77cf14c81f1c05331).
Oryginalny PS1 niezależnie kończy tę scenę z etapem200, pozycją
[427294720,0,474415104], obrotem1024, HP92/92 i bronią9/pancerzem1.
Po rozmowie przy wejściu oba przebiegi osiągają M195, etap192 i pozycję
[43581440,0,-103743488]; obrót zależy tutaj od chwili zakończenia dialogu
(2652 native,2367 PS1), więc nie jest ogłaszany jako zgodny punkt obrazu.
Aktualny native bez sanitizerów odtworzył całą trasę do M195/cap93480,
z VRAM identycznym jak przed zmianami zegarów
(168734b703d5426c4c6b97ea68b869a9e7be3f3c7a000ec930e8c3d2be534426).

Automaty dialogów muszą obserwować wszystkie cztery TextboxEntry.
Rozmowa z Klampem używa też slotu1 (m.in. strona68); sprawdzanie samego
slotu0 daje fałszywe rozpoznanie zawieszenia. Potwierdzenie zwykłym inputem
kontynuuje oryginalny skrypt, bez naprawy gry ani zmiany flag.
Pełne replaye Continue C → muzeum/Klamp → M192/cap124558 przeszły
normalnie i pod ASan/UBSan/function na kandydacie zegarów2a61619ce (oba exit0).
Cały JSON i1048576 bajtów VRAM są identyczne oraz zgodne z wcześniejszym
przebiegiem diagnostycznym (021d84ecfc4d7ef1e9f62a7bbf9af9bb702842b0fd1d9c1d835ce89c2ccc4dfe).
Oryginalny PS1 niezależnie kończy spotkanie: M192, etap208, pozycja
[484245504,0,225443840], obrót1024, HP92/92, broń9/pancerz1 i lock2,
zgodnie z natywnym punktem. B/C niezmienione. Nie jest to pełny dzień2
ani pełny replay po późniejszym podpięciu122/187/188.

Dalszy diagnostyczny przebieg wraca przez M037 do komisariatu, kończy
drugą odprawę M047128–142 i odbiera Mod Permit (przedmiot55 w slocie10).
Oryginalny telefon tworzy nowy prywatny zapisD8192 bajty,
a56259c958f72204b29f970b4bf128977716bb530439233902ab84657663813e,
etap224; B/C niezmienione. To inny punkt niż wcześniejszy testowy D
zapisany przed muzeum — pliki znajdują się w osobnych katalogach.
Na aktualnym kandydacie3475041b2 Continue tego D/cap24000 przeszedł
normalnie i pod ASan/UBSan/function (oba PASS), w osobnych prywatnych
katalogach. Pełny JSON i VRAM są identyczne
(2265a2819edfc2968c231059ef7a3d1c511b8ff2a01ed2a34e2f63bcfa123a23),
wszystkie trzy plikiB/C/D bez zmian, Mod Permit zachowany i sterowanie
przywrócone w M042. Oryginalny PS1 przyjął ten nowy D przez własny ekran Continue i odtworzył
etap224, pozycję[39714816,-131072,-51904512], obrót0, HP92/92, broń9/pancerz1,
pojemność16, wszystkie50 slotów i wszystkie32 bajty rekorduM16 zgodnie
z native. Testy nie dotykały zapisów użytkownika.


Audyt oryginalnej tabeli interpretera wykazał25 niepodpiętych niezerowych
handlerów. Następne cztery,103/104/105/211, są teraz podpięte do istniejących
funkcji zegarów w system_opcodes.c. Poprawiono sygnatury callbacków bez
przenoszenia funkcji i bez nowych plików. Arytmetyka uruchamiania timera
jawnie zawija32 bity, zgodnie z addu/sll MIPS; zachowano nawet oryginalny
wzór (arg0+arg1)*216000+arg2*60. Konwersja HMS ponownie odczytuje wejście
po każdym zapisie, zachowując znaczenie aliasujących operandów.

Cały istniejący moduł time/counters.c używa teraz zwykłych pól hosta:
usunięto wymuszanie powtórzonych zapisów volatile służące dopasowaniu kodu,
bitfieldy zastąpiono jawnym formatem20/6/6, kroki liczników mają zdefiniowane
zawijanie32-bitowe, a iteracja odwołuje się do pól kolejnych GameTimerEntry
zamiast przesuwać wskaźnik do pojedynczego pola przez inne struktury.
Nie zmieniono zasad pauzy, filmu ani odliczania; oryginalne nieograniczone
tykanie przy odpowiednich flagach pozostaje. Nie dodano automatycznego
zatrzymywania przy zerze wbrew temu, co wykonuje CPU gry.

Niezależne wykonanie niezmodyfikowanego USA EXE porównano z natywnym C:
432 pakowania słowa/aliasów (82cfc71941e10949),36 inicjalizacji wszystkich
trzech zegarów (c9281cb7a579fbb4),98304 tyknięcia czterech liczników,
flagi/pauza/prawdziwe warunki filmu (250be6425a7b2025),4032 uzbrojenia
odliczania (97565315c9ec268a),19968 sekwencji zapytanie/flaga/zapytanie
z aliasami (6823bca859fc7725) i768 podpisanych konwersji HMS z pełnym
aliasowaniem wyjść (2e9b02aa3f9f9c80). Łącznie123540 przypadków; polecenia
sprawdzane także przez rzeczywisty dispatcher i zmienne aktora.
Cały fixture --assets przeszedł ASan/UBSan/function, cztery powiązane
CTesty PASS12.57s.

Świeże pełne replaye C → Ben/Daniel → M047 → Baker77 → recepcja/cap50806
przeszły normalnie i pod ASan/UBSan/function. Pełny JSON i1048576 bajtów
VRAM są identyczne, a cały VRAM zgadza się także z kandydatem sprzed
zmiany modułu zegarów (d13414dda3890ba43f8f5c74a14b84f6749165d27ec16fae4133f0cc4cf9d39f).
B iC niezmienione, HP92/92 i M16A1 zachowane, brak menu i otwartego dialogu.
Także świeże Continue C → telefon → zapisD → zamknięcie/cap28000 przeszły
normalnie i pod ASan/UBSan/function. Pełny JSON, VRAM
(ec6dde02638fae86b73183fd4c8ad64300fc6281573be46e82d62e71e93f26b1)
i wszystkie8192 bajtyD są identyczne. D ma nadal ten sam hash co przed
zmianą modułu (6026a670ba1caa68175677c3af3cc2c3c427770e3e20de65d83d9f1be3c064d0).
Każdy przebieg zapisu miał osobny prywatny katalog; B iC bez zmian.
Pozostaje21 niezerowych handlerów do portowania/weryfikacji. Nie jest to
dowód pełnego dnia2, autorskiej sceny z limitem czasu ani świeży pełny50/50.

Od telefonu Continue C zwykły input przechodzi M042/M043, wchodzi i wychodzi
z szatni M045 (oryginalna animacja drzwi154), a w dalszym korytarzu kończy
scenę z Benem i Danielem59–71. Wcześniej zatrzymywał ją niepodpięty73 przy
vblank35921/actor2:0/byte2716. Podpięto istniejący oryginalny
Script_SetBattleStateMode/USA80017d18: zeruje niskie3 bity flag kamery
i ustawia0x80, zachowując resztę. Funkcja pozostaje w system_opcodes.c;
dopasowano jej sygnaturę callbacku z void do int **unused.
1536 niehookowanych stanów CPU USA obejmuje wszystkie256 niskich bajtów
i sześć wzorców górnych bitów (hash9726c7e9d2657525). Test sprawdza także
nienaruszenie całego pozostałego stanu kamery i dispatcher bez operandów.
Cztery powiązane CTesty PASS12.86s, cały fixture --assets pod
ASan/UBSan/function PASS.

Dwa świeże pełne replaye C → M045 → Ben/Daniel → ruch po scenie/cap37060
przeszły normalnie i pod ASan/UBSan/function bez błędów i niepodpiętych
opcode’ów. Cały JSON i1048576 bajtów VRAM są identyczne; SHA256 VRAM:
25da587ae33529604f817590b11e6a3410817a5b1c30f05311b1640ea9fcbb40.
B iC pozostały niezmienione. Oryginalny PS1 niezależnie kończy ten sam
ciąg dialogów: etap164, pozycja[-10878976,262144,88408064], obrót3441,
flagi kamery112, move_lock2 — wszystkie te wartości zgadzają się z portem
przed dalszym inputem. 40 klatek Down w porcie zmienia z o−6553600,
HP92/92 i M16A1 pozostają, brak menu i otwartego dialogu. Żywa sesja obu
wersji dociera dalej do M047 (pusta sala odpraw na etapie164); ta dalsza
lokacja nie jest objęta powyższym replayem SAN. Dzień2 nadal nieukończony.

Kolejny legalny checkpoint powstał przez telefon w M042: zwykły input
od prywatnego Continue B, oryginalny dialog6 „Will you save your progress?”,
animacja telefonu, oryginalne menu231, wolny slot3/C, komunikat „File saved”,
zamknięcie menu i powrót sterowania/cap135047. PlikC ma8192 bajty i SHA256
48cdc8aada0c7b4ae99d93bbdafbcdd204d9e07ac6a593dd8f33e3f812569157.
B pozostał niezmieniony. To zapis interfejsem gry, bez wywoływania serializera
z debuggera, teleportowania lub ustawiania flag/stanu fabuły.

Dwa świeże Continue C od tytułu do cap24000 przeszły normalnie i pod
ASan/UBSan/function, z identycznym pełnym JSON/VRAM (SHA256 VRAM
7fc35189722c289d882476c270c5dd97d4032352b4999c165cd1e114ab55c485).
Oryginalny PS1, po zwykłym soft resecie i Continue z testowej karty,
niezależnie przyjąłC. Z portem zgadzają się M042, dokładna pozycja
[39714816,-131072,-51904512], HP92/92, pojemność16, wszystkie50 slotów,
indeksy broni9/pancerza1, ID260 i całe32 bajty rekordu M16A1.
Fabuła pozostaje na etapie152; nie jest to ukończony dzień2. Żywa trasa
obejmuje także mapę → M024 → mapę → M046 i dialog Eddiego192 oraz
rozmowy Nix17/Daniel19 w M042. Pełna generacjaC do cap135047 nie ma jeszcze
osobnego runtime SAN; testy Continue nie zastępują takiego dowodu.

Osobny świeży replay Continue C → telefon → zapis w wolnym slocie4/D →
zamknięcie menu/cap28000 sprawdza już samą ścieżkę telefonu także pod
ASan/UBSan/function. Normal i SAN mają identyczny pełny JSON, cały VRAM
(SHA256 ec6dde02638fae86b73183fd4c8ad64300fc6281573be46e82d62e71e93f26b1)
i wszystkie8192 bajtyD (SHA256
6026a670ba1caa68175677c3af3cc2c3c427770e3e20de65d83d9f1be3c064d0).
B iC pozostały niezmienione; powrót do M042 bez menu i dialogu, HP92/92,
M16A1 zachowane. Każdy replay miał osobny prywatny katalog karty.

Dalszy przebieg z M052 ujawnił niepodpięty opcode168 przy próbie otwarcia
zamkniętych drzwi działu uzbrojenia M051 (actor5:0, byte1172,
vblank64957, zapytanie o klucz214). Podpięto istniejący oryginalny
Script_CountItemsByValue/USA800194f8 do istniejącej tabeli i nagłówka;
Inv_CountByValue pozostaje bez zmian w active_list.c. 768 niehookowanych
stanów CPU USA potwierdza wynik, aliasowanie wejścia/wyjścia, pojemności
0/1/3/8/16/50, tożsamości broni zapisanej przez indeks, signed zwykłych ID
oraz trzy pule amunicji (hash d87c5049da16e855). Natywny dispatcher zapisuje
wynik do zmiennej aktora, kontrolując sąsiadów i nienaruszenie ekwipunku.
Cztery powiązane CTesty PASS13.58s, cały fixture aktorów z --assets także
ASan/UBSan/function PASS.

Dwa świeże pełne replaye tego kandydata z prywatnego Continue B do cap70135
przeszły normalnie i pod ASan/UBSan/function. Stan końcowy: M046, HP92/92,
M16A1 w slocie9, brak menu i dialogu. Pełny JSON i1048576 bajtów VRAM
są identyczne; SHA256 VRAM:
a785df459e1646cdeb000dae991d819b8899e1dbb0215b75c82ee6c7f2f04d0e.
ZapisB zachował hash70745ddae23fc9e5fa3a6232b724dc436c87271118e046ac35ea1fc152150b22.
Nie jest to nowy pełny zestaw50/50 ani dowód ukończenia dnia2.

Rzeczywisty input po poprawce pokazuje w M051 dialog197 „Authorized
Personnel Only — Weapon Department”, pozwala go zamknąć i wrócić przez
M043 do recepcji M046. Oryginalny PS1 niezależnie potwierdził ten sam
pokój i dialog197. Potem żywa sesja portu wychodzi na oryginalną mapę
Nowego Jorku, podróżuje do M024/Carnegie Hall i wraca na mapę; ta dalsza
podróż nie jest jeszcze objęta pełnym porównaniem runtime z sanitizerami.

Najnowsze poprawki zbrojowni pozostają w oryginalnych modułach: opcode154
steruje animacją geometrii,114 widocznością wpisu, a238 otwiera oryginalny
interfejs przechowywania ekwipunku (zaobserwowany argument1). Podpięto
istniejące funkcje, zachowując ich historyczne nazwy. Nie dodano plików ani
zastępczej logiki gry. W item/swap.c poprawiono oba kierunki wymiany z pustym
miejscem magazynu: natywne NULL nie może być dereferencjonowane. Oryginał
odczytuje wtedy bajt niskiego RAM; zaobserwowane0x5A przepuszcza tę operację.
Ograniczenia aktualnie noszonego wyposażenia i rodzajów19–21 pozostają.

Nowe fixture’y porównują niezmodyfikowany CPU USA z rzeczywistym dispatcherem:
154:10752 stanów (7eccb673b20e4be5),114:6144 (ab69ffd93fa3e525),
238:576 (7c7baf97c5d2fdd5; faza oczekiwania/pominięcia, konstruktor sprawdzany
w grze), wymiana magazynu:5184 (758e388bfa3e3cb1). Ostatni zbiór obejmuje
cztery kierunki, filtrowane indeksy, puste miejsca, maski i zachowanie danych
przedmiotów. Cztery powiązane CTesty PASS13.69s; cały fixture aktorów z
zasobami również przeszedł ASan/UBSan/function.

Zwykły headless replay prywatnego Continue B dochodzi przez Bakera, Tool,
amunicję, rozmowę z Torresem, otrzymanie M16A1 i samouczek M362 do magazynu
M052. Odkłada i odbiera M16A1, wybiera Cancel w dialogu56, zakłada broń przez
oryginalne główne menu i porusza Ayą. Cap57276: HP92/92, broń260 w slocie9,
menu statusu pokazuje M16A1 i16 nabojów. Niezależny PS1 potwierdza odbiór,
samouczek, cykl magazynu i założenie tej samej broni; wszystkie32 bajty jej
rekordu są identyczne. PS1 używa slotu8, bo nie podniesiono tam Tool.
To nie jest porównanie każdej klatki ani pełnego dnia2. Transfer parametrów
przez Tool, cały Tune-Up, pozostałe magazyny i pełna kampania nadal wymagają
weryfikacji. Nie wykonano nowego pełnego zestawu50/50.

Pełny replay z sanitizerami wykrył jeszcze odczyt poza buforem geometrii
w opcode113. M052 ma20 meshów, ale skrypt aktora3:0 przy vblank47041
(byte2088) i48683 (byte2516) adresuje rekord31 z głębokością−10/+10.
Oryginalny PS1 potwierdza długość danych16148, offset tabeli108 i20 meshów:
adres108+31*56=1844 mieści się w surowych danych. Poszerzony natywny
rekord trafiał już za alokację17432 bajtów. Nie wolno po prostu pominąć
tego polecenia ani tylko powiększyć pustej tablicy.

Loader zachowuje teraz długość i pierwotny offset danych obok wskaźnika
geometrii w istniejącym stanie gry. Zwykłe indeksy nadal używają natywnych
meshów. Oryginalny handler113 wykonuje pozostałe adresowania na zachowanych
bajtach pokoju, przez lokalny rekord i zapis tej samej części skalarnej;
odrzuca dostęp wykraczający poza cały oryginalny bufor. Dzięki temu zmiana
jest widoczna również przez pozostałych użytkowników surowych danych.
384 niehookowane stany CPU USA sprawdzają cały224-bajtowy obszar, zachowanie
sąsiednich natywnych meshów i dispatcher (hash b3feaabbff457d95), dodatkowo
sprawdzane są ujemny/nadmierny adres. Cztery powiązane CTesty PASS12.97s.
Cały fixture aktorów z --assets przeszedł ASan/UBSan/function. Pełne replaye
normal i ASan/UBSan/function zakończyły się kodem0 przy cap57276, bez
niepodpiętych opcode’ów i błędów sanitizerów. Cały końcowy JSON i1048576
bajtów VRAM są identyczne; SHA256 VRAM:
77a31aaf16fe210241681db801e33e8ca0aa354039e11c152b1910f97224bb11.
Jest to również ten sam końcowy stan i obraz co wcześniej w zwykłym replayu.
Prywatny8192-bajtowy zapisB zachował SHA256
70745ddae23fc9e5fa3a6232b724dc436c87271118e046ac35ea1fc152150b22.
Świeży oryginalny test original-game-melissa-encounter też PASS129.69s:
potwierdza zgłoszone wcześniej wejście do starcia, niezależnie od późniejszej
trasy zbrojowni. Wszystkie zmiany wykonano w istniejących plikach.

Port naprawia oryginalne moduły C na miejscu. Nie wymaga byte match ani
odbudowy plików wykonywalnych PS1. Porównania z CPU/obrazem oryginału służą
weryfikacji zachowania gry. Nie ma zastępczego silnika ani katalogu pc.

Najnowszy osobny przebieg z prywatnego Continue M034 wygrywa starcie
M034/M348, odbiera nagrody, przechodzi reporterów i nocną rozmowę, a potem
planszę Day2/Fusion i pytanie o zapis w M351. Nie jest to jeszcze pełna
trasa od New Game ani ukończony dzień2. Zapis przez oryginalne menu
jest akceptowany przez Continue portu i PS1. Kolejny przebieg od tego
zapisu przechodzi rozmowę z Bakerem i odbiór Mod Permit;
szczegółowe dowody i dalsze ograniczenia opisano niżej.

Automatyczny input od New Game przechodzi Melissę, zaplecze, przemianę
szczura i zwycięską walkę, następnie M020: dialog112, menu, użycie Medicine1
(HP11->45) i otrzymanie następnego leku. Osobna próba potwierdza zamknięcie
okna oraz powrót sterowania. Nowy test tej trasy przeszedł dwukrotnie.
Katalog zawiera50 testów; zamrożony kandydat b18423087 ma50/50 PASS.
Późniejsze poprawki M032/M033/M034 mają osobne wyniki opisane poniżej;
nie mają jeszcze świeżego pełnego50/50.
Aktualne testy powiązanych modułów oraz fixture’y ASan/UBSan/function przechodzą.
Pełny runtime z sanitizerami dla M020 także przeszedł, z identycznym stanem i VRAM. To nadal
początek kampanii, nie pełny port ani deklaracja pixel perfect.

Po wyjściu z M020 odtworzono kolejne zwycięskie starcie w korytarzu M012.
Oryginalny opcode72 (reset kamery przy graczu) jest podpięty do istniejącej
funkcji; wcześniej zatrzymywał skrypt po awansie poziomu. Powtórzenie inputu
do vblank40000 kończy się bez abortu: brak przeciwników, HP35/49, pojemność
ekwipunku11, brak menu i blokady ruchu. Zrzut obrazu pokazuje Ayę w korytarzu.
Test regresji obejmuje wyłączoną kamerę, wybrany viewport, ograniczenia kadru,
domyślne 30 klatek przejścia i rzeczywisty dispatcher. Cztery powiązane testy
(actor-animation, camera, stat-assets, player-input-assets) przeszły po zmianie.

Nowy test original-game-m017-survivor od New Game obejmuje także zamknięcie
odbioru leku, wyjście M020, dwie dalsze wygrane walki w M012 i awanse,
przejście M011 i powrót oraz pełny dialog85–89 z ocalałym w M017.
PASS223.91s, cap154203: HP14/53, pojemność12, zachowany ekwipunek,
dialog zamknięty, menuNULL i move_lock0. FNV VRAM16dbf16f81d33fc0 zgadza się
z niezależnym od CTest replayem tej samej trasy. To dowód powtarzalności
portu, nie niezależna referencja PS1 dla tych pokoi. Pierwsza wersja asercji
błędnie oczekiwała HP/pozycjiY z checkpointu M020; pełna trasa ujawniła tę
różnicę, więc finalny test sprawdza faktyczny stan New Game. Powiązane trzy
testy actor-animation/player-input-assets/camera także PASS12.25s.

Osobna sesja headless sterowana wyłącznie host inputem z checkpointu M020
potwierdza wejście i wyjście M021, ponowne przeszukanie ciała M020 (dialog115,
otrzymany klucz200), odblokowanie drzwi M018 (dialog118), odczyt dziennika
Melissy99–108 i otrzymanie klucza201. Oba klucze pozostają w ekwipunku po
powrocie do M012. Ten osobny checkpoint-replay nie jest dowodem zgodności
z PS1; jego input i zrzuty zachowano w roboczym /tmp. Pełną trasę od
New Game do M022 sprawdza także opisany poniżej test.

M022 ma natywnie podpięty oryginalny deskryptor slot70/801924e8 z istniejącej
tabeli klas D/D/E/B/C/A. Oryginalne funkcje pozostają w actor_classes.c;
mają typy callbacków PM, poprawiony wspólny prefiks stanu na 64 bitach,
inicjalizację pól C i odczyty pól D przez ich właściwe struktury. Odczyty
trygonometrii korzystają z rsin/rcos PsyZ: wszystkie 4096 par porównano
z USA EXE. Test zasobów sprawdza również 42 adresy metod z archiwum M022,
a test PM obejmuje wszystkie sześć wierszy, konfigurację/zapytanie, kontekst
callbacku, zwalnianie i granice slotów. Funkcja przekształcania celu D używa
bezpośrednio istniejącego stanu klasy zamiast drugiego widoku ze stałymi
offsetami PS1. Jej 300 stanów (kierunki, signed height, wskazany aktor)
zgadza się z oryginalnym CPU z mostkiem GTE; hash996cffcbf9215269.
Mostek używa tego samego GTE PsyZ, więc nie stanowi niezależnego dowodu
zgodności sprzętowego GTE. Fixture ASan/UBSan/function z zasobami także PASS.

Replay z checkpointu M020 przez klucze i dziennik wchodzi do M022 (cap88500)
zarówno w zwykłym, jak i w pełnym buildzie ASan/UBSan/function. Oba zwracają
identyczny JSON (HP41/53, klucze200/201, brak menu/blokady) i identyczny VRAM:
SHA256 6db7bc8899de145348a5833163e8894bd13d1742087fd8a20bb35775b0dd76fc.
Nie dowodzi to jeszcze wszystkich stanów ruchu klas ani zgodności obrazu M022
z PS1. Rozszerzona trasa od New Game do M022 ujawniła osobną awarię przy
vblank170117 w M012: kontakt gracza z przeciwnikiem bez aktywnego effect.
Entity_CheckActionIdMatch ma teraz właściwą sygnaturę contact_callback i
pomija pusty efekt, jak oryginalna funkcja8001d268, której odczyt bajtu0
nie daje aktywnego stanu1 (w działającej referencji PS1 bajt wynosi3).
Porównanie6720 stanów z CPU USA obejmuje brak rdzenia/efektu, różne stany
efektu, HP, grupy kontaktu, terminatory i signed16 part ID; hash a5fd542478aeafa5.
Nowy original-game-m022-keys przechodzi od New Game całą zapisaną trasę
przez M020, dalsze walki i awanse w korytarzu, M011/M017/M021, klucz200,
dziennik Melissy i klucz201, aż do odblokowania i wejścia do M022.
PASS292.93s przy cap200826, HP1/53, oba klucze, brak menu/blokady ruchu;
FNV VRAMb4f02c215f7e80c6 zgadza się z sesją debuggera sterowaną tym samym
inputem. Pięć powiązanych testów tego kandydata PASS305.56s, bez skipów.
Nie jest to świeży wynik pełnych 48/48 ani dowód całej kampanii.

Dalszy replay z checkpointu M020 wygrywa walkę z papugą w M022,
otrzymuje Medicine1 i przechodzi przez drzwi z kluczem201 do pokoju
prób z fortepianem (archiwum M319), dialog120–124 i film5. M023 jest
teraz statycznie podpięty przez oryginalny deskryptor118/80190720.
Naprawy pozostają w istniejącym m023.c i jego nagłówkach: natywne typy
callbacków, offsety pól aktora/kontekstu i koniec areny listy cząstek.
Dane deskryptora, skrypt63 halfwords, kolory i punkt stawu porównano
z archiwum USA. M005 i M023 używają tego samego istniejącego rekordu
roboczego aktywnego pokoju, zerowanego przy zmianie pokoju; nie są
jednocześnie aktywne. Nie dodano plików produkcyjnych.

Aktualny build przechodzi do M023: cap101480, dialog125 „Once again…”,
HP29/64, bez abortu; zrzut pokazuje Ayę i Eve na scenie. Test zasobów
wykonuje180 klatek oryginalnego programu PM, wszystkie trzy rodzaje zadań,
rysowanie rzeczywistego modelu z M023, kontrolę łańcuchów OT i granic
packetów oraz zakończenie zadań. 720 stanów drift callbacku zgadza się
z niezmodyfikowanym CPU USA (hash5f4bf4da84fc4de5). Cztery testy modułów
PASS12.96s, rozszerzony test zasobów PASS12.17s; świeży fixture
ASan/UBSan/function z zasobami również PASS. Pierwszy ring JointGlow
w oryginale czyta poprzednie dane stosu; port wyznacza pozycję stawu
przed rysowaniem, co usuwa niezainicjalizowany odczyt, ale wymaga jeszcze
porównania obrazu. To nie jest dowód całej drugiej walki z Melissą.

Po zamknięciu dialog125–126 dalszy skrypt M023 wymaga opcode226.
Podpięto istniejące Entity_CallAction (USA8001a390), bez zastępowania
funkcji: wielokąt jest odczytywany ze script_base bieżącego aktora,
offset ma zawijanie słowa jak MIPS, count jest u16, wynik trafia do
wskazanego argumentu. 2916 stanów z niezmodyfikowanym CPU USA i
rzeczywistym dispatcherem ma hashd1f7c244a77c2fa5; świeży fixture
ASan/UBSan/function z zasobami PASS. Replay omija ten abort, ale dalej
zatrzymuje się na niepowiązanym opcode54 (aktor2, script_byte4648).
Pełne 48 CTest uruchomiono dla wcześniejszego kandydata8810d644b;
ten przebieg nie obejmuje późniejszego podpięcia226.

Opcode54 jest teraz powiązany z istniejącym Render_CameraTrackEntityZoom
(USA80013e84; mimo dawnej nazwy funkcja prowadzi aktora do innego aktora).
3456 stanów bez hooków CPU USA ma hash174b6ba481a52f15: wybór gracza/NPC,
pomijanie martwego celu, zachowany natywny wskaźnik i serial celu, ruch,
skręt, dojście oraz ponowienie opcode w dispatcherze. Świeży fixture
ASan/UBSan/function z zasobami PASS. Replay z checkpointu M020 po obu
podpięciach przechodzi całą walkę M023: zwykłe menu zużywa zdobyte
Medicine1 (HP6→51), strzały kończą starcie z HP23/64, następuje dialog127–128,
wspomnienie szpitala M367 (dialog130–131) i powrót do M319 z dialog133.
Po zamknięciu dialog133 wraca sterowanie (cap109996, move_lock0, brak menu).
Stan początkowy sceny/dialog125, pozycja Ayi i HP29/64 potwierdzono też
w oryginalnym PS1; późniejszy przebieg walki nie ma jeszcze zsynchronizowanej
referencji RNG/obrazu. Trasa ta nadal zaczyna się od checkpointu M020.

Pełny zestaw dla zamrożonego kandydata8810d644b zakończył się48/48 PASS
w1050.68s (-j2, wymagany Disc1, bez skipów). Obejmuje M023 i poprzednią
trasę New Game→M022, ale nie późniejsze podpięcia226/54 ani całą drugą walkę.

Oryginalny PS1 zaakceptował też checkpoint M022 po papudze: brak papugi,
HP29/64 i ta sama pozycja wejściowa co Continue portu. Ta próba ujawniła
osobną awarię po świeżym Continue: statyczny model w M319 miał pusty
active_matrix. Render_TransformVertices używa teraz w tym przypadku
neutralnych obrotów kości, takich jak inicjalizowane przez istniejący
Render_SetupEntityPrims, oddzielonych od macierzy wynikowych. Nie pomija
rysowania i nie mnoży kolejno wyniku poprzedniej klatki. Test500 transformacji
obejmuje pięć obrotów modelu, rozgałęzienia kości, translacje i granice bufora.
Render_SetupEntityPrims czyści też active_matrix przy zmianie modelu, aby
nie zachować pozy w starej alokacji. Test inicjalizuje obiekt z niepoprawnym
starym wskaźnikiem i sprawdza jego usunięcie. Cztery powiązane testy
PASS13.11s; świeży fixture ASan/UBSan/function PASS.
Rzeczywisty Continue M022→M012→M319 (cap27649, HP29/64, move_lock0)
przechodzi zarówno w zwykłym, jak i pełnym buildzie z sanitizerami:
identyczny JSON i cały VRAM, SHA256
e05136a7d3d0c2559fa16d6483985baeb06720820183067653d9fc898bbe3078.
Wygląd statycznych modeli nadal wymaga dokładnego porównania z PS1.

Nowy original-game-m023-melissa przechodzi od New Game całą wcześniejszą
trasę do M022, Heal1 (HP1→31), walkę z papugą i zdobycie leku, użycie leku
w M319 (HP23→64), dialog120–126 i film5, całą drugą walkę z Melissą,
dialog127–128, wspomnienie szpitala M367 oraz powrót do M319/dialog133.
Kończy się przy cap222996 z HP21/64, kluczami200/201, bez menu i blokady
ruchu. Nie zmienia HP ani skryptów przez debugger. Pierwszy wynik331.99s;
po opisanej poniżej poprawce D PASS330.01s, pięć powiązanych testów
PASS342.88s. FNV całego VRAMc7b404688a203971 potwierdza niezależny replay;
zrzut końcowy obejrzano. To wynik pięciu testów aktualnego kandydata,
nie świeży pełny wynik49/49. Pierwotna asercja obrazu pochodziła z wariantu
z zachowywaniem starej pozy modelu; nowa inicjalizacja zmienia68 pikseli
tego kadru, przy identycznym pełnym JSON gry. Nie jest to referencja PS1.

Dłuższy runtime ASan/UBSan wykrył w ruchu papugi przesunięcie ujemnego
wyniku GTE w RoomLib_HandlerD. Przesunięcia, dodawanie i mnożenie tej
funkcji zachowują teraz zawijanie32-bitowego słowa MIPS, bez signed UB.
1296 stanów już zainicjalizowanego D porównano z CPU USA i mostkiem GTE:
ujemny ruch, skrajne wartości szybkości/progresu/współrzędnych, zwolnienie
i granice natywnej areny; hashc3b4322592dc574c. Mostek współdzieli GTE PsyZ,
więc nie dowodzi niezależnej zgodności sprzętowego GTE. Fixture z sanitizerami
PASS. Rzeczywisty replay M020→papuga→M023→M367→M319 (cap109996) po tej
poprawce przechodzi z ASan/UBSan/function: HP23/64, identyczny JSON i cały
VRAM jak w zwykłym buildzie, SHA256
d3d7038288b7c783d2e831cba311a7df8afeb3a68f475f2f488822ecb4ac34d9.

Po zapisie/Continue z M319 za Melissą próba dochodzi do „Jump in”, M026
i odzyskuje sterowanie. Podpięto istniejące render opcodes189–192 oraz
geometry124–127. 576 i2016 stanów bez hooków CPU USA sprawdza odpowiednio
zawężenia parametrów/flag oraz całe/pojedyncze sloty animacji, zerowe liczby
slotów, sąsiednie dane i dispatcher. Zachowano oryginalne OR z maską0x30,
także w wrapperze o dawnej nazwie ClearObjEntryFlags. Oba zestawy oraz
rozszerzony fixture ASan/UBSan/function PASS. M026 przy cap27134 ma HP21
i move_lock0; Heal1 podnosi HP do51. Kolejna próba wchodzi do M027,
wygrywa starcie (cap32002, HP29, move_lock0) i dochodzi do M028.
W M028 podpięto istniejący OrbitFlare (slot115/80193250) i klasę C
(slot73/801931dc, wiersz4). Fixture czyta oba deskryptory z rzeczywistego
archiwum; cały kontroler658 instrukcji jest zgodny z M012 po19 relokacjach.
Skrypt wymaga też opcode221: podpięto istniejący Entity_PolarToPosition,
zachowując kolejność zapisów i zawijanie słowa MIPS bez signed UB.
4320 stanów bez hooków CPU USA (w tym aliasowanie parametrów i skrajne
wartości) daje hasha8ae21be8bc9567d; fixture sprawdza też720 wywołań przez
rzeczywisty dispatcher. Cztery powiązane CTest PASS13.04s, świeży fixture
ASan/UBSan/function PASS. Replay do wejścia M028/cap33602 przechodzi
zarówno zwykły build, jak i runtime ASan/UBSan/function: HP23/64, trzech
wrogów, identyczny JSON oraz cały VRAM, SHA256
b70a39c181b463165c9af9704627072a262be2d610be0bde9c2ca06d1e5ad352.
Dalsza pierwsza próba zakończyła się śmiercią Ayi. Następny replay od
New Game wygrywa M027 i M028, używając wyłącznie zwykłych inputów: ruch
od ściany, Heal1 i ataki. Ruch w dół oraz na boki zmienia współrzędne;
wcześniejszy brak ruchu nie stanowi dowodu błędu sterowania. Po M028
pozostaje HP16, zdobyto Medicine1, blokada ruchu znika. Trasa wchodzi do
M031, zamyka dialog10–11 i odzyskuje sterowanie; cap246050 ma HP16/67,
capacity14, oba klucze i Medicine1, zero żywych wrogów, bez menu/blokady.
FNV całego VRAMb4dcd37be6d0bb88; kadr M031 obejrzano. Rozszerzono istniejący
runner testów o original-game-m028-sewer; większe lokalne tablice inputów
mieszczą trasę od New Game, host już alokował harmonogram według argc.
Nowy CTest PASS358.62s; pięć powiązanych testów PASS371.27s z wymaganym
Disc1, bez SKIP. To katalog50 testów, nie świeży wynik całego zestawu.

Osobna powtórka od zapisu M319 przez M027 i całą walkę M028 do cap37202
przechodzi zarówno zwykły build, jak i ASan/UBSan/function. Identyczny pełny
JSON i VRAM, SHA256
0245b88628ae2416cfc51b5e09c5a4b8451cf3a2abfbb32486a6952b13582694.
Kończy się bez żywych wrogów i blokady ruchu, HP43/67, capacity14.
To osobny start Continue; nie jest identyczny z przebiegiem od New Game
ani referencją PS1. Nie wprowadzono obejścia HP, zachowania przeciwników
ani nowych plików. Pełne kanały i kampania pozostają nieukończone.

Dalsze otwarcie skrzyni z amunicją w M031 wymaga opcode169. Podpięto
istniejący Entity_InitWithTimeout (USA80019540); deklaracja istniejącego
Menu_OpenEquipSlotSwap ma teraz zgodny zwracany s32. 416 stanów bez hooków
CPU USA potwierdza pierwsze wywołanie/fade, czekanie przy istniejącym menu,
rewind kursora, aktywność/flagi węzła, granice wyniku wyboru i anulowanie:
hash5f9e764ae2a58395. To nie obejmuje pierwszego tworzenia wszystkich
widgetów przez CPU oracle. Świeży fixture ASan/UBSan/function PASS, cztery
powiązane CTest PASS12.96s. Zwykły runtime od zapisu M319 przez kanały
przechodzi dawny abort actor6/script_byte1052; przy cap50000 otwiera menu
wyboru ekwipunku (obejrzano kadr), a Circle zamyka je i oddaje ruch przy
cap50608. Osobny runtime ASan/UBSan/function przechodzi także otwarcie i anulowanie
menu do cap50912: identyczny pełny JSON i VRAM jak w zwykłym buildzie,
SHA2561004acff5dfe649b83b32825f9a777f747e3a3abd07c4c2b2dcca2dac71f9806.
Pełne pobieranie/odrzucanie zawartości wszystkich skrzyń nadal wymaga
weryfikacji.

Lewe schody M031 prowadzą do M032. Podpięto brakujący descriptor
slot73/80192558 do istniejącej klasy C, wiersz4. Fixture czyta rzeczywisty
M032 z PE.IMG: siedem metod oraz cały blok764 instrukcji zgadza się z M028
po dokładnie26 relokacjach adresów. Rozszerzony fixture --assets przechodzi
z ASan/UBSan/function. Pełny zestaw50 testów zamrożonego b18423087 przeszedł w1423.44s (-j2,
wymagany Disc1, bez SKIP). M032 sprawdzono osobnymi buildami z aktualnego
źródła, nie nadpisując działających programów. Po zakończeniu zestawu
odświeżono root build o poprawkę M032; cztery powiązane CTest aktualnego
źródła PASS12.97s. Nie jest to świeży pełny wynik50/50 dla poprawki M032.

Próba od prywatnego Continue w M031 wchodzi do M032; cap29100 obejmuje
trwającą walkę, HP3/67, dwóch wrogów. Zwykły build i runtime z sanitizerami
mają identyczny JSON i cały VRAM, SHA256
8908398a57114c5f4119ef08c39d6620aa6d95cf199e5358dbcc768cc4f6bc04.
Osobna trasa z wcześniejszym Heal1 wygrywa całą walkę M032 i oddaje ruch
przy cap30038: HP27/72, zero żywych wrogów, menuNULL, move_lock0. Oba
buildy przechodzą, pełny JSON i VRAM identyczne, SHA256
30b228b73dae781ed326383edd055e2b0cacced63ba6a2e53207b8b1a8e8ebb6.
Dalszy zwykły input leczy do57, dochodzi do zamkniętej bramy/dialog12,
wraca do M031 i wchodzi prawymi schodami do M334 przy cap40906; kadr
M334 obejrzano. Ta próba zaczyna się od Continue, nie od New Game;
nie jest dowodem przejścia całych kanałów ani porównaniem obrazu z PS1.

Dalsza próba od prywatnego Continue M032 dochodzi do przełącznika przy
lewej ścianie (osobny prostokąt interakcji od samych drzwi), dialog13,
wybiera Press the switch, otwiera bramę i wchodzi do M033. Tam brakowało
opcode95 przy actor0/script_byte840. Podpięto istniejące funkcje
Entity_MarkAnimPending i Entity_AdvancePendingAnim (opcode95/96,
USA80018300/80018364), dostosowując wyłącznie sygnatury do dispatchera.
Wstrzymują/wznawiają zadania w trzech listach aktora; nie zastąpiono
oryginalnego mechanizmu tasków.

4480 stanów bez hooków CPU USA sprawdza puste/rozdzielone listy, wspólne
ogony, wyjątek dla bieżącego zadania, wszystkie flagi i wznowienie kursora:
hashea512f4d706a92b1. Fixture dodatkowo porównuje pozostałe pola węzłów
oraz odtwarza95→96 przez rzeczywisty dispatcher z pełnymi natywnymi
adresami skryptów. Świeży fixture ASan/UBSan/function PASS; cztery
powiązane CTest PASS13.20s. Replay od Continue M032 przez powtórne
starcie, Heal1, otwarcie bramy i wejście M033/cap33500 przechodzi zwykły
build i runtime ASan/UBSan/function: HP48/72, bez menu i blokady ruchu,
identyczny pełny JSON i VRAM, SHA256
39103f7db6b2518f74b8c697791b9cfe27d48cc39c21f6179cd9221cd4654b6c.
Kadr M033 obejrzano. To trasa od Continue, nie nowy przebieg od New Game.
Dalszy zwykły input zamyka dialogi15–22 z Eve i dociera do ładowania M034,
które w tej próbie zatrzymywało się na niepowiązanym deskryptorze
slot8/8018ff7c. Odpowiada istniejącemu modułowi room_lib/glow_orb.c
i jego setupowi; integrację opisano poniżej. Nie uznawać walki z aligatorem,
M034 ani zakończenia dnia1 za ukończone. Bez nowych plików produkcyjnych.

Moduł GlowOrb kompiluje się teraz w całości z istniejących glow_orb.c,
glow_orb_setup.c i trzech wspólnych modułów klas. Deskryptor M034/slot8
wiąże siedem oryginalnych metod oraz wszystkie dwanaście callbacków;
35 słów skryptu, trzy listy callbacków, rozmiary slotów i stałe porównano
z rzeczywistym archiwum M034. Wskaźniki właściciela i jego macierzy
korzystają z istniejących FieldActor/FieldFxObject zamiast offsetów PS1.
Nie przenoszono funkcji ani nie dodawano plików. Dotyczy to na razie
powiązania w M034; pozostałe pokoje używające GlowOrb wymagają weryfikacji.

Fixture porównuje 292 stany z CPU USA bez hooków (hash8fb5ea8e9d0d22fb),
w tym macierze właściciela, setup, spawner, kontroler i granice czasu flasha.
Oba warianty oryginalnego skryptu przechodzą po360 klatek rzeczywistego PM,
z renderowaniem pakietów, kontrolą granic, wygaszeniem i zwolnieniem właściciela.
Nie jest to jeszcze niezależne porównanie pikseli efektu z PS1.

Dalszy skrypt M034 ujawnił brak powiązań istniejących funkcji164,208,209.
Kopiowanie części modelu164 zachowuje oryginalną implementację; fixture
sprawdza także rzeczywisty indeks kości49 i jego zawijanie do s16.
Instrukcje208/209 ustawiają prostokąt i otwierają stylizowane okno tekstowe:
sygnatury korzystają z natywnych argumentów dispatchera i jawnego u16/s16.
4096 stanów prostokąta zgadza się z CPU USA (c532dce5369fff45), a432 stany
otwarcia z różnymi wolnymi slotami i signed page IDs dają114538d7aed539c5.
Każdy przypadek przechodzi także rzeczywisty dispatcher. Cztery powiązane
CTest PASS12.95s z wymaganym Disc1. Wynik pełnych50 testów ze starszego
zamrożonego kandydata nie jest świeżym pełnym wynikiem tych zmian.

Runtime ASan/UBSan/function wykrył przy wejściu M034 ujemne przesunięcie
przy kopiowaniu pozycji części modelu w Entity_FrameUpdate. Trzy osie
w istniejącym frame_update.c przesuwają teraz słowo unsigned, zachowując
bity wyniku PS1. Nie dodano alternatywnego mechanizmu wiązania aktorów.

Świeży fixture ASan/UBSan/function z wymaganym Disc1 i rzeczywistymi
zasobami przechodzi po wszystkich powyższych zmianach. Obejmuje testy
GlowOrb, instrukcji164/208/209 oraz porównanie metadanych M034.

Końcowy build ma5/5 PASS129.34s: actor-animation, player-input-assets,
camera, stat-assets oraz melissa-encounter (od New Game, zwycięstwo nad
pierwszą Melissą i powrót sterowania). Replay Continue M032 → walka →
przełącznik → M033/dialogi15–22 → M034/cap43000 przechodzi zwykły build
i runtime ASan/UBSan/function. Oba mają identyczny pełny JSON i VRAM:
SHA256 a7c3b0070625d8cd938d04f387344d96f5046c1edc0c763800f8c4f912ddc353.
Kadr obejrzano: Aya, aligator i stylizowany komunikat „There are 2 targets!”.
HP48/72, jeden przeciwnik, okno8 aktywne. To potwierdzenie wejścia do walki,
nie zwycięstwa ani zakończenia dnia1. Dalszy input zwykłego builda zamknął
okno i uruchomił akcje walki, lecz Aya zginęła; tej próby nie zaliczono jako
przejścia starcia. Prywatny checkpoint M034 zachowano wyłącznie w /tmp;
zapisy użytkownika nie były modyfikowane.

Próby M034 ujawniły brak natywnych programów PE92/93: użycie Heal1
zwracało -8 z Scene_LoadRoomAssets, po czym faza walki indeksowała PM
ujemnym numerem. HP mogło się zmienić mimo braku efektu, a animacja
rzucania zaklęcia nie oddawała poprawnie ruchu. Nie był to dowód działania
leczenia w walce, mimo wcześniejszych testów samej funkcji zmiany HP.

Cały istniejący fieldfx/sprite_effects.c kompiluje się teraz w natywnym
celu. Rodzina FanSweep/Glow/Spark/Ember ma zgodne callbacki z trzema
argumentami, natywny wskaźnik drugiej puli, dostęp do variables[6] przez
właściwy układ kontekstu oraz oryginalne skrypty i deskryptory92/93.
Wspólny widok kanału puli odpowiada FieldFxTaskSlot na PC. Bufor pozycji
EmberSpark ma również brakujące półsłowo wymagane przez oryginalny odczyt
GTE; sanitizer wcześniej wykrywał odczyt poza lokalną tablicą trzech s16.
Nie przeniesiono funkcji ani nie dodano plików. Pozostałe rodziny w tym
module i ich programy nadal wymagają osobnych powiązań i audytu ABI.

Test rodziny obejmuje100 stanów EmberSpark zgodnych z CPU USA i rzeczywistym
BIOS-em PS1 bez hooków (hash1026cb69f2dfeabd), w tym zawijanie signed
koordynatów i granice wieku18 klatek. Oba oryginalne skrypty przechodzą
po100 klatek pełnego PM: dwie zagnieżdżone pule, rysowanie, łańcuchy OT,
granice aren i zwolnienie slotu. Metadane deskryptorów, oba skrypty,
40 bajtów ścieżki koloru i oba kolory porównano z dostarczonym Disc1.
To nie jest niezależny dowód zgodności pikseli efektu z PS1.

Battle_AdvancePhase sprawdza teraz błąd przydziału efektu przed użyciem
indeksu PM i zatrzymuje wykonanie z konkretnym numerem programu zamiast
odczytywać pamięć sprzed tablicy. Specjalny Scan, który nie przydziela
slotu efektu, rozstrzyga swój warunek przed odczytem tablicy. Pozostałe
niepodpięte programy PE nie są przez tę diagnostykę uznane za działające.

Osobny PS1 zaakceptował także checkpoint M034 z portu: Continue wraca do
komunikatu „There are 2 targets!”, HP48/72 i pozycji
[-57212928,78643200,720896]. Karta i proces emulatora są prywatnymi
zasobami testowymi w /tmp; zapisów użytkownika nie zmieniano. Próby
ręcznego inputu hosta potwierdziły strzał w ogon (-22HP) i użycie Heal1,
ale nie zakończyły jeszcze całej walki z aligatorem.

Końcowy kandydat przechodzi4/4 powiązane CTest (13.14s). Świeży fixture
ASan/UBSan/function z wymaganym Disc1 i zasobami również PASS dla
metadanych,100 stanów CPU/BIOS i obu pełnych cykli efektu. Późniejszą
kontrolę błędu slotu w phase.c sprawdza runtime całej gry z sanitizerami.
Replay prywatnego Continue M034 → menu PE → Heal1 → ruch/cap19782
przechodzi zwykły build oraz ASan/UBSan/function. Pełny JSON i VRAM są
identyczne, SHA256
16116d6323d2f06afc8a60e3d878d8ed2951d79ad7e4694d5fe4edfec6ee370d.
HP48→72/72, menu zamknięte, prawidłowy slot efektu1, powrót do animacji4
i rzeczywista zmiana pozycji po100 klatkach inputu. Kadr obejrzano.
Ten replay używa prywatnego checkpointu w /tmp; nie jest nowym testem
całej trasy od New Game ani ukończeniem starcia. Katalog nadal ma50 testów;
pełny wynik50/50 sprzed tych zmian nie jest ich świeżym wynikiem.

Aktualizacja M034 → M348: cały istniejący moduł drop_field.c jest teraz
kompilowany, z 39 oryginalnymi callbackami i klasą M348/slot9. Powiązanie
sprawdza oryginalny deskryptor, 7 metod, 48 słów skryptu, 14 rozmiarów
rekordów i rampę kolorów. Poprawiono w miejscu natywne wskaźniki,
macierze, krok tablicy pierścieni i przesunięcia słów ze znakiem.
Nie przenoszono funkcji i nie dodano plików projektu.

Watchpoint sprzętowy wykazał błąd wcześniejszej adaptacji: zamknięcie
klasy zapisywało flags (oryginalne +0x4c), uszkadzając exitMode animacji3,
zamiast core_flags (+0). Poprawiono wspólne CloseTarget, DropField
Register/Close oraz flagę trafienia właściciela GlowOrb. Flaga trafienia
gracza nadal prawidłowo używa flags. Bez omijania animacji ani zastępczego
trybu przy brakującej animacji. Zmiana przywraca obrażenia i wymaga innego
inputu niż wcześniejsze replaye; historyczne HP72/cap19782 powyżej nie
opisuje aktualnego buildu. Stary przebieg może kończyć się Game Over
nawet przy kodzie wyjścia0, więc sam exit0 nie jest zaliczeniem walki.

Test istniejącego modułu obejmuje 428 stanów porównanych z niehookowanym
kodem CPU USA (hash b3bad0b238c533aa), 192 przypadki oryginalnego zapisu
flag (15ee6173ba0071b5) oraz dwa natywne kontakty GlowOrb z kontrolą
nienaruszenia sąsiednich pól. Metadane porównano z rzeczywistym M348.
Cztery testy CTest przeszły w13.12s; osobny test aktorów z zasobami przeszedł
ASan/UBSan/function. Nie powtórzono pełnych50 testów po tej zmianie.
Nie oznacza to niezależnej weryfikacji wszystkich rysujących callbacków.

Prywatny Continue M034 i wyłącznie rzeczywisty input hosta pozwoliły
pokonać ogon, przejść do M348, pokonać bossa, odebrać awans/nagrodę
oraz zamknąć dialog25 i wejść M359/cap31726, HP59. Ekran awansu obejrzano.
Nie wstrzykiwano HP, obrażeń ani stanu walki. Warianty DropField M174/M383
oraz cała dalsza kampania nadal wymagają weryfikacji.

Dwa świeże odtworzenia tego samego inputu bez debuggera, zwykłe oraz
ASan/UBSan/function, kończą się poprawnie na cap29806: M348, ekran awansu,
zero żywych przeciwników, HP59/72. Pełny JSON i cały VRAM są identyczne,
SHA256 dc295b4452cc43574b005f522e7846c9245b536d428d28bc5506c41489be105a.
Dalsze przejście do M359 potwierdzono osobno w żywej sesji; ten końcowy
odcinek nie jest objęty powyższym porównaniem dwóch replayów.

Dalsza weryfikacja M359/cap32326 obejmuje nagrodę259 i dialog135 przed
operą: pełny JSON i VRAM zwykłego buildu oraz ASan/UBSan/function są zgodne,
SHA256 eb4f4c084bede8e98d5a315ba2001d994129181e5ac2635bafe7e94a42e8268b.
Rzeczywisty input zamyka dialogi135–144, przechodzi rozmowę w samochodzie
M036 i nocną narrację M000. Następny skrypt sceny a80650c8 ujawnił opcode233
(USA80015c7c): odczyt maksymalnego HP przez Inv_DispatchCommand1107.

Podpięto istniejące Script_RunInvCommand w actor_opcodes.c, zmieniając
wyłącznie widok argumentów na natywną tablicę wskaźników interpretera.
Zachowano oryginalne wykonanie polecenia raz, pierwsze wygaszenie ekranu,
oczekiwanie na menu, rewind0x1c i zapis niezerowego wyniku. Nie ma nowej
implementacji ekwipunku. 1920 stanów z niehookowanym CPU USA obejmuje
odczyty HP, nieznane polecenia, wspólny argument wejścia/wyniku, flagi
węzła, menu obecne/nieobecne, wyniki0/387/-1 i flagi wygaszania;
hash2fe6ce5c1827001d. Dodatkowo rzeczywisty dispatcher wykonuje1107
i kończy węzeł. Cztery powiązane testy przeszły w13.05s; test aktorów
z zasobami przeszedł osobno zwykle i pod ASan/UBSan/function.
To nie jest dowód wszystkich poleceń ekwipunku ani ukończenia dnia2.
Przebieg z tą poprawką dociera do planszy Day2/Fusion (M351, a80650c8),
HP92/92, cap46534; zwykły build i ASan/UBSan/function mają identyczny JSON
i cały VRAM, SHA256
14bc817387cee592853d686a2f54aa3090df45f55a0f5b06c1f0345b019f64a0.

Potwierdzenie planszy ujawniło brak opcode180. Podpięto istniejące
Script_SetMoveFlag/Script_ClearMoveFlag180/181, zachowując oryginalne
ustawienie bajtu g_PlayerMovementEnabled na1/0. 512 stanów porównano
z niehookowanymi funkcjami USA800197d0/800197f0 i wywołano przez rzeczywisty
dispatcher; hasha6aa125c6851c325. Cztery powiązane testy po tej zmianie
przeszły w13.03s. Bez nowych plików, przenoszenia funkcji ani zastępczej
logiki przejścia między dniami. Rozszerzony test233 przechodzi także przez
rzeczywisty dispatcher: otwarcie menu, ponowne oczekiwanie bez powtórnego
odczytu HP oraz zamknięcie z wynikiem387. Cztery testy po rozszerzeniu
fixture’a przeszły w12.51s.

Żywa sesja dochodzi do pytania o zapis (M351/cap49000, dialog182,
HP92/92). Wybór Yes zatrzymuje się na niepodpiętym opcode231
Script_OpenMemCardDialog (vblank49003, actor1:0, script_byte7168).
Cały przebieg prywatnego Continue M034 → walka → M348/nagrody → M359 →
M036 → M000 → plansza dnia2 → pytanie o zapis M351/cap49000 przechodzi
zwykły build i ASan/UBSan/function: oba exit0, identyczny pełny JSON
oraz cały VRAM (SHA256 89a0dc8e65fb644c98e894aaa56a2513834500faec7829d2327a7b9482e40adb). HP92/92,
pojemność16, nagroda259 zachowana. Obejrzano kadry reporterów, samochodu,
nocnej narracji, planszy Fusion i pytania o zapis. Końcowy fixture aktorów
z cyklem menu także przeszedł ASan/UBSan/function z wymaganymi zasobami.
Pełnych50 testów po tych poprawkach nadal nie powtórzono.

To następny konkretny punkt pracy; nie uznawać zapisu po dniu1 ani
właściwej eksploracji dnia2 za ukończone. W kadrze pytania o zapis
widać także nakładające się elementy tekstu; nie deklarować zgodności
obrazu na podstawie samej zgodności obu natywnych replayów.

Eksperymentalny zapis wykonany w trakcie przejścia do M348 nie odtworzył
spotkania po Continue; przyczyna nie została ustalona. Nie traktować go
jako obsługi zapisu w dowolnym momencie. Wszystkie takie pliki i przebiegi
pozostają w /tmp; karty użytkownika nie są zmieniane.

Osobny PS1 zaakceptował także zapis M319 po drugiej Melissie wygenerowany
przez oryginalne funkcje portu: HP21/64, zgodna pozycja Continue, Melissa
nieobecna przy fortepianie; zwykły input dochodzi do wyboru Jump in.
To sprawdzenie konkretnego checkpointu, nie wszystkich zapisów.
Oryginalny PS1 zaakceptował także checkpoint wejścia M334 z portu:
Continue w M334, HP57/72, pozycja startowa [0,31064064,-41943040].
To osobne wczytanie, nie porównanie ciągłego przebiegu, RNG i wszystkich
interakcji w tej lokacji.

Pozostaje do zbadania osobna różnica: żywy rdzeń gracza przed zapisem
zachowywał maxHP53, choć SaveAya i późniejsze Continue/PS1 wskazują64.
Nie uznawać jeszcze roundtripów zapisu ani synchronizacji statystyk po
awansie za w pełni zgodne.

Jeden zapis M012 wygenerowany przez oryginalne funkcje portu zaakceptował
rzeczywisty PS1: File loaded, identyczne HP14/45, pozycja i yaw432 po Continue.
Nie jest to dowód wszystkich save’ów i trybów. Headless runner pozwala podawać
input, raportuje stan aktorów/menu/walki i slotów ekwipunku oraz zapisuje VRAM.
Własny fork PsyZ pozostaje khasinski/parasite-eve-psyz.

Pełna kampania, wszystkie pozostałe lokacje/tryby, Disc2, pełna zgodność obrazu,
hi-res, późniejsze hi-FPS oraz kompletne testy/platformy/paczki pozostają
w zakresie celu. Dalsze sekcje zawierają historię napraw: wcześniejsze
sformułowania o zatrzymanym opkodzie lub teście w toku opisują tamten etap;
aktualne wyniki znajdują się przy późniejszych wpisach.

### Oryginalna pozycja wpisu geometrii i rozmowa z Bakerem

Opcode113/USA80018a9c jest podpięty do istniejącego
Script_SetSceneEntryAnim w system_opcodes.c. Funkcja ustawia współrzędne
i głębokość wpisu geometrii; nie odtwarza animacji postaci. Zastąpiono
stary krok56 bajtów indeksowaniem natywnego GeomEntry i odczyty low16
argumentów jawną konwersją s16. Oryginalne Render_TransformPoint nadal
wykonuje zawijanie współrzędnych, clamp i zapis12-bitowej głębokości.
Usunięto jego nieużywane wypełnienie stosu służące dopasowaniu kodu PS1;
nie przeniesiono funkcji ani nie dodano plików projektu.

7920 stanów z niehookowanym CPU USA porównuje wszystkie48 bajtów
nieposzerzonej części trzech wpisów, signed/high-word argumenty,
normalne/odwrócone/punktowe ograniczenia i zawijanie głębokości;
hash9d5d2d1d374efb69. Natywny test kontroluje także nienaruszenie pełnych
wskaźników i sąsiednich rekordów oraz wykonuje każdy przypadek przez
rzeczywisty dispatcher. Cztery testy powiązane po oczyszczeniu funkcji
przeszły w13.16s; fixture z wymaganymi zasobami przeszedł osobno
ASan/UBSan/function.

Rzeczywisty Continue z plikuB po dniu1 przechodzi rozmowę9–18 w M042,
wchodzi do M041, dialog20 i otrzymuje Mod Permit55. Zamknięcie okna
oddaje ruch;80 klatek Down zmienia z o-15728000,80 Up wraca do tej samej
pozycji (obrót różni się zgodnie z inputem). Samodzielny oryginalny PS1
z tego samego checkpointu także kończy rozmowę, przyznaje55 i pozwala
wyjść do M042; kadr Mod Permit obejrzano po obu stronach, w RAM PS1
potwierdzono nagrodę. Nie synchronizowano każdego momentu obu przebiegów.

Wcześniejszy replay do M041/cap30932 przeszedł zwykle i pod
ASan/UBSan/function z identycznym JSON/VRAM; był sprzed usunięcia
nieużywanego wypełnienia stosu, więc nie zastępuje weryfikacji finalnego
kandydata opisanej dalej.

Finalny kandydat po usunięciu wypełnienia stosu przechodzi dwa świeże
replaye bez debuggera: Continue plikuB → M042/dialog9–18 → M041/dialog20
→ Mod Permit → zamknięcie okna → ruch Down/Up → wyjście do M042 →
korytarz M043 → recepcja M046, cap35776, HP92/92, pojemność16 i permit55.
Zwykły build i ASan/UBSan/function mają identyczny pełny JSON i cały VRAM,
SHA256 e4c2cccd75cba36dcf926860f956b079caa944609d8c17c2d02c25f5f4ffc22d.
PlikB w prywatnej karcie pozostaje niezmieniony. Kadr recepcji obejrzano.

Dalszy zwykły input wraca do M042, przechodzi
korytarz M043 i recepcję M046 oraz wraca do M043 i wchodzi M051/M052.
Ten dalszy odcinek rozpoznano w żywej sesji sprzed usunięcia wypełnienia;
nie jest objęty powyższym porównaniem finalnego kandydata do cap35776.
Rozszerzony finalny replay przechodzi także powrót M046 → M043 → M051
→ M052 i odbiór Tool22, a następnie zamknięcie okna nagrody/cap40776.
Zwykły build i ASan/UBSan/function przeszły z identycznym pełnym JSON
oraz całym VRAM, SHA256
1ec34ad852a5eb13cb5c8672a40906a718bc0e9f51a4db98bfd882ddeca374ad. HP92/92, permit55 i Tool22 w ekwipunku,
menuNULL, dialog zamknięty; plikB niezmieniony. Kadr Tool obejrzano.
Jeszcze dalsza żywa sesja odbiera Ammo+6 i zaczyna dialog25–29 z Torresem;
ten ostatni odcinek nie jest objęty porównaniem cap40776 ani niezależnym
przebiegiem PS1. Czekanie po dialog29 odsłania następny niepodpięty
opcode154 w M052, actor2:0, script_byte1724, vblank44807; nie pomijać go
ani nie uznawać rozmowy za zakończoną. Nie uznawać modyfikacji broni ani całej zbrojowni za
zweryfikowane na podstawie samego otrzymania Toola.

To osobne odtworzenie od checkpointu, nie pełna trasa New Game.
Pełnych50 testów ani całego dnia2 nadal nie potwierdzono.

### Zapis po dniu1 przez oryginalny interfejs

Opcode231 korzysta teraz z istniejącego Script_OpenMemCardDialog w
actor_opcodes.c; zmieniono wyłącznie sygnaturę callbacku i powiązanie
interpretera. Oryginalny dwustopniowy yield/rewind8, konstruktor menu,
wygaszanie i flagi pozostają na miejscu. Bez nowych plików.
576 niehookowanych stanów CPU USA80015af0 pokrywa pierwszy yield i
przypadek już aktywnego ładowania (hash4016f1aa27ca1435); realny dispatcher
sprawdza ponowienie i zakończenie. Ten korpus nie uruchamia konstruktora
menu — robi to poniższy rzeczywisty przebieg. Cztery testy powiązane
przeszły w14.75s; osobny fixture aktorów z wymaganymi zasobami przeszedł
ASan/UBSan/function.

Prywatny Continue M034 → walka/M348 → reporterzy/M359 → samochód/M036 →
narracja/M000 → Fusion/M351 → Yes → Slot1 → pusty plikB → File saved →
wyjście z menu wchodzi do M042/dialog9, cap52104, HP92/92 i pojemność16.
Kadr File saved obejrzano. W katalogu /tmp/pe-day2-save231-entry powstał
BASLUS-006620000000B, 8192 bajtów, SHA256
70745ddae23fc9e5fa3a6232b724dc436c87271118e046ac35ea1fc152150b22.
Początkowy plikA zachował SHA256
6742b60331a4a685f8d5ae9d4775519783ac406806dcb463e1b20ec8b34d7638.
Karty użytkownika nie są używane do tych zapisów.

Dwa świeże odtworzenia całej tej trasy bez debuggera — zwykłe i
ASan/UBSan/function — przeszły do M042/cap52104. Pełny JSON, cały VRAM
(SHA256 0115be205a2d5140d1a5b7cd6236e9a8f298d8d50637bf82806c0eb0117db6a1) i wszystkie8192 bajty
nowego plikuB są identyczne; początkowy plikA w obu katalogach pozostał
niezmieniony. Każdy replay zaczyna od osobnej kopii karty M034.

Osobny proces Continue z samym plikiemB ładuje Fusion, a po zwykłym
potwierdzeniu wchodzi do M042/dialog9: HP92/92, pozycja
[-3342336,-131072,1835008], pojemność16, wszystkie50 slotów ekwipunku
zgodne z zapisem, w tym nagroda259. Pierwsza próba inputu z za długim
Down wybrała pusty slot3 i nie była zaliczeniem; finalny input trzyma
Down przez4 klatki i wybiera prawidłowy slot2.

Osobny Continue z tym samym inputem przeszedł także pełny runtime
ASan/UBSan/function do M042/cap26000. JSON i cały VRAM są identyczne
ze zwykłym procesem Continue, SHA256
67ce367b80fb7cdb453374f419cdf2e82d683c2f0dcbb63b1c2a6e44d8edd2a1. Porównanie nie obejmuje jeszcze
dalszej rozmowy ani M041.

Ten sam plikB umieszczono na prywatnej karcie własnego headless PS1
(PID49001). Oryginał wyświetlił File loaded; po Fusion wchodzi do
M042/a8004148. Bez modyfikacji stanu rozgrywki potwierdzono w RAM
oryginału HP92/92, powyższą pozycję, pojemność16 i wszystkie50 slotów
identyczne z Continue portu. Zachowano kadry File loaded, Fusion i M042
w /tmp. To zgodność tego konkretnego checkpointu, nie wszystkich zapisów,
ciągłego RNG ani wszystkich klatek/renderingu. Kadry M042 obejrzano;
nie były synchronizowane do identycznej klatki animacji.

Dalszy input zamyka rozmowę w M042 i ujawnia niepodpięty opcode113
Script_SetSceneEntryAnim w M041/a80040c8, actor1:0, script_byte84,
vblank54561. Nie traktować eksploracji dnia2 ani kampanii za ukończone.
Pełnych50 testów po tej zmianie nadal nie powtórzono.

## Sprawdzenie widoczności EX Game przy uruchomieniu

EX Game jest odblokowywane przez skan karty w menu, bez wczytywania zapisu,
zgodnie z oryginalnym MenuCard_HandleListInput/MemCard_HasAnyPresentFile.
Lokalny `bu00/BASLUS-006620000001G` ma w nazwie znacznik titleStyleFlag=1.
Dwa świeże headless uruchomienia tego samego executable do 10900 VBlank,
w odrębnych katalogach tymczasowych, potwierdziły: przy pustym katalogu
node 6 ma intensity 0 (EX Game ukryte), a przy kopii istniejących zapisów
intensity 256 i y=140 (widoczne). Nie wczytywano gry i nie zmieniano zapisów
użytkownika. Fixture --assets również przeszedł; obejmuje sześć scenariuszy
menu porównywanych z CPU USA, w tym pojawienie się/usunięcie odpowiedniej karty.
To oczekiwane zachowanie oryginału, więc nie zmieniono warunku odblokowania.

## Bieżące naprawy: pamięć M004, skrypty M005 i prezentacja SDL

Watchpoint wskazał zapis z Render_DrawSpriteEntry poza 128 KiB buforem
D_8010BD00, do sąsiedniej natywnej listy OT. Dwa zestawy pakietów M004
zajmują na ARM64 149856 bajtów. Istniejący wspólny bufor TIM/pakietów ma
teraz 8 MiB, zgodnie z maksymalnym 12-bitowym zakresem sektorów tekstur.
Render_PrepareFrame sprawdza cały zestaw przed pierwszym zapisem; loader
przekazuje błąd inicjalizacji renderera. Usunięto też wymuszenie volatile
potrzebne wyłącznie do historycznego byte match projekcji.

Istniejący fixture kamery wykonuje oryginalny Render_LoadMeshEntry dla 4511
wpisów geometrii Disc 1. Sprawdza oba zestawy pakietów, ich granice i guard
bezpośrednio za przydziałem; największy zestaw ma 192864 bajty. Osobny przypadek
przekraczający pojemność sprawdza odrzucenie bez zmiany workspace i scrolla.
Regresja sterowania została rozszerzona o CROSS w dialogach M004 i przebieg
do 58000 VBlank: dialog 35 zamknięty, game_frame 6211, player
[-134283264,-21360136,489422848]. Oczekiwany FNV całej VRAM to
`d355ef4c28cf53bb`, SHA-256
`a9007a60763479f9332a5fff95dc3b7721bed37d7edb5bd3850d591696d29ac9`.
To regresja konkretnego przebiegu, nie dowód zgodności całego obrazu z PS1.
Dwa pełne przebiegi CTest przeszły 39/39 bez skipów z wymaganym Disc 1;
drugi trwał 402,74 s i obejmował nową regresję 58000 VBlank oraz punkty koloru
z odniesienia PS1. Po naprawach M005/UV pełny zestaw przechodzi ponownie:
39/39, bez skipów, 405,93 s. Świeży instrumentowany executable tego kandydata
zachowuje identyczne pełny stan JSON oraz VRAM M004 do 58000 VBlank.
Świeży instrumentowany executable do 58000 VBlank ma identyczny stan i VRAM
ze zwykłym programem; fixture geometrii i --assets także przechodzą
ASan/UBSan/function z instrumentowanym PsyZ/SDL. Limit nadal używa _Exit,
więc nie jest dowodem naturalnego shutdown ani braku wycieków.

PsyZ ignorował DisplayMode.rgb24 w prezentacji SDL GPU/OpenGL. Oba istniejące
backendy odtwarzają teraz trzy spakowane bajty VRAM na piksel filmu. Test
wszystkich pięciu szerokości PS1 przeszedł w headless i na Metal, także ze
ścieżką prezentacji. OpenGL nie jest wspieranym backendem na macOS i nie został
tu uruchomiony. SDL event watch przekazuje prawdziwe zdarzenia klawiatury do
oryginalnych pakietów pada; Return i keypad Enter mapują START. Istniejący
fixture sprawdza wciśnięcie, puszczenie, focus i ponowny reset rejestracji.

Zgłoszony dolny pas wynikał z prezentacji 240 linii mimo ustawionego przez grę
zakresu 224. Istniejące backendy uwzględniają teraz rejestr zakresu pionowego
i tryb podwójnej wysokości. Test przełącza RGB15/RGB24 i 224/240 linii,
umieszczając inny kolor poza widocznym zakresem; przechodzi headless i na Metal.
Odczyt RGB15 przez publiczne API przechwytywania działa również headless.

Oryginalny opkod 206 (USA 800181CC) jest podpięty do istniejącego
Entity_SetFieldByTypeId. Test wykonuje selekcję aktora, subtype, pomijanie
usuniętych/nieobecnych aktorów, bajt komendy, timery i HP przez dispatcher.
Komenda pola 255 ustawia podłogę przed utworzeniem nowej Ayi; oryginalny
Battle_SetContextField wykonuje ten niezależny zapis przed odczytem playera.
Test obejmuje brak playera i wartość -1111 używaną przez M005. Podpięto również
istniejący opkod 29 (USA 80017E20), Script_CondBranch; zachowuje pełny wskaźnik
hosta i 32-bitowy wrap przesunięcia. Obie gałęzie oraz dispatcher przechodzą.
Kontrolowane wejście oryginalnym loaderem do M005 po M010 dochodzi do 30000
VBlank i 2999 klatek sceny, z Ayą, Melissą i pozostałymi aktorami, bez abortu.
To test inicjalizacji z wymuszonym tokenem w debuggerze, nie naturalne przejście
M004 ani dowód ukończenia spotkania/walki. Świeże --assets przechodzi sanitizery.
Następnie potwierdzono także naturalne wejście od New Game: po dialogu M004
sam input prowadzi Ayę na pozycję przed schodami, skręca na rampę i dochodzi do
M005. Nowy `original-game-stage-transition` w istniejącym fixture wykonuje
właściwy executable z CUE i --pad-at do 60634 VBlank. Sprawdza M005,
video_state 0, pozycję [-107559689,-72810496,35984650] i pełną VRAM:
FNV `4b5ea0ce332f7bf5`, SHA-256
`fb248ac6b643f620032ee08c3dcd64015ce56c40ac55f0fda2ac625c4e650a6e`.
Odtworzenie z CLI odpowiada kontrolowanemu przejściu z samym inputem w debuggerze;
test przeszedł w 92,46 s. To dodatkowy 40. test po pełnym zestawie 39/39,
bez dalszych zmian kodu gry. Osobny aktualny build BUILD_TESTING=OFF przechodzi.

Porównano pierwszą scenę M010 z rzeczywistym obrazem oryginalnego Disc 1
w headless DuckStation (software, SLUS-00662, klatka 16604, dialog 9).
Headless port zachowywał kolory skóry, natomiast Metal pokazywał szare dłonie.
Tekstury i palety obu portowych przebiegów były identyczne. Winne były
niepodpisane korekty UV: shader używał abs pochodnych i dodatkowego przesunięcia
przy odbiciach, co pobierało sąsiednie texele na obróconych modelach.
Shader korzysta z obu podpisanych pochodnych, próbkowania w całkowitych
współrzędnych PS1 i nie potrzebuje dawnego FixupFlipUV. Przebudowano istniejące
nagłówki SPIR-V/MSL/DXIL. Na Metal rzeczywista scena ma znów cieliste dłonie;
sześć kontrolnych punktów skóry różni się od PS1 o 0–1 poziom RGB555.
Fixture uruchomienia ma osiem punktów skóry/mankietów z tego odniesienia,
z tolerancją dwóch poziomów. Nie deklarujemy pełnego pixel perfect.

Nowy test obróconego trójkąta wykazał błąd na Metal przed naprawą i przeszedł
po niej. Dziesięć powiązanych testów GPU oraz trzy testy headless przechodzą.
Pełny zestaw GPU: 41/42; istniejąca różnica obrazu abr_untextured (97,78%,
pierwszy piksel RGB 0/3/0 wobec 0/4/0) jest identyczna na rendererze z e910fd1
sprzed zmiany UV. Nie jest nową regresją i pozostaje otwartym defektem.
MSL wykonano na Metal; DXIL/SPIR-V przebudowano, ale nie wykonano ich tu na
Windows/Vulkan. Fork zawiera e910fd1 i 9fd00d8. Inicjalizacja M005 i naturalne
wejście po schodach są potwierdzone; dalszy dialog, spotkanie i walka pozostają
do przejścia oraz weryfikacji.

Dalszy przebieg z samym inputem przechodzi dialogi 38–44 w M005. Podpięto
istniejące Scene_TestGlobalFlags/SetGlobalFlags/ClearGlobalFlags (81/82/83):
300 stanów masek, w tym bit znaku, zgadza się z rzeczywistym CPU USA
(`7d8e36c81c7f7588`), także przez dispatcher. Następny zatrzymujący opkod 85
to istniejący Boot_MemCardStepInit, historycznie mylnie nazwany: przygotowuje
audio/tekstury i wywołuje Battle_StartEncounter. Funkcja pozostała w swoim
module, ma zgodne ABI oraz propaguje błąd ładowania zamiast zaczynać walkę
z niepoprawnymi zasobami. 192 stany faz/flag/yield odpowiadają CPU USA
(`dcc852ebc349be75`); test obejmuje dispatcher i błąd ładowania tekstur.
Przebieg wykonawczy przeszedł następnie do opkodu 137. Istniejące ustawienia
trybu walki 137/138 mają teraz ABI dispatchera i są podpięte; oryginalny CPU
zapisuje odpowiednio 6/5 pod 8009D28C i zwraca 1. Fixture sprawdza oba przez
dispatcher. To potwierdza dalszy postęp skryptu, nie ukończenie walki.
Aktualny executable z tymi zmianami dochodzi do kolejnego niepodpiętego
opkodu 89 (M005, aktor 0:0, script_byte 4344). Pełny fixture
`original-game-player-input-assets` przechodzi w 11,94 s; świeży build tego
fixture z ASan/UBSan/function i instrumentowanym PsyZ/SDL również przechodzi.

Podpięto następnie istniejące Entity_GetField/Entity_FindByTypeId (89/139),
normalizując ABI pierwszej funkcji w system_opcodes.c. 12288 stanów oryginalnego
CPU USA obejmuje wszystkie bajty komendy, ich powtórzenie z wyższymi bitami,
signed HP, wybór rekordów i skasowanie statusFlags2 przez odczyt komendy 130
(`bc5b5bf975c75f8d`). Oba warianty przechodzą przez dispatcher; test sprawdza
także pustą listę, brak subtype/type i pomijanie usuniętych aktorów.
Ślad naturalnego wykonania potwierdza Battle_StartEncounter przy VBlank 64481,
fazę 58, global_flags 2178, a następnie zakończenie ładowania przy 64487.
Nie dowodzi to jeszcze poprawnego przebiegu ani ukończenia starcia.

Dalszy skrypt Melissy wymaga opkodu 26. Script_Random korzysta teraz z funkcji
Draw_PackPrimColor zapisanej w C w istniejącym draw/prim_color.c zamiast MIPS.
Zachowuje ten sam generator, niskie 16 bitów słowa i podpisany iloczyn zakresu;
równe granice zwracają pełne surowe słowo. 4096 wywołań oryginalnego CPU USA
sprawdza zakresy ujemne/odwrócone/równe i wszystkie 33 słowa stanu po każdej
operacji (`f2f87de64cb28a19`), również przez dispatcher. Pierwotne słowa
instrukcji czytane przez ujemny ogon generatora pozostają danymi w gpu_regs.c.
Następny opkod 111 podpina istniejący Entity_AllocSlot przez jego oryginalny
Script_InitEntityMoveState z poprawnym ABI. Test 5632 alokacji z oryginalnym
hashem CPU USA wykonuje teraz połowę pierwszych przydziałów przez dispatcher,
sprawdzając także wyczerpanie siedmiu slotów, reuse i wrap identyfikatorów.
Świeży fixture --assets z tymi zmianami przechodzi ASan/UBSan/function
z instrumentowanym PsyZ/SDL. Naturalny przebieg doszedł teraz do niepodpiętego
opkodu 90 (M005, Melissa 2:0, script_byte 2076). Dla następnej naprawy wykonano
12288 oryginalnych stanów CPU USA zapisu pól 90 (`2249f25ad3a196bd`), ale to
na razie przygotowane odniesienie, nie dowód poprawności natywnego zapisu.
Oryginał w osobnym headless DuckStation zatrzymał się podczas sekwencji operowej;
nie posłużył jeszcze jako obrazowe odniesienie dla pierwszego starcia.
Pełny zestaw obecnego kandydata przechodzi 40/40 bez skipów w 403,20 s,
z wymaganym Disc 1; obejmuje też ponowne naturalne wejście do M005 (95,63 s).

Kolejna naprawa podpina Entity_SetField (90) i normalizuje jego ABI w miejscu.
12288 stanów zapisu odpowiada teraz odniesieniu CPU USA `2249f25ad3a196bd`,
łącznie z całymi 0xD8 bajtami stanu i podłogą, przez oba opkody 90/206.
Skrypt przeszedł do 183, konfiguracji rekordu ataku. Istniejące polecenia
112/183/176 mają zgodne ABI i są podpięte razem: pełne sześć rekordów,
zawężenia 8/16 bitów, parametry i kroki wejścia/wyjścia przechodzą 216 stanów
CPU USA (`f942d785949a4441`) oraz dispatcher, bez zmiany ich algorytmów.
Przebieg doszedł dalej do 207 (aktor 6:0, script_byte 6264), a po podpięciu
do 149 (script_byte 6272). Istniejący sygnał 207 ustawia g_BattleEnemyEscapeFlag
na 1, jak jednobajtowy zapis USA pod 8009D244. Podpięto też pozostałe ustawienia
trybu walki 149/150 (0/8, USA 8009D28C); test wykonuje teraz wszystkie cztery
ustawienia 137/138/149/150 przez dispatcher. Fixture --assets przechodzi
w 12,69 s i z ASan/UBSan/function oraz instrumentowanym PsyZ/SDL.
Nie jest to jeszcze dowód ukończenia pierwszej walki ani pełnej kampanii.
Aktualny naturalny przebieg z kompletem tych podpięć dochodzi do kolejnego
niepodpiętego opkodu 106 (Melissa 2:0, script_byte 3272). Osobny pełny
executable z ASan/UBSan/function, zbudowany przed podpięciem 207/149/150,
przeszedł tę samą drogę do oczekiwanego abortu 207 bez raportu sanitizerów.
To dowód dla tego wcześniejszego kandydata do wskazanego punktu, nie test
wykonania ostatnich sygnałów ani naturalnego zakończenia procesu.

Podpięto następnie istniejące polecenia PM 106–110. Pm_ScriptSendCommand0
ma ABI int **, bez dawnego WrapperArgs; wartości komendy pozostają 32-bitowe,
a odczyt przez 108 przekazuje pełne adresy hosta. Istniejący test prawdziwego
zadania 117 obejmuje teraz dispatcher: slot/właściciela, pięć parametrów
skalarnych, trzy różne wyniki odczytu (w tym 89ABCDEF) i zapis/odczyt stanu.
Fixture --assets przechodzi także ASan/UBSan/function z instrumentowanym SDK.
Ślad gry potwierdził efekt 117 w slocie 0 przy VBlank 64553, mode 0,
HP Ayi 45 i jednym żywym przeciwniku. Samo uruchomienie efektu było poprawne;
następna odpowiedź kolizji powodowała odczyt przez NULL w Geo_FindNearestEdge.

Przyczyną były dwa odrębne natywne stany wielokąta ścian. Opkod 172 zapisywał
g_EntityCollisionWallSlot/Param, ale Entity_ApplyCollisionResponse czytał nigdy
nieustawiane g_WallPolygonVertices/Count. Oryginalny CPU zapisuje i czyta tę samą
parę 8009D2F8/8009D264; osobny wielokąt podłogi to 8009D248/8009D1CC.
Usunięto duplikat, a istniejący wskaźnik ma właściwy typ PolygonVertex *.
Regresja łączy prawdziwy dispatcher 172 z odpowiedzią kolizji, zachowując
oddzielną podłogę i pełny wskaźnik hosta. 242 przypadki położeń przy narożnikach
i krawędziach, przy dwóch tempach ruchu, odpowiadają CPU z mostkiem PsyZ dla COP2
(`99967d7c2a10f007`). Nie jest to niezależny dowód sprzętowej zgodności GTE.
Przed naprawą pełny program z sanitizerami zgłaszał offset od NULL w tej samej
funkcji; po naprawie fixture przechodzi w 12,06 s i z ASan/UBSan/function.
Naturalny przebieg dochodzi teraz do niepodpiętego opkodu 100 (Melissa 2:0,
script_byte 3368). Dalszy przebieg walki nadal wymaga weryfikacji.
Pełne 40 istniejących testów przechodzi ponownie bez skipów w 407,35 s.
Pełny executable tej naprawy z ASan/UBSan/function i instrumentowanym SDK
dociera do tego samego oczekiwanego abortu 100 bez wcześniejszego raportu
sanitizerów; nie jest to test zakończonej walki ani naturalnego shutdown.
Następnie dodano `original-game-battle-entry` do istniejącego fixture i CMake,
bez dalszych zmian logiki gry. Przebieg od New Game z samym --pad-at kończy się
przy 64554 VBlank, M005, global_flags 2178, move_lock 4, game_frame 2138,
zamkniętym dialogu 44 i pozycji Ayi [1441792,-72810496,56885248]. Sprawdza pełną
VRAM: FNV `a2276ccc2247ed21`, SHA-256
`f2a8d2f23b068d0edf9f95ccd0c4f1ae7915d33f5f5001af3db5073eb369e683`.
Nowy, dodatkowy 41. test przeszedł w 98,85 s; obraz tego punktu obejrzano.
HUD i dalsza interakcja walki nie są jeszcze potwierdzone tym punktem.

Podpięto istniejący Script_RunEntityCommand (100). 12960 porównań z CPU USA
obejmuje sześć rekordów, rodzica dla kind 2/4, przejścia animacji, wrap klatki,
flagi, zwrot wyniku i ponowienie instrukcji przez dispatcher
(`aa6e5b4256b2bf25`). Entity_SetActionMode wykonuje się jako oryginalny CPU;
granica żądania audio jest rejestrowana, bez wykonywania audio w tym oracle.
Osobne wcześniejsze testy audio pozostają wymagane; to nie dowód dźwięku walki.

Naturalny przebieg ujawnił potem odczyt przez NULL w FieldFx_UpdateTasks:
zakończenie animacji prawidłowo zeruje aktywny rekord, a efekt żyje dłużej.
Istniejąca procedura nie zapisuje już sygnału do nieobecnego rekordu i nie
stosuje wtedy bramek jego fazy. Nie dodano zastępczego rekordu ani niskiej
pamięci PS1. Porównanie 972 stanów zadania/właściciela z CPU USA obejmuje
brak rekordu oraz fazy 0–4, stop/sygnał/wait, HP i tryb aktora
(`15a8baf2abf03bd9`). Oryginał po boot ma pod adresem 0 bajt 3; oracle daje
ten sam hash dla 3 i 4. Odczyty NULL nie oznaczają więc fazy zero. Natywny
kod nie powiela zapisu pod adres 0 przy zakończeniu efektu bez odbiorcy.
Podpięto też istniejące sprzątanie 151/152/153 z poprawnym ABI: regresja
wykonuje prawdziwe zadania PM przez dispatcher, sprawdza właściciela,
wybór gracza/aktora, pomijanie usuniętego aktora i brak subtype oraz reset
obu banków. Naturalny przebieg dochodzi do opkodu 198 (Aya 0:0,
script_byte 4832); dalsza walka pozostaje niepotwierdzona.
Skorygowany kandydat przechodzi pełne 41/41 bez skipów w 413,59 s.
Świeży pełny executable z ASan/UBSan/function i instrumentowanym SDK
dochodzi do tego samego oczekiwanego abortu 198 bez raportów sanitizerów;
nie oznacza to ukończenia walki ani naturalnego shutdown. Następnie wzmocniono
regresję 151/152/153 o aktywne wpisy drugiego banku. Bez dalszych zmian logiki
gry fixture --assets przechodzi ponownie w 12,10 s oraz z ASan/UBSan/function.

Wcześniejsze zatrzymanie oryginału w DuckStation wynikało z włączonej
przyciskiem Start pauzy (global_flags 8001). Ponowny Start przywrócił 8000
i normalną sekwencję operową, w tym film z pożarem teatru. Nie był to
potwierdzony błąd emulatora; odniesienie pierwszej walki nadal wymaga dojścia.
Oryginał doszedł już do sterowania Ayą w M004 po pożarze (klatka 55110).

Podpięto istniejący opkod 198, oczekujący na akcję aktora: 2880 przypadków
odpowiada wykonaniu CPU USA (`27a316c7f7829479`). Następnie podpięto istniejące
operacje flag aktora 74/79/80/185/186/193/194/195, zachowując ich moduły
i algorytmy; 2304 przypadki CPU i dispatchera dają `ab04cf6d428f3bf9`.
Naturalny przebieg od New Game do 82000 VBlank kończy spotkanie z Melissą:
M005, dialog 60 zamknięty, Melissa nieobecna, tryb walki 12, zero przeciwników,
odblokowany ruch, HP Ayi 14/45. Zwykły i świeży instrumentowany executable
ASan/UBSan/function mają identyczną pełną VRAM, SHA-256
`93cfc546507339fa3b83301e213d1ad26ac9fbee4d77aec2251f006a3c4d12c4`;
FNV `c7ae3598591ad7c3`. Limit nadal kończy proces przez _Exit.

Niezależnie oryginał PS1 przeszedł spotkanie do tej samej pozycji Ayi,
trybu walki i liczby przeciwników. Porównanie całych 320×224 RGB555 daje
71660/71680 identycznych pikseli; pozostałe 20 leży na Ayi i różni się
o najwyżej dwa poziomy kanału. Nie jest to pełny pixel perfect. HP oryginału
wynosi 17/45, a global_flags 0x8000 wobec 0 portu. Przy różnych harmonogramach
inputu/RNG różnice te pozostają niewyjaśnione i wymagają dalszej weryfikacji.

Pełne dotychczasowe 41 testów przeszło bez skipów w 411,22 s. Następnie
rozszerzono istniejącą diagnostykę o tryb/liczbę przeciwników/HP i dodano
42. test `original-game-melissa-encounter` w istniejącym fixture: sprawdza
powyższy stan i pełną VRAM. Nowy test oraz ponowiony bootstrap przeszły
odpowiednio w 128,26 s i 145,11 s, z wymaganym Disc 1 i bez okna.

Naturalne wyjście po spotkaniu prowadzi do M009 (`0xA80004C8`), za scenę.
Pierwszy długi harmonogram inputu wielokrotnie przechodził przez te same drzwi
w obie strony. Ślad Script_EncodeBarcode/Boot_RunFrame potwierdził poprawny
wybór obu tokenów; nie był to dowód błędu loadera ani celu przejścia.
Po puszczeniu kierunku przy wejściu, przebieg do 95751 VBlank pozostaje w M009:
game_frame 504, move_lock 0, HP 14/45, pozycja Ayi
`[61903440,-21495808,-178751558]`, yaw 3536. Obejrzano obraz lokacji;
pełna VRAM ma FNV `1a99c887686e1a86`, SHA-256
`ae6271a22ca7d9ec26ed1b808e261553ce2c4be84f854b8e559dca392044ad45`.
Oryginał PS1 także doszedł do M009, ale pozycja/fazy animacji nie są identyczne:
70096/71680 pikseli RGB555 zgadza się w tych dwóch ujęciach. To porównanie
różnych momentów sceny, nie dowód pixel perfect. Kod gry pozostał bez zmian;
istniejący fixture rozszerzono o `--native-backstage` i regresję stanu/VRAM.
Nowy 43. test `original-game-backstage-transition` przeszedł z wymaganym Disc 1
w 144,71 s. Dalsze podejście do otworu i próba zamknięcia dialogu ujawniają
niepodpięty opkod 67 (M009, Aya 0:0, script_byte 1072). Oryginał po tym dialogu
przechodzi do M011; port nie ma jeszcze dowodu tego przejścia.

Opkod 67 podpięto do istniejącego Script_GetMenuChoice bez zmiany implementacji
ani przenoszenia funkcji. Wszystkie 256 wartości bajtu g_MenuEquipSlotIndex,
w tym signed -1 przy anulowaniu, odpowiadają rzeczywistemu CPU USA 80017DE4
z wykonanym oryginalnym getterem (`ffdfd872a5725525`). Istniejący fixture
dialogów sprawdza także pełny zapis do zmiennej skryptowej przez dispatcher,
zakończenie instrukcji i niezmieniony wynik wyboru. Test --assets przeszedł
w 12,39 s; świeży fixture ASan/UBSan/function z instrumentowanym PsyZ/SDL
także przeszedł. Ponowne naturalne przejście do dialogu M009 jest w toku;
samo podpięcie nie dowodzi jeszcze wykonania M011.

Ponowne przejście wykonało wybór i weszło do M011 (`0xA80010C8`), gdzie
zatrzymał je opkod 93 (aktor 2:0, script_byte 260). Podpięto istniejący
Script_SetCurrentEntityField20: jest to zapis pełnej 32-bitowej skali ruchu,
bez zmiany funkcji. 72 przypadki CPU USA 800182E0 dają `3bf7cb6b9837996d`;
fixture sprawdza cały stan aktora oraz bezpośredni i skryptowy zapis.
Test --assets przeszedł w 12,14 s; świeży ASan/UBSan/function z instrumentowanym
PsyZ/SDL także przeszedł. Dalszy przebieg wykonawczy jest w toku.
Odniesienie PS1 przeszło już M011 do korytarza M012 (`0xA8001148`), a następnie
samym ruchem uruchomiło film przemiany szczura. Nie jest to jeszcze dowód
tych zdarzeń w porcie.

Po zgłoszeniu logu procesu uruchomionego 15 października o 22:32 ponowiono
test spotkania z Melissą na bieżącym executable: `original-game-melissa-encounter`
przeszedł w 125,93 s, sprawdzając stan po spotkaniu i pełną VRAM. Opkody
82 i 206 są podpięte. BIN z używanego przez użytkownika katalogu
`parasite-pc-backup-20260927/disc` jest bajtowo zgodny z obrazem testowym:
SHA-256 `7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4`.
Zmieniony executable wymaga ponownego uruchomienia procesu.

Dalszy przebieg po podpięciu 93 opuszcza M011, lecz loader M012 zwraca -2:
brakuje natywnego deskryptora slotu 115, USA 8018FCB4. Odczyt oryginalnego
banku wskazuje kontroler 8018F20C, initializer 8018FC54 i skrypt 8018FC78;
odpowiadający wspólny moduł orbit_flare.c i script_args.c istnieje, ale nie
jest jeszcze wpięty do natywnego celu. Nie pominięto deskryptora i nie uznano
M012 za działające. To bieżący punkt dalszego portowania.

Następnie cały istniejący orbit_flare.c i script_args.c włączono do natywnego
celu. Funkcje pozostały na miejscu: oba callbacki mają natywne ABI, odwołania
do aktora/zdarzenia/kontekstu/listy używają kanonicznych pól, a współrzędne
16.16 są składane z unsigned shift. Init zwraca pełny wskaźnik hosta do
oryginalnego trzywyrazowego rekordu, zachowując 32-bitowe argumenty.
Deskryptor 115 jest wiązany dla M012/M013/M027/M030 z kontrolą adresu USA 8018FCB4;
rekord jest zerowany przy ponownym załadowaniu. Nie dodano plików ani
zastępczego efektu. Pozostałe kopie tego modułu w innych lokacjach wymagają
jeszcze walidacji i podpięcia swoich deskryptorów.

Fixture porównuje cztery oryginalne deskryptory, osiem callbacków/rozmiarów,
initializer i wszystkie 30 półsłów skryptu z rzeczywistym PE.IMG.
Cały wspólny zakres 3332 bajtów kodu/danych w tych czterech bankach jest
identyczny (SHA-256 `883c0511f4de689bb0b14b05548026d2da4ae292d52d12dd2d1054646712e13e`);
fixture sprawdza go przy każdym odczycie, zamiast zgadywać zgodność po samym
adresie callbacka. Inne kopie mogą mieć inne adresy i nadal wymagają audytu.
36 stanów CPU inicjalizatora daje `3c0af86f8810cdb5`, 216 stanów ruchu cząstek
`f3429d82d5c5a69d` (BIOS rand jest kontrolowaną granicą), a 8640 stanów
lotu/wygaszania kontrolera `a1ad64689555871d`. Ostatni zbiór wykonuje rzeczywiste
rsin/rcos, test wielokąta i odległości; obejmuje trafienie gracza, wyjście poza
podłogę, wrap licznika, dodatnie/ujemne promienie i sygnały. Nie obejmuje
porównania rysowania z rasteryzacją PS1. Osobny natywny test 160 klatek
uruchamia prawdziwe Scene_LoadRoomAssets/Pm_SendCmd/Start/Exec/Stop, trzy
kontrolery authored script, macierze, cząstki i oba bufory rysowania. Sprawdza
pełne linki OT, granice areny, naturalne wygaszenie i sygnał zakończenia 4.

Naturalny przebieg od New Game z samym inputem osiąga 104000 VBlank,
M012, game_frame 770, move_lock 0, HP 14/45, pozycję Ayi
`[19202048,0,170917888]`, yaw 0. Obraz korytarza obejrzano. FNV całej VRAM
to `8e856f4fc0b74e6d`, SHA-256
`41e4269d9449efe9d415e2db13a528affcda28e99517c80b9e9ad087d6ebe79c`.
Nowy 44. test `original-game-hallway-transition` w istniejącym fixture
przeszedł w 150,96 s. Actor-animation i --assets przeszły razem w 12,23 s
przed rozszerzeniem oracla kontrolera; rozszerzony fixture bez assets także
przeszedł. Po zachowaniu leniwego odczytu ownera w callbacku uruchomiono pełny
zestaw 44 testów oraz świeże sanitizery; ich wyniki są jeszcze w toku.
Film przemiany szczura i walka nadal wymagają przejścia w porcie.
Sanitizer wskazał błąd wyłącznie w nowym odczycie fixture: dodanie pełnego
adresu PS1 do bufora przed odjęciem bazy tworzyło pośredni wskaźnik poza
obiektem. Odejmowanie jest teraz wykonywane na liczbach przed dodaniem offsetu.
Świeży --assets z ASan/UBSan/function i instrumentowanym PsyZ/SDL po tej
poprawce i rozszerzeniu do czterech banków przeszedł bez raportów. Regresja
M012 powtórzona w pełnym zestawie przeszła w 155,35 s; pozostała część
zestawu jest jeszcze w toku. Dalszy input w porcie przeszedł do M013
(`0xA80011C8`), gdzie ujawnił niepodpięty opkod 77 (Aya 0:0, script_byte 440).
Odczyt tokenu i obrazu oryginału PS1 potwierdza, że M013 jest tutaj wariantem
areny pierwszej walki ze szczurem. Wcześniejsze określenie tego przejścia jako
bocznego pokoju było niepotwierdzoną interpretacją numeru mapy. Sam abort nie
dowodzi poprawnego filmu ani przebiegu walki w porcie.

Opkod 77 podpięto do istniejącego Script_SetEntityField1C bez zmiany funkcji.
72 stany CPU USA 80017EA4 dają `3bf7cb6b9837996d`. Rozszerzony istniejący
fixture ruchu sprawdza oba settery 77/93, pełny stan aktora i dispatcher.
Świeży izolowany fixture --assets z ASan/UBSan/function i instrumentowanym
PsyZ/SDL przeszedł. Izolowany executable z nowym podpięciem jest w trakcie
przejścia do M013. Główny build pozostawiono bez tej zmiany do zakończenia
trwającego zestawu 44 testów poprzedniego kandydata.

Przebieg po podpięciu 77 dotarł do opkodu 202 (M013, aktor 2:0,
script_byte 32). Podpięto istniejący Script_SetEntityField1E6: zapisuje
16-bitowy indeks tabeli renderera przez natywny RenderObjectEntity.table_index.
12 stanów CPU USA 80019298 daje `7b1afe75d03b90c1`; fixture kontroluje
zawężenie, sąsiednie półsłowo, cały rekord aktora i dispatcher. Świeży
izolowany --assets z ASan/UBSan/function i instrumentowanym PsyZ/SDL przeszedł
z oboma podpięciami 77/202. Dalszy executable dotarł do opkodu 219
(M013, aktor 2:0, script_byte 6020), Render_CameraTrackEntityAngle. Był to kolejny
brak dispatchera; pierwsza zwykła walka nie miała wtedy potwierdzenia.

Pełny zestaw poprzedniego kandydata zakończył się wynikiem 41/44: trzy testy
miały timeout (bootstrap, Melissa, --assets), bez innych błędów. Log pmset
potwierdza uśpienie 16 października 01:03:26, Thermal Emergency Sleep
01:04:01–01:12:29 i dalszy Maintenance Sleep do 01:22:15. Wyników timeout
nie zaliczono jako PASS. Powtórka jest sekwencyjna, z caffeinate ograniczonym
do procesu CTest (bez zmian globalnej konfiguracji lub zabezpieczeń termicznych).
Bootstrap przeszedł ponownie w 142,71 s, Melissa w 125,05 s, a --assets
w 11,75 s. Wszystkie 44 testy poprzedniego kandydata mają więc wyniki PASS
po powtórce trzech timeoutów; nie był to jeden nieprzerwany zielony przebieg.
Następnie główny executable przebudowano z podpięciami 77/202. Dwa powiązane
testy actor-animation i --assets przeszły na aktualnym buildzie w 12,05 s
(odpowiednio 0,47 s i 11,58 s). Pełnego zestawu nie powtarzano dla tych dwóch
podpięć; tamten przebieg zatrzymał się na opkodzie 219.

Podpięto następnie istniejący Render_CameraTrackEntityAngle (219), bez
przenoszenia funkcji. W tej samej funkcji różnice współrzędnych, negacja
prędkości, dodawanie kąta i sumy kwadratów jawnie zachowują 32-bitowy wrap
oryginału zamiast polegać na signed overflow C. Usunięto nieużywany stack_pad.
14400 niehookowanych wykonań CPU USA 80013988 daje `c90373fa22ae5275`:
gracz/zwykły aktor, start/wznowienie, pięć skal ruchu, cztery prędkości,
sześć par pozycji (także granica signed), sześć kątów i pięć kroków obrotu.
Fixture porównuje stan aktora/węzła/kursora, zachowanie pierwotnego celu po
zmianie argumentów oraz pełny dispatcher i retry/dojście. Dwa powiązane CTest
przeszły w 11,89 s (0,51 s i 11,38 s); świeży --assets z ASan/UBSan/function
i instrumentowanym PsyZ/SDL także przeszedł. Naturalny executable do 112000
VBlank zakończył się SIGSEGV; pełny przebieg z ASan/UBSan wskazał
Battle_StepPostBattle: dereferencję core=NULL podczas sprawdzania kind.
Oba analogiczne warunki tego modułu zachowują teraz obsługę aktorów bez core
bez odczytu niskiej pamięci PS1. W referencji PS1 odczyt RAM[5] wynosi
0x0c (kind jest bajtem +5), czyli nie 1. Ponowny naturalny przebieg
od New Game do 112000 VBlank zakończył się kodem 0, bez aborta/segfaulta.
Końcowy token A9400048, aktorów 0, combat.mode=-1; przebieg bez aktywnej
obrony Ayi dochodzi do śmierci/powrotu do ekranu początkowego, nie dowodzi
wygrania walki ze szczurem. VRAM SHA256
`69af40313c3c61e02abf60ae4949ed06be0567b315354616861e6767b92382ce`.

W debuggerze rzeczywiste Save_BuildHeader/Save_BuildCardFile utworzyły zapis
M012, 8192 bajty, SHA256
`184b9bde8c3bcfbaa447612d8e204b227849b5220204ef557a1a1160aa880e91`,
wyłącznie w katalogu tymczasowym. Continue ujawnił niezależną dereferencję
NULL w Menu_IsMemCardSlotCompatible: znaleziony slot ma metadataReady=0,
więc GetSlot celowo nie zwraca jeszcze rekordu. W istniejącej funkcji taki
slot jest tymczasowo niedostępny. 5400 wykonań rzeczywistego CPU USA z
oryginalnymi getterami (RAM[0]=3) daje `6d2dc156bb7a40e5`; native fixture
porównuje wszystkie wyniki, oba tryby, porty, stany i granice indeksów.
Po poprawce prawdziwy executable przechodzi Continue i osiąga M012 do
30000 VBlank: HP14/45, move_lock=0, brak otwartych dialogów, brak crasha.
To dowód jednego checkpointu, nie pełnego roundtrip wszystkich zapisów.
Oba powiązane CTest po tych poprawkach przeszły w 12,39 s. Świeży
--assets z ASan/UBSan/function i instrumentowanym PsyZ/SDL także przeszedł.

Próba walki ze szczurem z rzeczywistego checkpointu (Continue, wejście do
korytarza, Cross co 32 VBlank) ujawniła kolejny błąd zależny od układu pamięci.
Zwykły executable i pełny ASan runtime zatrzymały się w Draw_AllocColorGradient
na odczycie `0x100000000`; osobny build z symbolami przeszedł ten sam input.
To nie błąd rasteryzatora: MenuWidget_CreateSimpleNode zerował 32-bitowy
appearance.cursorTargetX w unii z 64-bitowym gradientPoints. Ponowne użycie
węzła pozostawiało górne 32 bity poprzedniego wskaźnika. Oryginalny konstruktor
zeruje teraz właściwe gradientPoints, bez zmiany API/modułu. Regresja wszystkich
65 deskryptorów wymusza ponowne użycie węzła z poprzednim natywnym adresem:
przed poprawką FAIL (`Reused simple panel 0 ... 0x100000000`), po poprawce
PASS. Dwa powiązane CTest przeszły w 12,93 s. Pełny sanitizer przebiegu bez
atakowania (34000 VBlank z checkpointu, śmierć Ayi) przeszedł po poprzednich
poprawkach core=NULL; nie dowodzi wygrania walki. Po poprawce konstruktora
zwykły executable i świeży pełny ASan/UBSan/function runtime z instrumentowanym
PsyZ/SDL przechodzą Continue i wygrywają walkę przy tym samym inputcie.
Oba do 32000 VBlank: scena M012, mode9 (zakończony level-up/oddanie sterowania
skryptowi), enemy0, HP11/45, move_lock0, Aya[-3407872,0,-95289344], yaw0.
Cała VRAM jest identyczna w obu buildach, SHA256
`81d469c604d966ad55aa47e0b5ac82660d860234624905c01e1152042bcec91e`.
Obejrzano obraz i sprawdzono dalszy ruch: po 1200 VBlank Up Aya osiąga
[-3407872,0,-291897344], nadal M012, HP11/45. Nie jest to jeszcze dowód
następnej lokacji ani zgodności tej walki z referencją PS1.
Naturalna trasa od New Game do 112000 VBlank również kończy zwycięską walkę:
M012, mode9, enemy0, HP11/45, Aya[-3407872,0,-95289344], yaw0, game_frame1437,
move_lock0, brak menu i otwartych dialogów. VRAM SHA256
`c34f11bf7344480a068e9c180d01676922138c5778a88a211e5c900f6517bbca`.
Nowy CTest original-game-rat-encounter powtarza tę trasę i sprawdza końcową
scenę, tryb, przeciwników, żywego gracza i odblokowany stan. CTest przeszedł
w 167,22 s. Pełnych 45 testów po tej zmianie nie powtarzano. Test korzysta z istniejącego fixture/executable i wyłącznie inputu;
nie dodano plików ani fixture zapisów do repozytorium. Dalszy input z
checkpointu wywołuje dialog117 przy końcowych drzwiach. Podejście do bocznych
drzwi przechodzi do tokenu A8002048, gdzie zatrzymuje się na niepodpiętym
opkodzie62 (aktor2:0, script_byte72). Następna lokacja nie jest jeszcze zaliczona.

W scenie A8002048 opkod62 odpowiada CPU USA 80017D3C,
Script_SetEntityField224 w istniejącym system_opcodes.c. Podpięto go do
dispatchera; sygnatura jest teraz int** zamiast short**, a promień cylindra
render_object.hit_cylinder.radius pochodzi jawnie z signed górnego półsłowa
argumentu. Nie zmieniono funkcji ani semantyki: oryginał wykonuje lh +2,
sh +224. 65536 rzeczywistych wykonań CPU (wszystkie górne półsłowa,
low=5aa5, sąsiednie półsłowo=a55a) daje `18f5d8e8953d7b25`.
Istniejący fixture porównuje return/radius/sąsiada, cały natywny rekord
aktora i 12 programów dispatchera. Oba powiązane CTest przeszły w 12,39 s.
Powtórka z rzeczywistego pre-rat checkpointu, wejście przez boczne drzwi,
kończy 42000 VBlank w A8002048: HP11/45, mode9, enemy0, move_lock0, brak menu
oraz otwartych dialogów, Aya[28639232,0,-1245184], yaw1024. Obejrzano render
zagraconej garderoby z postaciami. Cała VRAM SHA256
`032edcda79b08c60bc0f54ffbdc48e09ea7a72c55187f28216eb244303bbb163`.
Nie jest to jeszcze dowód całej zawartości tej lokacji. Świeży --assets
z ASan/UBSan/function i instrumentowanym PsyZ/SDL przeszedł. Pełny zestaw
45 CTest pozostaje w toku; executable podczas jego wykonywania nie jest
przebudowywany.

Oryginalne Save_BuildHeader/Save_BuildCardFile utworzyły także rzeczywisty
checkpoint po szczurze (8192 bajty, wyłącznie /tmp), SHA256
`1a909415ef6d6e671b90053f0ec85ea647146a295ddcfd8398e1975299736f1b`.
Continue z tego checkpointu dochodzi do M012, ale dalsza droga zatrzymuje
się na niepodpiętym opkodzie210 (aktor6:0, script_byte1132). Nie uznano tego
checkpointu za w pełni zweryfikowany roundtrip. Odczyt oryginalnej tabeli
wskazuje dla 210 CPU80019DB8, istniejący Script_GetFloorTableValue; pomimo
historycznej nazwy odczytuje GAME_TIME_COUNTER. Dalsze wiązanie wymaga
sprawdzenia zakresów i referencji CPU. Nie dodano projektu edytora
zapisów ani danych zapisu do repozytorium.

Następnie podpięto istniejące Script_GetFloorTableValue (210, CPU80019DB8)
i Boot_CheckModeSwitch (165, CPU800155FC). Pierwszy bez zmiany implementacji
odczytuje current jednego z czterech g_GameTimers. 672 rzeczywiste wykonania
CPU obejmują wszystkie liczniki, 12 wzorców 32-bitowych oraz wynik zapisany
do osobnej zmiennej, argumentu indeksu lub każdego z 12 słów tabeli:
`74d6afc47a7ad4ed`. Fixture porównuje cały stan i 48 programów dispatchera.
Drugi zachowuje oryginalne czekanie na tryb kamery 0/4, cofnięcie kursora o
8 bajtów i active=1; zmieniono wyłącznie sygnaturę na wspólne int** z
nieużywanym argumentem. 864 stany CPU (wszystkie tryby, górne flagi, aktywność
i flagi węzła) dają `8ed4b8d9c2727b75`; fixture sprawdza również rzeczywisty
retry i wznowienie dispatchera. Świeży --assets z ASan/UBSan/function oraz
instrumentowanym PsyZ/SDL przeszedł dla obu podpięć.

Nowe 210/165 są sprawdzane w osobnym executable; głównego buildu nie zmieniono
w trakcie pełnego CTest kandydata 0cb304300. W nim rat-encounter przeszedł
w 167,82 s, hallway-transition w 156,67 s, field-control w 410,08 s; pozostałe
przypadki są jeszcze w toku. Nie jest to ukończony wynik 45/45.

Po 210 Continue dochodzi do niepodpiętego 165. Po obu podpięciach przebieg
z dalszym inputem nie abortuje, lecz kończy śmiercią Ayi/powrotem do title.
Osobne Continue bez dalszego ruchu do 24000 VBlank poprawnie odtwarza M012,
HP11/45, move_lock0, brak menu/dialogów, Aya[20316160,0,264437760], yaw480.
Checkpoint po zwycięstwie utworzono debuggerem w mode9, poza normalnym punktem
zapisu; nie zakładamy, że zapisuje niezakończone skrypty lub ich pozycję.
Zatem sam ponowny przebieg zdarzenia nie stanowi jeszcze dowodu błędu
produkcyjnego Save/Continue. Dalsza walidacja wymaga normalnego punktu zapisu
lub potwierdzenia tej samej semantyki w PS1. Token A8002048 dekoduje się przez
oryginalne Text_EncodeBase32 do M0020I: garderoba to M020, nie zgadywany
numer na podstawie kolejności odwiedzania. Osobny executable z 210/165
przechodzi ten sam pre-rat checkpoint, walkę i drzwi do M020, a następnie
ruch/confirm do 45000 VBlank bez crasha: HP11/45, mode9, enemy0, move_lock0,
Aya[835232,0,-29049184], yaw512, brak menu/dialogów. Nie uruchomił jeszcze
interakcji z postacią. Zmierzony input Down+Left w M020 zmienia oba X/Z na
ujemne. Sam pomiar połączonych kierunków nie ustala obrotu kamery;
wcześniejszy wniosek o 90 stopniach nie był wystarczająco potwierdzony. Trwa kolejne
podejście do aktora2 w [11730944,0,15794176], wyłącznie przez input.

Pełny zamrożony zestaw kandydata 0cb304300 zakończył się 45/45 PASS w
729,44 s, bez SKIP/timeout/błędów (dwa procesy CTest, caffeinate ograniczone do
zadania). Obejmuje pierwszy pełny test New Game -> zwycięstwo nad szczurem.
Dopiero potem główny executable przebudowano z 210/165. Oba powiązane CTest
actor-animation i player-input-assets przeszły na tym buildzie w 12,52 s
(0,58 i 11,94 s). Nie przenosimy wyniku 45/45 na nowego kandydata jako
świeżego pełnego przebiegu.

Osobny headless proces PS1 uruchomiono od boot z tymczasową kartą 128 KiB.
Karta zawiera jeden oryginalny blok zapisu M012 wygenerowany przez port;
format katalogu sprawdzono według MemoryCardImage::Format/WriteFile w
źródłach DuckStation. Dane karty są wyłącznie w /tmp i w niepersistowanej
karcie własnego emulatora, bez używania katalogu bu00 użytkownika.
Przy frame15517 rzeczywisty PS1 pokazuje Select File to Load: slot1,
imię Ayi, Level1, HP14/45, DAY1, Theater, Time00:23:17. PS1 następnie
potwierdza File loaded i przy frame19525 wraca do M012. Odczyt rzeczywistej
pamięci PS1 przez RSP24700 potwierdza HP14/45, pozycję
[19202048,0,258736128], yaw432: identyczne z natywnym Continue dla tego
checkpointu. Obejrzano ekran korytarza. Plik save ma SHA256
`184b9bde8c3bcfbaa447612d8e204b227849b5220204ef557a1a1160aa880e91`,
a tymczasowa karta MCR `9c7baad575a6e0af008c4def4a47f5e690c2f0df5c0c5307b860b74b44eb3052`.
To rzeczywisty odczyt PC -> PS1 jednego zapisu, nie dowód wszystkich stanów,
wersji, odwrotnego kierunku ani pełnej zgodności obrazu/audio.

Dalsza kalibracja M020 odbywa się w tym samym headless executable, zatrzymywanym
w istniejącym diagnostycznym limicie VBlank. Komendy LLDB w /tmp zmieniają
wyłącznie wirtualny pad i następny limit; nie przestawiają aktorów, HP, flag
rozgrywki ani skryptów. Przy 48185 VBlank input Right20, Up90, Right75 po
wcześniejszej trasie uruchamia prawdziwy dialog112, state2, move_lock1,
Aya[5434217,0,11054464], yaw2652. Cross8 zamyka go; po idle160 stan jest
odblokowany. To potwierdza jedno zdarzenie w M020, nie wszystkie interakcje.
Utworzono też wyłącznie tymczasowy blok Save_BuildHeader/Save_BuildCardFile
M020 do dalszych prób; nie jest jeszcze zweryfikowanym punktem zapisu gry.


W M020 dalszy input uruchomił 167 (aktor 7:0, script_byte 1428), potem232
(script_byte 1480). Podpięto istniejące Script_GiveItem (USA 800194B0) i
Script_ShowReceivedItem (USA 80015BAC), bez przenoszenia ani przepisywania
funkcji. 1152 wykonania rzeczywistego CPU z prawdziwym resident packiem i
niehookowanym Inv_AddItem oraz jego pomocnikami dają `c4267725611c01f9`.
Obejmują 24 ID (w tym skrajne), pojemności 0/1/10/50, pełną/pustą listę,
dziurę, zajęte 128 rekordów ekwipunku, amunicję blisko999, listę zastępczą
oraz alias wejścia/wyjścia. Porównywane są return/result, oba konteksty listy,
całe PlayerInventoryState, InventoryItemWorkspace i bity zaznaczeń.
Native VM osobno sprawdza 12 programów dla Medicine1: brak miejsca/puste sloty
oraz alias tej samej zmiennej skryptu. Sanitizer ujawnił signed overflow w
Inv_AddItem dla INT_MIN-1; w tej samej oryginalnej funkcji indeks przekazany
do Item_LookupBaseData powstaje teraz przez unsigned odejmowanie, zgodne
z wrap CPU. Golden nie zmienił się i świeży stat-assets ASan/UBSan/function
przeszedł.

Dla232 5760 rzeczywistych wykonań CPU z niehookowanymi Render_BeginSceneLoad,
Menu_CreateItemUsePanel i konstruktorami widgetów daje `a8b80c9059a27965`:
cztery prawdziwe rodzaje przedmiotów, flagi gry/globalne/kamery i fazy węzła.
Fixture porównuje sterowanie (return/flags/active/cursor), sprawdza rzeczywiste
widgety w natywnym ABI i pełny dispatcher yield/retry/open. Świeży --assets
ASan/UBSan/function z instrumentowanym PsyZ/SDL również przeszedł. Trzy
powiązane CTest (actor-animation, player-input-assets, stat-assets) przeszły
na głównym buildzie w 12,86 s. Diagnostyczny raport istniejącego Boot_PollHost
zawiera teraz bazową pojemność i wszystkie 50 slotów ekwipunku, wyłącznie
odczytywane przy limicie; nie jest to nowa warstwa logiki gry.

Z samego pre-rat checkpointu pełny replay do 49932 VBlank pokazuje Medicine1
w oryginalnym oknie (opis Restores45HP), HP45/45 i sloty
[256,257,258,512,6,0,...]. Lek użyty wcześniej podniósł HP11->45 i zmniejszył
liczbę przedmiotów5->4; odbiór z obiektu przywraca5. VRAM SHA256
`604b82ed2acb26fb1dbc91685f5d6ba105ca2e40610274a48a356f96e6dbfcb1`.
Dalszy Cross50000/hold8 do 50200 zamyka okno: menuNULL, flags0, move_lock0,
HP45 i przedmiot zachowany, bez crasha.

Nowy 46. CTest original-game-m020-items w istniejącym fixture wykonuje wszystko
od New Game: Melissa, szczur, M020/dialog112, menu/use Medicine1, odbiór
następnego leku. Dwa uruchomienia przeszły w 192,91 s i 192,55 s do 131932 VBlank.
Kończy na otwartym oknie odbioru, sprawdza scenę/pozycję/HP/modalność/sloty.
VRAM FNV `276e21abd46696b5` zgadza się między niezależnym pełnym runnerem
i CTest. Nie jest to pełny świeży wynik46/46 ani referencja pikseli PS1.
Zamknięcie okna jest osobnym wyżej opisanym przebiegiem.

Przyspieszony replay z tymczasowego bloku M020 po Continue wyrównuje wejście
Left40 i odtwarza ten sam dialog, użycie leku, odbiór i zamknięcie do 32240:
HP45/45, menuNULL, flags0, move_lock0, ta sama pozycja[-33929350,0,19598750]
i sloty pięciu przedmiotów. Świeży pełny runtime ASan/UBSan/function z instrumentowanym PsyZ/SDL również
przeszedł: końcowy JSON jest identyczny, cała VRAM SHA256 obu buildów
`1f38362dcbdf80ba66c3ce4255208af0d5f7bdf9da664f0f3a320bc090e79a59`.
Tymczasowy blok M020 ma SHA256
`7cfd69d3d9764f776fa3c3fb4dcec17b601c6caf315926f8c7aa308889668704`.
Fixture nie zastępuje dowodu normalnego punktu zapisu.

W nowej próbie referencji PS1 przyciski Up/Right nie przesuwały postaci:
pad jest w trybie analogowym, co oryginalny Field_HandleStateTransition
obsługuje przez lewą gałkę. Odczyt własnego AIControl::GetAxisIndex potwierdza
LLeft/LRight/LDown/LUp (oraz odpowiedniki prawej gałki). LUp=0.5 przez 20 klatek
przesunął Ayę z [19202048,0,258736128] do[19121248,0,255460128]. Sama wcześniejsza
próba z D-padem nie jest zaliczona jako referencja walki. Pomocnicze drivery
w /tmp przerywają teraz przed advance przy odpowiedzi ERR; porównanie walki
wymaga świeżego, wyrównanego początku i prawidłowego inputu analogowego.

Nie dodano plików ani równoległej implementacji gry. Poniższy opis nieznanego
producenta uszkodzonej listy GPU jest historyczny. Cała kampania, walka,
zapisy/Continue, Disc 2 i pozostałe bramki nadal wymagają ukończenia i dowodów.

## Bieżące naprawy: początek dialogu M004

Podpięto oryginalne opkody 128/129. Zmieniają bit przechodniości 0x80
trójkąta kolizji, a nie flagę sprite'a. Istniejące implementacje zachowują pełne
wskaźniki hosta i oba stride trójkątów (22/28 bajtów). 3072 stanów obu funkcji,
wszystkich wartości kind i trzech sąsiednich rekordów odpowiada rzeczywistym
funkcjom CPU USA (`12d5c65f0d2fc325`); sprawdzany jest też dispatcher i guardy.

Właściwy executable kończy film 3 (ostatnia klatka 1151), wchodzi do M004
(`0xA8000248`) i otwiera dialog 33. Przebieg do 52 000 VBlank zakończył się
poprawnie; obejrzano render sali z Ayą i jej towarzyszem. Regresja sterowania
obejmuje teraz pięć uruchomień od New Game, w tym ten dialog wraz z pozycją
playera i pełną VRAM. Budżet czasu testu zwiększono do 360 s.

Istniejący diagnostyczny `--pad-at` alokuje bufor według argc, zamiast odrzucać
65. zdarzenie. Input nadal przechodzi przez oryginalny pakiet pada. Scenariusz
START w fixture uruchomienia obejmuje 129 zdarzeń. Nie dodano nowych plików,
warstw ani implementacji gry. `bu00/` pozostaje nietknięte.

Dalszy harmonogram CROSS po 52 000 VBlank ujawnia uszkodzenie listy GPU:
GPU_Enqueue czyta nieprawidłową długość pakietu, a awaria jest w Draw_RenderFrame
→ DrawOTagEnv. Sanitizer potwierdza
przekroczenie queue_buf[16384] w PsyZ libgpu.c:200; producent wadliwego linku
pozostaje do ustalenia. Analiza adresu sugeruje link w środku natywnego wpisu OT,
ale ta hipoteza nie jest jeszcze potwierdzona. Ten dłuższy przebieg nie jest
poprawnym przejściem M004 i nie zaliczamy go jako zakończonej rozgrywki.

Pełna weryfikacja kandydata: 39/39 CTest z wymaganym Disc 1, headless, bez skipów
(równoległość -j2); świeże ASan/UBSan/function z instrumentowanym PsyZ/SDL:
--assets, --texture-pages, statystyki oraz właściwy executable do 52 000 VBlank.
Pełny stan diagnostyczny i 1 MiB VRAM M004 są identyczne ze zwykłym programem;
SHA-256 `c4af86e59e672fa12bc6ac4e7cf0b1870533e1c003460e32e8e1a497db3cd01a`.
Osobny build bez testów przechodzi. Poniżej zachowano dowody poprzednich etapów.

## Bieżące naprawy: przejście z holu do M372

Opkody 120/121 używają natywnego ABI dispatchera i oryginalnego kasowania /
ustawiania bitu aktora 0x20; 128 stanów odpowiada CPU USA
(`f3302e2482610125`). Opkody 3/70 zachowują podpisane współrzędne s16 i czas
u16 oryginalnego scrolla; 720 stanów oraz wykonanie przez dispatcher odpowiada
CPU USA (`93501ab9832625cd`). Podpięto istniejący 130 do oryginalnego loadera
geometrii kamery. Oryginalny zapis inputu wychodzi z M003, uruchamia M372
(`0xA8067148`) i film nr 3. Ograniczony przebieg 42 000 VBlank osiąga klatkę
257 tego filmu; zrzut RGB24 został także obejrzany. Dłuższa próba kończy film
i dochodzi do M004 (`0xA8000248`), gdzie pozostaje niepodpięty opkod 128,
aktor 1:0, bajt skryptu 132. Nie zaliczamy tego jako ukończonej eksploracji.

Debugger wskazał potem awarię w Scene_FreeEntityTable przy usuwaniu aktora
przed pierwszą inicjalizacją banku procesów efektów. Oryginalny moduł pomija
nieobecne natywne banki, zachowując zatrzymywanie zadań w obecnych bankach.
Regresja obejmuje brak obu banków, każdą kombinację niezależnej obecności,
zachowanie zadań innego właściciela i rzeczywiste usunięcie aktora/playera.
Pięć testów jednostkowych modułów przeszło. Świeże ASan/UBSan/function
(z instrumentowanym PsyZ/SDL) przechodzą --assets, --texture-pages, statystyki,
regresję lifecycle i właściwy executable do 42 000 VBlank. Pełny stan JSON oraz
1 MiB VRAM są identyczne ze zwykłym przebiegiem; SHA-256 VRAM:
`210df09ec7c48ebfae3fa39a13785f7a58efcde356062765b8708525f633f03f`.
Rozszerzony istniejący test sterowania obejmuje również wejście do M372 i
257. klatkę filmu 3. Pełna ponowna weryfikacja tego kodu gry przeszła:
39/39 CTest z wymaganym Disc 1, headless, bez skipów; test sterowania obejmuje
cztery rzeczywiste uruchomienia od New Game (bezruch, ruch, M003, M372/film 3).
Osobny build bez testów przechodzi. Bez nowych plików repozytorium,
wydzielania funkcji ani równoległego silnika. `bu00/` pozostaje nietknięte.

Poniżej zachowano wyniki poprzedniego kandydata i historię prac.

## Bieżący kandydat: oryginalne dialogi, imię, ruch i wejście do M003

Oryginalny New Game przechodzi M431/film 2 → M010/dialog → ekran imienia →
M002 → M003 (hol opery). Poprzednie wpisy mylnie nazywały token `0xA8001048` mapą M001;
oryginalny parser odczytuje go jako M010. Token M002 to `0xA8000148`.
Naprawiono istniejące opkody 13/34/35, 101/102, 119 i podpięto oryginalny 239.
Nie dodano plików ani nie wydzielono funkcji produkcyjnych.

Porównania z CPU USA obejmują 432 stany otwarcia dialogu, 576 wait/close,
432 prędkości, 96 resetów ruchu i 486 przypadków wielokąta. Przywrócono
rzeczywiste stałe AKAO i aliasy 16 fal LFO; 73 728 stanów głosu odpowiada CPU.
Dwie oryginalne funkcje zwalniania głosu używają teraz natywnych rekordów
zamiast stride PS1, który nadpisywał wskaźniki programu; 1296 przypadków
porównuje wynik i zachowanie pozostałych pól. Siatka imienia maskuje przesunięcie
bitowe jak oryginalna instrukcja MIPS.

Krótki przebieg M002, zwykły i ASan/UBSan/function, przeszedł po poprawce
aliasu banku menu: 40 VBlank prawego inputu daje 20 klatek symulacji,
zmianę X o 5 241 600 jednostek 16.16 i yaw 3072. Pełny zestaw przeszedł
39/39 bez skipów z wymaganym Disc 1. Następnie rozszerzony istniejący test
`original-game-field-control` przeszedł ponownie: bezruch, krótki ruch i wejście
z M002 do M003, z pozycją, yaw i pełną VRAM. Timeout zwiększono proporcjonalnie
do trzech uruchomień. Zmiany po pełnym zestawie dotyczą tego fixture,
dokumentacji oraz timeoutu, bez zmian kodu gry.

Świeże ASan/UBSan/function (również PsyZ/SDL) przechodzą --assets, --texture-pages,
statystyki, dwa przebiegi właściwego executable M002 oraz wejście do M003
(35 500 VBlank). Stan i cały 1 MiB VRAM w M003 są identyczne w programie zwykłym
i instrumentowanym: SHA-256
`ea86b0accc634c6dae419c16a9dd2b615252fb9caa7acf35692d6b6c3d62f6c3`.
Osobny build bez testów i końcowy build z testami również przechodzą.

Dźwięki menu korzystają ze wspólnego pola oryginalnego stanu gry, zamiast
niezależnej pustej tablicy `g_AkaoBgmHandle`. Test z prawdziwym bankiem Disc 1
potwierdza, że cztery callbacki menu kolejkują właściwe komendy AKAO i wskaźniki
sekwencji. To nie jest jeszcze dowód poprawnego końcowego PCM ani całego audio.
JSON diagnostyczny obejmuje aktorów, dialogi, menu i blokadę ruchu.
Dalsza próba chodzenia i interakcji w M003 zatrzymuje się na niepodpiętym
opkodzie 121 (`Entity_SetCurrentFlag20`), aktor 0:0, bajt skryptu 964.
Funkcja istnieje w oryginalnym `system_opcodes.c`; wymaga poprawienia ABI
i podpięcia wraz z weryfikacją. Tego przebiegu nie zaliczamy jako ukończonego.

Cała eksploracja, walki, kampania, zapisy/Continue, Disc 2 i pozostałe bramki
akceptacji nadal są nieukończone. Ograniczenie VBlank kończy przez `_Exit`,
więc nie dowodzi naturalnego shutdown ani braku wycieków. `bu00/` pozostaje
nietknięte. Poniższe wpisy to historia wcześniejszych kandydatów.

## Aktualny stan: oryginalny New Game dochodzi do M001

Właściwy executable wykonuje oryginalny wybór New Game, ładuje M431, odtwarza
film nr 2 i przechodzi do M001 (`0xA8001048`). Kolejne skrypty tworzą aktorów,
konfigurują modele, animację, kamerę, geometrię i fade. Najdalszy bieżący przebieg
zatrzymuje się na niepodpiętym opkodzie 13, aktor 0:0, pozycja skryptu 940 bajtów.
To nadal nie jest działająca eksploracja, walka ani pełna kampania.

Naprawy są w istniejących oryginalnych modułach, bez nowych plików i bez
wydzielania funkcji. Scene_StreamTexturePage nie ucina już wskaźnika tabeli do
32 bitów. Zachowuje oryginalny cache, czyta obraz i waliduje zakres oraz bloki
TIM przed uploadem. Błędy odczytu/poll/range/TIM nie zmieniają cache ani VRAM;
Draw_InitPipeline, Boot_RunFrame i main propagują błąd zamiast startować wadliwą
scenę. Test wykonuje wszystkie 22 rzeczywiste wspólne banki TIM (strony 2..23),
porównuje pełną VRAM, trafienia cache oraz 18 błędów i poprawny retry. Pozostałe
wpisy tabeli nie są automatycznie uznawane za poprawne TIM-y.

Dispatcher używa istniejących funkcji oryginalnych: AKAO 234, zdarzeń 28,
przejścia sceny 49, tworzenia aktorów 8, akcji/animacji 46/78, obrotu ku aktorowi
75, flag 63..66 i 91/92, geometrii 116/117/123, kamery 132, fade 133..136 i
156, renderowania 155, promienia cienia 157, zakresu menu 166, poleceń 237 oraz
czekania 140/143. 141/142/144/145 to autentyczne oryginalne funkcje return 1,
zweryfikowane w EXE; pozostałe niepodpięte pozycje nadal kończą się jawnym błędem.
Poprawiono natywne ABI tablic argumentów i deklaracje funkcji geometrii, bez
adapterów kopiujących wybrane funkcje. Mnożenie promienia cienia i różnica
współrzędnych obrotu zachowują 32-bitowy wrap bez signed overflow w C.

Świeże porównania z rzeczywistymi funkcjami CPU USA: przejście sceny 174 stanów
(8ae1600ce89fd57c), zakres menu 2048 (9ba3598d49695b25), wybrane gałęzie poleceń 237: 96
(0dd78fa3cadbad1b), klatka animacji 2304 (fb78ff1167f295fc), flaga/kamera 432
(c4b5a5fd1165f305), promień cienia 88 (0cb313b7d38c9570), geometria 288
(18383904d1c6e665), fade 192 (0ddf13ae684d5bb5), wait fade 256
(3ae97f7ab9537f25), obrót ku aktorowi 900 (34c38eb49612044f).
Istniejący fixture sprawdza też pełne wskaźniki hosta, dispatcher, cofnięcie
kursora/yield, wykonanie kolejnych ticków i zachowanie sąsiednich rekordów.

Continue nie wywraca się już w MenuWidget_EaseNodePosition: NULL jest sprawdzany
przed odczytem listy. Ograniczony przebieg otwiera ekran kart; faktyczny odczyt
zapisu i powrót do gry pozostają niezweryfikowane. Pełne zrzuty 1 MiB VRAM dla
zwykłego i instrumentowanego przebiegu Continue mają SHA-256
d253cef8c7e5647e2bef6441b2436429e91e8e104b0d88d18c175fc76eac9be2.
Test executable obejmuje teraz cztery scenariusze: start bez inputu, START,
New Game do filmu sceny oraz Continue. JSON zawiera licznik klatek gry i filmu;
błędy niepodpiętych opkodów wskazują scenę, aktora i pozycję skryptu.
Limit VBlank nadal wymusza _Exit, więc nie dowodzi naturalnego shutdown/leak-free.

Weryfikacja tego samego stanu źródeł: build z testami i osobny build bez testów;
38/38 CTest z wymaganym Disc 1, headless, bez skipów. Świeże instrumentowane
ASan/UBSan/function (również PsyZ/SDL) przechodzą --assets, --texture-pages,
statystyki oraz właściwy executable Continue do 14 000 VBlank. Dłuższy New Game
w zwykłym i instrumentowanym executable kończy się znanym abortem na opkodzie
13 w M001, bez wcześniejszego raportu sanitizerów; tego przebiegu nie zaliczamy
jako ukończonej rozgrywki. Żadnych nowych plików repozytorium; bu00/ nietknięte.

Poniższe wpisy opisują wcześniejsze etapy. Wzmianki o istniejącym `pc/`,
nieosiągalnym New Game i starych liczbach testów są historyczne.


## Aktualny stan: usunięcie porzuconego `pc/`

Usunięto cały katalog `pc/`, jego targety CMake i siedem fixture (14 testów CTest) zależnych od
równoległej implementacji. Nie przeniesiono ich kodu do innych plików. Pozostałe
testy wykonują oryginalne moduły; nie deklarujemy zachowania pokrycia usuniętych
testów. Kopię usuniętych plików, wraz z czterema niezacommitowanymi zmianami,
zachowano poza repozytorium w `/tmp/pe-pc-backup-20261015-192011`.
Katalog zapisów `bu00/` pozostał nietknięty. Nie dodano plików do repozytorium.

Weryfikacja po usunięciu: build z BUILD_TESTING=ON i osobny build z OFF
przechodzą; 38/38 pozostałych testów CTest przechodzi headless z wymaganym
Disc 1 (bez skipów), w tym trzy przebiegi właściwego executable i New Game.
Usunięto zależność CI od FFmpeg; git diff --check przechodzi.

Oryginalny New Game w ograniczonym przebiegu headless dochodzi do sceny M431 i
uruchamia film nr 2. Nie jest to jeszcze dowód ukończenia filmu, eksploracji,
sprawnego Continue, pełnego audio ani kampanii. Poniższe wpisy są historycznym
zapisem postępów; wzmianki o istniejącym `pc/` dotyczą stanu sprzed usunięcia.


Plan obejmuje całą grę, nie tylko boot i filmy. Celem jest ukończony natywny projekt C11: pełna kampania, oba dyski, wszystkie lokacje i sceny, eksploracja, walka ATB, PE, przeciwnicy, przedmioty, ekwipunek, menu, audio, zapis/odczyt, Tutorial oraz tryby i zawartość dodatkowa. Każda funkcja i każdy element zawartości mają otrzymać automatyczny test oraz mierzalny warunek odbioru.

„Perfekcyjny” oznacza pełny uzgodniony zakres, poprawne zachowanie, brak znanych defektów i komplet wymaganych dowodów automatycznych. Sam build, procent pokrycia kodu, ekran menu albo liczba przetestowanych plików nie dowodzą ukończenia gry. Nie można udowodnić braku błędów dla wszystkich możliwych kombinacji stanu i sprzętu; dlatego jawnie definiujemy katalog funkcji, niezmienniki, warianty, obsługiwane platformy i testy błędów. Żadna pozycja zakresu nie może zniknąć z raportu przez pominięcie testu.

Byte match, odbudowa EXE PS1, historyczne kompilatory, kolejność instrukcji i adresy kodu nie są wymagane. Oryginał jest wzorcem zachowania i źródłem danych; port ma normalne typy, wskaźniki, moduły i build C. Poprawiamy błędy pamięci i przenośności. Zmiany balansu i tempa pozostają osobnymi, opcjonalnymi funkcjami.

## Obowiązujący kierunek portowania

Katalog `pc/` ma zostać całkowicie usunięty wraz z zależnościami builda. Nie dodajemy nowych plików podczas przywracania gry 1:1; naprawiamy całe oryginalne moduły w ich obecnych plikach. Nie przenosimy implementacji `pc/` pod inną nazwę i nie uznajemy jego frontendu za działający port oryginalnej gry. Istniejące niezacommitowane zmiany użytkownika wymagają zachowania podczas usuwania porzuconej implementacji.

Nie rozwijamy dalej równoległego silnika w `pc/`, nie wydzielamy po kilka funkcji do nowych plików i nie dodajemy nowych warstw abstrakcji. Docelowy program uruchamia oryginalny przepływ gry z poprawionych istniejących modułów; potrzebne zmiany platformowe w PsyZ trafiają do własnego forka. Obecność `pc/` jest pozostałością do usunięcia, a nie docelową architekturą portu.

## Ostatnia zweryfikowana integracja

Główny target parasite-eve-pc buduje teraz src/main/boot/main.c oraz oryginalne moduły; nie zawiera żadnych obiektów źródłowych pc/ ani zależności FFmpeg. Usunięto pc/main.c bez przenoszenia jego implementacji. BUILD_TESTING=OFF pomija cały podkatalog pc/; świeży build w /tmp/pe-original-no-pc przechodzi. Pozostałe źródła pc/ nadal służą części testów i muszą zostać usunięte; to nie jest ukończenie wymaganego usunięcia całego katalogu. Makefile, README, CONTRIBUTING i opis architektury wskazują rzeczywisty program zamiast porzuconego frontendu. Domyślny świeży configure ma BUILD_TESTING=OFF i nie tworzy pc/; make test oraz CI jawnie włączają testy i odrzucają pusty zestaw.

Oryginalny main przyjmuje --disc/--disc2 CUE/BIN i --headless. Oba strumienie pozostają otwarte; role wynikają z oryginalnych markerów USA, a Render_InitDisplayLists wybiera właściwy zamontowany obraz według oryginalnego trybu i ponownie wiąże jego PE.IMG/tabele. Brak Disc 1, błędny obraz i dwie kopie tej samej roli kończą się błędem przed startem systemu. Błędy startu zatrzymują audio i zamykają obrazy. Sprawdzono rzeczywisty Disc 1 oraz odrzucenie jego duplikatu; autentycznego Disc 2 nadal brak, więc wspólny zestaw i przejścia między danymi obu płyt pozostają niezweryfikowane.

W istniejącym boot/init.c dodano opcjonalną diagnostykę hosta aktywowaną przez main. --frame-limit N wymusza zakończenie na określonym VBlank, --dump-vram zapisuje 1 MiB surowej VRAM, a JSON stdout podaje licznik, scene token, oba słowa flag, stan filmu, przyciski i rzeczywisty DISPENV. --pad-at VBLANK:MASK steruje pakietem oryginalnego pada przed niezmienionym mapowaniem przycisków/repeat/combo. Diagnostyka nie tworzy nowej implementacji rozgrywki ani plików źródłowych. Limit używa _Exit, bo może wypaść wewnątrz zablokowanego callbacku SDK: jest wymuszoną granicą testu, nie naturalnym zakończeniem gry ani dowodem sprzątania/leak-free całej aplikacji.

Pierwszy start oryginalnego programu wykazał awarię Draw_PresentFrame w menu: input_queue.c indeksował natywną OT krokiem u32 i przechowywał wskaźnik prezentowanego obrazu jako int. Cały moduł pozostaje na miejscu; baza OT i rekordy buforów mają teraz RenderNativeTag*, a obraz void*. Test sprawdza oba wybory bufora, pełne 8192 linki OT i pełny adres obrazu. Oryginalny list_screen.c przywraca po filmie własne 240-liniowe strony RGB24; kopiowanie przywróconych stron gry RGB15/224 powodowało widoczne uszkodzenie kolorów menu. Zrzut prawdziwego programu pokazuje poprawne logo i Press Start Button z oryginalnych zasobów.

Nowe original-game-bootstrap-cli/arguments/bootstrap uruchamiają właściwy executable. Dwa przebiegi z CUE kończą się na 8000/13000 VBlank; pierwszy kontroluje całą VRAM (FNV 6c2aef67a1f42efb), drugi podaje START od 10000, sprawdza oryginalny bit pada 4, zatrzymanie filmu i RGB24 menu. Zwykły program i w pełni instrumentowany ASan/UBSan/function dają identyczne zrzuty: SHA-256 6c5dfde98aa573113074160dc6d917bbb401f7f6ed04f603d258b3843ba42cb1 przy 8000 oraz 00d2c7c7bf56fb6f8b8575c2ac0934c2860406af3976c9b3f0d13737fa8c4242 przy 13000. To regresja deterministycznego portu, nie porównanie z obrazem PS1. Pełny zestaw ma 51/51 PASS z wymaganym Disc 1, bez SKIP. Sanitizery wykonują oba starty oryginalnego main; regresje oryginalnych zasobów/statystyk są sprawdzane osobno. Sześć ujemnych prób CLI odrzuca brak obrazu, duplikat roli, błędny/brakujący limit, maskę/kolejność wejścia i capture bez limitu.

Nie zweryfikowano audio całego uruchomienia main; osobne testy AKAO/XA nie dowodzą poprawnego dźwięku intro/menu. Nie ma jeszcze dowodu wejścia przez New Game do gry: próbny harmonogram kolejnych przycisków pozostał w przepływie title/intro. Kontroler opcji title, właściwa scena startowa, trwałe zapisy, cała kampania, drugi obraz, pozostałe platformy i pełne usunięcie pc/ nadal wymagają pracy. Cztery niezacommitowane pliki użytkownika w pc/ oraz bu00/ zachowano.

### Wcześniejsza integracja boot_display

Cały oryginalny boot_display, w tym playback.c i playback_stop.c, kompiluje się na miejscu. Zachowano wszystkie istniejące funkcje. Odtwarzanie korzysta z istniejącego callbacku odczytu obrazu w PsyZ i jego dekodera XA, z ograniczeniem do zakresu pliku. Usunięto pętle kolejki napędu, sprawdzanie klapki, DS callbacks i exit(1) z obsługi transferu. Oryginalny ring sektorów i liczniki nadal mają właściciela w tym module. Blokada audio chroni stan odtwarzania; w istniejącym read_image para seek/read ma blokadę FILE, ponieważ wątek audio i główny korzystają z tego samego zamontowanego strumienia.

RunDisplay zwraca błąd inicjalizacji, uruchomienia, renderowania albo sprzątania i nie nadpisuje błędu dekodera/areny udanym zakończeniem. Kończy odtwarzanie i przywraca Boot_VsyncCallback przed shutdown. Oryginalny main wywołuje tę funkcję zamiast func_801235DC i sprawdza jej wynik. Próba linkowania main bez żadnych obiektów źródłowych pc/ przechodzi. Nie jest to jeszcze produkcyjny bootstrap: main nadal nie przyjmuje ścieżek CUE/BIN, a normalny target używa porzuconego frontendu pc/.

Własny fork PsyZ dffa767 obsługuje callback obrazu także dla XA bez zamontowanego osobno CUE. Po zakończeniu wybranego kanału nadal czyta obraz z ciszą do EOF, uwzględnia czas odrzuconych sektorów i nie dekoduje przypadkowego późniejszego klipu. Zmiana prędkości przy zaległej ciszy nie powoduje ujemnego licznika ani nieskończonego strumienia. GetlocL zachowuje pozycję po EOF i nie nadpisuje wyniku BCD bajtami statusu podczas CdControlB/CdSync. Usunięto też niezdefiniowane przesunięcie ujemnych próbek ADPCM. Testy CD/XA pod ASan/UBSan/function: 20 PASS; te same nowe testy z poprzednim kodem dają 18 PASS/2 FAIL.

Nowy tryb --credits-playback w istniejącym fixture uruchamia rzeczywisty RunDisplay we wszystkich trzech konfiguracjach: RGB24 (13 608 klatek, 31 obrazów), tekst sprite (13 608 klatek), tekst i obrazy sprite (13 737 klatek, wszystkie 30 obrazów). Łącznie 40 953 klatki bez okna, z oryginalnym dekodowaniem, tekstem, callbackami, prezentacją, zakończeniem, shutdown i ponownym wejściem. W obu konfiguracjach sprite sterownik testowy pobiera po 735 klatek PCM na VBlank i podaje START dopiero po zakończeniu audio. Po 30 próbek całej VRAM wykazuje odpowiednio 263 822 i 1 109 762 widoczne piksele w obszarach ekranów; nie jest to porównanie obrazu z PS1.

Na dostępnym Disc 1 nie ma XASTREAM/CREDITS.XA. Test transportu używa autentycznego FMV1/FMV000.STR na kanale 1: 88 200 klatek stereo jest identycznych z odczytem tego samego materiału przez CUE w PsyZ; oba pełne przebiegi sprite konsumują po 439 830 klatek PCM do granicy pliku. Także w wariancie tekstowym fixture jawnie wybiera kanał 1 tego filmu zamiast oryginalnego kanału 0 napisów. Nie zastępuje to dowodu właściwej muzyki końcowej ani niezależnej zgodności dekodowania/timingu z hardware. Sprawdzono cztery błędne starty, EOF, błąd odczytu, brak pliku, błędny workspace i błąd bufora w trakcie całego RunDisplay, z odłączeniem callbacku po zakończeniu.

Weryfikacja bieżącego kandydata: pełny build i 48/48 original-game-* z wymaganym Disc 1, headless; ASan/UBSan/function z instrumentowanym PsyZ/SDL dla całego --credits-playback i regresji --credits-sprites, --credits-images, --credits-text, --assets oraz statystyk. Bez nowych plików repozytorium i bez wydzielania funkcji produkcyjnych. Pełna kampania, bootstrap oryginalnego programu z obrazów, docelowy build bez pc/, właściwe XA napisów i usunięcie całego pc/ pozostają niezakończone. Niezacommitowane zmiany użytkownika w pc/ i katalog bu00/ pozostają nietknięte.

## Historia wcześniejszych etapów

Poniższe wyniki opisują poprzednie etapy; ich ówczesne braki należy czytać wraz z bieżącym stanem powyżej.

Cały istniejący boot_display/sprites.c kompiluje się natywnie: wszystkie osiem funkcji pozostaje na miejscu. Pakiety, OT i wskaźniki mają pełną szerokość hosta; dwa oryginalne obszary po 300 pakietów i dwa OT mieszczą się w istniejącym workspace. Każdy enqueue sprawdza pojemność przed zapisem. Przywrócono obie siedmioelementowe tablice callbacków z oryginalnymi funkcjami oraz callback obrazu przy ponownym wejściu w tryb sprite. Inicjalizacja resetuje stan jak świeżo załadowany overlay. Stany, pozycje, kroki jasności z wrap i kolejność renderowania zachowują oryginalny algorytm.

Tryb --credits-sprites w istniejącym fixture porównuje 33 060 przypadków z wykonaniem kodu USA na CPU MIPS (f412ed31c8597b85): konstruktory, wszystkie znaki/cztery style, fade i stany obrazu. Sprawdza natywne linki OT, nagłówki, canary, 168 atomowych błędów pojemności, oba pełne obszary po 300 pakietów i mapowanie obu tablic callbacków do danych USA. Wszystkie 31 prawdziwych obrazów przechodzi oryginalny dekoder w 527 krokach i 310 uploadach RGB15. Dla czterech pozycji i obu buforów 248 prezentacji porównuje całą VRAM z osobnymi, niescalonymi komendami rysowania. Obie ścieżki używają tego samego PsyZ, więc nie jest to dowód sprzętowego pixel-perfect.

Własny fork PsyZ 8771bf6 naprawia LP64 MargePrim i odbiór scalonych komend DR_* + primitive: zachowuje natywne słowa poleceń i oznacza spakowany strumień zastrzeżonym GP0 NOP w porzuconym wewnętrznym nagłówku. Test obejmuje jedno-/dwusłowowy tryb, DrawPrim/OT i kopie pakietów; poprzedni kod odtwarza odczyt poza buforem pod ASan, poprawka przechodzi. Szersze GPU headless: 8 PASS/31 FAIL po poprawce wobec 7 PASS/31 FAIL przed nią, identyczny zbiór błędów obrazów (backend raportuje GPU device not initialized). Nie uznajemy całego zestawu SDK za zielony.

Weryfikacja: pełny build i 47/47 original-game-* z wymaganym Disc 1; świeży ASan/UBSan/function z instrumentowanym PsyZ/SDL dla --credits-sprites, --credits-images, --credits-text, --assets, --fx-effects i statystyk. Nie dodano plików repozytorium ani nie wydzielono funkcji produkcyjnych. Cały RunDisplay oraz XA nadal wymagają integracji; próba oryginalnego main wciąż wskazuje func_801235DC. Pełna kampania i usunięcie całego pc/ pozostają niezakończone.

Cały istniejący boot_display/image_display.c jest kompilowany natywnie, ze wszystkimi siedmioma funkcjami zachowanymi na miejscu. Font pixels/CLUT i RECT mają pełne natywne typy wskaźników zamiast s32; oba rekordy ekranów mają właściciela i oryginalne współrzędne VRAM. Usunięto sztuczne temporaries związane z układem rejestrów i odczyt nagłówka przez niepowiązany symbol SDK. ABI callbacków text/transition/glyph uwzględnia rzeczywiste argumenty i brak wyniku. Jasność glyph pozostaje tym samym dodawaniem nasycającym z palety; współrzędne wyliczane są jako offsety, więc ujemne Y nie tworzy wskaźnika przed obiektem. Zachowano także oryginalne przejście do sąsiedniego wiersza przy X na prawej krawędzi, gdy adres nadal mieści się w obrazie. Zapisy poza całym buforem są pomijane. Błędne indeksy/bufory, workspace, nadmierny nagłówek RLE i błąd wznowienia VLC ustawiają State/Done=-1; nie jest to pełna walidacja dowolnie uszkodzonego strumienia VLC.

Tryb --credits-images w istniejącym fixture: 1096 porównań z rzeczywistym CPU USA BrightenGlyph/BrightenImageRegion (62c94d8d86727434), wszystkie 256 bajtów znaków, cztery style/poziomy nasycenia z różnymi R/G/B, oba bufory, krawędzie wierszy i clipping Y. Cztery dodatkowe przypadki chronią granice całego bufora. Wszystkie 31 autentycznych obrazów przechodzi oryginalne StepImageDisplay: 899 kroków VLC/MDEC i 310 uploadów RGB24. Test porównuje całą VRAM 1024×512 po stagingu, następnie oba bufory prezentacji, z dekodowaniem całej klatki przez ten sam PsyZ MDEC. Sprawdza kroki timer/index przy przełączeniu obrazu, zakończenie po ostatnim, dziesięć błędnych stanów/workspace oraz ponowną inicjalizację. To dowód zgodności blitu glyph z CPU USA i kompletności stagingu/prezentacji; użycie tego samego MDEC nie dowodzi niezależnej sprzętowej zgodności dekodowania.

Weryfikacja: pełny build i 46/46 original-game-* z wymaganym Disc 1, headless; świeży ASan/UBSan/function z instrumentowanym PsyZ/SDL przeszedł --credits-images oraz regresje --credits-text, --assets i statystyk. Nie dodano plików ani nie wydzielono funkcji produkcyjnych. Cały RunDisplay, sprite mode, XA oraz callback tables wymagają dalszej integracji; pełne wykonanie delegującego CallInitSystem pozostaje do sprawdzenia z tym przepływem. Próba oryginalnego main nadal zgłasza func_801235DC. Kampania i usunięcie całego pc/ pozostają niezakończone.

Całe istniejące boot_display/text_screen.c i playback_file.c są kompilowane w oryginalnym targetcie, ze wszystkimi dotychczasowymi funkcjami zachowanymi na miejscu. Trzy historyczne symbole szerokości liter są widokami jednej 104-bajtowej tablicy 26×4; natywne indeksowanie usuwa sztuczne przesunięcia baz zależne od ASCII. Style 0..3 zachowują dane, w tym zerową czwartą kolumnę; nieprawidłowy styl szerokości zwraca 0. Scroll dodaje z jawnym zawijaniem u32. Istniejący CdRom_LoadBootAudio po wczytaniu banku 71 wiąże 745 linii i 31 obrazów do bufora danych, sprawdzając zakresy, terminatory, kolejność obrazów, magic TIM i ograniczony ciąg ścieżki. Przed próbą ładowania usuwa poprzednie powiązania; błąd metadanych daje -4 bez częściowo opublikowanych tablic. To kontrola metadanych, nie pełny walidator skompresowanych obrazów/TIM-ów.

BootDisplay_InitPlaybackFile korzysta z rzeczywistego katalogu zamontowanego obrazu; usunięto DsInit/status kolejki, shell state i nieskończone retry wyszukania. Zachowano początek/koniec pliku w sektorach, liczniki i ośmiosektorowy ring buffer; -1 oznacza brak obrazu/pliku, -2 błędny zakres/workspace, przed zmianą stanu. Dostarczony Disc 1 nie zawiera katalogu XASTREAM ani CREDITS.XA wskazanego w banku. Test potwierdza ten błąd. Dodatnia ścieżka rozwiązywania zakresu używa istniejącego FMV1/FMV000.STR; nie dowodzi odtwarzania muzyki napisów z Disc 2.

Nowy tryb istniejącego fixture --credits-text porównuje z CPU USA 2560 wyników indeksów/szerokości (6fa8e01cd1156db8) i 5280 układów prawdziwego tekstu przy czterech stylach, pozycjach obejmujących wszystkie linie, granicach clippingu, ułamkach scrolla i pięciu krokach z wrap (8bd818222def553d). Wszystkie 656 800 callbacków glyph mają zgodne współrzędne/znak/styl, wynik zakończenia, końcowy scroll i kursor linii. Callback glyph jest rejestratorem testowym: to dowód układu napisów, nie renderowania ich pikseli. Dziesięć uszkodzeń metadanych kończy się -4 z całymi tablicami NULL; pięć błędów file opener zachowuje poprzedni zakres/bufor. Ponowne ładowanie przywraca poprawne dane. Pełny build i 45/45 original-game-* z wymaganym Disc 1 przeszły; świeży ASan/UBSan/function z instrumentowanym PsyZ/SDL przeszedł --credits-text oraz regresje --assets i statystyk. Natywne callback tables, image_display/sprites, odtwarzanie XA i cały RunDisplay nie są jeszcze uruchomione. Próba oryginalnego main nadal zgłasza func_801235DC; kampania i usunięcie pc/ pozostają niezakończone. Nie dodano plików repozytorium ani nie wydzielono funkcji produkcyjnych.

Cały oryginalny kontroler i pętla fx_common wykonują się teraz natywnie w dotychczasowych plikach. Pozostały stan FX ma rzeczywistych właścicieli w effects.c; InitScene odtwarza wartości początkowe danych overlayu, m.in. PageChanged=1, ScreenFade=-1, DrawTintedCopies=1 i DemoResource=-1, oraz zeruje delty ruchu przy ponownym wejściu. Ruch i blend używają jawnego 32-bitowego zawijania zamiast signed-shift/overflow UB. Wywołania Render_SetupColorTable w StepEffects i SwitchResource przekazują pięć zainicjalizowanych liczb zamiast adresu pojedynczego short, za który funkcja mogła czytać kolejne cztery wartości ze stosu. Błędny wariant/odsyłacz wyboru lub cykl bez dostępnej pozycji kończy kontroler błędem -3 zamiast odczytu poza tablicą lub zawieszenia. Zachowano wszystkie oryginalne funkcje i ich kolejność aktualizacji, także istniejącą asymetrię bankowania dwóch flyerów.

Istniejący fixture --fx-step porównuje pełne CPU StepEffects/StepPathNodes/StepMotionBlend z USA: 21 624 stany w 44 sekwencjach (e9ce596e8e1fa6c1), wszystkie dziesięć wariantów i oba banki modeli, przeglądanie w obu kierunkach, przełączanie widoków, start/anulowanie/powtórzenie playback, wyjścia, całe demo oraz ścieżka presetu 7. Porównuje stan wyboru/playback/resource, oba tory kamery, liczniki, fade/blend, wszystkie pozycje i delty ruchu oraz sześć węzłów ścieżek. Cztery dodatkowe uszkodzone grafy wyboru kończą się kontrolowanym błędem. Referencja wykonuje kontroler/konstruktory/sampling, a wywołania textbox/audio są kontrolowane; ich efektów PCM/pikseli nie obejmuje ten hash.

--fx-frames wykonuje rzeczywisty FxCommon_RunFrameLoop aż do naturalnego zakończenia, bez zastępowania jego renderera ani wymuszania końca poprawnego przebiegu flagą testową. Dwa potwierdzenia SDL przechodzą przez oryginalne pakiety pada i Field_HandleStateTransition: 116 klatek playback/wyjścia w dzień i w nocy. Całe demo trwa po 1216 klatek dla obu banków: łącznie 2664 klatki i w sprawdzonym przebiegu 982 504 podpięte pakiety. TIM-y (3 wspólne i 262 dzienne/nocne) pobiera fixture z rzeczywistych banków i ładuje istniejącym Draw_LoadTimTable; nie jest to jeszcze zintegrowany start przez Boot_LoadOverlayTables/main. Test kontroluje oba bufory, granice bieżącej areny, cały łańcuch OT, brak cykli/błędnych długości i odczytuje obraz obu ekranów z VRAM co 20 klatek; wszystkie 130 próbek po początkowym fade zawierają ponad 10 tys. widocznych pikseli. To wykonanie oryginalnej sceny FX, nie kampanii; widoczność obrazu nie dowodzi zgodności ze sprzętem ani hi-res. Fixture nadal kontroluje żądania przestrzennych SFX, a BGM jest pusty: te przebiegi nie dowodzą dźwięku sceny.

Weryfikacja bieżącego kandydata: pełny build i 44/44 original-game-* z wymaganym Disc 1; świeży ASan/UBSan/function z instrumentowanym PsyZ/SDL przeszedł --fx-step i całe --fx-frames headless oraz regresje --fx-motion, --fx-effects, --fx-models, --fx-display, --fx-camera, --assets i statystyk. Próba linkowania oryginalnego main ma obecnie jeden brakujący symbol: wejście boot_display (func_801235DC). Bootstrap CUE/BIN oryginalnego programu, pełne audio sceny, kampania i usunięcie całego pc/ pozostają niezakończone. Żadnych nowych plików repozytorium, wydzielonych funkcji produkcyjnych ani równoległego silnika.

Pełny istniejący FxCommon_InitScene wykonuje się teraz natywnie z rzeczywistym obrazem gry: 187 węzłów, w tym 132 węzły dziewięciu tras. Właściciele tablic pozostają w effect_init.c, a stan playback/resource w effects.c. Naprawiono również selektory wejścia: oryginalne StartPreset/AutoPlay/AutoVariant/ConfirmExit/ConfirmExitVariant zaczynały od -1, nie od zera. SelectRoomVariant przywraca te wartości przy każdym wejściu. InitScene zeruje własne runtime/setup/motion records i przywraca odpowiednie początkowe pola kamery/playback, zastępując ich dawny reset przez ponowne załadowanie overlayu. Siedem kursorów halfword ze sztucznym stride 26 zastąpiono dostępem do pól istniejącego rekordu setup. Liczba punktów tras ma kontrolę pojemności przed tworzeniem kolejnej pary; nie jest to pełna walidacja dowolnie uszkodzonych offsetów zasobów.

Istniejący fixture aktorów rozszerza --fx-scene: 900 scenariuszy USA, dziesięć wariantów pokoju, oba banki modeli, trzy zestawy flag i wszystkie 15 obszarów wejścia (hash 8a8774df804ba8b5). Porównuje 520 bajtów setup, kolejność wszystkich 187 węzłów, ich typy/LOD/progi/marginesy, offsety zasobów, pozycje/obroty/linki, stan playback, kamerę i dziesięć pozycji ruchu. Dodatkowe asercje obejmują wszystkie tablice wskaźników, w tym przeplatane pary początkowe i współdzielony ogon. Każde wejście zaczyna od celowo zabrudzonych runtime/setup/motion records. Cztery błędy read/poll docierają do InitScene jako -1 przed przebudową grafu; test potwierdza faktyczne liczniki odczytów oraz poprawną inicjalizację po ponownym BindImage. Referencja wykonuje oryginalne CPU InitScene, SelectRoomVariant, konstruktory i sampling; podmienia InitDisplay/InitFadeQuads, odczyty zasobów i konfigurację SDK/textbox, osobno pokryte wcześniejszymi testami. To dowód inicjalizacji stanu, nie pełnej pętli, gameplayu ani niezależnych pikseli PS1.

Weryfikacja: pełny build, 42/42 original-game-* z wymaganym Disc 1, headless; świeży ASan/UBSan/function z instrumentowanym PsyZ/SDL dla --fx-scene, --fx-motion, --fx-effects, --fx-models, --fx-display, --fx-camera, --assets i statystyk. Próba linkowania oryginalnego main ma obecnie 24 brakujące symbole: pozostały stan kontrolera FX i wejście boot_display. Nie dodano plików repozytorium ani nie wydzielono funkcji produkcyjnych. Pełne StepEffects/frame loop, uruchomienie kampanii oraz usunięcie pc/ pozostają niezakończone.

W istniejącym effect_init.c przywrócono dwa dziesięcioelementowe zestawy progów odległości i rzeczywiste macierze światła/koloru używane przez InitScene. Macierze nie są jednostkowe; deklaracje mają teraz właściwy typ GteMatrix zamiast pojedynczego bajtu. Istniejący fixture --fx-motion odczytuje i porównuje wszystkie 104 bajty bezpośrednio z PE.IMG. Te dane leżą za końcem historycznego, 26-sektorowego wycinka skanera kodu, więc sam wycinek nie jest kompletnym źródłem danych sceny. Właścicielem sześciu wskaźników PathNodes jest ten sam oryginalny moduł; PathTailNodes wskazuje ostatnie dwa sloty tej tablicy, zgodnie z nakładającymi się adresami USA, zamiast tworzyć niezależny stan. Test sprawdza oba kierunki aliasowania. Pełny build i 41/41 original-game-* z wymaganym Disc 1 przeszły; świeży ASan/UBSan/function z instrumentowanym PsyZ/SDL przeszedł --fx-motion headless. Próba linkowania oryginalnego main ma teraz 43 brakujące symbole. Pełne InitScene/frame loop, kampania i usunięcie pc/ nadal pozostają niezakończone. Nie dodano plików ani nie przeniesiono funkcji.


Dołączono całe istniejące motion_endpoints.c i camera_path.c (wszystkie trzy oryginalne funkcje zachowane na miejscu). Oba tory target/eye mają rzeczywistych właścicieli wektorów kroku, akumulatorów, końców i liczników; scalar aliases wskazują te same pola zamiast tworzyć oddzielne globals. Usunięto volatile/pins związane z historycznym układem instrukcji. SetMotionEndpoints i StartCameraMove zachowują kolejność odczytów/zapisów przy aliasujących się wejściach, 32-bitowe SUBU i SLLV z maską 31; shift zapisany w stanie nadal zachowuje oryginalny dolny bajt argumentu. Odpowiednie dodawanie i SRAV w istniejącym kontrolerze korzystają z tej samej jawnej arytmetyki 32-bitowej.

Nowy original-game-fx-motion używa prawdziwego banku tekstur z CUE/BIN. Porównania z niehookowanym CPU USA: 1728 konstruktorów z 36 kombinacjami aliasów a/b, czterema pozycjami (również granice s32) i dwunastoma wartościami czasów na obu torach (8ad0fc610fb630f5); 2400 StartCameraMove na rzeczywistych krzywych przy kontrolowanych dziesięciu rekordach setup (f71e670bec43b4dd). W obu porównuje się pełne origin/target/step/accum/shift/remaining, kamerę i path-wrapped. Nie są to jeszcze automatyczne przejścia pełnego kontrolera effects.c.

Wszystkie 79 sąsiednich par z 80 rzeczywistych krzywych przechodzą inicjalizację i cały swój bajtowy licznik do zatrzymania, z pięciu pozycji źródłowych, dodatkowo przez wrap kroku 255→0: 47 836 stanów (c29a21ef98b25f06). Zachowano oryginalne zawężenia indeksu do s16/u8, licznika do u8 i końcowe zatrzymanie na remaining=1. InitCameraPath zwraca 0/-1, StepCameraPath 0 dla postoju, 1 dla kroku, -1 dla błędnego selektora/odrzuconego count. Jedenaście przypadków błędów pozostawia cały obserwowalny stan kamery, liczniki i wrapped bez częściowej zmiany. Nie dowodzi to bezpieczeństwa arbitralnie uszkodzonych offsetów: bazowy sampler nadal nie przyjmuje długości tabeli.

Preset 7 w oryginalnym InitScene wywołuje istniejący InitCameraPath i propaguje błąd -2, zamiast utrzymywać drugą kopię inicjalizacji. Błąd kroku ścieżki trafia do buffer.error=-3; frame loop sprawdza błąd przed aktualizacją/rzutowaniem kamery. Pełnego InitScene/StepEffects/frame loop jeszcze nie uruchomiono. Próba linkowania oryginalnego main ma obecnie 49 pozostałych importów (stan/data FX i boot_display); nie dodano stubów. Wszystkie sześć dotychczasowych plików C fx_common kompiluje się teraz w całości, bez nowych plików lub wydzielania funkcji. Weryfikacja: 41/41 original-game-* z wymaganym Disc 1, świeży ASan/UBSan/function z instrumentowanym PsyZ/SDL dla --fx-motion i wcześniejszych trybów/zasobów/statystyk, headless. Kampania, pełna integracja sceny i usunięcie pc/ pozostają niezakończone.

Całe istniejące effects.c i effect_init.c dołączono do natywnego targetu, zachowując odpowiednio wszystkie 16 i dwie dotychczasowe funkcje w tych samych plikach. Fade, nocna przesłona i markery używają RenderNativeTag, natywnego OT i pełnych wskaźników. Usunięto FxCommonPacketTag z adresem 24-bitowym, unię adres/wskaźnik oraz sztuczny packet cursor z paddingiem. DrawMode ma natywne słowo komendy SDK. Preflight obejmuje cały fade albo wszystkie aktywne markery: błąd -1 nie zostawia częściowych pakietów ani zmienionego OT. Zerowy poziom markerów nadal emituje sam statyczny draw mode. Testy obejmują 92 granice pojemności na LP64.

InitFadeQuads ma własne oryginalne osiem gradientów i sześć draw modes; rekordy setup sceny mają właściciela w effect_init.c. InitScene, blend i markery korzystają z jednej tablicy dziesięciu pozycji 16.16, zamiast oddzielnych obiektów/aliasów X/Y/Z i g_FxCommonMotionWhole. StepMotionBlend zachowuje oryginalną kolejność x/y/z i 32-bitowe ADDU oraz padding. SceneAudioSlot0Handle/Key wskazują kanoniczne keys[0][1]/keys[0][0] w stanie gry (oryginalne USA 800B0DB5/800B0DB4), bez prywatnych kopii stanu audio.

Nowy przypadek original-game-fx-effects w istniejącym fixture: inicjalizacja gradientów/modes (hash 353e0cebf2681c3d) oraz 2118 scenariuszy fade/marker/night i 7028 alokowanych pakietów (e11d5d9a899124c5). Obejmuje krańce s16, oba bufory, różne zestawy włączonych markerów, wybraną paletę, trzy widoki, wszystkie payloady i pełne łańcuchy OT 0/9/10/4095, również statyczne pakiety. 4352 aktualizacje blendu porównują pełne dziesięć rekordów z counterem, paddingiem i nienaruszonymi deltami (fd879abe57c21976). Referencja wykonuje rzeczywiste CPU USA i funkcje SDK, przekazując wyłącznie instrukcje COP2 do PsyZ. Nie jest niezależnym sprzętowym wzorcem GTE/pikseli wszystkich efektów.

Oryginalna pętla nie kompresuje już pustych wpisów OT na potrzeby DMA PS1. Wykonanie bloku USA 801925A0..8019262C na 256 układach pustych/rzadkich/gęstych potwierdza zachowanie kolejności wszystkich właściwych pakietów przed/po kompresji. Natywny ClearOTagR z pozostawionymi pustymi wpisami ma ten sam hash kolejności 8eff7149a4f01905, w obu buforach. Test uwzględnia terminator SDK. Pętla wysyła natywne OT i typowane środowiska obu klatek; pełnej pętli jeszcze nie uruchomiono.

Test pikseli przechodzi przez prawdziwe Boot_InitMemoryLayout → FxCommon_InitDisplay → ClearActiveOrdering → DrawFullScreenFade → SDK DrawOTag. Dla obu klatek i intensywności 0/32/127/255 sprawdza całe 1024×512 VRAM (łącznie 4 194 304 słowa): poprawne odejmowanie od szarego tła oraz nienaruszone piksele poza klatką 320×240. Działa headless, bez okna. Nie obejmuje jeszcze rzeczywistych tekstur etykiet markerów ani całej sceny.

Weryfikacja: 40/40 original-game-* z wymaganymi zasobami Disc 1. Świeży rebuild ASan/UBSan/function z instrumentowanym PsyZ/SDL i przebiegi --fx-effects, --fx-models, --fx-display, --fx-camera, --assets oraz statystyk. Nie dodano plików, nie wydzielono funkcji ani nie dodano zastępczych stubów. FxCommon_RunFrameLoop i InitScene mają teraz rzeczywiste implementacje w linkowaniu. Próba linku oryginalnego main ujawnia 80 dalszych importów: głównie właścicieli/aliasów stanu FX, pozostałe moduły ruchu kamery i func_801235DC z boot_display. Cała inicjalizacja, kontroler efektów, kampania oraz usunięcie całego pc/ pozostają niezakończone.

Cały FxCommon_DrawPolyResource w istniejącym model_buffers.c ma teraz natywne nagłówki pakietów i linki OT. Zachowano wszystkie osiem strumieni oraz czteropakietowy podział pobliskich FT3/FT4, kolejność projekcji i trzy zestawy UV zależne od odległości. Stałe kroki pakietów i wyliczanie adresów przez maski 24-bitowe zastąpiono sizeof/offsetof i pełnymi wskaźnikami, korzystając ze wspólnego istniejącego makra linkowania czterech rendererów. Workspace pozostaje wspólny dla wywołań. Globalny DepthBase ma własność w tym samym module i jest odrębny od DepthBias, tak jak w oryginale (USA 801EA5E0 / 801EA5E4).

Rozszerzono istniejący test --fx-models: oba rzeczywiste zasoby wielokątów z banków dzień/noc oraz zestaw wszystkich ośmiu strumieni, obie orientacje, 14 odległości i offsety głębokości 0/500/1000. 252 wykonania i 6322 pakiety zgadzają się z CPU USA pod Unicorn, z samymi instrukcjami COP2 przekazanymi do PsyZ: hash 8389f407efdeab28. Porównanie obejmuje wszystkie payloady i pełne łańcuchy każdego OT. Każde z sześciu osiągalnych porównań progów 500/501/510/701 wykonało się w obu kierunkach. Redundantna gałąź USA 80198A90 nie jest osiągalna: wejście do 80198A78 wymaga głębokości <501, a delay slot poprzedniego branch ustawia wówczas warunek <510 na prawdę. Nie wymuszano tej gałęzi hookiem CPU.

26 dodatkowych przypadków weryfikuje brak miejsca na pojedynczy pakiet lub komplet czterech pakietów subdivision oraz ujemny/zbyt duży indeks OT. Błędy -1/-2 pozostawiają arenę i OT bez zapisów; GT3/GT4 zachowują swoje oryginalne odrzucenie złej głębokości. Oryginalny niezabezpieczony zapis przed OT przy ujemnym DepthBase nie jest odtwarzany jako poprawne zachowanie. Sanitizer wykrył też signed-shift UB przy pozornym rozszerzaniu znaku współrzędnej środka quada: zastąpiono je zwykłym odczytem s16, z niezmienionym wynikiem USA. Usunięto dopasowujące padding/volatile i pomocnicze wyrównane sloty kursorów. Wszystkie osiem dotychczasowych funkcji model_buffers.c pozostaje w tym samym pliku; żadnych nowych plików ani wydzielonej implementacji. Moduł przechodzi również -Wall/-Wextra bez ostrzeżeń.

Weryfikacja: 39/39 original-game-* z wymaganym Disc 1, rozszerzony test nadal zawiera wcześniejsze 2040 przypadków modeli. Świeży rebuild ASan/UBSan/function z instrumentowanym PsyZ/SDL; --fx-models, --fx-display, --fx-camera, --assets i statystyki. Zgodność pakietów z CPU USA przy wspólnym GTE PsyZ nie dowodzi niezależnej zgodności pikseli ze sprzętem ani pełnej kampanii. Aktualna naprawa shaderów i uproszczenie OT są opisane wyżej. Cała inicjalizacja/pętla effects.c i usunięcie całego pc/ pozostają niezakończone.

Trzy kompletne istniejące funkcje FxCommon_DrawModel/DrawModelTinted/DrawModelFlat naprawiono na miejscu w model_buffers.c, zachowując wszystkie osiem strumieni wielokątów i dotychczasową kolejność operacji. Ich pakiety mają natywne RenderNativeTag, linki OT pełnej szerokości i rzeczywiste sizeof pakietów; wspólny typowany workspace zastępuje fizyczny scratchpad PS1 również w DrawPolyResource. Zachowuje ponowne użycie danych między wywołaniami. Kolor tintu ma rzeczywiste cztery bajty, zgodnie z oryginalnymi kopiowaniami słowa. Prototypy funkcji SDK RotAverageNclip uwzględniają host long.

Oba oryginalne obszary pakietów powiększono proporcjonalnie do maksymalnego wzrostu pakietu F3 (PS1 20 bajtów, native 32 na LP64). Test sprawdza, że oba nadal mieszczą się przed następnym obszarem oryginalnego Boot_InitMemoryLayout. W buforze istnieje granica i błąd: -1 przy braku miejsca na cały pakiet, -2 przy indeksie OT poza 0..4095. Nieprawidłowy indeks nie zapisuje OT; frame loop sprawdza błąd przed wysłaniem klatki do GPU. Pełnego frame loop jeszcze nie uruchomiono.

Nowy test original-game-fx-models w istniejącym fixture odczytuje oba prawdziwe banki z CUE/BIN: wszystkie 85 modeli nocnych i 85 dziennych, trzy renderery, cztery ustawienia GTE/bias. 2040 przypadków i 19 951 pakietów zgadza się z wykonaniem USA MIPS (hash 22489a443c2917c3): wszystkie słowa payloadów, liczby pakietów i pełna kolejność łańcuchów każdego OT. Wszystkie osiem klas wielokątów rzeczywiście emituje pakiety; guards aren/OT oraz niewykorzystane końcówki pozostają nienaruszone. Osobno wymuszono brak miejsca i nieprawidłowe głębokości. Referencja wykonuje CPU i SDK USA bez hookowania funkcji, przekazując wyłącznie instrukcje COP2 do PsyZ. Nie jest to niezależny sprzętowy wzorzec pikseli ani dowód pełnych scen.

Porównanie wykryło błąd PsyZ RotAverageNclip3/4: AVSZ nadpisywał MAC0 przed return, więc zwracano głębokość zamiast NCLIP. Poprawka i test obu funkcji (przód/tył kamery, oba windingi, zachowanie odrzuconych wyjść) są w naszym forku khasinski/parasite-eve-psyz, commit 5ec8289. Wszystkie 2040 przypadków zgadza się dopiero po tej poprawce. Sam test SDK przechodzi; pełna suite SDK ma obecnie 306 PASS / 47 FAIL / 1 SKIP, zatem nie jest zaliczona. W stosunku do wcześniejszego logu 306/46/1 doszedł timingowy błąd events::hw_vblank_trap_event_fires_once_per_vblank; pozostałe 46 nazw błędów jest tych samych. To pozostaje otwartym problemem weryfikacji SDK.

Weryfikacja bieżącego kandydata: 39/39 original-game-* z wymaganymi zasobami Disc 1. Świeży rebuild ASan/UBSan/function z instrumentowanym PsyZ/SDL; --fx-models, --fx-display, --fx-camera, --assets i fixture statystyk przechodzą.

Po opisanych wyżej naprawach shaderów i DrawPolyResource pozostaje pełna inicjalizacja i pętla sceny. Nie usunięto funkcji, nie przeniesiono kodu i nie dodano plików repozytorium. Oryginalny main nadal nie linkuje z powodu FxCommon_RunFrameLoop i func_801235DC; uruchomienie kampanii i usunięcie całego pc/ pozostają niezakończone.

Inicjalizacja displaya w istniejącym fx_common/model_buffers.c używa teraz dwóch rzeczywistych FxCommonFrame z DRAWENV/DISPENV PsyZ. Usunięto stride 0x78, bajtowe offsety środowisk i zapisy przez void**; istniejący SelectBufferAllocation przyjmuje FxCommonBuffer*. Frame records i aktywny wskaźnik mają właściciela w oryginalnym module, a dwa obszary pakietów nadal korzystają z oryginalnej współdzielonej pamięci sceny. ClearActiveOrdering zeruje natywne OT i cofa odpowiedni kursor bez naruszania pakietów. Wywołania PutDrawEnv/PutDispEnv w effects.c dostosowano do typowanych pól; nie uruchomiono jeszcze całej pętli sceny.

Nowy przypadek w istniejącym fixture, original-game-fx-display, sprawdza headless obie klatki i środowiska 320×240, pełne linki/length 4096 wpisów każdego OT (z rzeczywistym terminatorem PsyZ), oba resetowane kursory i guards. Test zapełnia i sprawdza wszystkie 1024×512 pikseli VRAM: init czyści dokładnie dwa obszary 320×240, zachowując pozostałe piksele. Weryfikuje również OFX/OFY/H. Sam test displaya nie dowodzi działania rendererów modeli; aktualna weryfikacja trzech DrawModel jest opisana wyżej, a późniejsza naprawa DrawPolyResource jest opisana na początku sekcji.

Dołączono całe istniejące psyq/libgte/fog_02.c i reg03.c. Fog_02 zachowuje 32-bitową arytmetykę bez signed-shift/overflow UB oraz odrzuca zerowe H bez zmiany rejestrów. 770 przypadków rzeczywistego USA SetFogNearFar potwierdza DQA/DQB i clampy (2b60b43c2716c8c8). Wszystkie 15 funkcji REG03 porównano w 450 scenariuszach, każdorazowo wszystkie 64 odczytywalne rejestry danych/control (3bcbb387746aa626). Referencja wykonuje CPU USA, przekazując wyłącznie COP2 do PsyZ; to test zgodności transferów i obliczeń CPU, nie niezależny sprzętowy wzorzec GTE. Żadnych nowych plików ani wydzielonych funkcji. Weryfikacja bieżącego kandydata: 38/38 original-game-* z wymaganymi zasobami, świeży rebuild ASan/UBSan/function z instrumentowanym PsyZ/SDL i przebiegi aktorów --fx-display, --fx-camera, --assets oraz statystyk. Pełny renderer, entrypoint gry i usunięcie pc/ pozostają wymagane.

Cały istniejący fx_common/camera.c oraz model_buffers.c dołączono do natywnego targetu bez wydzielania funkcji. Zweryfikowana część obejmuje kamerę, płaszczyzny widoku, pulę 200 węzłów, ścieżki i loader zasobów; samo dołączenie model_buffers.c nie oznacza działającego renderera tego overlaya. Jedna natywna struktura węzła zastępuje trzy niezgodne widoki i surowy stride 108; zachowuje nakładanie skalarnych pól zwykłych węzłów i LOD przy ponownym użyciu, a wskaźniki zasobów i list pozostają pełnej szerokości hosta. Zniknął niezainicjalizowany wskaźnik macierzy roboczej. Projekcja i limity audio korzystają z kanonicznego stanu gry.

Świeże porównania z rzeczywistym kodem USA: 105 scenariuszy puli/list/konstruktorów/zwalniania/reuse (hash 405afb1f82d27f95), 21 600 przypadków czterech funkcji cullingu (a642cfccbdbba705), sześć ustawień kamery i wszystkich czterech płaszczyzn (ab55eca183324865; padding MATRIX pomijany). Referencja kamery wykonuje instrukcje CPU z binariów USA, a COP2 przez PsyZ, więc nie jest niezależnym wzorcem sprzętowego GTE. Dwa skrajne ustawienia powodujące wyjątek signed ADD w oryginalnym VectorNormal nie należą do sześciu przypadków.

Loader pobiera rzeczywiste zakresy PE.IMG 76–79 do własnych buforów danych; nie wczytuje kodu. Test z CUE/BIN porównuje wszystkie 83 968 bajtów tekstur i 182 272/204 800 bajtów modeli noc/dzień, również niezmienione końcówki buforów. Cztery błędy read/poll propagują się jako -1..-4, błędne/za duże zakresy i overflow LBA jako -5 przed I/O. Status przechodzi przez istniejące SelectRoomVariant, InitScene, RunFrameLoop do oryginalnego main; całej tej ścieżki inicjalizacji jeszcze nie uruchomiono. Wszystkie 80 krzywych z rzeczywistego banku tekstur porównano w 80 784 próbkach z niehookowanym kodem USA (8fe143d65b03392d). API samplera nadal nie przyjmuje długości tabeli, więc nie dowodzi to bezpieczeństwa dowolnie uszkodzonych offsetów.

Weryfikacja: 37/37 original-game-* z wymaganymi zasobami Disc 1, headless; świeży rebuild fixture aktorów/statystyk z ASan/UBSan/function oraz instrumentowanym PsyZ/SDL, przebiegi --fx-camera, --assets i statystyk zakończone powodzeniem. Nowy przypadek original-game-fx-camera rozszerza istniejący fixture; nie powstały pliki repozytorium. Pełny renderer/frame loop fx_common, inicjalizacja boot_display, uruchamianie oryginalnego main z obrazami i usunięcie całego pc/ pozostają niezakończone. Próba linkowania samego oryginalnego entrypointu nadal wskazuje FxCommon_RunFrameLoop i func_801235DC; nie dodano zastępczych stubów.

Konfiguracja PsyZ/SDL i definicja SDK `__psyz` należą teraz do głównego CMakeLists.txt; SDK buduje się w build/psyz, niezależnie od pc/CMakeLists.txt. To usuwa własność SDK z porzuconego frontendu, ale nie usuwa jeszcze katalogu pc/: istniejący program diagnostyczny i część fixture nadal od niego zależą. Nie przeniesiono implementacji frontendu ani nie dodano plików.

Oryginalny movie opcode 53 (USA dispatcher 800910A0 → Script_LoadSceneData 80014E30) jest teraz przypięty do natywnego Script_RunQueue. Ta sama funkcja w actor_opcodes.c używa standardowego int** ABI i podpisanego dolnego halfword argumentu, istniejącego g_CdLoadAreaBase oraz prawdziwych MenuCardFirst_InitVideoBuffers/OpenVideo/RequestVideoDepthFlip. Usunięto dwa odczyty wykonywalnych banków, nieskończone retry/poll, trzy address-only calls i unię mieszającą s32 z natywnymi wskaźnikami. Cold start odtwarza rozmiar obrazu/flip flag, które dawniej resetował odczyt kodu. Nie powstały pliki ani funkcje produkcyjne. Zniknęły niewykorzystywane g_MdecOverlayLoadBase oraz trzy aliasy dawnych code-bank ranges; oryginalne 81 granic PE.IMG nadal pozostaje i jest sprawdzane jako całość. Fixture kolejki/ruchu/spawn linkują wymagane oryginalne moduły obrazu i AKAO.

Handler zgłasza -1 dla złego argumentu/brakującego lub niewyrównanego workspace/niezamontowanego obrazu, -2 dla zajętego odtwarzacza i -3 dla błędu otwarcia/dekodowania. Po błędzie otwarcia zamyka callback/player, odtwarza flagi, stan audio, mix i display/draw environments. Script_RunQueue raportuje ujemny wynik i przerywa proces, zamiast traktować go jako true i kontynuować grę bez wymaganej sceny. Osobny proces testowy wykonuje rzeczywisty błędny opcode i potwierdza zakończenie SIGABRT; nie jest to gotowy interfejs odzyskiwania błędów produktu.

Test czyta rzeczywisty wpis opcode 53 z EXE i uruchamia pięciosłowowy skrypt przez oryginalny VM. Dwa cold/recovery przebiegi z dostarczonego CUE/BIN dają po 149 klatek (298 łącznie), po 596 VBlank podczas odtwarzania i pełne porównanie wierszy VRAM ze whole-frame MDEC. W odróżnieniu od samego OpenVideo obejmują realne żądanie zmiany głębi: pierwsza czarna klatka RGB15 zachowuje STP=8000, callback przełącza do RGB24, następny frame zamyka flip state 2→0 i reszta filmu działa w RGB24. Drugi skrypt używa argumentu 10000h (ten sam dolny halfword 0). Sprawdzane są brak odczytu banków kodu, granice work arena, odrzucenie NULL/złych argumentów/aktywnego playera, brak pliku i ponowne odtworzenie po błędzie. Ten sam PsyZ MDEC jest używany w obu porównywanych ścieżkach; nie dowodzi to sprzętowych pikseli, XA/PCM ani całej cutsceny ze skryptem lokacji i wznowieniem gameplay. Pełny build, 36 original-game-* z wymaganym Disc 1 i świeże ASan/UBSan/function aktorów/statystyk z instrumentowanymi PsyZ/SDL przeszły. Produkcyjne linkowanie oryginalnego main nadal wskazuje FxCommon_RunFrameLoop i func_801235DC. fx_common, boot_display, pełna kampania, zestaw obu płyt i usunięcie pc/ nadal pozostają niezakończone.

Siedem rzeczywistych programów efektów komend 108–114 jest teraz statycznie powiązanych w istniejących glint_effects.c i image_effects.c. Cały image_effects.c zachowuje wszystkie swoje funkcje na miejscu; callbacki, sześć initializerów zwracających pełny adres i hook komendy 114 mają właściwe natywne ABI. Oryginalne deskryptory i skrypty pozostają danymi C, a Scene_LoadRoomAssets instaluje te same sloty 23..29 bez odczytu dziewięciu sektorów kodu PS1. Usunięto niepotrzebny g_PmTaskProgramBuffer. Cold bind odtwarza zerowe cache timingów kolorów, kolory/origin i obie flagi cross-flash; cached bind ich nie resetuje. Oryginalne zwolnienie 114 nadal usuwa siedem bindings i flagę 10000, a następny bind odtwarza dane. Odczyt pozostałych zasobów/TIM zachowuje swoje błędy I/O; uproszczenie dotyczy wyłącznie statycznego banku kodu. Nie dodano plików, funkcji produkcyjnych, wrapperów ani nowej warstwy runtime.

Integracja wykryła błąd poprzedniego portowania: dawne +8 w PS1 task slot oznacza end/listę za stanem kontrolera, a nie start. Oba całe moduły używają teraz kanonicznego slot.end. Poprzedni test pojedynczych kontrolerów miał osobny bufor i nie dowodził tego umiejscowienia. LinkEffect ma rzeczywiste 17 par historii (280 bajtów), a trail/glint mają oryginalne pełne ośmiobajtowe stany. Arena w istniejącym FieldFxTaskContext ma na 64-bitowym hoście 4856 bajtów zamiast 2428, ponieważ natywne nagłówki/list entries zwiększają jednoczesne zużycie programów 111–113 ponad pierwotną pojemność. Rozmiary slotów PM i alokacji wynikają z rzeczywistych sizeof; pola i algorytmy oryginalnych zadań pozostają te same.

Fixture porównuje siedem deskryptorów USA 801F1BD8..801F1EF0, wszystkie 56 callback slots i rozmiary, sześć initializer bindings/pełnych adresów wyniku, wszystkie 102 halfwords siedmiu skryptów i dane image effects bezpośrednio z dostarczonego PE.IMG. 1350 rzeczywistych przypadków Pulse/Cosine/Sine image CPU (USA 801F0BA0/801F1070/801F15DC) daje hash 920c3087dc472d02; porównuje wszystkie 20 bajtów, status, pięć faz, 18 wartości timera z wrap i pięć trybów. Test native wykonuje każdy program przez Scene_LoadRoomAssets/Pm_SendCmd/Pm_Start/Pm_Exec aż do naturalnego 4→5→free, widzi wszystkie przewidziane task IDs, sprawdza równoczesne zadania i całe wygaszenie cząstek oraz handshake 109+114 przez 44 klatki przed zatrzymaniem. Oba bufory, wszystkie OT links i niewykorzystane końce aren są kontrolowane. SFX używa poprawnego pustego katalogu, a model to mały niepusty fixture: to nie jest dowód PCM, sprzętowych pikseli ani wykonania tych programów przez pełny gameplay/animację Aya. Pełny build, wszystkie 36 original-game-* z wymaganym Disc 1 oraz świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL przeszły headless. Pozostałe resident/room task programs, fx_common, boot_display, kampania i usunięcie pc/ pozostają niezakończone. Próba oryginalnego main bez frontendu/test backing nadal wskazuje dwa brakujące symbole FxCommon_RunFrameLoop i func_801235DC; sam ten wynik nie dowodzi gotowości do uruchomienia.

Cały istniejący `menu_memcard/glint_effects.c` kompiluje się i jest linkowany statycznie, ze wszystkimi ośmioma oryginalnymi funkcjami na miejscu. Cząstki i kontrolery mają rzeczywisty trzyargumentowy ABI callbacków, bez rzutowań niezgodnych funkcji. Odczytują kanoniczne owner.actor, active task.start oraz render_object.target_x/y/z zamiast dawnych padding views. Macierze/offsety używają wspólnych typów GTE, historie typed FieldTrailPair. Stan cross-flash ma 0x288 bajtów jak rzeczywisty deskryptor USA 801F1C78; ostatnia historia wymaga 11 par, a nie dziewięciu. Poprzedni typ nadpisywał 32 bajty poza stanem. Moduł posiada rzeczywistą wspólną flagę g_BattleCrossFlashStop i 252 bajty oryginalnych szablonów/ścieżek kolorów z banku 68. Ścieżki są wyrównane i zawierają tylko własne dane do terminatora, bez sąsiednich rekordów z dawnymi adresami funkcji. Nie dodano plików ani funkcji produkcyjnych.

38 880 przypadków czterech rzeczywistych funkcji USA 801ED858/801EE594/801EF560/801F0068 daje hash 445fa6d24cc84f79. Obejmuje mode 0/1/3, rozpoznane i nieznane stany, 18 wartości timera (także signed wrap), sześć zestawów pozycji/prędkości, trzy wysokości podłogi i dwa RNG seeds. Wykonuje się cały kod CPU; jedynie BIOS rand jest kontrolowany. Porównywane są wszystkie 20 bajtów cząstki, wynik i zużycie RNG. Natywny test wykonuje inicjalizację, update, draw i pełne wygaszenie list wszystkich czterech kontrolerów przez 91 klatek, oba bufory i oba warianty palety; łącznie 560 wywołań drawerów kontrolerów oraz ich cząstki. Sprawdza rozmiary list, ramki zakończenia, zatrzymanie cross-flash, odzyskanie wszystkich cząstek, ochronę areny i wszystkie natywne łańcuchy OT. Używa małego niepustego modelu do ścieżki SpinRing oraz pustego poprawnego katalogu dźwięków, więc nie dowodzi słyszalnych SFX ani zgodności geometrii/pikseli z PS1. W oryginalnym rendererze modelu ostatnie próbne UV przed clippingiem mogą leżeć po committed cursor; test pozwala na jeden niepodpięty pakiet roboczy, ale każdy podpięty pakiet musi mieścić się przed kursorem.

Test ujawnił rzeczywisty odczyt o dwa bajty za sześciobajtową pozycją w dźwięku przestrzennym. Istniejący Akao_Calc3DPan w cdrom/scene_assets.c przekazuje teraz wyrównany ośmiobajtowy GteShortVector i używa s32 zamiast hostowego long. Mnożenie zastępuje signed left-shift dla ujemnej panoramy; kwadrat odległości jawnie zachowuje oryginalne 32-bitowe zawijanie. 2916 rzeczywistych przypadków USA 8006DFA8 daje hash dd3f0ddcc154e8eb: trzy macierze, trzy projekcje, dziewięć pozycji z clippingiem, trzy offsety ekranu, cztery zakresy głębokości i trzy zakresy głośności. Cały CPU/SDK działa w MIPS, a instrukcje COP2 obsługuje ten sam PsyZ GTE. Native fixture używa dokładnie sześciu zaalokowanych bajtów i sprawdza też odtworzenie offsetów ekranu. To nie jest niezależna walidacja sprzętowego GTE ani końcowego PCM.

Budowanie i wszystkie 36 original-game-* z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL również przeszły, włącznie z nowymi korpusami oraz kompletnymi cyklami efektów. Oddzielny test original-game-glint uruchamia cykle kontrolerów i panoramę bez plików gry. Siedem descriptorów/skryptów tych efektów jest teraz powiązanych w rzeczywistych tabelach task programs, zgodnie z opisem powyżej. Kompletne SFX, pozostałe programy, fx_common, boot_display, pełny runtime i kampania oraz usunięcie pc/ pozostają otwarte. Próba linkowania oryginalnego main bez frontendu/backing testowego wskazuje obecnie FxCommon_RunFrameLoop i func_801235DC; sam link nie jest dowodem uruchomienia gry.

Istniejący `item/active_list.c` posiada rzeczywisty 32-bajtowy `g_InvItemMessageBuffer` (USA A1B50..A1B6F). Te same lokalne CopyText/AppendText i Inv_GetItemEffectData zachowują format FF, kolejność nazwy/przyrostka i nienadpisane bajty bufora, ale kontrolują pojemność oraz NULL/koniec resident pack przed odczytem. Brak tekstu, nieprawidłowy indeks albo zbyt długi wynik daje pusty FF-terminated message zamiast nadpisania sąsiednich danych lub odczytu NULL. Dodawanie offsetów indeksu jest jawnie 32-bit unsigned, bez signed-overflow UB dla INT_MIN. Nie dodano plików ani funkcji produkcyjnych; oryginalne funkcje pozostały na miejscu.

2560 przypadków rzeczywistego USA Inv_GetItemEffectData 80054A88 z oryginalnymi Text_LookupTable4/8 i kompletnym dostarczonym resident bank daje hash 443a865aea5e972f. Obejmuje item 0 i wszystkie 255 authored names, pięć trybów (0/1/2/3/INT_MAX), oba warianty kolejności i wszystkie 32 bajty wyniku, nie tylko tekst do FF. Maksymalny prawidłowy wynik ma 26 bajtów; oryginalna pojemność wystarcza. Fixture statystyk wykonuje te same przypadki oraz 126 kombinacji długości nazw/przyrostków po obu stronach limitu 32, NULL z lookup, ujemne/skrajne indeksy, niekończący się tekst na końcu resident pack, prawidłowy terminator w ostatnim bajcie i pointer na koniec banku. To dowód składania tekstu i kontroli wyjścia, nie pełnej walidacji wszystkich uszkodzonych katalogów/offsetów banku ani wyświetlania każdej wiadomości w czasie walki.

Istniejący test aktorów odczytuje wszystkie 47 rekordów pierwszego odtwarzacza z rzeczywistego banku 68, włącznie z nazwami i sześcioma końcowymi bajtami. Wykonuje oba oryginalne odtwarzacze w RGB15 oraz RGB24 przez rzeczywiste OpenVideo/StepVideo/ResetDisplayState: po 149 klatek FMV000, 596 łącznie, 11 920 slice uploads i 2384 VBlank. Wszystkie wiersze VRAM odpowiadają dekodowaniu całej klatki; pierwszy czarny obraz RGB15 zachowuje oryginalny bit STP (8000). Ten sam dekoder PsyZ obsługuje obie ścieżki, więc nie jest to niezależny dowód zgodności pikseli ze sprzętem PS1. XA/audio, wszystkie filmy, przejścia scen i kompletna kampania pozostają do weryfikacji.

Rozpoznanie pozostałych overlay entry points: historyczna konfiguracja boot_display umieszcza text_screen przy VMA 80122FA4+638=801235DC, co wskazuje na istniejący BootDisplay_RunDisplay jako docelowe wejście. Nie zastąpiono wywołania samym aliasem: cały moduł, callback/data ownership, native GPU i dawny CD playback muszą być najpierw zintegrowane i wykonane; dwa pozostałe wejścia 80121A00/80122040 zostały potwierdzone w rzeczywistym kodzie banku 68 jako MenuCardFirst_ResetDisplayState/MenuCardFirst_StepVideo. Cały istniejący video.c jest teraz linkowany statycznie, z własnym stanem i wszystkimi 47 rekordami tabeli USA 80122438. Oryginalna pętla frame_loop.c wywołuje te funkcje bez wrapperów i aliasów adresów. Nie dodano nowych plików ani nie wydzielono funkcji.

Pełny build i wszystkie 35 `original-game-*` z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL przeszły, w tym pełny korpus komunikatów i granice pamięci. Próba produkcyjnego linkowania oryginalnego main bez frontendu/testowego backing nadal zgłasza 2 brakujące symbole: FxCommon_RunFrameLoop i func_801235DC. Liczba nierozwiązanych symboli nie jest miarą gotowości gry; main wymaga też rzeczywistego startu z obrazami i integracji wykonania. Pełny start, frame runtime, overlaye, kampania, wszystkie komunikaty w scenach, logiczny zestaw obu płyt i usunięcie `pc/` pozostają niezakończone.

Oryginalny `battle/turn.c` posiada rzeczywistą siedmioelementową listę `g_BattleActionActors`. `Battle_ProcessActionSlot` zachowuje kolejność i dotychczasowe gałęzie, a czyszczenie pełnej listy i próba dopisania ósmego aktora nie wychodzą poza tablicę. Usunięto powtórną tentative definition fade level i komentarze historycznego codegen przy tej ścieżce. Istniejący `text/tables.c` zawiera pełne 62 bajty obu par komunikatów ucieczki z USA 8009159C/800915C0, wraz z terminatorami i paddingiem; fixture porównuje je z EXE. Nie dodano plików, funkcji produkcyjnych ani alternatywnej obsługi walki.

56 448 przypadków USA Battle_RollEscapeChance 800255E4 daje hash 02675eb74a42ca49. Obejmuje signed rank boundaries, HP i progi, wszystkie osiem stanów licznika prób, martwe/brakujące core i blokadę ucieczki na początku/końcu listy, oba warianty komunikatu i dwa RNG seeds. Zachowano signed-byte narrowing szansy po mnożnikach, także ujemne wartości po zawinięciu. Cały kod funkcji i jej pomocnicy wykonują się jako rzeczywisty MIPS; jedynie BIOS rand dostaje kontrolowany LCG zgodny z SDK. Porównywane są wynik, pozostałe bity stanu, wzrost prób, wybrany wskaźnik komunikatu, flaga overlayu oraz następna wartość rand, więc blokada nie może ukrycie konsumować RNG. To nie jest jeszcze dowód wyrenderowania całego komunikatu ani udanej zmiany sceny po ucieczce.

129 600 przypadków realnego USA Battle_ProcessActionSlot 80027A08 bez hookowanych callee daje hash 049bef90c93d53e1: cztery fazy, pięć kinds, HP -1/0/1, dziesięć fade levels, trzy commands, dwa action IDs, sześć konfiguracji siedmioelementowej listy, rodzic/peer/player exclusion i oba draw slots. Wywoływany jest prawdziwy Render_FadeEntityColor z poprawnym modelem o zerowej liczbie pakietów; porównywane są RGB, flagi, wynik, licznik i pełna lista wskaźników. Gałąź ładowania efektu 0x5B dla action ID 6 i actor==queue actor nie jest wykonywana w tym korpusie i pozostaje do testu integracyjnego. Osobny natywny przypadek sprawdza brak dopisania ósmego aktora. Nie deklarujemy testu pełnej geometrii zanikania w tej ścieżce ani pełnej kolejki walki.

Pełny build i wszystkie 35 `original-game-*` z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL przeszły, włącznie z korpusami ucieczki i listy akcji oraz testem przepełnienia. Produkcyjna próba linkowania oryginalnego main bez frontendu/testowego backing nadal wykazuje 6 brakujących symboli (fx_common, trzy overlay entry points, cross-flash flag i bufor komunikatu przedmiotu). Sam ten wynik nie dowodzi gotowości uruchomienia: pełne frame/scene/battle runtime, wszystkie zasoby, kampania, oba obrazy jako jeden zestaw i usunięcie `pc/` pozostają otwarte.

Oryginalne BuildTargetList/SortTargets/SwapRecords pozostają w `battle/setup.c` i operują na kanonicznym `BattleTarget`, bez aliasu BattleTargetWords i rzutowania rekordów na słowa. Moduł posiada siedem miejsc na cele i osobny ósmy rekord końca. Oryginalny znacznik po siódmym celu nachodził na saved-X Aya; zachowano jawne wyzerowanie wyłącznie X, a właściwa zapamiętana pozycja ma typed storage w oryginalnym `attack.c`. Natywny NULL nie nadpisuje Y. Nie dodano plików ani funkcji; nie przeniesiono istniejącej logiki. Tworzenie listy odrzuca brak gracza i ogranicza liczbę celów do siedmiu zamiast nadpisywać sąsiednią pamięć przy wadliwej populacji.

Naprawiono istotny błąd C ABI: definicja Battle_BuildTargetList była void, chociaż Battle_Update przypisuje jej wynik do g_BattleTargetCount. USA 8002156C rzeczywiście zwraca ostateczny indeks/liczbę celów w V0. Definicja i wspólny prototyp zwracają teraz int; istniejący fixture sprawdza ten wynik. 82 944 przypadki rzeczywistych USA 8002156C/800216E4/80021850, bez hookowania funkcji pomocniczych, dają hash cd7e48e9d9318f85. Korpus obejmuje wszystkie maski żywych siedmiu celów, filtry flag, brak core, HP 0/-1, remisy, różne kolejności i pozycje gracza w liście, skrajne współrzędne, rzeczywiste sqrt/ratan2, uporządkowane wskaźniki, dystans, kąt i końcowy znacznik oraz wyłączne wyzerowanie saved-X. Wyniki obejmują liczebności 0..7, w tym 24 przypadki pełnej siódemki. Osobne przypadki natywne sprawdzają nadmiar celów i brak gracza. To nie jest jeszcze wykonanie całego cyklu wyboru celów/inputu, animacji Aya, kolejki akcji ani pełnej walki.

Pełny build i wszystkie 35 `original-game-*` z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL przeszły, włącznie z nowym korpusem i granicami listy. Próba produkcyjnego linkowania oryginalnego main bez frontendu/testowego backing nadal wykazuje 9 brakujących symboli. Pełny entrypoint, kompletna walka/fx_common/overlaye, kampania, zestaw obu płyt oraz usunięcie `pc/` pozostają niezakończone.

W istniejącym `fieldfx/sprite_render.c` przywrócono sześć oryginalnych funkcji osadzonych w USA EXE w zakresie E0060..E0808: Clear/Spawn/UpdateScriptSprites, dwie procedury animacji E026C/E03A0 i właściwy drawer E051C. To translacja tych samych funkcji MIPS do C, bez zastępczego efektu, nowego modułu ani przenoszenia dotychczasowych funkcji. Oryginalne 20-bajtowe rekordy i wspólne parametry rysowania mają zwykłą pamięć zamiast obszaru poniżej bufora animacji i scratchpada. Zachowano oryginalny dobór slotów także przy 21./22. wywołaniu Spawn, nasycanie licznika do 20, wcześniejsze przygotowanie UV oraz zmianę granicy pętli aktualizacji po wygaśnięciu efektu. Sprite strip i RGB fade emitują oryginalne FT4 przez natywne linki OT i aktualny bufor animacji. Nie dodano plików.

Istniejący fixture porównuje 910 scenariuszy z rzeczywistym USA MIPS wszystkich sześciu funkcji, bez hookowania ich logiki; jedynie instrukcje COP2 są przekazywane do tego samego PsyZ GTE. Hash 1c171246cd9ca458 obejmuje 12 243 wywołania Spawn, 8950 aktualizacji, pełne 22 rekordy obserwacji wokół oryginalnego limitu, Clear z dziurami, ponowne użycie, oba tryby i tryb nierozpoznany, opóźnienia 0/1/3/17/255, skrajne signed sizes, oba bufory, RGB i wszystkie FT4 payloads/kolejność OT. Długie cykle obejmują 1030/520/200 kroków aż do wygaśnięcia. Natywne testy dodatkowo sprawdzają wyczerpanie bufora przy 22 wartościach pojemności oraz zerowy mianownik i clipping poza OT; granice nie naruszają pamięci ani prawidłowości łańcucha. To nie jest niezależny test sprzętowego GTE/obrazu ani dowód wykonania tych opcode'ów w pełnym runtime wszystkich scen; dispatcher 2400/2402 wskazuje już tę samą oryginalną implementację, lecz pełne przejścia skryptów pozostają otwarte.

Pełny build i wszystkie 35 `original-game-*` z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL przeszły, wraz z korpusem sprite'ów i granicami bufora. Produkcyjna próba linkowania oryginalnego main bez frontendu/testowego backing nadal wykazuje 11 brakujących symboli. Cały entrypoint, pozostałe overlay entry points/fx_common, pełna kampania i skrypty, oba obrazy jako jeden zestaw oraz usunięcie `pc/` pozostają niezakończone.

Oryginalny `entity/frame_update.c` posiada teraz kompletną tabelę 13 callbacków typu aktora z USA 800915DC: typ 0 uruchamia istniejące Entity_IntegratePositionFull, typy 1..12 istniejące Entity_IntegratePositionConditional, wszystkie contact callbacks są NULL jak w EXE. Usunięto niezależną pustą tablicę testową. Tworzenie aktora odczytuje istniejące `g_GameState.bank_slots` zamiast osobnego symbolu pod dawnym adresem B0E70; fixture również zasila kanoniczne sloty. `draw/pipeline.c` ustawia bit przejścia w rzeczywistym `g_ScreenTint.fadeMode` zamiast nieistniejącego dekompilacyjnego symbolu BCFEE. Nie dodano plików ani funkcji produkcyjnych.

Fixture porównuje wszystkie 104 bajty oryginalnych bindingów z EXE i ich natywne odpowiedniki. Pełny test tworzenia aktorów obejmuje 414 bloków pokoi, 1756 NPC spawns, cztery Aya spawns z inicjalizacją ekwipunku i oryginalny startowy descriptor mapy 431. Sprawdza callbacki nowo tworzonych Aya/NPC oraz odrzucenie NULL i typów 13..255 bez naruszenia listy aktywnych/wolnych aktorów. Istniejący test ruchu wykonuje rzeczywiste bindingi typów 0 i 12, w tym callback-before-motion, freeze, input lock i zawijanie słowa. Fixture tworzenia aktorów linkuje cały istniejący moduł ruchu zamiast dostawiać pustą tablicę reguł. Nie dowodzi to wykonania wszystkich skryptów ani kontaktów i frame runtime każdej sceny.

Pełny build i wszystkie 35 `original-game-*` z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów, statystyk, ruchu i tworzenia aktorów z instrumentowanymi PsyZ/SDL przeszły. Produkcyjna próba linkowania oryginalnego main bez frontendu/testowego backing nadal ujawnia 13 brakujących symboli. Dalsze rozpoznanie istniejącego naming_review_main3 wskazuje, że Clear/Spawn/UpdateScriptSprites pochodziły z kodu engine data blob (USA E0060/E00CC/E01BC), a nie z pustych funkcji do pominięcia. Ich natywna implementacja i dowód wykonania nadal pozostają otwarte. Pełny entrypoint, kampania, fx_common, oba obrazy jako jeden zestaw i usunięcie `pc/` są niezakończone.

Cały istniejący `render/wireframe.c` jest dołączony do natywnego buildu. Ta sama oryginalna funkcja projekcji i rysowania krawędzi używa istniejącego natywnego LINE_G2, draw-mode packets i pełnych linków OT; usunięto 24-bitowe maskowanie adresów, dawne rozmiary pakietów i pozostałości wymagań historycznego codegen. Zwykłe lokalne tablice modułu zastępują dwa scratchpad views współrzędnych/głębokości dla maksymalnie 255 wierzchołków. Nie dodano plików, produkcyjnych funkcji ani zastępczego renderera.

Oryginalny `Render_AnimationFrame` w `status_hud.c` odczytuje kanoniczne `Combatant.stateFlags`, `action->range` i współrzędne `FieldActor` zamiast fikcyjnych struktur o offsetach PS1. OT jest przesuwane o dwa natywne tagi, nie osiem bajtów. Arytmetyka skali, kąta i licznika zachowuje zawijanie 32-bitowe bez signed-overflow UB. Oba oryginalne inicjalizatory pamięci powiększają dwa istniejące bufory animacji według największego współczynnika wzrostu nagłówka GPU; fixture sprawdza ich rozdzielenie i granicę następnej puli. Istniejąca funkcja wireframe otrzymuje pojemność bufora, sprawdza ją przed zapisem i odrzuca błędne indeksy wierzchołków oraz głębokości indeksujące przed OT.

Istniejący fixture wykonuje 17 280 przypadków realnego USA 8007041C (hash dd1b134b66169daf) i 672 klatki pełnego USA Render_AnimationFrame 800347B4 (hash abf9dfc230518165). CPU, oryginalne SDK matrix helpers i obsługa pakietów wykonują się jako rzeczywisty MIPS; wyłącznie instrukcje COP2 przekazywane są do tego samego PsyZ GTE. To niezależne porównanie algorytmu CPU, ale nie niezależny dowód zgodności GTE ze sprzętem PS1. Porównywane są projekcja i głębokości, zmiany modifier flags, pełne payloady i kolejność OT, skale, yaw, floor/back/front cases, blend modes, trzy widoki oraz canonical actor range/position, oba bufory i zawijanie licznika. Korpus pełnej animacji zaczyna się od klatki 1: inicjalny positional sound z klatki 0 nadal wymaga osobnego dowodu. Próby pojemności 0..176 bajtów obejmują pełne trzy krawędzie i trzy tryby rysowania, guards i poprawność częściowych linków. Osobno sprawdzane są błędny indeks, clipping poniżej/przed OT oraz scratch dla 255 wierzchołków. API nadal nie przekazuje długości wejściowego modelu; nie deklarujemy bezpieczeństwa dowolnego uszkodzonego pliku ani sprzętowej zgodności pikseli.

Pełny build i wszystkie 35 `original-game-*` z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL również przeszły, wraz z korpusami wireframe, wywołującą animacją i granicami pamięci. Próba linkowania oryginalnego main bez frontendu/testowego backing nadal zgłasza 16 brakujących symboli. Definicji FieldFx_ClearScriptSprites/FieldFx_UpdateScriptSprites obecnie nie znaleziono w drzewie; ich deklaracje w pipeline/frame_loop nie są implementacją i wymagają odtworzenia brakującego oryginalnego modułu, bez zastępczych pustych funkcji. Pełny entrypoint, fx_common runtime, wszystkie sceny i tekstury, kampania, zestaw obu płyt i usunięcie `pc/` pozostają niezakończone.

Cały istniejący `draw/textbox.c` korzysta teraz z natywnego indeksowania buforów, pakietów sprite/TILE i linków OT. Usunięto pomocnicze unie adresów 32-bitowych, stride 4/24/28 oraz pozostałości wymagań historycznego codegen. Wszystkie 16 dotychczasowych funkcji pozostaje w tym samym module; nie dodano produkcyjnych funkcji ani plików. Fog i kursor wyboru mają rzeczywiste pakiety PsyZ dla obu buforów. Oryginalny wspólny obszar danych fontu i slotów tła (widoki przesunięte o cztery bajty) ma jednego właściciela; loader sceny i tekst odczytują te same tpage/CLUT. Istniejący `text/tables.c` zawiera dokładne 68 bajtów tego obszaru oraz 156 bajtów właściwej tabeli 78 podstawowych glifów USA, porównywanych z EXE. Dawne deklarowane 256 rekordów obejmowałoby już sąsiednie tabele debugowego wyboru map.

Oba oryginalne inicjalizatory pamięci (`Boot_InitMemoryLayout` i `Draw_InitDisplay`) rezerwują po 329 natywnych pakietów glifów zamiast zachowywać dawną liczbę bajtów. Fixture porównuje oba układy i granice kolejnego obszaru. Renderer kontroluje pojemność przed każdym glifem; trzy długie strumienie (zwykły tekst, imiona, rozszerzone glify) kończą się na 329 pakietach z nienaruszonymi guards. Błędny indeks podstawowego glifu, ponad pięć cyfr lub ponad pięć podstawień liczb przerywa rysowanie zamiast wychodzić poza tabelę. Nie jest to jeszcze audyt wszystkich rzeczywistych tekstów USA ani bezpieczeństwa arbitralnie uciętych strumieni: API źródła wiadomości nadal nie przekazuje długości, a brak szukanego page marker wymaga osobnej walidacji zasobów.

Istniejący fixture porównuje 6656 klatek rzeczywistych USA 800371B0/80037870 wraz z niehookowanymi funkcjami GPU: hash a279a1e503166b41. Obejmuje wszystkie 78 podstawowych glifów, wybrane granice rozszerzonych stron i FB, imię, liczby, kolory, wybory/input, końce stron, sekcje, opóźnienia, cztery style, oba bufory, również cztery aktywne textboxy oraz surowe i wyliczone przez loader tpage/CLUT. Porównywane są stany i wszystkie komendy/payloady w kolejności OT. Dodatkowe zerowe słowa native MargePrim są pomijane przy hashowaniu semantycznych komend, ale ich wyzerowanie i rzeczywista długość pakietu są sprawdzane. To dowód tego korpusu komend, nie niezależny sprzętowy wzorzec pikseli wszystkich dialogów ani wykonanie pełnej kampanii.

Pełny build i wszystkie 35 `original-game-*` z wymaganym Disc 1 przeszły headless po tych zmianach. Świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL przeszły, w tym cały korpus dialogów, oba inicjalizatory pamięci i przypadki przepełnienia. Produkcyjna próba linkowania oryginalnego main bez frontendu/testowego backing wykazuje nadal 17 brakujących symboli. Pełne font/tekstury podczas wszystkich scen, runtime wejścia gry, kampania, oba obrazy jako jeden zestaw i usunięcie `pc/` pozostają niezakończone.

Cały istniejący `menu/save_slot.c` jest dołączony do natywnego buildu. Panel Scan odczytuje HP i maksymalne HP z kanonicznego `EnemyCombatant`, zamiast z fikcyjnego rekordu o starych offsetach PS1. Obie oryginalne funkcje pozostają w swoim pliku. Istniejący `text/tables.c` zawiera 310 rzeczywistych bajtów tekstów Scan; test porównuje je z USA EXE. Tablica liczb ma prawidłowy terminator -1: oryginalne zero powodowało odczyt poza lokalną tablicą. Porównanie obejmuje dwa pola liczb używane przez tekst HP, a nie nieużywane pola wypełnione dawniej przypadkowymi bajtami stosu.

Oryginalny `draw/wipe.c` używa natywnych trójkątów, TILE i pakietów trybu rysowania oraz pełnych wskaźników OT. Zachowano istniejące funkcje, kolejność krawędzi, kolory i geometrię; nie dodano produkcyjnych funkcji ani plików. Wyczerpanie areny nie dereferencjonuje nieprzydzielonych trójkątów, TILE ani sprite'a. Rezerwacja sprawdza pozostałą pojemność przed tworzeniem wskaźnika końca. Niepotrzebne aliasy `AyaBonusPoolRecord` i `D_800B0D88_read/write` zastąpiono istniejącymi kanonicznymi polami postaci i obiektu gracza.

Istniejący fixture aktorów porównuje 18 560 przypadków Scan/komunikatów z wykonanym rzeczywistym USA 8003495C/80034DE0 wraz z oryginalnymi funkcjami pomocniczymi: hash 2c8a283ef8f19f95. Obejmuje fazy, typy komunikatu, słabości, timery, brak/martwy cel, cztery tryby menu i oba warianty tekstu, stan czterech textboxów, dwa pola liczb HP, payload wszystkich pakietów tła i kolejność OT. Dodatkowe 260 przypadków sprawdza granice areny i linki przy braku miejsca. Nie jest to dowód renderowania całych tekstów: `Menu_DrawTextboxEntries` i fog nadal wymagają usunięcia historycznych offsetów/stride oraz integracji prawdziwych font tables.

Pełny build i wszystkie 35 `original-game-*` z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów i statystyk, z instrumentowanymi PsyZ/SDL, również przeszły, włącznie z nowym korpusem i granicami areny. Próba linkowania oryginalnego main bez frontendu/testowego backing nadal zgłasza 22 brakujące symbole. Pełny entrypoint, renderowanie wszystkich dialogów, kampania, oba obrazy jako jeden zestaw i usunięcie `pc/` pozostają niezakończone.

Oryginalny status_hud.c używa teraz kanonicznych native page/sprite/tile packets także w listach akcji, symbolach statusu, markerze wartości, floating damage/recovery/MISS, układzie tła i kolejce znacznika. Usunięto stare stride 28/48/0x118/0x578, odejmowanie ośmiu bajtów od sprite pointer i 32-bitowy krok OT tam, gdzie dotyczyły tych ścieżek. Mode 3 etykiety wartości w DrawStatusOverlay także wskazuje właściwy native packet. Zniknęły wyłącznie niepotrzebne aliasy do x/y/clut/UV/RGB i sztuczne BattleStatusMarkerBody/BattleHalfwordAddress/BattleSpriteWord, bez zastępczego backing. Porównanie zestawu definicji funkcji przed/po zmianie potwierdza wszystkie 20 dotychczasowych funkcji: nie przeniesiono ich ani nie dodano plików/produkcyjnych funkcji.

Istniejący fixture porównuje 38 142 wykonania z niehookowanym EXE USA: 1056 list akcji (kategorie, turn mode i 0..10 pozostałych akcji), 54 symbole (wszystkie dziewięć), 2400 wartości (0..99 i signed offsets), 34 560 floating/MISS panels (obie klatki, dziesięć slotów, tryby/style, signed 16-bit values, timery i pozycje), 48 układów tła i 24 kroki znacznika z wrap licznika. Obejmuje kolejność packet IDs w ośmiu OT layers, całe GPU payloads i post-state; pełne native pointers oraz zachowanie długości OT są sprawdzane po stronie C. Hash fa7672b3016c55b0 odpowiada rzeczywistym 8003205C/800323C8/800327D8/80032B0C/80031D6C/80033430. Nie jest to sprzętowy pixel oracle HUD ani dowód pełnego DrawStatusOverlay z target/GTE, Render_AnimationFrame/wireframe i całej walki.

Pełny build i 35 original-game-* z wymaganym Disc 1 oraz świeże ASan/UBSan/function aktorów i statystyk (instrumentowane PsyZ/SDL) przeszły headless. Próba linkowania oryginalnego main bez frontendu/testowego backing zgłasza nadal 28 brakujących symboli. Pełny game/frame runtime, kampania, obie płyty jako zestaw i usunięcie pc/ pozostają niezakończone; sam spadek liczby symboli nie jest odbiorem działającej gry.

g_SceneAreaType jest teraz kanonicznym g_GameState.requested_entity_bank, a g_RoomLightTint rzeczywistym trzybajtowym polem wspólnego GeomScrollRecord. View reset, skrypty i oświetlenie modeli nie mają osobnych zmiennych. Model/spawn fixtures uruchamiają oryginalne Render_InitViewState zamiast definiować własny biały tint; odpowiednie targety linkują cały dotychczasowy camera.c. Nie dodano plików ani adapterów.

Oryginalny render/object.c posiada prawdziwe resident data USA: osiem słów bazy z 80091A38, 240 normalnych z 80091A58 i tablicę 4×4 page deltas z 800921D8. Fixture porównuje wszystkie 1984 bajty z EXE przed ich użyciem, zamiast ładować tabelę do własnego backing. Usunięto zastępcze definicje z model/scene/entity/camera fixtures. Historyczna nazwa g_IdentityMatrixSource jest myląca: baza ma pierwszy signed halfword -4096, a nie +4096 z wcześniejszych testów. Zachowano prawdziwą wartość z danych gry.

Szerszy w pełni instrumentowany model fixture wykrył rzeczywisty błąd Render_DrawObject: przy ostatniej części first=355/count=16/total=371 końcowy NCCT triple odczytywał dwie normalne już z następnej sekcji modelu (index 256 poza 240-entry table). Disassembly USA 8003B97C potwierdza pracę trójkami i te nadmiarowe odczyty RAM. Natywny kod powtarza ostatnią prawidłową normalną wyłącznie w nieużywanych lanes, zachowując kolory prawdziwych wierzchołków i dotychczasowy wybór coded base colour. Istniejący test porównuje main/alternate colours pełnej trójki z końcówkami długości 1/2, ze wszystkimi trzema miejscami colour marker i nieprawidłowymi indeksami w nieużywanych lanes. To poprawka bezpieczeństwa oryginalnej funkcji, nie osobny shader ani nowy renderer.

Pełny build i 35 original-game-* z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów, statystyk i zasobów scen, z instrumentowanymi PsyZ/SDL, przeszły; po poprawce nadmiarowego odczytu również cały model --room-gpu corpus przechodzi: 414 room texture sets/9017 uploads, 878 model buffers, 732 models z widocznymi pikselami, 1 515 584 pixels i 301 224 native OT packets. Test zachowuje pełne porównanie upload VRAM i obu draw slots; nie jest niezależnym sprzętowym pixel oracle całej gry. Produkcyjny link oryginalnego main nadal zgłasza 40 brakujących symboli. Full frame runtime, kampania, obie płyty i usunięcie pc/ pozostają niezakończone.

Całe oryginalne battle/enemy_slots.c i battle/enemy_movement.c są dołączone do natywnego buildu. Usunięto niezgodny FacingCore oparty na starych offsetach; reakcja na trafienie używa kanonicznego EnemyCombatant. Oryginalna arytmetyka double zachowuje kolejność operacji, ale wykonuje się w C zamiast przez ABI soft-float PS1. Odrzut oraz mnożenie/odejmowanie nagród jawnie zawijają się do 32 bitów. Battle_SlotFree ma typ rzeczywistego argumentu BattleEntity, bez udawania void**.

Naprawiono rzeczywistą kolizję pamięci: sześć 16-bajtowych event records obejmuje historyczne sloty 0x68/0x6C/0x70, które równoległy widok gracza uznawał za pointers. Widening tych slotów rozsuwał enemy tail względem EntityAnimEventCore. W istniejących FieldActorState/Combatant/EnemyCombatant pozostają wspólne scalar slots i sześć nieprzerwanych event records; native action/attributes/aux pointers są polami rozszerzenia za scalar state. Nie dodano rejestru, lookup layer ani plików. Static assertions kontrolują wspólny rozmiar, flags/status/action pointers, selected-event pointer, records, parameters i assetId. Scan/Confuse/Confuse i StartEnemyAttackEffect używają właściwego EnemyCombatant zamiast player view. Save wire format pozostaje 8192-bajtowy, a jego dotychczasowe pełne porównania z EXE nadal przechodzą.

Oryginalny szablon enemy USA 800109B0 ma pełny native initializer w enemy_slots.c. Istniejący fixture porównuje cały scalar obraz 0xD8 z EXE, normalizując tylko pointer, oraz sprawdza brak fikcyjnych pointers w rozszerzeniu. 5632 przydziały, wszystkie 128 kombinacji flag siedmiu slotów, wyczerpanie puli, zwalnianie/ponowne użycie i cztery początki licznika ID odpowiadają niehookowanym MIPS Alloc/Free/Clear, hash 81c2f5af25011535.

147 456 przypadków obrażeń odpowiada realnemu USA 80028574 wraz z jego oryginalnymi soft-float callees, hash 19758ebd45a2608d. Tylko BIOS rand ma kontrolowany odpowiednik; korpus obejmuje trzy tryby obrażeń, cztery statystyki, sześć poprawnych shot counts, elementy, status pairs, reakcje/kategorie, dwa RNG seeds i cztery defenses. Target count jest stały (4), a kind 1 kończy reakcję wcześnie. Osobne 40 320 przypadków rzeczywistej reakcji USA 80028C48 porównuje sześć selected records, resume/motion phases, PE flag, pozycje i facing; hash 6496fbe76c786105. Nie oznacza to wykonania wszystkich możliwych stanów ani pełnej animacji każdej techniki.

11 520 przypadków realnego USA 80028E94 odpowiada natywnej śmierci, hash 85c5db394010ef9d: cztery fazy, persist/visibility, kind/frame, fade steps, child propagation, oba draw slots, reward wrap, pełna/pusta tabela loot i auxiliary value. Oryginalne interpolation/color functions oraz kolejka AKAO 21 wykonują się naprawdę. Korpus ma poprawny pusty model (zero packets), wyłączony death positional asset i HP gracza 0, aby oddzielić samo usunięcie przeciwnika od pełnej sekwencji końca walki. Nie dowodzi obrazu modelu, dźwięku śmierci ani przejścia od ostatniego przeciwnika do zwycięstwa. Kanoniczne zero-initialized storage 10 reward slots i hold timer należy do encounter.c, a cztery istniejące textbox records do textbox.c; nie są to zastępcze funkcje.

Pełny build i 35 original-game-* z wymaganym Disc 1 przeszły headless. Świeże ASan/UBSan/function aktorów, statystyk i zasobów scen, z w pełni instrumentowanymi PsyZ/SDL, również przeszły. Próba linkowania oryginalnego main bez frontendu/testowego backing nadal wykazuje 45 brakujących symboli. Cała gra, pełny battle/frame runtime, wszystkie overlays, oba obrazy jako zestaw oraz usunięcie pc/ pozostają niezakończone.

Całe oryginalne battle/player_turn.c i battle/phase.c są składnikami natywnego buildu. setup.c posiada rzeczywistą 45-elementową kolejkę z native actor pointers; osobne scalar symbols udające bajty 1/3 g_BattleActionAnimModes usunięto. Phase i attack używają indeksowania tej kolejki zamiast PS1 stride 8, a phase odczytuje pozycję przez pole posX zamiast offsetu 0x28. Kopię 40-bajtowej tabeli dźwięków w phase wykonuje zwykłe przypisanie struktury; usunięto packed block-copy types, testy wyrównania adresów i etykiety potrzebne tylko historycznemu codegen. Tabela 20 programów efektów PE pochodzi z USA 8001074C i jest porównywana z EXE w istniejącym fixture. Wywołania synchronizacji amunicji mają prototyp C11 i jawny docelowy action record.

Oryginalny Battle_UpdatePlayerTurn z prawdziwymi native animation/aim/character-action callees odpowiada 2880 wykonaniom niehookowanego USA 80021F38: hash 782dd423afe0eac1. Korpus obejmuje 12 modes, trzy fazy animacji, pięć wysokości celu, committed state, cztery pozycje kolejki (0/3/11/42), zakończenie oraz martwy następny cel. Sprawdza też wspólne animation bytes i poprawny krok kolejki przy native pointers. Nie wykonuje wyczerpania/przeładowania amunicji, wszystkich efektów PM/audio ani całego Battle_AdvancePhase; tych ścieżek nie uznajemy za zweryfikowane. Enemy movement/slots nadal wymagają integracji, w tym uzgodnienia nakładających się widoków enemy/event core i usunięcia soft-float ABI zależnego od PS1. Nie wydzielono funkcji ani nie dodano plików.

Pełny build i 35 original-game-* z wymaganym Disc 1 przeszły. Świeże ASan/UBSan/function aktorów i statystyk z instrumentowanymi PsyZ/SDL również przeszły headless. Próba produkcyjnego linkowania oryginalnego main bez frontendu/testowego backing nadal ujawnia 53 brakujące symbole, w tym enemy lifecycle, field sprites, overlay entry points i dane. Pełna gra i usunięcie pc/ pozostają niezakończone; same dołączone moduły nie oznaczają działającej kampanii.

Do natywnego buildu dołączyły całe oryginalne battle/context.c i entity/combat.c. g_PlayerEntity i D_8009D254 wskazują teraz jeden kanoniczny wskaźnik gracza zamiast dwóch niezależnych zmiennych. Wszystkie dotychczasowe deklaracje widoku battle korzystają ze wspólnego nagłówka. Zapis przerwanej animacji trafienia pozostaje w oryginalnym combat.c; nie wydzielono funkcji. Timery mają jawne signed narrowing i zawijanie 32-bitowego akumulatora, bez signed-shift/overflow UB. Sygnatura script animation setup jest C11; sześć rekordów zdarzeń ma kontrolę granic.

Istniejący fixture sprawdza 65 536 kombinacji statusów/buffów, faz, HP/PE i obu draw slots z prawdziwym USA Entity_UpdateTimers 0x8001F9C4, włącznie z rzeczywistymi poison/animation callees: hash a1f411039c73e485. Automatyczne medicine/cure są wyłączone w tym korpusie i nadal wymagają integracji. Osobne 8748 kombinacji pozycji fixed-point i facing angle odpowiada realnemu 0x800305C8 z oryginalnym ratan2, hash 37f5f1d4d13308e7. To nie jest dowód wszystkich ścieżek obrażeń, reakcji, zdarzeń skryptowych ani pełnej walki.

Inv_CountByValue czyta itemId kanonicznego equipment record. Dawny g_ItemCodeRemapTable był adresem wewnątrz tej samej puli (USA BEEB0), a nie osobną tablicą wymagającą sztucznego backing. Istniejący test obejmuje zliczanie zwykłych IDs, powtórzone authored equipment IDs, ostatni slot 0x17F, signed -1 i specjalne ammo counts.

CdRom_LoadBootAudio zachowuje oryginalny reset audio i odczyt display-data range 71, ale nie ładuje już wykonywalnego overlayu MIPS range 67 ani nie wywołuje FlushCache. Oryginalny init.c posiada zwykłą pamięć danych/decode scratch; main.c kończy start przy błędzie odczytu. Brak obrazu, błąd poll, NULL workspace, puste/odwrócone/zbyt duże zakresy i przepełnienie LBA kończą się błędem, bez nieskończonych retry. Headless fixture porównuje cały payload 165 sektorów z Disc 1 i wszystkie pozostałe bajty areny jako guards. Nie dowodzi to wykonania całego boot_display overlayu; jego funkcje, legacy CD playback i Script_LoadSceneData nadal wymagają integracji.

Całe istniejące mesh_list.c, mesh_entry.c i mirror_model.c są teraz również składnikami aplikacji, z function/data sections, bez nowych plików i przenoszenia funkcji. Pełny build i wszystkie 35 original-game-* z wymaganym Disc 1 przeszły; świeże ASan/UBSan/function aktorów i statystyk, z instrumentowanymi PsyZ/SDL, również przeszły. Próba linkowania oryginalnego main bez frontendu/testowego backing nadal zgłasza 54 brakujące symbole. Produkcyjny main, pełna kampania, oba obrazy jako jeden zestaw i usunięcie pc/ pozostają niezakończone; wyników frontendu pc/ nie traktujemy jako dowodu ich wykonania.

Oryginalny start i soft reset w asset/game_state.c nie oczekują już na DsReset/DsSystemStatus/pokrywę napędu ani na insert-disc UI. Render_InitDisplayLists waliduje zamontowany obraz, wymagany tryb 1/2 i czyści oryginalny obszar 320×448; błędny tryb, brak obrazu lub wymaganej płyty zwracają błąd przed zmianą VRAM. main.c propaguje ten błąd. To nie łączy jeszcze obu płyt w jeden zestaw: brak Disc 2 nadal daje błąd, a nie pozorny sukces.

Do buildu dołączyły całe istniejące asset/unload_table.c i menu/main_update.c; istniejący moduł camera.c jest też składnikiem targetu aplikacji, z function/data sections dla GC linkera. UnloadTableEntries zachowuje oryginalną kolejność wywołań resident 0..7, handlera 85 i wymienionych w scenie 8..84, usuwa scene handlers 8..84 i task programs 30..103, zachowując pozostałe. Brak wymaganej tabeli/archive zwraca -1 bez częściowej mutacji. Test sprawdza native callback ABI, kolejność, powtórzone i skrajne IDs, granice zachowywanych rekordów i oba warianty flagi 0x80. Usunięto dekompilacyjny draw_slot_record.h; status_hud/main_update czytają kanoniczny g_ActiveDrawSlot. W main_update usunięto też obcinanie g_ActiveActor do u32 oraz wywołanie Entity_SetActionMode przez niezgodny typ z dodatkowym argumentem. Pozostałe ścieżki modułu wymagają runtime proof; sam pełny build nie oznacza gotowej walki.

Pełny restart SPU ujawnił rzeczywistą awarię: Akao_MessageHandlers miała wcześniej tylko binding 0x24. Przywrócono komplet 256 wpisów z USA 0x8009C0C0 do istniejącego voice_driver.c, wskazujących 46 istniejących funkcji voice_bank.c (w tym oryginalny Noop wyłącznie w jego oryginalnych slotach). Wszystkie docelowe funkcje mają wspólne native callback ABI void(void*), bez adapterów ani przenoszenia funkcji. Istniejące widoki skalarnych poleceń są odczytywane przez lokalne memcpy, a funkcje korzystające z wiadomości otrzymują prawdziwy native record. Komendy 20/30/34 używają kanonicznych pól; 34 zachowuje dwa native pointers przez union współdzielony ze slotami arg1/arg2, które ta komenda zastępuje domyślnymi wartościami. Nie zmienia to offsetów pozostałych argumentów.

Oryginalny spu_transfer.c posiada teraz prawdziwe 96 bajtów silent ADPCM z EXE, wyrównaną 40-bajtową tabelę alokatora i native long event handle; sequencer.c posiada kanoniczne pomiary czasu przerwania. Przestarzałe komentarze o historycznym planowaniu instrukcji i rzutowania pól banku na słowa usunięto. Nie dodano plików, równoległego sekwencera ani wrapper handlers.

Headless fixture porównuje wszystkie 256 bindings z realnym EXE, pełny footprint VRAM przy wyborze obrazu i silent sample bytes. Wykonuje dwa kolejne Boot_SyncShutdown/Boot_Shutdown, rzeczywisty upload do SPU, po cztery jawnie dostarczone zdarzenia do prawdziwego Akao_TimerCallback, rolling history, zamknięcie zdarzenia i original soft-reset token/room variable. Test wysyła także komunikaty 20/30/34 przez rzeczywistą kolejkę i sprawdza dwa natywne stream pointers oraz domyślne maski. Są to kontrolowane hardware-event injections; nie dowodzą częstotliwości timera hosta ani PCM wszystkich technik/muzyki. Nie stwierdzamy wykonania każdego wariantu wszystkich 46 handlers.

Pełny build, 35 original-game-* i C11 syntax checks main/main_update przeszły. Świeże ASan/UBSan/function aktorów i statystyk, z instrumentowanym SDK/SDL, również przeszły. Oddzielny świeży sanitizer AKAO przeszedł procedural fixtures oraz 240-tick corpus: 414 pokoi, 599 sekwencji, 8883 kanały, PCM hash 25836548e09d8a00; jeden krótki rekord nadal nie jest wykonywany. To ograniczona długość korpusu, nie pełne utwory ani pełna gra. Próba połączenia pełnego oryginalnego main bez frontendu/testowego backing nadal ujawnia 75 brakujących symboli render/field/battle/overlay i danych. Produkcyjny entrypoint, pełna kampania, oba obrazy jako jeden zestaw, XA/MDEC fidelity i usunięcie pc/ pozostają niezakończone. Niezacommitowane pliki użytkownika w pc/ i bu00 pozostają poza commitem.

Własny fork PsyZ ma natywną implementację MDEC w istniejącym src/psyz/libpress.c: RLE/dequantization, całkowitoliczbowy IDCT, trzy komponenty koloru, formaty 4/8/15/24-bit, signed/STP, komendy quant/scale, częściowe odbiory i status błędu. Dane transferu zachowują słowa 32-bit; prywatny interfejs bufora nie wymaga wyrównania native u_long. DMACallback ma rzeczywisty rejestr, a ukończenia zlecone z callbacku czekają na jego powrót, co zachowuje wymaganą kolejność oryginalnych dwóch slice buffers bez rekurencji. Sync w natywnym SDK dotyczy synchronicznego zakończenia transferu, nie historycznego bitu rejestru. To uproszczony model SDK, nie emulacja timingów DMA/FIFO. Legacy DecDCTvlc nadal jest nieobsługiwany; gra używa swojego oryginalnego DecDCTvlc2. Nie dodano plików ani alternatywnego odtwarzacza gry.

Istniejący fixture wykonał rzeczywiste MenuCard_InitVideoBuffers/OpenVideo/StepVideo/Draw_RenderFrame/ResetDisplayState dla FMV000: 149 wyrenderowanych RGB24 klatek i 2980 oryginalnych slice uploads. Każdy pełny obszar VRAM zgadza się z osobnym odbiorem pełnej klatki MDEC po ułożeniu pionowych macroblocks; pierwszy obraz jest w całości czarny. Licznik, ostatnia odczytana klatka 150, flaga zakończenia i dokładnie 596 VBlank potwierdzają oryginalną ścieżkę czterech VBlank na klatkę. Ten test porównuje dwie drogi tego samego dekodera: nie jest niezależnym pixel oracle. Oddzielne porównanie wszystkich 150 RGB24 klatek z FFmpeg dało średni bezwzględny błąd kanału 0.1990413, maksymalny 10 i różnicę w około 17.09% kanałów. FFmpeg nie zastępuje wzorca PS1; zgodność hardware rounding/YUV nadal jest nieudowodniona i nie deklarujemy pixel perfect.

Pięć testów MDEC w istniejącym SDK test_mdec.c przeszło z ASan/UBSan/function: płaskie bloki we wszystkich formatach/sign/STP, kolejność callbacków i dzielony odbiór, komendy ucięte i granice wyjścia, quant/scale oraz 128 wygenerowanych strumieni. Pełne 35 original-game-* przeszło headless z Disc 1. Świeży fixture aktorów przeszedł także po połączeniu z w pełni instrumentowanym SDK i SDL. Ten przebieg wykrył czytanie GPU code poza pustym native OT: istniejący GPU_Enqueue odczytuje je teraz wyłącznie dla niezerowej długości pakietu. W SDK poprawiono też unsigned shift bitu 31 w istniejącym teście SPU, który zatrzymywał UBSan. Sanitizer statystyk również przeszedł.

Pełny zestaw SDK nie jest zielony: ostatni izolowany przebieg z kopiami fixtures zgłosił 306 pass, 46 fail i jeden skip, w tym headless framebuffer references i event timing. Podczas pracy jeden przebieg uruchomiono błędnie z katalogu repo: fixture SDK utworzył/usunął własne BASLUS-00000PSYZ00/01, a próba rmdir bu00 zatrzymała test na istniejących zapisach gry. Siedem plików zapisów zachowało wcześniejsze daty modyfikacji; utworzony pusty bu10 usunięto. Kolejne przebiegi SDK odbywały się wyłącznie w /tmp, a testy gry w build/ z prywatnymi ścieżkami. Pliki użytkownika nie są stage'owane.

Commit PsyZ dabd7dd jest utrzymywany w khasinski/parasite-eve-psyz. Native MDEC i ścieżka obrazu filmu mają teraz rzeczywiste wykonanie, lecz XA/audio filmu, sprzętowy wzorzec pikseli, pełna zgodność SDK headless, runtime całego produkcyjnego entrypointu, kampania i usunięcie pc/ nadal wymagają pracy. Hi-res pozostałych scen i późniejsze hi-FPS nie są przez te testy dowiedzione.

Oryginalny menu_memcard/video_playback.c i jego współdzielone istniejące .inc czytają teraz STR bezpośrednio przez oryginalny czytnik surowych sektorów CUE/BIN. Usunięto negocjację LIBDS, pętle oczekiwania/restartu napędu i przerwania DMA CD. Istniejący rekord pliku używa PeDiscFile, a bufor strumienia przechowuje nagłówek i złożony payload klatki. Zachowano oryginalny VLC, wybór koloru, wymiary, licznik, flagę końca i fade volume. Walidowane są kolejność/zgodność części, liczba sektorów, długość, wymiary, zakres licznika oraz wielkość deklarowanego wyjścia MDEC względem rzeczywistej areny 0xFA00. EOF/błąd przerywa pracę zamiast nieograniczonej próby restartu. PlayVideo obsługuje nieudane otwarcie i odtwarza zapisany ekran przy zakończeniu/pominięciu; StopVideo nie przepełnia nieaktywnego bajtu. Kanoniczny CdlATV g_AkaoCdVolume ma backing w oryginalnym voice_driver.c. Nie dodano plików ani warstwy odtwarzacza w pc/.

Istniejący fixture porównuje wszystkie 190 rzeczywistych złożonych klatek z 21 filmów Disc 1 z niezależnym odczytem sektorów użytym do oracle VLC, wraz z geometrią RGB16/RGB24, licznikiem i zakończeniem. Obejmuje 15 błędnych nagłówków/metadanych, zakres obrazu, niepełną klatkę, EOF, brak zamontowanego obrazu i zwolniony bufor. Nieudane PlayVideo wraca bez pozostawienia movie-active lub flagi 0x200. Pełny build, 35 original-game-* oraz świeże ASan/UBSan/function aktorów/statystyk przeszły headless; po końcowej walidacji zakresu licznika ponowiono assets i sanitizer aktorów. Próba linkowania MenuCard_RunListScreen z oryginalnymi obiektami i istniejącym fixture zakończyła się sukcesem bez brakujących symboli i bez atrap DS. To nie jest jeszcze linkowanie/uruchomienie pełnego produkcyjnego entrypointu.

Podczas integracji potwierdzono dodatkowy rzeczywisty brak: PsyZ src/psyz/libpress.c nadal zawiera NOT_IMPLEMENTED dla MDEC_reset/in/out/sync/status. Dlatego nie uznajemy filmu ani pełnego menu runtime za działające na podstawie linkowania. Sektory XA są obecnie pomijane podczas składania wideo; audio i synchronizacja całego filmu pozostają otwarte. Należy uzupełnić MDEC we własnym forku PsyZ oraz podłączyć i zweryfikować XA, bez zastępowania oryginalnego odtwarzacza implementacją pc/movie.c. Pełna gra, oba dyski jako jeden zestaw i usunięcie pc/ nadal są niezakończone.

Cały istniejący mdec_diag/vlc_c.c jest teraz natywnym C11 zamiast inline assemblera MIPS. Zachowuje obie funkcje DecDCTvlcSize2/DecDCTvlc2, tablice VLC, DC wersji 2/3, trzy predyktory wersji 3, AC/escape/EOB, padding końca i oryginalne granice zawieszania/wznawiania dekodowania. Wskaźniki zapisanego stanu są natywne; odczyt/zapis danych jest jawnie little-endian. Dodano odrzucanie NULL, wznowienia bez zawieszonej pracy oraz przekroczenia deklarowanego obszaru wyjściowego. API nadal nie przyjmuje długości bufora wejściowego: to nie jest dowód bezpieczeństwa dowolnych uciętych/uszkodzonych strumieni.

Z rzeczywistego Disc 1 odtworzono 190 klatek STR: wszystkie 150 FMV000 i pierwsze dwie z pozostałych 20 filmów FMV1. Całe wyjściowe strumienie MDEC i wartości powrotu porównano z prawdziwym USA 0x8010C89C w Unicorn MIPS; dodatkowo pierwsze dwie klatki każdego filmu sprawdzono przy limitach 2/17/256, po każdym z 36 710 wywołań incremental. Nie hookowano dekodera. Trwały istniejący fixture czyta surowe sektory przez oryginalny moduł obrazu, pomija przeplatane sektory XA, składa klatki i sprawdza 21 pełnych hashy, liczbę wznowień, finalne obrazy buforów i wartowniki. Kontrolowana klatka V3 obejmuje rozmiary DC 0..8, znaki, trzy predyktory i AC escapes; pełny wynik i 1/55/10/1 wywołań dla limitów 0/2/17/256 odpowiadają MIPS. To test strumienia współczynników, nie porównanie końcowych pikseli MDEC ani audio filmu.

MenuCard_PlayVideo przestał odczytywać i kopiować wykonywalny overlay PS1 oraz wywoływać FlushCache. Istniejący moduł posiada wyrównaną statyczną arenę 0x55E00 o oryginalnym podziale; odrzuca nieprawidłowy indeks i konflikt aktywnego filmu. Odbiór klatki nie kontroluje pokrywy napędu. OpenVideo resetuje limit VLC, który wcześniej resetowało ponowne ładowanie kodu overlayu. Nie dodano plików ani równoległego odtwarzacza. Cały build, 35 original-game-* i świeże ASan/UBSan/function aktorów/statystyk przeszły headless; po końcowych poprawkach ponowiono test assets i sanitizer aktorów. Próba linkowania pełnego menu potwierdziła usunięcie zależności VLC, FlushCache, buforów ładowania i tabeli LBA; nadal brakuje sześciu funkcji DS (Control/ControlB/QueueLen/Read2/SearchFile/SystemStatus), kanonicznego AkaoCdVolume i CdStreamReady. Pełne odtwarzanie filmów, runtime menu, kampania oraz usunięcie pc/ pozostają niezakończone.

Do natywnego buildu dołączył cały oryginalny mdec_diag/vlc_build.c. Jego oryginalny algorytm rozwija teraz prawdziwe 3597 bajtów skompresowanej tablicy USA z PE.IMG bank 67 (sektory 927..965), zapisanych w tym istniejącym module. Usunięto używany wyłącznie do zewnętrznej historycznej etykiety vlc_build_externs.h oraz komentarze wymuszające zachowanie dawnego kompilatora; nie dodano plików ani alternatywnego dekodera. Istniejący test porównuje pełne 69 632 bajty rozwiniętej tablicy przez FNV-1a 62fd6458f3443095 z wykonaniem prawdziwego USA 0x8010BD4C w Unicorn MIPS, bez hooków procedury, i sprawdza wartowniki przed/za buforem. Pełny build, wszystkie 35 original-game-* i świeży ASan/UBSan/function aktorów przeszły headless z wymaganym Disc 1. Przywrócenie tablicy nie oznacza działania DecDCTvlc2: natywny dekoder, LIBDS/strumień i usunięcie ładowania kodu overlayu z PlayVideo nadal wymagają pracy. Odtwarzanie filmów, pełna gra i usunięcie pc/ pozostają niezakończone.

Cały istniejący menu_memcard/video_playback.c dołączył do natywnego buildu, bez korzystania z pc/movie.c i bez nowych plików. Movie-active, wide, frame i index używają kanonicznego GameState zamiast niezależnych kopii historycznych etykiet. Nagłówek klatki używa StHEADER SDK, prostokąty RECT, a callback MDEC ma natywny typ funkcji. Przywrócono 47 oryginalnych rekordów filmów i ich nazwy; wszystkie pola i nazwy test porównuje z rzeczywistym bankiem USA Disc 1. Usunięto dekompilacyjny video_playback_externs.h i komentarz wymagający odtwarzania rejestru MIPS.

Headless test wykonuje oryginalne InitVideoBuffers, SetVideoDisplay i ResetDisplayState: jeden/dwa obszary pamięci, obie gałęzie zachowania grafiki, oba bufory i RGB16/RGB24. Sprawdza layout wskaźników, kanoniczny stan, odrzucenie nieprawidłowych parametrów, niezmienność aren i całe 1024×512 VRAM po backupie oraz po odtworzeniu nadpisanych regionów. Pełne 35 testów original-game przeszło; po poprawce obliczania wskaźnika w nowym teście ponownie przeszły test assets i świeży ASan/UBSan/function aktorów, a sanitizer statystyk również przeszedł. To nie jest dowód dekodowania ani dźwięku filmu. MenuCard_PlayVideo nadal zawiera ładowanie overlayu PS1; brakujące native LIBDS/VLC i pełne linkowanie/runtime menu wymagają dalszej pracy. Katalog pc/ i pełna gra pozostają niezakończone.

Wiążąca zasada: port powstaje przez naprawę całych oryginalnych modułów w ich istniejących plikach. Nie dodajemy nowych plików, nie wydzielamy pojedynczych funkcji i nie budujemy równoległej implementacji gry ani nowych warstw abstrakcji. Katalog `pc/` jest pozostałością błędnego kierunku i ma zostać całkowicie usunięty, bez przenoszenia go pod inną nazwę. Jego użycia trzeba zastąpić działającymi oryginalnymi modułami; samo usunięcie katalogu nie oznacza ukończenia portu. Kryterium pozostaje wierność oryginalnej rozgrywce i obrazu, sprawdzana samodzielnie przez sterowanie stanem gry i inputem oraz obserwację wyników, preferencyjnie headless. Zgodność bajtowa z EXE nie jest wymagana. Zachowujemy niezacommitowane zmiany użytkownika podczas porządkowania.

Cały oryginalny battle/attack.c dołączył do natywnego buildu. Battle_ApplySpellEffect korzysta z kanonicznych g_ActiveActor, D_8009D254, flags i move lock. Usunięto używany tylko do historycznego dostępu przez rekordy battle_word_records.h; semantyczny czterobajtowy wynik Scan pozostał zwykłym typem w battle.h. Wspólny SavePromptBlock przechowuje teraz zarówno rzeczywisty cel Scan, jak i słowo wyniku widziane przez istniejący panel informacji, bez odrębnych kopii etykiet. Przywrócono prawdziwe 20 action sound IDs z USA 0x80010760 i porównano pełne 40 bajtów z EXE. Operacje PE i junk używają jawnego 32-bitowego wrap zamiast signed overflow; Haste operuje na rzeczywistym słowie statusu zamiast aliasować je niepowiązanym typem struct.

Wykonano 81 920 przypadków oryginalnego dispatchera Battle_ApplySpellEffect: wszystkie 20 numerów, wszystkie kombinacje ośmiu bitów ailment, redukcja kosztu pancerza, cztery stany HP (w tym wrap u16), energia 0/pełna/INT32_MIN, cztery wzorce odporności celu i dwa seedy. HP, PE, statusy gracza/celu, wynik i cel Scan, move lock, timer Haste, zmiana animacji Confuse oraz końcowy RNG zgadzają się z wykonaniem USA 0x80024250 wraz z prawdziwym Entity_SetActionMode 0x8001A680 w Unicorn MIPS. Jedyny hook emuluje BIOS rand; animacja ma poprawny kontrolowany nagłówek dziewięciu klatek, nie jest to dowód działania wszystkich modeli gry. Test dotyczy kontekstu walki: ścieżka przestrzennego audio rzucania PE poza walką, item effects, pozostałe funkcje attack.c i pełne animacje ofensywnych technik nadal wymagają wykonania. Zgodność 20 numerów dispatchera nie oznacza ukończenia wszystkich technik w całej pętli walki.

Build, 35 testów original-game-* oraz świeże ASan/UBSan/function fixture aktorów i statystyk przeszły headless z Disc 1. Nie dodano plików ani funkcji produkcyjnych. Aktualna próba linkowania pełnego menu wskazuje już MenuCard_PlayVideo; po dołączeniu całego oryginalnego odtwarzacza mogą ujawnić się dalsze zależności. Nie jest to jeszcze pełny boot/menu runtime, kampania ani usunięcie pc/.

Cały istniejący battle/status_hud.c dołączył do buildu. Oryginalna inicjalizacja Battle_DrawHPBar ma kanoniczne pule pakietów dla obu buforów, a G4/TILE/F3/LINE mają natywne nagłówki. Draw_InitDrawModeSprtPacket/TilePacket w istniejącym textbox.c inicjalizują prawdziwe natywne rekordy i wykonują oryginalny MargePrim; nie adresują sprite'a przez stałe +8 PS1. Usunięto niewykorzystywany include global_slot.h oraz dawne przesunięte widoki HP digits/background tiles; inicjalizacja i aktywny HUD używają rzeczywistych członków paged packets. Wspólny BattleHudOrigin zastąpił odrębne skalarne etykiety X/Y. Wybór trybu zachowuje oryginalne rzutowanie do u8, ale odrzuca nieprawidłowe indeksy tabeli zamiast czytać poza nią. AddPrim ma deklarację zgodną z rzeczywistym SDK.

Pełne payloady wszystkich 226 pakietów po oryginalnej inicjalizacji obu buforów zgadzają się z wykonaniem USA 0x80030894 w Unicorn MIPS na resident danych Disc 1. Test zatruwa pule przed init i sprawdza natywne długości scalonych/niescalonych pakietów. Draw_StatusIcons nie wylicza już wskaźników przez int i stride PS1: działa na natywnym poolu i OT. Wszystkie 65 536 list (oba bufory × cztery tryby × wszystkie kombinacje 13-bitowego statusu) mają identyczną kolejność ikon i pełne sprite payloads jak oryginalna funkcja 0x800334AC.

Oryginalny Battle_DrawActiveStatus razem z Battle_DrawEnemyHP i Draw_StatusIcons wykonano dla 19 200 kombinacji bufora, walki, widoczności PE, trybu położenia, liczników ATB/PE, 0..9999 HP, statusu i klatek pulsowania. Pełne uporządkowane listy wszystkich warstw OT, payloads oraz zmiany liczników zgadzają się z rzeczywistym USA 0x80033A40. Uwzględniono dolne/górne clampy PE i przepełniony ATB. Testy i wzorce MIPS nie hookują tych funkcji i nie kopiują ich algorytmów jako oracle. Test nie jest jeszcze pixel reference całego HUD-u ani wejściem w prawdziwą walkę. Pozostałe floating panels, spell/target overlays i część dalszych metod status_hud.c nadal mają historyczne offsety i wymagają naprawy; sam build całego modułu nie oznacza ich poprawności.

Build, 35 testów original-game-* oraz świeże ASan/UBSan/function fixture aktorów i statystyk przeszły headless z wymaganym Disc 1. Nie dodano plików ani funkcji produkcyjnych. Próba linkowania pełnego menu ma obecnie trzy braki: Battle_ApplyDamage, Battle_ApplySpellEffect i MenuCard_PlayVideo; włączenie ich całych modułów może ujawnić dalsze zależności. Pełny boot/menu runtime, walka/kampania i usunięcie pc/ pozostają niezakończone.

Cały istniejący draw/color_quad.c dołączył do buildu po naprawie native GPU ABI. Oryginalny Draw_AllocColorQuad nadal tworzy dwa gradientowe POLY_G4, półprzezroczysty TILE tła i komendę trybu, z zachowaniem kolorów, geometrii, proporcji wypełnienia i obu przesunięć Y zależnych od walki. G4/TILE mają natywne address/length; linkowanie nie obcina wskaźników do 24 bitów i nie nadpisuje długości OT. Wstępna kontrola wymaga miejsca na cały łańcuch i prawidłowego wyrównania; przy braku miejsca nie zmienia bufora, OT ani kolorów renderera. Native packet layouts mają assertions. Pakowane textbox tables nadal jawnie używają swojego dotychczasowego 32-bitowego RenderFlatTilePacket, bez zmiany ich stride; cały textbox/wipe nie jest jeszcze przeniesiony do natywnego ABI.

Wszystkie cztery GPU payloads porównano przez hash z 176 wykonaniami rzeczywistej USA funkcji 0x80061044 w Unicorn MIPS (bez hooków): oba położenia, dodatnie/zerowe/ujemne wymiary i skrajne półsłowa. Osobno test sprawdza pełne natywne adresy, długości, zachowanie OT, granice areny i brak mutacji po odrzuceniu rysowania. Ten sam rzeczywisty łańcuch przechodzi przez renderer PsyZ headless dla sześciu klatek: odczyt pełnego VRAM 320×240 potwierdza obszar wskaźnika, półprzezroczyste tło, wypełnienie i brak pikseli poza nim. To dokładny oracle payloadów oraz kontrola obszaru renderowania, nie pixel reference wszystkich interpolowanych kolorów całego HUD-u.

Build, 35 testów original-game-* z wymaganym Disc 1 i świeże ASan/UBSan/function fixture aktorów i statystyk przeszły. Nie dodano plików ani funkcji produkcyjnych. Próba połączenia pełnej pętli title/menu ma pięć pozostałych braków: Battle_ApplyDamage, Battle_ApplySpellEffect, Battle_DrawActiveStatus, Battle_SetWindowBoundsByMode i MenuCard_PlayVideo. Dalszy HUD, pełna pętla menu, lokacje/kampania i usunięcie pc/ pozostają niezakończone.

Do natywnego buildu dołączyły całe istniejące menu/equip_select.c, menu/item_actions.c i menu/item_use_panel.c. Poprawiono ich callback ABI, parametry i zmienne trzymające natywne wskaźniki; usunięto dawny undef wspólnych statystyk oraz zduplikowaną inline kopię Menu_CreateEquipItemSelectionView, używając zachowanej definicji w equip_select.c. Przywrócono istniejące scratch/snapshot records i stat-bar backing w ich modułach. HP, EXP, level, pojemność, PE flags, BP i stat levels mają aliasy kanonicznego AyaSaveState. Trzy kontury rename/preview porównano w całości z EXE; zestaw testuje obecnie 23 tablice UI/sortowania.

Układ USA 0x800A1D9C..0x800A1F84 jest teraz pojedynczą unią danych: 100-elementowy packed list nakłada się od +0xA8 na fallback ammunition record, po którym od +0xC8 następuje dziewięć specjalnych rekordów. Pozwala to zachować oryginalne widoki wspólnej pamięci i poprawne przesuwanie wskaźnika fallback→special ammo w ramach jednej tablicy rekordów. Static assertions i test sprawdzają rozmiar, offsety oraz wspólne adresy. To układ istniejących danych, bez nowej warstwy obsługi ekwipunku.

Test przechodzi przez rzeczywiste zapamiętanie/odtworzenie dwóch rekordów, Inv_BuildDisplayFromList, Inv_SpendAmmo, Inv_StepScrollDisplay i BattleCmd_UndoPending. Przeniesienie jest najpierw tylko podglądem, zatwierdzenie zmienia runtime i zapisuje natywne linki do rollbacku, a cofnięcie odtwarza obie ilości i opcjonalny pool. Usunięto zapis przez NULL do nieobecnego trzeciego rekordu. Osobno oryginalny item_use_panel wybiera przedmiot z filtrowanej listy, zapisuje poprzednią broń w komendzie i pozwala ją przywrócić. Test odbywa się bez aktywnego field actor; synchronizacja wyposażenia w scenie nadal wymaga dowodu. Pełne obrazy dwóch rekordów oraz wyniki 4032 wariantów Inv_SpendAmmo zgadzają się z rzeczywistym wykonaniem USA 0x80056B24 w Unicorn MIPS, bez hooków.

Build, 35 testów original-game-* z wymaganym Disc 1 i świeże ASan/UBSan/function statystyk oraz aktorów przeszły headless. Nie dodano plików ani funkcji produkcyjnych. Próba linkowania pełnego title/menu loop ma sześć pozostałych braków: Battle_ApplyDamage, Battle_ApplySpellEffect, Battle_DrawActiveStatus, Battle_SetWindowBoundsByMode, Draw_AllocColorQuad i MenuCard_PlayVideo. Nowo dołączone ekrany nie mają jeszcze kompletnego przebiegu render/input; pełny runtime menu, lokacje/kampania i usunięcie pc/ pozostają niezakończone.

W istniejących modułach przywrócono 20 zestawów oryginalnych danych USA: sześć tabel rankingu sortowania, pięć konturów paneli, stany akcji przedmiotów, teksty pomocy, glify wyposażenia, cztery tablice glifów sortowania, toggle glyphs i osiem zakresów kategorii przedmiotów. Test odczytuje odpowiadające im zakresy EXE bezpośrednio z Disc 1 i porównuje komplet danych. Glify trybów wyposażenia 1/2 mają dwa rzeczywiste wiersze; usunięto bias etykiety PS1 w poprzedni kontur panelu, normalizując indeks do mode-1.

Inv_CompareItemsForSort sprawdza NULL przed odczytem rodzaju i podtypu, zamiast czytać niską pamięć jak kod PS1. Dla poprawnych rekordów 64 800 wyników (90 ID × 90 ID × cztery rankingi × dwa rankingi podtypów) zgadza się z wykonaniem oryginalnego USA comparatora 0x8005AFFC i lookupu 0x800532B4 w Unicorn MIPS, bez hooków i bez kopii comparatora jako oracle. Osobno sprawdzono bezpieczne porównania pustych slotów. Osiem wariantów pełnych oryginalnych operacji sortowania obu schowków i ekwipunku zachowuje multizbiór ID, powiązanie założonej broni/pancerza i poprawne indeksy listy transferu. To weryfikacja comparatora i niezmienników pełnego sortowania, nie hash całego porządku ze stdlib qsort ani renderowania menu.

Build, 35 testów original-game-* z wymaganym Disc 1 oraz świeże ASan/UBSan/function fixture statystyk i aktorów przeszły headless. Nie dodano plików ani funkcji produkcyjnych. Pełne menu nadal wymaga pozostałych ekranów, callbacków battle HUD/effects i movie playera; jego pełny link/runtime, kampania oraz usunięcie pc/ pozostają niezakończone.

Do buildu dołączyły całe istniejące item/swap.c, menu/skills.c, menu/equip_screen.c i menu/main_menu.c. Callbacki inputu mają właściwe natywne sygnatury node/unsigned flags, a callbacki rysowania nie przyjmują już adresów widgetów jako int. W całym swap.c usunięto dawne przesunięte bazy rekordów przedmiotów; Inv_LookupItemData ma zgodny z konsumentami typ ItemDataRecord*. Lista transferu aliasuje drugie 50 elementów g_InvPackedList, lista aktywna ma rzeczywiste 10 rekordów, a specjalny schowek wskazuje SaveRuntimeState. Przywrócono backing istniejącego 128-byte bufora etykiet, czterech kosztów kategorii i jednego MenuBonusStatWorkspace z trzema widokami dziewięciu słów. Scalar labels wyposażenia aliasują te same bajty co array labels.

Test prawdziwych widgetów obejmuje 64 przebiegi regulacji Y (16 pozycji × dwa kierunki × confirm/cancel), zachowanie bez inputu i granice 0/15. Osiem kombinacji flag sprawdza oryginalny Menu_DeferredExec i wstrzymanie polling. Test resident inventory wybiera oba oryginalne schowki, weryfikuje wszystkie kategorie lookupu i pełną dziesięcioelementową listę aktywną. Inv_SetActiveList przyjmuje natywny argument zależny od trybu; porównanie amunicji nie pakuje już wskaźnika do int. Sam tryb 5 przetestowano na aktywnym Combatant dla 22 rodzajów i pięciu wartości amunicji, z zachowaniem pozostałych bitów. Cały callback porównywania/rollback nadal wymaga dalszej weryfikacji. Build, 35 testów original-game-* i świeże ASan/UBSan/function aktorów i statystyk przeszły headless z Disc 1. Nie dodano plików ani funkcji produkcyjnych. Pełny title/menu loop nadal nie linkuje się: potrzebuje pozostałych ekranów, tabel UI, battle HUD/effects i movie playera. Rendering całego menu, wejście w lokację, kampania i usunięcie pc/ pozostają niezakończone.

Całe istniejące item/actions.c jest w natywnym buildzie. W nim i w całym item/active_list.c usunięto odwołania do g_EquipItemDataTable/g_KeyItemDataTable, czyli dawnych baz przesuniętych poza obiekt: wyposażenie i specjalne rekordy mają jawne poprawne indeksy, a opisy oznaczone flagą 0x10 wskazują istniejące okna metadanych zapisu. g_InvPackedList ma rzeczywisty backing 100 halfwords (USA 0x800A1D9C..0x800A1E64), nie tentative array długości 1. g_InvBaseCapacity aliasuje kanoniczną pojemność Aya przy +0x0C. Inv_RemoveActiveListItem używa natywnego wskaźnika listy, zamiast konwersji do s32; zwalnianie equipment record ograniczono do ID 0x100..0x17F, aby usunięcie specjalnego ID nie pisało poza tablicą. To naprawa bezpieczeństwa pamięci; historyczny kod EXE miał tylko dolną granicę ID.

Test resident inventory obejmuje wyszukiwanie i typy 100 mieszanych pozycji, oba warianty własnych opisów, dziewięć specjalnych opisów i bazowe opisy przedmiotów. Wszystkie 160 list wygenerowanych przez filtry (32 maski × pełny filtr/cztery wykluczenia) porównano przez hash z wykonaniem rzeczywistych funkcji USA 0x800542A0/0x800543CC w Unicorn MIPS, bez hooków i bez kopii algorytmu jako oracle. Osobno pełny filtr zapisuje wszystkie 100 indeksów, a rzeczywiste usuwanie wszystkich pozycji zwalnia właściwe rekordy wyposażenia i zachowuje specjalne. Build, 35 testów original-game-* oraz świeże ASan/UBSan/function fixture statystyk i aktorów przeszły z Disc 1 headless. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla menu nadal nie linkuje się z powodu pozostałych modułów ekranów, danych UI i odtwarzacza filmu; renderowanie całego inventory, wszystkie operacje actions.c, kampania i usunięcie pc/ pozostają niezakończone.

Oryginalna ścieżka potwierdzenia udanego odczytu działa na prawidłowym stanie New Game: Save_InitMetadataState → Save_BuildHeader → Save_BuildCardFile → Save_LoadCardFileIntoRuntime → rzeczywisty input handler dialogu → Save_CancelUiFlow → Inv_SetActiveList(12). Test zatruwa runtime przed odczytem, odtwarza cały obraz, sprawdza brak reakcji bez przycisku, potwierdzenie, zamknięcie widgetów, wyzerowanie callbacku i wynik menu 2. Przebieg odbywa się w kontekście tytułu bez field actor, jak przed spawnem sceny; nie dowodzi jeszcze przejścia do lokacji ani odtworzenia aktywnej walki.

Etykiety slotów wyposażenia i schowka Wayne’a wskazują teraz kanoniczne pola SaveRuntimeState. Dwa zapisane wybory mają prawdziwe dwuelementowe tablice zamiast odrębnych scalar definitions i nierozwiązywalnego symbolu tablicy. Wszystkie lookupy specjalnych rekordów w istniejącym armor_capacity.c oraz scaled lookup w inventory_slots.h adresują dziewięć g_KeyItemRecords przez indeks item ID minus 0x200, bez bazy poza obiektem. Testy na resident danych sprawdzają oba warianty New Game, 128 rekordów wyposażenia, dziewięć specjalnych, nieprawidłowe ID oraz zapamiętanie/przywrócenie obu list i fallback po usunięciu override. Build i 35 testów original-game-* przeszły headless z Disc 1; świeże ASan/UBSan/function obejmują fixture statystyk i aktorów. Nie dodano plików produkcyjnych. Pełna pętla menu nadal ma brakujące zależności, a usunięcie pc/ i pełna gra pozostają niezakończone.

Pełny `Save_LoadCardFileIntoRuntime` jest teraz połączony i wykonywany z oryginalnymi dialogami. Do buildu włączono całe istniejące moduły menu/save_screens.c, menu/lists.c, menu/item_detail.c, battle/turn.c i battle/enemy_actions.c; poprawiono sygnatury callbacków do natywnych wskaźników. `Asset_OpenPeImage` ładuje oba oryginalne zestawy deskryptorów widgetów z EXE, a test porównuje wszystkie rekordy z niezależnym odczytem. Usunięto inventory_category.h: liczniki kategorii są polami ammo kanonicznych g_KeyItemRecords, zgodnie z dawnym nakładaniem adresów, bez drugiej kopii stanu. Oryginalne tabele animacji/skali przeciwników i krawędzi wipe mają backing w swoich istniejących modułach; aliasy pól battle-menu zastąpiono dostępem do kanonicznego rekordu przez memcpy.

Test pełnego loadera obejmuje poprawny obraz 8 KiB, odtworzenie runtime/timerów/liczników, utworzenie powiadomienia i odroczonego callbacku, siedem uszkodzeń pliku oraz nieprawidłowy link z poprawnie przeliczonym CRC. Każdy błąd pozostawia stan gry bez zmian, tworzy prawdziwy dialog z resident tekstem i pozwala zamknąć go przez oryginalny input handler. Ponowny poprawny odczyt działa bez zmiany bajtów pliku. Build, 35 testów original-game-* i świeże ASan/UBSan/function aktorów oraz loadera przeszły headless z Disc 1. Nie dodano plików ani funkcji produkcyjnych. To nie jest jeszcze pełny Continue: callback zakładania ekwipunku, renderowanie całych dialogów, walka i kampania wymagają dalszej weryfikacji. Katalog pc/ nadal istnieje i jego usunięcie pozostaje niezakończone.

Oryginalny Save_BuildCardFile tworzy pełny zgodny plik USA 0x2000. W istniejącym save_flow.c naprawiono 64-bitowy formatter SJIS: cztery liczby nie są już błędnie czytane ze stride wskaźnika, a oba authored format strings i bufor tytułu mają backing w module. g_PlayTimeFrameCounter aliasuje rzeczywisty script variable 74 (0x800A7918), bez osobnej kopii. Serializer/deserializer ogona 0x8A8 używa memcpy i jawnie konwertuje native AyaBattleState do oryginalnego 0x70-byte rekordu: omija host padding i zapisuje dwa stabilne identyfikatory rekordów, zamiast fragmentów natywnych wskaźników. Odtwarzanie wiąże je z właściwymi native obiektami. Dopuszczalne są NULL i dwa oryginalne stałe powiązania; inne linki zwracają błąd przed kopiowaniem ogona lub mutacją runtime. Save_StartWriteSlot nie rozpoczyna transferu po błędzie składania pliku.

Porównano 24 kompletne pliki z wykonaniem EXE USA 0x80040B80 w Unicorn MIPS (tylko bzero/memcpy/strcpy zastąpiono zwykłymi operacjami pamięci): oba style tytułu, trzy czasy i wszystkie cztery kombinacje NULL/canonical links. Każdy z 8192 bajtów uczestniczy w hashu, razem z prawdziwą ikoną/paletą resident, tekstem lokalizacji, runtime image i CRC. Osobno sprawdzono CRC algorytmem 16-bitowym, zapis i odczyt niewyrównanego ogona, odtworzenie wszystkich jego grup pól po zatruciu stanu, oryginalne maskowanie flags oraz odmowę nieznanego linku bez zmiany stanu/kursora/bufora. Gotowy plik przechodzi dodatkowo prawdziwy write/reopen/read/seek/update obu portów w izolowanym katalogu, bez dotykania save editora ani bu00 użytkownika. Build, 35 testów original-game-* z wymaganym Disc 1 i świeże ASan/UBSan/function aktorów (--assets) oraz loadera przeszły. Nie dodano plików ani funkcji produkcyjnych.

Save_LoadCardFileIntoRuntime poprawiono w miejscu: sprawdza CRC oryginalnego obrazu przed odtwarzaniem stanu, nie zeruje checksum w buforze, obsługuje odrzucenie linków i kasuje wcześniejszy success flag. Ta pełna funkcja, Save_RestoreHeader i wyświetlanie dialogów nadal nie mają dowodu wykonania — wymagają brakujących oryginalnych zależności UI. Powyższe testy potwierdzają producenta pliku, podstawowe I/O oraz parser ogona, nie pełny load/Continue ani prawdziwy zapis kampanii z dowolnej lokacji. Uszkodzone CRC przez całą ścieżkę UI, dalsze fault paths, pełna kampania i usunięcie pc/ pozostają niezakończone.

Próba pełnego linkowania MenuCard_RunListScreen ujawniła kolejną rzeczywistą przeszkodę: oryginalny save_flow.c wiązał open/read/write/lseek/close z host libc, zamiast z obsługą ścieżek bu00:/bu10: i flag BIOS w PsyZ. Istniejący psyq_bios.h korzysta teraz z eksportowanego psyz/system.h; usunięto ręczne niezgodne prototypy I/O i duplikaty deklaracji libc, a oryginalne targety zależą bezpośrednio od SDK. nm potwierdza symbole psyz_* w obiekcie save_flow. Dane menedżera, preview obu portów, format pliku z EXE i bufor 8 KiB mają backing w istniejącym save_flow.c. SaveStateBuffer aliasuje +0x100 bufora, a IconPalette rzeczywiste pole nagłówka +0x60, bez osobnych kopii. Pełny MemCard_InitState wykonuje się na tych danych; test zatruwa obie struktury i wszystkie resetowane globals, sprawdza pełne wyzerowanie, fd=-1 i właściwe aliasy. Poprawiono też long argumenty formatu %ld, dawny alias func_8018F2F4 na istniejący MenuCard_FlipScreenPages i pointer ABI Menu_RunFrameWithArg.

W forku khasinski/parasite-eve-psyz 631f5fd naprawiono flagi ponownego otwarcia pliku do zapisu na Unixie oraz obcinanie nazw: prawdziwy format Parasite Eve używa 20 bajtów, z literą slotu na końcu. Host path zachowuje 20 znaków i terminator, a DIRENTRY.name wszystkie 20 bajtów (bez NUL dla pełnej nazwy), także w kodzie Windows; dokumentacja SDK opisuje ten kontrakt. Nowy test w istniejącym fixture używa oryginalnego formatu nazwy i rzeczywistych usług PsyZ z izolowanym mapowaniem katalogów. Oba porty przechodzą create/close/reopen-for-write, zapis i odczyt 8192 bajtów, seek do końca, EOF, update przez read/write handle, pełną 20-bajtową nazwę i rozmiar z firstfile/nextfile oraz błędy nieistniejącego pliku i złego fd; pliki są usuwane, bez dotykania bu00 użytkownika. Build, 35 testów original-game-* z wymaganym Disc 1 i świeże ASan/UBSan/function aktorów (--assets) oraz loadera przeszły. Osobny build SDK z sanitizerami przeszedł 18 testów bu/path_adjustment/truncation w osobnym katalogu. Nie dodano plików produkcyjnych ani funkcji gry. To dowód ABI i podstawowych operacji plikowych, nie pełnego Save_BuildCardFile/CRC, menedżera polling, trwałego zapisu gry ani Continue. Ponowne pełne linkowanie ekranu nadal wykazuje brakujące oryginalne moduły menu/dialogów/battle i video. Pełna kampania i usunięcie pc/ pozostają niezakończone; Windows zmieniono zgodnie z tym samym kontraktem, ale nie zweryfikowano na tej platformie.

Całe istniejące image_nodes.c i fade.c dołączono do oryginalnego targetu. Węzły mają jeden kanoniczny MemcardImageNode z natywnymi, poprawnie typowanymi callbackami; usunięto trzy zbędne nagłówki z PS1 views i address-only entry symbols. Osiem istniejących callbacków wiąże się bezpośrednio z oryginalnymi funkcjami, bez wrapperów. BlendImage przyjmuje i ignoruje czwarty argument wspólnego ABI render callback, a callbacki create/attach przyjmują przekazywany węzeł; nieużywany return licznika normalizuje void ABI. Porównanie z MIPS wykryło rzeczywisty błąd zdekompilowanego UpdateFadeCounter: po dodatnim shift funkcja pomijała zapis intensywności, aktualizację kroku, callback i zakończenie. Oryginał 0x80192FE8 wykonuje je niezależnie od znaku; naprawiono tę funkcję w miejscu oraz signed left shift ujemnego kroku. Pule/free/pending/active należą do image_nodes.c. Brak miejsca na pięć węzłów grupy, pusta pula i zły kind nie powodują częściowej mutacji ani NULL dereference.

Deterministyczny test uruchamia oryginalne input remapping, HandleListInput, wszystkie fade callbacks i RedrawImageNodes na prawdziwym banku title. 1048 klatek z sześciu scenariuszy (bez kart, karta bez pliku, zapis na każdym porcie, usunięcie karty oraz pojawienie się zapisu w trakcie) zgadza się z wykonaniem kodu USA w Unicorn MIPS. Hash obejmuje wszystkie osiem węzłów, callback/image identities, pięć list heads/tails, timer/previous pad, dirty/overlay rectangles obu stron i każdy wygenerowany bajt aktywnego overlay. Sekwencje dochodzą do wskazania New Game (Y=160) i potwierdzenia wyboru; obejmują też pełną animację zielonego napisu i jego zanik po usunięciu karty. MemCard_HasAnyPresentFile w istniejącym save_flow.c korzysta teraz z pól obu natywnych portów i wszystkich 15 slotów, zamiast 0x418/0x830; ostatni slot każdego portu uczestniczy w porównaniu. Build, 35 testów original-game-* z wymaganym Disc 1 i świeże ASan/UBSan/function aktorów (--assets) oraz loadera scen przeszły; rozszerzone sześć sekwencji aktorów także ponowiono z sanitizerami. Nie dodano plików ani funkcji produkcyjnych. Test nie uruchamia całego MenuCard_RunListScreen: obsługa list i podawanie inputu są kontrolowane przez fixture, dźwięki mają niezaładowany BGM handle, a porównanie animowanych napisów dotyczy CPU overlay bytes, nie pełnego wyświetlonego framebuffera. Movie/card polling, rzeczywisty New Game/Continue, pełna kampania i usunięcie pc/ pozostają niezakończone.

Całe istniejące list_screen.c i page_flip.c kompilują się w oryginalnym targecie. Ekran ma natywne bufory DRAWENV/DISPENV, 4096 pełnych tagów OT na stronę, areny pakietów, overlay i osobne miejsce na konwersję RGB24→RGB555 (z zachowanym dodatkowym retail group). Usunięto PS1 offsety 0x80/0x4080/0x1C080, obcinanie adresów free list i oddzielne symbol views tych samych wskaźników. Tabela obrazów i presets odnoszą się do rzeczywistych danych w już załadowanym banku 70, bez kopiowania frontendowych implementacji. Test uruchamia cały oryginalny MenuCard_ShowLogoFade: 480 callbacków VSync porównuje wszystkie piksele obu framebufferów z niezależnym odczytem 4-bitowego TIM/CLUT i regułą modulacji, także per-page overlay uploads. MenuCard_FlipScreenPages przywraca oba kompletne tła RGB24 zgodnie z bankiem. Oryginalny EXE 0x80190660 rysuje w tej pętli pierwszy sprite; mimo inicjalizacji drugiego zachowano rzeczywistą sekwencję wywołań.

Test wykrył rzeczywisty błąd PsyZ DrawPrim: cwb traktował packed u32 payload jak u_long i wszystkie komendy jak prymitywy. Fork khasinski/parasite-eve-psyz a980b49 normalizuje pakiety jak istniejąca ścieżka OT i wykonuje komendy E1/E3/E4. PSYZ_HEADLESS=1 podłącza istniejący rasteryzator programowy bez okna i GPU device, z przyspieszonym deterministycznym VSync; CTest original-game-* korzysta teraz z tego trybu i dummy audio. Build, wszystkie 35 testów z wymaganym Disc 1 oraz świeże ASan/UBSan/function aktorów (--assets) i loadera przeszły. Osobny pełny build PsyZ z sanitizerami przeszedł siedem wybranych testów: nowy DrawPrim z TPAGE/AREA i packed sprite, ReadTIM, zachowanie VRAM przy display mask oraz cztery testy callbacków VSync. Nie dodano plików ani funkcji produkcyjnych gry; fork dodaje jedynie wewnętrzne rozpoznanie wybranego backendu. To dowód startowego logo/page flips w natywnym 1× headless; pełne MenuCard_RunListScreen, node/fade callbacks, Continue/New Game, film, hi-res i GPU backend po poprawce DrawPrim nadal wymagają dalszych dowodów. Zrzuty frontbuffer przez API window capture nie są objęte tą integracją; obserwacja testu używa StoreImage. Pełna kampania i usunięcie pc/ pozostają niezakończone.

Oryginalny CdRom_LoadInitialImage w istniejącym cdrom/scene_assets.c ładuje pełny bank startowego ekranu title/card: 133 sektory (272384 bajty) z resident range 70. g_StrFileDirLba aliasuje kanoniczną tablicę sektorów, zamiast pozostawać niezdefiniowanym osobnym symbolem. Zachowano reset sześciu kluczy/banków audio, enqueue F0, flagi i przejście do 320×240 RGB24. Fizyczne retry loops zastąpiono zwracaniem błędu odczytu/poll; walidowane są NULL, pusty/nadmierny extent i overflow LBA. Istniejący main propaguje błąd ładowania ekranu i błąd montowania zamiast ponawiać je bez końca. Test używa produkcyjnego bufora: cały bank zgadza się z niezależnym odczytem obrazu, guards pozostają niezmienione, a brak obrazu i ucięty archive kończą się błędem; po błędach poprawny odczyt działa. Sprawdzono także queue F0, reset banków/kluczy, flagi i rzeczywisty GetDispEnv. Build, 35 testów original-game-* z wymaganym Disc 1, świeże ASan/UBSan/function loadera i C11 syntax check main przeszły. Nie dodano plików ani funkcji produkcyjnych. To ładowanie danych i ustawienie wyświetlania; nie dowód wykonania MenuCard_RunListScreen, odtwarzania filmu ani pełnego linkowania/uruchomienia main. Nie wymuszono osobno błędu poll synchronicznego backendu. Integracja oryginalnego ekranu, pełna kampania, oba obrazy jako jeden zestaw i usunięcie pc/ pozostają niezakończone.

Oryginalny Draw_AllocTexturedRectAlt (cały pasek statystyki w istniejącym draw/text.c) przeszedł porównanie pełnej sekwencji pakietów i końcowego stanu z wykonaniem EXE USA 0x8006062C: 7344 kombinacje 12 wartości (w tym INT_MIN/INT_MAX), szerokości -1..49, trzech pozycji, dwóch fontów i dim. Porównanie obejmuje cyfry, znak minus, sprite podkładu, wszystkie segmenty, końcowy draw page, kolory i przywrócenie kursora/stosu. Naprawiono NULL dereference po braku miejsca na segment, sprawdzanie rozmiaru alokacji i negację INT_MIN z zachowaniem zachowania MIPS. Usunięto nieużywany 32-bitowy union linku. Dodatkowe 97 limitów wolnego miejsca kontroluje granice areny i pełne łańcuchy OT także przy częściowo odrzuconym pasku. Build, wszystkie 35 testów original-game-* z wymaganym Disc 1 oraz świeże ASan/UBSan/function aktorów (--assets) i loadera scen przeszły. Nie dodano plików ani funkcji produkcyjnych. Nadal nie jest to dowód obrazu całego interfejsu ani działającej kampanii; oryginalny main/overlaye i usunięcie pc/ pozostają niezakończone.

Naprawiono istniejący emiter Draw_AllocSprite w draw/text.c oraz formaty sprite/page w dotychczasowych nagłówkach: pełne natywne adresy OT, osobne długości i rozmiary zgodne z PsyZ, bez zmiany oryginalnego payloadu obrazu. Rezerwacja pary sprite/page jest atomowa; brak miejsca nie zmienia areny ani OT. Istniejące łączenie pakietów wipe i końcowego page paska także zachowuje pełne adresy. Test na rzeczywistych danych porównuje payloady wszystkich 156 deskryptorów sprite w trzech pozycjach i dwóch stanach dim z wykonaniem EXE USA; pełna macierz tekstu ma teraz 9216 przypadków, włącznie z page 1/code 0 wywołującym sprite. Pozostałe 2808 przypadków ikon i 96 cyfr nadal przechodzą. Wszystkie 35 testów original-game-* z wymaganym Disc 1 oraz świeże ASan/UBSan/function aktorów (--assets) i loadera scen przeszły. Nie dodano plików ani funkcji produkcyjnych. To dowód payloadów i powiązań objętych testem, nie kompletnego renderera: textbox, pozostałe pakiety wipe/battle, pełny pasek, pixel reference sceny, pełna gra i usunięcie pc/ nadal wymagają pracy.

W istniejącym render_prim.h RenderTexturedQuad ma native tag address/length i zachowany 36-bajtowy payload FT4; assertions uwzględniają nowe offsets i wyrównanie. Oryginalny draw/text.c rezerwuje sizeof packet zamiast stałych 40 bajtów w emiterze cyfr; emitery quadów ikon/tekstu i quad slices paska zapisują pełne OT links. Przy braku miejsca emitery cyfr/ikon/tekstu nie dereferencjonują NULL. Test cyfr porównuje 96 payloadów i native chains dla obu fontów/dim, negative values, modulo 10, INT_MAX oraz zawijania x/y z wykonaniem EXE USA 0x8005F844/0x8005F874 w MIPS. Test real resident data porównuje 2808 payloadów ikon (156 deskryptorów × 3 modes × 3 pozycje × dim) z EXE 0x8005ED18 oraz 9210 przypadków stron fontu/control bytes/cursor/page/advance z 0x8005EED4. Pominięto wyłącznie page 1/code 0 delegujące do osobnej, nadal nienaprawionej ścieżki sprite. Kontrolowane są też długości tagów, arena guards i odmowa allocation na granicy. Build, 35 CTest bez SKIP oraz świeże ASan/UBSan/function aktorów z --assets (obejmujące nowe przypadki) i pełnego fixture loadera przeszły. Nie dodano plików ani funkcji produkcyjnych. Arytmetyka glyph geometry/UV/colour/cursor pozostała oryginalna, bez zastępczego renderera. Shared FT4 layout dotyczy także przyszłych konsumentów battle panels, ale ich pełne fazy nie są jeszcze objęte tym dowodem. Sprite emitter, draw-mode/wipe/bar packets, pixel reference renderowanej sceny, pełna gra i usunięcie `pc/` pozostają niezakończone.

Naprawiono wspólny format GpuCmdPacket do rzeczywistego PsyZ DR_AREA/DR_TWIN: osobne native address/length i dwa native command words, ze static assertions rozmiaru/payload offset. Dotychczasowy 12-bajtowy rekord był zbyt mały dla stores SDK. Istniejący Draw_LinkDrawArea zachowuje pełny adres i długości zamiast maskować 24 bity; oryginalne Draw_AllocPrimRect/Full korzystają z tego samego poprawionego łączenia co widget clipping. NULL po przepełnieniu areny nie uszkadza OT. Test wykonuje trzy oryginalne ścieżki dla obu buforów, w tym parent-derived widget position, weryfikuje E3/E4 command payloads, kolejność wszystkich trzech native links, długości i granice całej areny; obejmuje dokładnie odrzucony allocation na końcu 16 KiB. Build, 35 CTest z wymaganym Disc 1 i świeże ASan/UBSan/function fixture aktorów oraz pełnego loadera przeszły. Main.c przechodzi C11 syntax check; jego dwa stare address-only entry names zastąpiono istniejącymi nazwami FxCommon_RunFrameLoop i MenuCard_RunListScreen, zgodnie z naming_review.csv i definicjami oryginalnych funkcji, bez wrapperów. Skorygowano opis poprzedniego loadera na zasoby startowe, nie samo menu. Nie dodano plików ani funkcji produkcyjnych. Pozostałe pakiety tekstu/wipe i native layout modułu menu_memcard nadal wymagają naprawy; to dowód poprawnych clipping commands i links, nie pixel reference renderowanej sceny ani pełnej pętli gry. Usunięcie `pc/` pozostaje niezakończone.

Oryginalny Boot_LoadOverlayTables przeszedł oba warianty flagi 0x2000: audio bank 204, trzy główne TIM calls, 262 TIM calls z banku 73 lub 74 i directory bank 75. Wszystkie słowa VRAM porównano z niezależnym odczytem sektorów/TIM, a directory bytes z obrazem. Trzeci wpis głównej tabeli wskazuje w retail na obraz o zerowej wysokości; zachowano wywołanie i jego no-op, zamiast odrzucać je jako błędny magic. Sześć historycznych symboli sektorów aliasuje kanoniczną tablicę (67,71..75), bez osobnych kopii. Loader nie ponawia I/O bez końca i zwraca etap błędu do oryginalnego main; usunięto cache flush po surowym banku, który pozostaje danymi, nie kodem hosta. Test braku obrazu wykrył kolejną pętlę w blokującym CdRom_ReadAssetSectors: oryginalna maszyna chunk/transfer kończy teraz błędy odczytu/poll/upload, resetuje fazę i odrzuca nieprawidłowy kind/index/buffer/limit oraz nieznaną fazę. Boundary read korzysta bezpośrednio z entry[1], bez nieprawidłowo wyrównanego pośredniego SceneSectorDirectory. Test obejmuje brak obrazu, extent, nieprawidłowe argumenty/fazę, uszkodzony sample header po nieblokującym odczycie i udane ponowne ładowanie obu wariantów. Build, 35 CTest z wymaganym Disc 1, świeży ASan/UBSan/function całego fixture loadera i native C11 syntax check main.c przeszły. Nie dodano plików ani funkcji produkcyjnych. To ładowanie zasobów startowych i porównanie VRAM; ten loader poprzedza pętlę fx_common, więc nie stanowi dowodu wykonania menu/pixel reference wyświetlonego obrazu; walidacja wszystkich uszkodzonych TIM i pozostałe fault paths, pełna kampania, wspólny zestaw obu płyt i usunięcie `pc/` pozostają niezakończone.

Pamięć bootu należy teraz do istniejącego boot/init.c, zamiast fixture: trzy niezależne native work areas, scene scratch i loaded-scene/directory arena z zachowanym nagłówkiem -8. Oryginalne Boot_InitMemoryLayout i Draw_InitDisplay nadal wyznaczają swoje pola; usunięto definicje buforów i directory pointer z testu aktorów. Real-asset test wykonuje po kolei Boot_InitSystem, Boot_InitGameState, Boot_InitSubsystems i Scene_LoadSceneData, korzystając z produkcyjnego scratch/bank backing, bez wstrzykiwania ich wskaźników. Przy usuwaniu kolejnej atrapy odkryto, że voice-trigger table nie jest zerowa: 20 rekordów ma backing w istniejącym asset/game_state.c, z oryginalnymi czterema parami action/frame i IDs 1000/1001. Całe 160 bajtów porównano z EXE USA 0x80094488 przed init, a po init potwierdzono wyłącznie reset pierwszych czterech par IDs, bez utraty authored keys. Obie kopie tablicy z fixture usunięto. Build, 35 CTest z wymaganym Disc 1 oraz świeże ASan/UBSan/function fixture aktorów i pełnego loadera przeszły; dalsze 14 real area/day banks, VRAM comparisons i animacje nadal przechodzą po wspólnym starcie. Nie dodano plików ani funkcji produkcyjnych. Rozmiary aren obejmują sprawdzony układ native; nie jest to dowód wszystkich banków kampanii. Pełny main loop, pozostałe dane/moduły, oba dyski jako jeden zestaw i usunięcie `pc/` pozostają niezakończone.

Oryginalny `Boot_InitSystem` wykonuje GPU/GTE/SPU init, rejestrację menedżera kart i native pad reset. Boot_InitMemCard korzysta z istniejącego Boot_ResetPadInput zamiast nieobsługiwanego PadInitDirect/PadStartCom; inicjalizacja fizycznego CD (DsInit/DsSetDebug) została usunięta. MemCard_InitManager zachowuje oryginalne zdarzenia i _bu_init przygotowujące katalogi hosta, bez rozruchu fizycznego InitCARD/StartCARD/_card_auto. Kanoniczne port states, event handles/flags i manager-ready należą do istniejącego save_flow.c. Critical sections wiążą się z rzeczywistymi PS1_Enter/ExitCriticalSection w PsyZ. Event handles i callback returns mają native long ABI; także callback AKAO zarejestrowany przez ten sam OpenEvent ma zgodny typ, bez zmiany tick logic. Test uruchamia pełny Boot_InitSystem i osiem rzeczywistych DeliverEvent przez dispatcher PsyZ, kontroluje właściwe flagi, unikalność handles, reset pada i idempotentną rejestrację/wyzerowanie present obu portów. Testy pracują w build/, bez dotykania bu00 użytkownika. Build, 35 CTest z wymaganym Disc 1 oraz świeży ASan/UBSan/function testu aktorów przeszły. Nie dodano plików ani funkcji produkcyjnych. To inicjalizacja systemu/zdarzeń, nie dowód zapisu/odczytu plików, recovery z błędów kart ani Continue. Próba pełnego linkowania oryginalnego main.c nadal ujawnia wiele brakujących modułów gameplay/asset/battle i danych; dawny CD notice flow wymaga dostosowania do obrazów. Pełna pętla aplikacji, kampania i usunięcie `pc/` pozostają niezakończone.

Oryginalny `Boot_InitSubsystems` wykonuje się w całości. Do buildu weszły całe istniejące draw/gpu_regs.c, draw/prim_queue.c, battle/setup.c i script/sfx_queue.c; brakujące enemy slots i frame counter mają backing w swoich oryginalnych modułach. Naprawiono per-frame RNG bez zmiany sekwencji: fizyczne adresy 0x80070DCC..0x80070E4C zastępuje jeden 33-word obiekt (14 oryginalnych instruction words jako dane algorytmu, dwa cursors i 17 seed words). Retail tail OR po zejściu poniżej zera czyta zakres -4..-64, także oba cursors i wcześniejsze instrukcje — nie zastąpiono go 17-elementowym ringiem. Hash streamu i pełnego stanu zgadza się z wykonaniem oryginalnych funkcji USA 0x80070D10/0x80070D6C w Unicorn MIPS w punktach 0/1/5/17/2000/8192 losowań; ponowny init odtwarza prefix. Pełny test subsystem start sprawdza warm-up 2000 draws, battle counters/slots, script/entity pools, zerowanie variable bank i SFX queue oraz flagę startową 0x4000. Nietypowy reset jedynie pierwszej active flag potwierdzono w EXE 0x80034F40..0x80034F4C; free-list initializer czyści wszystkie później. Reset SFX używa memset zamiast zapisu struct przez int*, a deklaracje wywołanych funkcji mają rzeczywiste void ABI. Build, 35 CTest z wymaganym Disc 1 i świeży ASan/UBSan/function testu aktorów przeszły. Nie dodano plików ani funkcji produkcyjnych. `Boot_InitSystem` nadal wymaga naprawy inicjalizacji memcard/event storage, wejścia pada i dawnych usług krytycznych/CD; nie jest to pełny boot aplikacji ani dowód wszystkich funkcji battle/setup czy SFX dispatch. Kampania i usunięcie `pc/` pozostają niezakończone.

Oryginalny `Draw_InitDisplay` wykonano dla mode 0/1/2/3 przez rzeczywiste CdRom_SetSeekPos/Render_SetCDDCSlot/Draw_RenderFrame: test sprawdza układ pamięci, dwa przebiegi fade, obie areny buforów, reset zatrutej fazy loadera oraz tokeny 0xA80830C8/0xA80651C8 (pozostałe tryby zachowują token). Przy podłączeniu naprawiono w istniejącym render/frame_update.c obcinanie pełnego adresu GPU do u32; oba pakiety fade łączą się teraz z natywnym OT bez utraty górnych bitów. SceneInitSubphase aliasuje kanoniczne pole game-state +0xEE (retail 0x800B0DC6), zamiast niezależnego niezdefiniowanego symbolu. Osobny test wykonuje oryginalny init i kroki tint dla obu slotów, czterech blend modes, stop/hold i duration 0/1/2/4/7; kontroluje RGB każdej klatki, pełne links, długości payloadu, niezmienione sąsiednie entries i naturalne zakończenie. Goldeny RGB pochodzą z wykonania EXE USA 0x80068E24 w Unicorn MIPS. Build i 35 CTest z wymaganym Disc 1 przeszły; po rozszerzeniu do czterech trybów i zatruciu fazy ponowiono oba testy aktorów oraz świeży ASan/UBSan/function. Nie dodano plików ani funkcji produkcyjnych. To dowód sterowania startem wyświetlania i pakietów/kolorów fade, nie pixel reference całej sceny, pełnego boot loop ani gry; usunięcie `pc/` pozostaje niezakończone.

Oryginalny `Scene_LoadSceneData` wykonano w całości na Disc 1: resident pack, pełny Menu_FullInit, 0x10A50 bajtów startup resources z oboma texture-page lookups, upload ADPCM/instrumentów oraz końcowy bank 0x1400. Backing startup blob należy do istniejącego room_load.c; stare kopie przez alignment cursors PS1 zastąpiono memcpy, zachowując rozmiary i kolejność. Funkcja zwraca etap błędu zamiast ponawiać plikowe I/O bez końca; nie gubi już -1 w bool poll expression. Waliduje workspace/extents, a boot/main.c propaguje błąd do EXIT_FAILURE. main ma standardowy typ int; bezpośredni oryginalny callback zastąpił przełączanie stosu MIPS, a nieużywany boot_stack.h usunięto. Cały main.c przechodzi native C11 syntax check, ale nie jest jeszcze entry pointem gotowej aplikacji. Istniejący real-asset test porównuje wszystkie wczytane bloki z niezależnymi odczytami obrazu, instrument table i SPU RAM oraz pełny hash save image z referencji MIPS. Nie wstrzykuje już startup resources/table pointers do późniejszej inicjalizacji efektów. Obejmuje nieprawidłowy extent, brak obrazu, ucięcie każdego z czterech odczytów, błędny sample magic i udany ponowny start. Build, 35 CTest bez SKIP i świeży ASan/UBSan/function pełnego fixture loadera przeszły. Nie dodano plików ani funkcji produkcyjnych. To wykonanie bootstrapu, nie pełnej pętli gry; rendering menu/textboxów, dalsze sceny/kampania, trwały zapis/Continue, Disc 2 i usunięcie `pc/` pozostają niezakończone.

`Menu_FullInit` wykonuje się teraz w całości na rzeczywistym resident pack Disc 1. Do buildu włączono całe istniejące moduły memcard/save_flow.c, menu/inventory.c, item/armor_capacity.c, draw/textbox.c i draw/wipe.c. Poprawiono deklaracje native libc, callback ABI menu (pełne wskaźniki, unsigned event flags), adresowanie buforów oraz backing delayed callback/name glyphs i wspólną parę kolorów blend. Saved blend aliasuje pole save image. Najważniejsza poprawka: `PlayerInventoryState`/Aya/inventory są widokiem `SaveRuntimeState` od +0x20, a nie osobną kopią; memset i serializacja obejmują rzeczywisty stan postaci/ekwipunku. Static assertions sprawdzają offset +0x20 oraz equipment +0xCC i dotychczasowy rozmiar save image. Test zatruwa cały obraz i uruchamia oryginalny pełny init: hash wszystkich 0x12E4 bajtów zgadza się z wykonaniem EXE USA 0x800527C8 w Unicorn MIPS (tylko trzy BIOS bzero zastąpione zerowaniem pamięci). Sprawdza także powiązania pól, pool links, nazwy, kolor, timery i callback odroczony do trzeciego ticka bez powtórnego wykonania. Build, 35 CTest bez SKIP i świeże ASan/UBSan/function fixture stat-assets i aktorów przeszły. Nie dodano plików ani funkcji produkcyjnych. Próba linkowania pełnego Scene_LoadSceneData z jego istniejącym modułem wskazuje już tylko brak backing g_StartupResourceBlob. Nie jest to jeszcze wykonanie bootu sceny ani renderowania menu: pakiety textbox/wipe/text nadal wymagają naprawy native GPU ABI, a zapis na dysku/Continue, kampania i usunięcie `pc/` pozostają niezakończone.

Oryginalny `Draw_InitBuffers` używa natywnych rekordów: usunięto wyliczanie drugiego rekordu przez adres pola pierwszego i stały układ PS1, zastępując je bezpośrednim ustawieniem screen.y/h obu buforów. Stores porównano z instrukcjami USA 0x8005E68C..0x8005E69C. Backing dwóch framebuffer records należy do istniejącego input_queue.c. Cały istniejący draw/text.c włączono do buildu, więc reset koloru i fontu wykonują oryginalne funkcje. Dawne scalar symbols BaseX/Y aliasują kanoniczne dwa bajty game-state (+0xD8/+0xD9), które już inicjalizuje Boot_InitGameState, zamiast osobnego storage. Test boot-layout zatruwa rekordy przed resetem i sprawdza arenas/OT, oba draw/display rects, kolory/font/cursor oraz offset Y 0/15 i odrzucenie -1/16. Build, wszystkie 35 CTest z wymaganym Disc 1 oraz świeży ASan/UBSan/function testu aktorów przeszły. Nie dodano plików ani funkcji produkcyjnych. Włączenie całego text.c nie oznacza jeszcze naprawy wszystkich jego packetów PS1 do native GPU ABI ani pixel-perfect tekstu. Próba linkowania Menu_FullInit nadal wykazuje sześć braków: Draw_BlendColor, MemCard_ClearDelayedCallback, Menu_ResetStateFlags, Menu_SetBattleEquipMode, Menu_SetEquipSlotIndex i g_TextboxNameGlyphs. Pełny bootstrap, kampania i usunięcie `pc/` pozostają niezakończone.

Oryginalny `save/game_data.c` posiada teraz kanoniczny `SaveRuntimeState`. Dwie dotychczas niezależnie deklarowane etykiety podglądu nazw aliasują jego ostatnie 32 bajty (+0x12C4), z zachowaniem rozmiaru obrazu zapisu 0x12E4 i starego widoku remainder. Adresy wyboru okna (0x800C0DE0/0x800C20A4, +0x10 dla armor) potwierdzono w instrukcjach USA `Save_SelectMetadataWindow`. Istniejący test wykonuje wybór/edycję obu okien zapisu i przedmiotów oraz sprawdza wspólną pamięć i limit nazwy; zastępowanie ósmego znaku dziewiątym potwierdzono osobnym wykonaniem oryginalnej funkcji MIPS. Build, 35 CTest z wymaganym Disc 1 oraz świeży ASan/UBSan/function testu aktorów przeszły. Nie dodano plików ani funkcji produkcyjnych. Próba pełnego linkowania `Menu_FullInit` nadal ujawnia brakujące zależności draw/text, memcard, menu/inventory, armor_capacity, textbox i dane framebuffer/name glyphs. Bootstrap, trwały zapis/Continue, kampania i usunięcie `pc/` pozostają niezakończone.

Do natywnego buildu włączono cały istniejący `time/counters.c`, z kanoniczną tablicą czterech timerów i aliasami pól zegara zamiast niezależnych scalar symbols. W oryginalnym `item/active_list.c` odtworzono backing 521-bajtowego RNG; dawne etykiety XOR to widoki tej samej tablicy (+489 oraz -32), teraz wyrażone poprawnymi indeksami wewnątrz tablicy. Usunięto drugi tentative definition indeksu. Istniejący test aktorów sprawdza reset liczników oraz hash stanu i 4096 kolejnych losowań dla czterech wartości zegara, w tym 0xFFFFFFFF. Goldeny pochodzą z wykonania funkcji 0x800528F0/0x80052B2C oryginalnego EXE USA w Unicorn MIPS, nie z kopii algorytmu C. Build, 35 testów oryginalnego kodu z wymaganym Disc 1 i świeży ASan/UBSan/function testu aktorów przeszły. Nie dodano plików ani funkcji produkcyjnych. Nie jest to dowód poprawności wszystkich trybów tickowania timerów ani ukończenia bootu, pełnej gry czy usunięcia `pc/`.

Oryginalny Asset_OpenPeImage ładuje teraz resident data z obrazu: BOOT filename z SYSTEM.CNF wskazuje EXE, którego USA header/ranges są walidowane przed publikacją 450 room ranges, 81 sector boundaries i 506-byte audio directory. W istniejącym scene_assets.c są ich kanoniczne storage; BoundsNext aliasuje SectorTable+1, a SceneAssetLbaTbl aliasuje +69. Usunięto backing arrays i ręczne kopiowanie tych danych z fixture; test zatruwa tablice, montuje rzeczywisty Disc 1 i porównuje komplet danych z niezależnym katalogiem, po czym wykonuje loader pokoi i program M005 bez wstrzykiwania tabel. Urealniono także dane ujemnego testu I/O do jednej wspólnej tablicy granic. Stary self-referencing macro w actor_opcodes.c zastąpiono bezpośrednim odczytem pierwszego elementu aliasu. Nowy błąd -3 oznacza nieprawidłowe dane BOOT/EXE/tabel; brak marker/archive zachowuje -2. Pełny build, 35 CTest bez SKIP oraz świeży ASan/UBSan/function obu fixture przeszły z wymaganym Disc 1. Nie dodano plików ani funkcji produkcyjnych. Parser BOOT nie zakłada nazwy EXE drugiej płyty, ale Disc 2 i wspólny zestaw obrazów nadal nie są zweryfikowane; pełna pętla aplikacji, kampania i usunięcie `pc/` pozostają niezakończone.

Połączony program efektów M005 przeszedł 110 klatek przez oryginalne Scene_LoadRoomAssets/Pm_SendCmd/Pm_Start/Pm_Exec na rzeczywistym archiwum pokoju po Scene_LoadRoom. Test widzi wszystkie trzy task IDs, sygnał w klatce 30, trafienie gracza przez beacon w klatce 80, oryginalne zakończenie skryptu, action-state 4 oraz naturalne 4→5→free slot PM. Weryfikuje łańcuchy OT, granice areny i zachowanie kolorów obu modeli w całym przebiegu. Przy pełnym linkowaniu usunięto jednobajtowe atrapy task bank 108..114 z fixture: process.c posiada kanoniczne jawnie NULL native bindings i brak bindings po odczycie daje -13 bez instalacji raw PS1 data. Ich extent 969..978 porównano z EXE USA; test obejmuje także udany odczyt z niepowiązanym bankiem i brak mutacji rejestru. Te siedem programów nadal jest niezaimplementowanych, bez pozorowanej obsługi. Pełny build, wszystkie 35 CTest z wymaganym Disc 1 (bez SKIP) oraz świeży ASan/UBSan/function fixture aktorów i loadera przeszły. Nie dodano plików ani funkcji produkcyjnych. To przebieg programu efektów, nie pełnego skryptu lokacji ani gry; audio PCM, pixel reference, pełna kampania i usunięcie `pc/` pozostają niezakończone.

Beacon M005 zweryfikowano na rzeczywistych modelach po oryginalnym loaderze przez 12 aktywnych update/render klatek i terminal update przy age 12. Test wykonuje oryginalne JointBeacon i listę dzieci, oba bufory, wszystkie modelowe passy oraz kontrolowane wejście gracza w wiązkę w klatce 8. Poza wiązką flaga trafienia pozostaje pusta; wewnątrz ustawia się player-state 0x4000 i owner core_flags 0x91000005. Każda klatka zachowuje kolory obu modeli po save/scale/draw/restore, pełne adresy i długości łańcuchów OT oraz guard areny. RNG ma jawny seed i przywracany stan. Build, trzy CTest z wymaganym Disc 1 i świeży ASan/UBSan/function przeszły. Nie zmieniano funkcji produkcyjnych ani dodawano plików. Sterowanie callbackami jest bezpośrednie: połączony przebieg wszystkich trzech efektów przez PM, audio, pixel reference, pełna scena/kampania i usunięcie `pc/` nadal wymagają pracy.

Usunięto rzeczywisty dostęp do PS1 scratchpada 0x1F800000 w istniejącym `FieldFx_ModelScaleColors`: pojedynczy roboczy word jest teraz zmienną lokalną, bez zmiany mnożenia, limitu 0x7FFF ani RGB stores. Test save→scale→draw→restore obejmuje pięć faktorów (0/64/128/512/65535), cztery typy packetów, oba bufory i transparency, z golden RGB, kontrolą packetów i pełnym przywróceniem modelu. W realnym M005 znaleziono modele C5462704 (15 GT4) i C5862704 (32 FT4); oryginalny JointBeacon mode 0 wykonano po `Scene_LoadRoom` na tych danych, natywnym aktorze z 21 macierzami i prawdziwej liście callbacków. Sprawdzono pozycję (0,-40,932) z jointa 20, zmianę action-state 1→2, obydwa model pointers, listę ośmiu dzieci i guard. Build, trzy CTest z wymaganym Disc 1 oraz świeży ASan/UBSan/function obu fixture przeszły. Nie dodano plików ani funkcji produkcyjnych. To init beacona i naprawa wspólnej ścieżki kolorów, nie pełny update/render/collision beacona ani całego programu ze sceny; kampania i usunięcie `pc/` nadal niezakończone.

Przebieg oryginalnego skryptu M005 rozszerzono do 76 klatek PM: orbiter i cross-flare pracują równocześnie, orbiter kończy się po 32 klatkach od uzbrojenia, a cross-flare rysuje wszystkie aktywne ages 1..34. Test kontroluje przejście liczby zadań 1→2→1, wiek i identyfikatory slotów, quady GT/FT, pierścienie G4, pakiety blend, pełne natywne łańcuchy OT i guard. Osobno wywołano oryginalny terminal callback przy age 35; interpreter zatrzymano przed następną klatką inicjalizującą beacon, więc nie jest to jeszcze dowód pełnego programu. Build, dwa CTest z wymaganym Disc 1 i świeży ASan/UBSan/function przeszły. Nie zmieniano kodu produkcyjnego ani dodawano plików; test wykonuje istniejący skrypt i callbacki. Audio/pixel reference, beacon, pełna kampania i usunięcie `pc/` pozostają niezakończone.

Przetestowano pierwszy żywy etap oryginalnego programu M005 przez `Scene_LoadRoomAssets(117)`/Pm_SendCmd/Pm_Start/Pm_Exec: 40 klatek, oryginalny skrypt wait/signal, spawnowanie/aktualizacja/render orbitera oraz uzbrojenie fade po sygnale initializer w klatce 30. Test kontroluje pozycję jointów w wspólnym rekordzie, listy OT (pełne adresy, długości payloadu i zakres areny), oba bufory, guard oraz stop zwalniający slot. Wejście aktora ma 21 natywnych macierzy; archive fixture ma puste katalogi TIM/audio, więc to nie dowód audio ani pełnej sceny M005. W wywołanej oryginalnej ścieżce Asset_SearchByKeyType sanitizer wykrył signed-shift UB maski TIM; maski x/y mają teraz unsigned operand i zachowują oryginalne bity. Build, trzy testy z Disc 1, ponowny test po kontroli OT oraz świeży ASan/UBSan/function ścieżki przeszły. Nie dodano plików ani funkcji produkcyjnych. Cross-flare i beacon nie są jeszcze objęte pełnym przebiegiem programu; kampania, uruchomienie gry z oryginalnej pętli i usunięcie `pc/` nadal niezakończone.

Oryginalny program M005 jest statycznie powiązany: trzy callbacki, oryginalne sizes 12/4/36, initializer i 30-halfword script. `Scene_LoadRoom` rozpoznaje slot 117/address 0x80190B44 i zapisuje program w `g_FieldFxTaskPrograms[32]`, bez dostępu poza tablicę klas PM. Reread zeruje oryginalne dane room-local. Przy podłączeniu uzupełniono brakujące oryginalne dane shape rendering (52 vertices, sześć selekcji/counts, unit scale i zero rotation), porównane z EXE USA; cały istniejący PSY-Q `msc06.c` kompiluje się natywnie. Floor Y należy do istniejącego pipeline.c zamiast fixture. Oryginalny orbiter init odczytuje natywne `FieldFxTaskSlot.end`; poprzedni wspólny widok czytał na hoście payload/start zamiast końca. Test inicjalizuje 24 cząstki i sprawdza guard. Loader sprawdza teraz 238 pokojów, 2795 wpisów geometrii i 23 651 152 bajty packetów; M005 przechodzi cold/cached load, descriptor/script checks i reset danych. M023 pozostaje ujemnym testem niepowiązanego descriptoru; 176 niepustych pokojów nadal wymaga powiązań. Build, 35 CTest z wymaganym Disc 1 (bez SKIP), ponowne target checks oraz świeży ASan/UBSan/function fixture aktorów i room-loadera przeszły. W fixture poprawiono błędną kontrolę indeksu 117 w 85-elementowej tablicy. Nie dodano plików ani funkcji produkcyjnych. To dowód loadera, metadata i init cząstek; pełny przebieg trzech efektów, rasteryzacja, scena/gameplay i kampania nadal wymagają weryfikacji, podobnie jak usunięcie `pc/`.

Naprawiono trzy istniejące widoki event-state używane przez M005 i room_lib: sygnał/variant wskazuje natywne `FieldFxTaskContext.variables[0]`, a active/bD wskazuje jego `flags`. Stałe offsety PS1 0x12/0x0D nie uwzględniały poszerzonych wskaźników. Static assertions i test z oryginalnym initializer sprawdzają odczyt sygnału, zapis przez oba widoki oraz zachowanie pełnego argumentu i pól PC/cursor. Pełny build i trzy CTest z wymaganym Disc 1 przeszły bez SKIP. Nie dodano plików ani funkcji produkcyjnych. Program M005 nadal wymaga rejestracji i testów wszystkich callbacków; liczba pokojów pozostaje 237, kampania i usunięcie `pc/` nadal niezakończone.

Oryginalny `room_lib/script_task_var.c` kompiluje się w całości z natywnym ABI initializer (intptr_t, pełny adres). Rekord M005 0x80190B80 ma jedno 20-bajtowe backing w istniejącym `m005.c`; leading halfword, anchor +4 oraz fade +12 współdzielą tę pamięć. Istniejący mechanizm PE_OVERLAY obejmuje nazwę backing dla przyszłego linkowania innych pokoi. Test sprawdza mode 0/1, zapisy obu SVECTOR views i rzeczywiste `FieldFx_InitTaskProgram` z oryginalnym initializer, bez callbacka zastępczego. Początkowy rekord porównano z M005 w dostarczonym CUE/BIN. Pełny build, trzy CTest z wymaganymi danymi i świeży ASan/UBSan/function przeszły headless. Nie dodano plików ani funkcji produkcyjnych; naprawiono istniejący współdzielony moduł. Pełny program M005 nadal nie jest zarejestrowany, loader nadal go odrzuca; liczba pokojów pozostaje 237, kampania i usunięcie `pc/` nadal niezakończone.

Uzupełniono dane w całym istniejącym module M005: cztery oryginalne seed vectors, track orbiter 32 bytes, track cross-flare 80 bytes oraz dwa natywne cache pointers modeli. Seed unions zachowują byte view i właściwe wyrównanie SVECTOR; wywołania transformacji używają typed vector view. Pozostałe trzy callbacki M005 dostosowano na miejscu do wspólnego trzyargumentowego ABI, bez wrapperów. Automatyczny test czyta zakres M005 z dostarczonego CUE/BIN przez istniejące I/O i porównuje wszystkie siedem rekordów danych (w tym tint). Pełny build i trzy testy oryginalnych modułów/assetów/loadera z wymaganym Disc 1 przeszły bez SKIP. Nie dodano plików ani funkcji produkcyjnych. Dane i kompilacja nie są dowodem wykonania pozostałych callbacków; rekord skryptowy, initializer i rejestracja programu nadal wymagają pracy. M005 nadal nie jest akceptowany przez loader, a kampania i usunięcie `pc/` pozostają niezakończone.

Naprawiono natywne widoki jointów: `RoomFxTransformOwner.transforms` oraz `RoomM005BeaconPool.matrix` wskazują teraz pole `FieldActor.render_object.matrices` w rzeczywistym układzie hosta, zamiast stałego PS1 +0x238. Widok flag beacona wskazuje kanoniczne `FieldActorState.flags`; static assertions sprawdzają również action-state pointer. Istniejące funkcje transformacji i look angles przetestowano na prawdziwej strukturze aktora i jointach 0/20: identity/yaw 90°, ujemne i zawijane translacje oraz zapis właściwych flag/stanu. Direction seed (0,0,-16384) i zero vector mają backing w istniejącym `geometry.c`, porównany automatycznie z EXE USA. M005 wywołuje Psyz_Rand, nie libc rand (potwierdzono w natywnym obiekcie); nie zastąpiono RNG. Pełny build, trzy testy z wymaganym Disc 1 i świeży ASan/UBSan/function przeszły headless. Nie dodano plików ani funkcji produkcyjnych. To naprawa zależności całego M005, nie gotowy program/scena; liczba pokojów pozostaje 237, kampania i usunięcie `pc/` nadal niezakończone.

Oryginalna wspólna tablica `g_FieldFxTpageSlots[16]` ma teraz natywne backing w istniejącym `geometry.c`, z danymi potwierdzonymi automatycznie względem EXE USA 0x800E11E4. Dawne nazwy wpisów 1/2/3/4/9/11, palette entry 3 i tpage entry 1 aliasują właściwe elementy wspólnych tablic zamiast niezdefiniowanych osobnych scalarów. Test sprawdza adresy wszystkich aliasów; pełny build i trzy testy oryginalnego renderowania/assetów/loadera z wymaganym Disc 1 przeszły bez SKIP. Nie dodano plików ani funkcji. To kolejna zależność efektów, nie powiązanie całego programu M005; liczba pokojów pozostaje 237, a kampania i usunięcie `pc/` nadal niezakończone.

Podłączono oryginalny resident handler 85 (descriptor 0x800E13D4): init zasobów, init kontekstu, send/init programu, draw/update zadań i zamknięcie. Wszystkie funkcje pozostały w istniejącym `geometry.c`, z właściwym ABI PM bez wrapperów. Tablica `g_FieldFxTaskPrograms[104]` należy do istniejącego `task_list.c`; usunięto jej dwa backing arrays z fixture. Brak programu lub indeks poza natywną tablicą daje jawny błąd przed zajęciem slotu. `g_FieldEffectAssetTable` aliasuje kanoniczny `g_GameState.startup_resource_blob` (+0x148 PS1). Test PM wykonuje alloc/send/start/update/stop programu kontrolnego przez handler 85 i sprawdza pełny adres argumentu oraz brak alokacji przy błędzie. Oddzielny test na prawdziwym Disc 1 sprawdza sześć modeli startup bank 3, cztery tpage i pełny upload VRAM 64×256. Descriptor i siedem adresów callbacków porównano automatycznie z EXE USA. Pełny build, sześć CTest bez SKIP i świeży ASan/UBSan/function obu fixture przeszły headless. M005 nadal wymaga natywnego programu, danych/callbacków i rejestracji slotu 117; jego loader nadal zwraca -2. Nie dodano plików ani funkcji produkcyjnych; 237 pokojów, kampania i usunięcie `pc/` pozostają niezakończone.

Analiza rzeczywistych danych M005 wykazała, że slot 117/address 0x80190B44 zawiera `FieldFxTaskProgram` (trzy callbacki efektów, rozmiary, initializer i script), a nie `PmCommand`. Powiązanie tego pokoju nadal wymaga osobnej rejestracji programu i handlera 85. Naprawiono w istniejącej ścieżce oryginalnego silnika obcinanie adresu zwracanego przez initializer: program initializer, kontekst argumentu i callbacki zadań/list zachowują teraz `intptr_t`. Widoki RoomEnt i kontrolera powtarzania animacji uwzględniają poszerzone pole; static assertions pilnują offsetów. Test korzysta z rzeczywistego bind/init/spawn/update/draw i wskaźnika hosta do kontrolowanego stanu, sprawdzając także pominięcie initializer przy skip. Pełny build, pięć testów kolejki/ruchu/aktorów/room-loadera z wymaganymi danymi oraz świeży ASan/UBSan/function przeszły. Nie dodano plików ani funkcji produkcyjnych. Test initializer jest kontrolowanym wejściem, nie uruchomieniem całego programu M005. M005 nadal odrzucany przez loader; 237 pokojów, pełna kampania i usunięcie `pc/` pozostają w dotychczasowym stanie.

Naprawiono istniejący `FieldGt4Packet`: 32-bitowy tag powodował nadpisywanie payloadu przy zapisie natywnego linku; teraz ma `RenderGpuTag` i zachowany 48-bajtowy payload GT4. Deklaracje MulRotMatrix w czterech istniejących nagłówkach odpowiadają jego rzeczywistemu zwracanemu wskaźnikowi. Test oryginalnego draw sprawdza oba bufory, pięć wariantów blend, kolory obu końców, UV/CLUT/tpage, tag length, pełny link OT oraz guard. Oryginalny render driftu M005 uruchomiono przez `FieldFx_DrawCallbackList` dla age 0..6, palet 3/4 i alternate flag, z kontrolą packetów i granic bufora. Fixture ustawia także parametry depth cue; oryginalny EXE potwierdza sprawdzanie MAC0 po RTPT, więc zachowano tę bramkę. Build, cztery testy z wymaganymi assets oraz świeży ASan/UBSan/function przeszły headless. To dowód packetów, nie porównania rasteryzacji z PS1. M005 nadal wymaga pozostałych callbacków i task/class bindings; nie dodano plików ani nowego kodu gry. Pełna kampania i usunięcie `pc/` pozostają niezakończone.

Cały istniejący `room_m005_theater/m005.c` (pięć funkcji) kompiluje się teraz w natywnym C11 bez wydzielania procedur. Dwa callbacki list cząstek mają właściwe trzyargumentowe ABI bez function-pointer cast. Oryginalny drift callback wykonano przez prawdziwą listę: trzy cząstki, zawijanie halfword XYZ, ostatni ruch przy age 6, wygaszenie i ponowne użycie slotu. Moduł `geometry.c` posiada wspólne parametry sprite, black color i sześć wpisów palety; stare nazwy dwóch halfword pól aliasują ten sam blok. Palette/black color porównano automatycznie z EXE USA; drift tint i zerowe wektory potwierdzono w danych M005. Cały istniejący `libgte/mtx_001.c` dostarcza MulRotMatrix. Pełny build, pięć testów oryginalnych modułów z wymaganymi danymi i świeży ASan/UBSan/function przeszły. Pozostałe callbacki M005, render driftu, RNG oraz jego task/class descriptors nadal wymagają integracji i dowodów; `Scene_LoadRoom` nadal odrzuca M005. Nie dodano plików ani wrapperów. Liczba obsługiwanych pokojów pozostaje 237; kampania i usunięcie `pc/` nadal niezakończone.

Cały istniejący moduł `room_m272_liberty/m272.c` jest teraz linkowany statycznie. Jego siedem callbacków ma natywne ABI PM, a dane prostokątów i descriptor slot 15/address 0x8018F1A8 potwierdzono w rzeczywistym pokoju USA. `Scene_LoadRoom` wiąże tę klasę i przy ponownym odczycie przywraca zerowe dane overlayu. Test z Disc 1 sprawdza cały descriptor, trzy prostokąty, zachowanie 20 480 pikseli zapisanego paska i 49 152 pikseli kopiowanego obrazu w obu fazach, callbacki oraz cold/cached room reload. Korpus natywnego loadera obejmuje teraz 237 pokojów, 2785 wpisów geometrii i 23 501 072 bajty packetów; 177 pozostałych niepustych pokojów nadal wymaga powiązań. Pełny build, cztery testy kamery/room-loadera/cienia oraz świeży ASan/UBSan/function z rzeczywistymi danymi przeszły headless. Nie dodano plików ani wrapperów, wszystkie siedem oryginalnych funkcji pozostało w module. To dowód ładowania i kopiowania VRAM, nie całej sceny fabularnej, kampanii lub usunięcia `pc/`.

Test `original-game-camera` jest już całkowicie odłączony od źródeł i nagłówków `pc/`. Dane wejściowe pochodzą z resident table USA w EXE, a pokoje czyta istniejące `CdRom_ReadSectorsFromLba` z modułu `scene_assets.c`; katalog i rekordy interpretuje przez istniejące `SceneRoomDirectory`/`SceneRoomRecord`. Zachowany hash 509 widoków i 30 540 klatek potwierdzono na Disc 1 bez SKIP, także w świeżym ASan/UBSan/function. Pełny build przeszedł. Fixture czyta wyłącznie dane potrzebne kamerze; nie zastępuje `Scene_LoadRoom`, który w części lokacji nadal zatrzymuje się na niepowiązanych overlayach. To usunięcie zależności testu, nie dowód wejścia do tych lokacji w grze. Nie dodano plików ani funkcji produkcyjnych; cały katalog `pc/` nadal pozostaje do usunięcia.

Test `original-game-camera` nie linkuje już `pc/camera.c`, `pc/matrix.c` ani `pc/model.c`. Odczytuje widoki przez istniejące struktury `GeomState`/`CameraViewport` i wykonuje oryginalny moduł kamery. Przed odłączeniem równoległej implementacji utrwalono hash 30 540 klatek dla 509 widoków USA Disc 1, przy dotychczasowych porównaniach każdej klatki; nowy test zachowuje ten przebieg, kontrolę clamp i nienaruszenie danych pokoju. Pełny build i test z wymaganymi danymi Disc 1 przeszły. To dowód regresji, nie wizualnej zgodności z PS1. Odczyt katalogu pokojów nadal używa `pc/room.c`; katalog `pc/` pozostaje do całkowitego usunięcia. Nie dodano plików ani kodu produkcyjnego.

Dispatcher ma 73 powiązania: opcode 213 odłącza bieżącego aktora. EXE USA potwierdza zerowanie parent przed końcowym dostępem do niskiej pamięci PS1; natywne C pomija dereferencję NULL, zachowując flagi aktora i nie zmieniając flag dawnego rodzica. Test kolejki obejmuje cztery warianty listy aktorów. Build oraz cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Ta poprawka istniejącego modułu nie dowodzi pełnej integracji gry ani usunięcia `pc/`.

Dispatcher ma 72 powiązania: oryginalny opcode 214 odłącza bezpośrednie dzieci bieżącego aktora i czyści odpowiednie flagi linków. Powiązanie potwierdzono w EXE USA; ABI poprawiono w istniejącej funkcji. Test kolejki obejmuje dwoje dzieci rozdzielonych obcym aktorem, zachowanie obcego parent link i kolejności listy. Oddzielny opcode odpinania bieżącego aktora nadal wymaga analizy starego kodu (wyzerowanie parent przed późniejszym odczytem). Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 71 powiązań: oryginalny opcode 205 kopiuje współrzędne hit-cylinder do pozycji aktora w 16.16. Powiązanie potwierdzono w EXE USA; ABI i przesunięcia wartości ujemnych poprawiono w istniejącej funkcji. Test kolejki odczytuje pozycję przez oryginalny vector query i sprawdza zachowanie base position, prędkości oraz flag. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 70 powiązań: oryginalny opcode 203 zwraca kąt do punktu względem yaw bieżącego aktora. Powiązanie potwierdzono w EXE USA. Test kolejki obejmuje cztery kierunki osiowe i trzy yaw, zachowując wynik 4096 zgodnie z oryginalnymi porównaniami. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 69 powiązań: oryginalne opcode'y 200/201 zdejmują/ustawiają flagę aktora 0x20000. Powiązania potwierdzono w EXE USA; ABI poprawiono w istniejących funkcjach. Test kolejki odczytuje flagi po obu krokach przez oryginalny query opcode i sprawdza zachowanie pozostałych bitów. Nie jest to dowód całego zachowania roam w scenie. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 67 powiązań: oryginalny opcode 199 ustawia flagę węzła 0x80. Powiązanie potwierdzono w EXE USA; ABI poprawiono w istniejącej funkcji. Test kolejki sprawdza dalsze wykonanie takiego węzła po yield przy blokadzie aktora 0x1000 i globalnym player-only gate 0x100. Kontrolny węzeł bez flagi zachowuje kursor/licznik i rusza po zdjęciu blokad. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 66 powiązań: oryginalny opcode 196 dodaje kąt yaw aktora z oryginalnymi halfword porównaniami i jednokrotnym zawinięciem. Powiązanie potwierdzono w EXE USA. Test kolejki obejmuje sześć przypadków, granice 4095/4096, krok ujemny i krok większy niż pełny obrót; wynik odczytuje przez oryginalny vector query. Nie zastąpiono zachowania normalizacją modulo 4096. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 65 powiązań: oryginalny opcode 227 zapisuje wskaźnik floor polygon do danych skryptu i licznik halfword (dotychczasowa nazwa funkcji inventory pozostaje historyczna). Powiązanie potwierdzono w EXE USA. Offset oblicza się w natywnej szerokości; test kolejki sprawdza pełny adres hosta i zawężenie licznika. Ten dowód nie obejmuje wykonania geometrii polygonu. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 64 powiązania: oryginalne opcode'y 228/229 odczytują liczbę przedmiotów i limit slotów przez istniejący moduł inventory. Powiązania potwierdzono w EXE USA. Test kolejki obejmuje dziewięć kombinacji bazowego limitu/bonusu, clamp do 50 oraz pominięcie zajętych slotów poza aktywnym limitem. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 62 powiązania: oryginalny opcode 220 ustawia flagę aktora 0x1000000. Powiązanie potwierdzono w EXE USA; ABI poprawiono w istniejącej funkcji. Test kolejki odczytuje wynik przez oryginalny query opcode i sprawdza zachowanie pozostałych bitów. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 61 powiązań: oryginalny opcode 225 zapisuje mesh tint level we wspólnym stanie geometrii. Powiązanie potwierdzono w EXE USA. Test kolejki sprawdza sześć wartości, signed byte narrowing i zachowanie sąsiedniego stanu kamery. To sprawdzenie stanu, nie nowy dowód obrazu sceny. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 60 powiązań: oryginalny opcode 222 liczy ustawione bity word. Powiązanie potwierdzono w EXE USA; wartość robocza ma unsigned typ, zachowując modulo-32-bitowe odejmowanie dla bitu 31 bez signed overflow. Test kolejki obejmuje osiem wzorców, w tym zero, wszystkie bity i sam najwyższy bit. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 59 powiązań: oryginalny opcode 217 zwraca atan2 w jednostkach kąta PS1. Powiązanie potwierdzono w EXE USA; test kolejki sprawdza cztery kierunki osiowe i wynik ujemny bez normalizacji. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 58 powiązań: oryginalne opcode'y 215/216 zwracają sin/cos w 16.16. Powiązania potwierdzono w EXE USA; przesunięcie ujemnego wyniku poprawiono w istniejących funkcjach na unsigned word shift. Test kolejki sprawdza cztery kąty ćwiartkowe, w tym wyniki ujemne. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 56 powiązań: oryginalne opcode'y 223/224 ustawiają i zdejmują bit 1 flag aktora, używany przez stan ruchu/grawitacji. Powiązania potwierdzono w EXE USA; ABI poprawiono w istniejących funkcjach. Test kolejki odczytuje flagi po obu zmianach przez oryginalny query opcode i sprawdza zachowanie pozostałych bitów. Nie jest to nowy dowód działania fizyki w pełnej scenie. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 54 powiązania: oryginalne opcode'y 173/174 zdejmują/ustawiają bit 2 movement lock. Powiązania potwierdzono w EXE USA; ABI poprawiono w istniejących funkcjach. Test kolejki odczytuje lock po obu zmianach przez oryginalny opcode 44 i sprawdza zachowanie pozostałych bitów. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 52 powiązania: oryginalny opcode 172 ustawia wskaźnik wall-slot do fragmentu skryptu i parametr halfword. Powiązanie potwierdzono w EXE USA. Offset oblicza się w natywnej szerokości; test kolejki sprawdza pełny adres hosta i zawężenie parametru. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 51 powiązań: oryginalne opcode'y 170/171 ustawiają i zdejmują flagę sceny 0x2000 w kanonicznym game state. Powiązania potwierdzono w EXE USA; ABI poprawiono w istniejących funkcjach. Test kolejki sprawdza oba kroki z yield między nimi i zachowanie pozostałych bitów, w tym bitu 31. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 49 powiązań: oryginalny opcode 148 odczytuje wspólny battle mode state. Powiązanie potwierdzono w EXE USA. Test kolejki sprawdza sześć wartości, w tym -1 i najwyższą wartość signed word, oraz brak modyfikacji stanu. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 48 powiązań: oryginalny opcode 147 ustawia tempo interpolacji animacji i flagę renderowania 2, z synchronizacją obiektu gracza. ABI poprawiono w istniejącej funkcji, zachowując signed halfword argumentu. Test kolejki obejmuje pięć wartości tempa (zero, ujemne i zawężenie), zwykłego aktora/gracza oraz zachowanie wcześniejszych flag. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 47 powiązań: opcode'y 30/50/55/57 ustawiają i zdejmują flagi aktora 0x80 oraz 0x4000. Powiązania potwierdzono w EXE USA; ABI zmieniono w istniejących funkcjach bez wrapperów. Test kolejki odczytuje stan po każdym opcode'zie przez oryginalne zapytanie aktora i sprawdza zachowanie pozostałych bitów. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 43 powiązania: oryginalny opcode 27 ustawia prędkość aktora i aktualizuje focus height kamery dla gracza. Przesunięcie wartości ujemnej poprawiono w istniejącym module na operację bitową word. Test kolejki obejmuje sześć prędkości, wartości ujemne/zero, zawężenie halfword i zachowanie stanu kamery dla zwykłego aktora. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Świeża regresja po integracji 42 powiązań dispatchera: wszystkie 35 testów `original-game-*` przeszły z wymaganym obrazem Disc 1, bez SKIP. Cały rozszerzony fixture kolejki, z oryginalnymi modułami kompilowanymi z bieżących źródeł, przeszedł ASan/UBSan/function. To dowód objętych scenariuszy modułowych, nie pełnej kampanii: brak pełnego bootstrapu/frame loop, pozostałych opcode'ów i usunięcia `pc/` nadal uniemożliwia uznanie portu za ukończony.

Dispatcher ma 42 powiązania: opcode'y 11/12 wykonują oryginalny zapis/odczyt siedmiu wektorów aktora. Test kolejki sprawdza również nieznany selector, zawężenie rotacji do signed halfword, synchronizację base position i ustawienie flagi kamery wyłącznie dla gracza. W scenariuszu pozycji floor lock jest aktywny, więc ten dowód nie obejmuje nowego wyszukiwania podłogi. Testy kolejki i ruchu linkują cały oryginalny moduł kamery jako źródło wspólnego stanu. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Test oryginalnego opcode 17 obejmuje już każdy z 32 pojedynczych bitów licznika held, przycisk aktywny/nieaktywny oraz oryginalną zmianę operandu przez LZCR. Zachowuje osobny przypadek bitu 31, którego operand nie jest nadpisywany. Wszystkie te maski trafiają do prawidłowego elementu tablicy liczników. Build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Zerowa i złożone maski trybu licznika nadal wymagają analizy; nie uznano ich za zweryfikowane. Nie dodano plików ani kodu produkcyjnego. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 40 powiązań: oryginalny opcode 17 czyta held/pressed/released oraz licznik trzymania przycisku ze wspólnego stanu pada. Test kolejki sprawdza cztery tryby dla najwyższego bitu maski, przycisk aktywny/nieaktywny i wynik zapisany do zmiennej skryptowej. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Zerowa maska w trybie licznika pozostaje niezweryfikowaną ścieżką starego kodu, nie objętą tym dowodem. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 39 powiązań: opcode 240 pyta, czy sterowanie graczem jest odblokowane, na podstawie bitu 0 kanonicznego movement lock. Powiązanie potwierdzono w EXE USA. Test rzeczywistej kolejki obejmuje 16 kombinacji niższych flag, zachowanie najwyższego bitu i odczyt bez modyfikacji lock. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 38 powiązań: oryginalny opcode 20 ustawia/wyłącza kursory skryptów aktora i next value węzła. Dodatnie offsety halfword są przeliczane w natywnej szerokości adresu, a ujemne argumenty wyłączają wybrany slot. Test rzeczywistej kolejki sprawdza wszystkie trzy sloty, nieznany selector, pełny adres hosta, zachowanie pozostałych slotów i dwa ticki ustawienia/wyłączenia. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 37 powiązań: oryginalne opcode'y 22/23 obliczają prędkość do punktu i oczekują na zakończenie ruchu. W istniejącym module ujednolicono ABI oczekiwania i przesunięcie czasu do 16.16 wykonuje się na unsigned word. Test kolejki sprawdza trzy składowe prędkości z wpływem gravity Y, flagę ruchu, cofanie kursora i kontynuację po zdjęciu flagi przez fixture. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełne sterowanie ruchem przez frame loop i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 35 powiązań: oryginalny opcode 84 zwraca sumę bezwzględnych różnic XYZ aktorów. Powiązanie potwierdzono w EXE USA. Test kolejki obejmuje osiem kombinacji znaków, wybór gracza, pominięcie martwego wpisu i brak celu. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 34 powiązania: opcode 69 wykonuje oryginalny odczyt kąta do aktora. Powiązanie potwierdzono w EXE USA. Test kolejki obejmuje cztery kierunki, trzy yaw, wyszukiwanie z pominięciem martwego wpisu, gracza i brak celu. Zachowuje również oryginalny wynik 4096 dla pełnego obrotu, bez normalizacji do zera. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 33 powiązania: oryginalny opcode 94 zwraca wektor aktora wybranego przez type/sub-ID albo gracza. Powiązanie potwierdzono w EXE USA. Test rzeczywistej kolejki obejmuje siedem selectorów wektorów oraz nieznany selector, martwy wpis poprzedzający żywego, brak dopasowania i brak gracza. Siedem operandów sprawdza także odczyt dodatkowych trybów operandów z drugiego słowa instrukcji. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 32 powiązania: oryginalny opcode 14 odczytuje pola aktora. W istniejącej funkcji usunięto odczyt adresu argumentu przez 32-bitowy typ, który obcinał wskaźnik hosta. Test kolejki obejmuje pięć selectorów, wyszukiwanie z pominięciem martwego wpisu, brak dopasowania, gracza/brak gracza oraz oryginalne bezpośrednie dopasowanie bieżącego aktora niezależnie od jego death flag. Sprawdza signed frame i zachowanie wyjścia dla nieznanego selectora. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 31 powiązań: oryginalny opcode 47 ustawia cel klatki animacji z odczytem unsigned halfword i ograniczeniem do ostatniej klatki. Powiązanie potwierdzono w EXE USA. Test rzeczywistej kolejki wykonuje 47 → 48 → 32, sprawdzając ograniczenie zakresu, flagę 0x200, oczekiwanie/cofanie kursora, przejście po dopasowaniu klatki i zakończenie w następnym ticku. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Oryginalne opcode'y bitowe 40/41/42 używają unsigned word mask i pięciu najmłodszych bitów indeksu, zgodnie z instrukcjami SLLV potwierdzonymi w EXE USA. Usunięto signed-shift UB dla bitu 31 oraz indeksów poza 0..31. Test rzeczywistego interpretera sprawdza ustawianie/testowanie/czyszczenie dla indeksów -2..65, zachowanie pozostałych bitów, wynik maski i zakończenie węzła. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 30 powiązań: opcode'y 19/25/43/44/45 zachowują oryginalne flagi aktora, numer węzła oraz ustawianie/czyszczenie/odczyt movement lock. Powiązania potwierdzono w tablicy EXE USA. Test rzeczywistej kolejki wykonuje sekwencję z zachowaniem istniejących bitów, odczytem do zmiennych skryptowych i zakończeniem węzła. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełna pętla gry i usunięcie `pc/` pozostają niezakończone.

Dispatcher ma 25 powiązań: oryginalne opcode'y 4 i 24 oznaczają węzły do usunięcia. W istniejącym module ujednolicono ABI callbacków, zachowując 16-bitowy odczyt numeru węzła. Test rzeczywistej kolejki sprawdza wszystkie trzy listy aktora, pominięcie bieżącego węzła przez opcode 4, usunięcie siebie/wybranego innego węzła i brak dopasowania przez opcode 24, zachowanie flag oraz pozycję kursora. Pełny build i cztery testy kolejki/ruchu/aktorów/lifecycle przeszły. Nie dodano plików ani funkcji produkcyjnych. Pełny interpreter lokacji i pętla gry nadal wymagają integracji.

Dispatcher ma 23 powiązania: dodano oryginalne opcode'y 37/38/39 odczytu identyfikatora field SFX oraz odczytu/zapisu sub-ID aktora. Program wykonywany przez oryginalną kolejkę sprawdza teraz także zapis 300 jako byte 44, odczyt sub-ID i field SFX do zmiennych skryptowych, następnie ruch/yield i zakończenie. Pełny build i trzy testy kolejki/ruchu/aktorów przeszły. Nie dodano plików ani funkcji produkcyjnych. Pozostałe opcode'y oraz pełna pętla gry pozostają niezintegrowane.

Resident dispatcher `g_ScriptQueueOpcodeHandlers` należy teraz do istniejącego `flow_opcodes.c`, z 20 powiązaniami potwierdzonymi w tablicy EXE USA. Usunięto trzy backing arrays z testów; fixture'y kolejki nadal mogą nadpisywać wybrane wpisy do obserwacji traversal. Callbacki oczekiwania i animacji dostosowano w istniejących modułach do wspólnego `int **` ABI, z zachowaniem odczytu signed halfword tempa animacji. Brak handlera powoduje jawny komunikat i przerwanie wykonania. Test wykonał przez rzeczywisty `Script_RunQueue` oraz produkcyjny dispatcher program opcode 68 → 10 → 32: ruch/yield, przypisanie zmiennej dopiero po dotarciu i zakończenie węzła, dla aktora i gracza. Aktualizacja pozycji między tickami pozostaje w fixture. Pełny build, osiem testów kolejki/ruchu/aktorów/scen/tekstur/GPU, ponowne testy po uporządkowaniu deklaracji i świeży ASan/UBSan/function przeszły. Pozostałe powiązania, RNG i całe skrypty lokacji nadal wymagają integracji. RNG USA czyta także poza 17-word state, m.in. poprzedzające dane/kod; proste zawinięcie indeksu do 17-elementowej tablicy nie zachowuje jego zachowania. Moduły RNG pozostają bez takiej adaptacji i opcode 26 jest niepowiązany. Nie dodano plików ani wrapperów produkcyjnych; `pc/` i pełna pętla gry nadal wymagają pracy.

Cały istniejący `src/main/script/node_actions.c` (siedem funkcji) dołączył do natywnego targetu bez wydzielania kodu. Usunięto nieużywany nagłówek wymagający layoutu PS1; przesunięcia sin/cos i klatek animacji zachowują 32-bitowe bity bez signed-shift UB. Test wywołuje oryginalny opcode ruchu do punktu absolutnego i porównuje go z odpowiednim oryginalnym ruchem względnym: zwykły aktor/gracz, skręt, stan oczekiwania, różne układy zapamiętanych operandów i zakończenie ruchu bez ograniczenia skrętu. Dla ograniczonego skrętu kontroluje zgodność do 80 kroków, bez założenia zakończenia krótkiego ruchu; nie zastępuje oryginalnego sterowania. Siedem przypadków oczekiwania na klatkę animacji obejmuje kierunek naprzód/wstecz, przekroczenie celu i zawinięcie zakresu oraz cofanie kursora. Pozycje pomiędzy wywołaniami aktualizuje fixture. Pełny build, siedem testów aktorów/kamery/scen/tekstur/GPU, ponowny test rozszerzonych przypadków i świeży ASan/UBSan/function przeszły. Pozostałe entry pointy modułu i pełne wykonanie przez interpreter/frame loop nadal wymagają osobnej weryfikacji. `pc/` pozostaje do usunięcia; gra nie jest jeszcze grywalna.

`g_PlayerBattleEntity` jest teraz aliasem `g_GameState.scene_object`: oryginalny adres 0x800B0CEC odpowiada game state +0x14. Usunięto trzy osobne backing records z testów modeli, tekstur i spawnów. Oryginalne `Script_PlayEntityAnim` oraz `Script_CopyEntityRenderObject` wykonują się w istniejącym module opcode'ów. Test sprawdza pięć wartości tempa, start/wait/finish, zachowanie operandów i kursora oraz synchronizację obiektu gracza i movement lock. Kopiowanie modelu wywołuje oryginalne inicjalizowanie obiektu i transformowanie kości dla dwóch indeksów, wyboru gracza i przeszukiwania listy z martwym aktorem. Współrzędne ujemne są konwertowane na 16.16 przez przesunięcie bitów zamiast niezdefiniowanego signed shift. Pełny build, siedem testów modeli/aktorów/scen/tekstur/GPU i świeży ASan/UBSan/function przeszły. Nie dodano ani nie przeniesiono plików lub funkcji produkcyjnych. Nadal potrzebne są wywołania tych opcode'ów przez pełny interpreter/scenę; katalog `pc/` nadal wymaga usunięcia.

Oryginalne opcode'y ruchu/skrętu aktora `Render_CameraTrackRelativeOffset` i `Render_CameraStepAngleFade` wykonują się na natywnych węzłach skryptów i kursorze. UBSan wykrył przesunięcie ujemnego cosinusa; operacje sin/cos w istniejącym module poprawiono na przesunięcie 32-bitowych bitów, zachowując wynik PS1. Test obejmuje 81 par kątów z granicami i remisem 180°, kierunek krótszego łuku, zapamiętanie operandów, zakończenie i cofanie kursora. Sprawdza ruch do celu bez ograniczenia skrętu oraz trzy kroki ruchu z ograniczonym skrętem dla aktora i ścieżki prędkości gracza. Pozycje pomiędzy wywołaniami aktualizuje fixture; pełne wywoływanie przez interpreter/pętlę scen nadal wymaga integracji. Pełny build, siedem testów kamery/aktorów/scen/tekstur/GPU, ponowny test rozszerzonych przypadków i świeży ASan/UBSan/function przeszły. Nie dodano ani nie przeniesiono plików lub funkcji produkcyjnych.

Uruchamiamy oryginalny kod `src/main/` i `src/overlays/` na PC. Modyfikujemy go w miejscu, usuwając zależności sprzętowe i dostosowując pamięć oraz wskaźniki; zachowujemy logikę, kolejność aktualizacji i zachowanie gry. Nie budujemy równoległego silnika ani nowych warstw abstrakcji. PsyZ zapewnia istniejące API platformowe.

Aktualne wiążące wymaganie użytkownika: żadnych nowych plików ani wydzielania funkcji. Naprawiamy całe istniejące moduły w miejscu. Dotychczasowe wydzielenia należy scalić z powrotem, zachowując poprawki i sprawdzając kompletność funkcji względem oryginalnych modułów. Katalog `pc/` ma zostać usunięty: nie jest docelowym miejscem ani logiki gry, ani osobnej implementacji platformowej. Potrzebne dostosowania platformy należy integrować w istniejących modułach oryginału i PsyZ. Automatyczna weryfikacja pozostaje wymagana; jej zależności od `pc/` również wymagają przebudowy. Historyczne wpisy poniżej opisujące wydzielanie funkcji lub dopuszczające `pc/` nie stanowią zgody na kontynuowanie tego podejścia.

Usunięcie `pc/` jest obowiązkiem bieżącej integracji, a nie porządkowaniem odkładanym do końca portu. Kolejne prace mają zastępować jego zależności wywołaniami całych oryginalnych modułów i usuwać zbędne implementacje. Samo kompilowanie lub testowanie oryginalnych funkcji obok równoległego runtime nie spełnia tego wymagania. Wierność 1:1 dotyczy zachowania oryginalnej gry; nie oznacza byte match ani wymogu zachowania sprzętowych ograniczeń PS1.

Nie ma wyjątku dla „tymczasowego” frontendu, adaptera platformowego ani testów w `pc/`. Istnienie tego katalogu jest nadal nierozwiązanym błędem architektury. Kryterium usunięcia obejmuje brak katalogu oraz brak zależności builda i działającego programu od jego implementacji; samo usunięcie targetów lub przeniesienie plików nie stanowi integracji oryginalnej gry.

Test `original-game-stat-assets` nie zależy już od `pc/room.c` ani nagłówków `pc/`: czyta dane przez oryginalny moduł `src/main/cdrom/scene_assets.c`. Wczytuje dokładnie zakres resident banku z danych EXE USA (w dostarczonym obrazie 15 sektorów), zamiast czytać cały bufor 0x8004 i fragment następnego banku. Kontrole granic tabel dotyczą wczytanego zakresu. Oryginalne moduły menu/input oraz test aktorów również nie potrzebują już ścieżki nagłówków `pc/`. Pełny build i trzy testy statystyk, animacji/efektów aktorów oraz lifecycle przeszły z wymaganym obrazem Disc 1. Pozostałe zależności runtime i testów od `pc/` nadal wymagają usunięcia; gra nie jest jeszcze grywalna.

Oryginalne inicjalizatory celów trzech burstów w `src/main/fieldfx/burst.c` używają natywnych callbacków i resident state tego samego modułu. Dwa inicjalizatory z audio czytają `g_GameState.cd_range_read_mode` oraz `scene_object_tables[2]`, zamiast deklarować niezależny stan AKAO i wskaźnik nagłówka. Odczyty adresów 0x800B0CE8 i 0x800B0E14 potwierdzono w obu oryginalnych funkcjach EXE USA. Test aktorów linkuje cały oryginalny moduł AKAO; sprawdza dwuelementowe listy celów, współrzędne, indeks/marker końca i rzeczywiste wpisy kolejki dźwięku dla flag 0, 1 i 255. Pełny build, pięć testów aktorów/scen/tekstur/GPU i ASan/UBSan/function przeszły. Pozostałe callbacki i metadata trzech klas burst nadal wymagają związania z natywną fabryką PM; ten test nie dowodzi ich pełnego renderowania ani grywalnej sceny. Błędny sparse view `game_audio_state.h` w opcode module nadal wymaga zastąpienia polami wspólnego stanu przed integracją tego modułu.

Trzy klasy burst są już związane z oryginalną fabryką PM pod resident ID 4, 5 i 6. Wszystkie callbacki tego modułu mają natywne ABI, a resident skrypty, listy funkcji, rozmiary, skale, macierze i sprite state należą do istniejącego `burst.c`. Zachowano wszystkie 76 dotychczasowych funkcji, bez nowych funkcji, plików ani wydzielania kodu. Skrypty i rozmiary porównano z EXE USA. Test wykonuje osiem klas (cztery glowy, point sprite i trzy bursty), siedem akcji gracza i 160 klatek na cykl, z dwoma celami burstów, rysowaniem przed aktualizacją, komendą zakończenia, zwalnianiem/restartem oraz kontrolą buforów. Sprawdza też osiem entry pointów przekazywania natywnych adresów list celów. Pięć callbacków pierwszego burstu pozostaje w oryginalnej liście; jego normalny skrypt uruchamia ID 0..3, nie nieużywany ID 4. Pełny build, sześć testów aktorów/scen/tekstur/GPU i świeży ASan/UBSan/function przeszły. Test błędów common TIM poprawiono na jawny wynik -1, zachowując sprawdzenie ponownego odczytu i zakończenia. Nadal brakuje pełnej pętli gry, integracji opcode module, referencyjnych obrazów rozgrywki i usunięcia pozostałego `pc/`; to nie dowód grywalnej kampanii.

Cały oryginalny `src/main/script/actor_opcodes.c` jest teraz kompilowany w natywnym targetcie: wszystkie 20 definicji funkcji pozostają w tym samym pliku. Usunięto duplikaty deklaracji kursora, niewłaściwe asercje layoutów runtime PS1 i niepotrzebne operacje cache/critical section. Widok `game_audio_state.h` usunięto; dispatcher sound opcode czyta i zapisuje kanoniczne pola `g_GameState`, a dwa banki robocze odpowiadają slotom 0/1 zwracanym przez oryginalny loader. `Akao_Cmd_19_Then_C0` przyjmuje pełny wskaźnik banku zamiast int. Kolejka odroczonych komend należy do istniejącego modułu opcode'ów; katalog banków audio ma wspólne resident storage w module archiwum zamiast oddzielnych kopii w testach. Test sprawdza pozycyjne pola, deferral i limit 16 komend, wybór archive record, skalę głośności oraz obie cached-bank ścieżki przez rzeczywiste komendy 0x19/0xC0 i rejestrację slotu. Dwanaście testów audio/aktorów/scen/tekstur/GPU oraz świeży ASan/UBSan/function przeszły z wymaganym Disc 1. Pełne dispatch tables, filmowe entry pointy tego modułu, wypełnianie katalogu banków w natywnym bootstrapie i spięcie całości z pętlą gry nadal wymagają pracy. Samo skompilowanie modułu nie potwierdza wszystkich jego opcode'ów ani grywalności.

Dotychczasowe implementacje logiki w `pc/` powstały przy błędnym podejściu i nie są podstawą portu. Poniższe historyczne wyniki dotyczą tych eksperymentów, nie uruchomienia oryginalnej gry. Pierwszy audyt 99 oryginalnych jednostek gry (bez historycznych bibliotek `psyq`) wykazał 9 przechodzących syntax check; to nie dowód kompilacji ani linkowania. Główne przeszkody obejmują stare layouty struktur roboczych, instrukcje MIPS/GTE i zależności sprzętowe. Sprawdzenia layoutów danych z płyt pozostają wymagane.

Oryginalny nagłówek `include/pe1/gte.h` ma 143 makra przeniesione z pojedynczych transferów/poleceń MIPS na bezpośrednie endpointy PsyZ. Test `original-game-gte` wywołuje te makra bez okna, sprawdza projekcję, MAC, rejestry kontrolne i niewyrównane host buffers; przechodzi także ASan/UBSan dla kodu testu i makr (biblioteka PsyZ nie jest instrumentowana). Złożone makra i oryginalny renderer nadal wymagają dostosowania. Oryginalne procedury matematyczne są linkowane przez `parasite-eve-original`; 100000 deterministycznych przypadków przechodzi `original-game-math` oraz ASan/UBSan. To postęp w kompilacji kodu źródłowego, nie działająca rozgrywka.

Makra ładowania/składania macierzy, strided columns i zapisu IR/MAC są przeniesione w oryginalnym nagłówku; pozostają złożone makra pozostałych operacji. `src/main/psyq/libgte/mtx_000.c` (oryginalny `MulMatrix0`) kompiluje się bez zmiany algorytmu i jest bezpośrednio linkowany do programu. Headless test wykonuje tę procedurę, w tym aliasowanie input/output, oraz oryginalne `gte_CompMatrix` i `gte_CompMatrixMac`: sprawdza obrót, saturację IR versus pełny MAC, translację i niewyrównany zapis macierzy. Testy oraz ASan/UBSan oryginalnych źródeł i makr przechodzą; PsyZ nadal nie jest instrumentowane. Oryginalny renderer i pełna gra nie są jeszcze uruchomione.

Nagłówek `include/pe1/gte.h` nie zawiera już assemblera MIPS: pozostałe transfery FIFO, operacje koloru/interpolacji, staging i zachowanie rejestrów OP używają bezpośrednio PsyZ. To nie oznacza, że każda rodzina makr ma już pełny dowód zachowania. Audyt kompilacji `src/main/psyq/libgte` wykazał 24/39 jednostek kompilujących się do host objects; reszta nadal zawiera assembler w plikach źródłowych lub makrach NOP. Oryginalna procedura `cmb_06.c` jest linkowana jako `Pe1_RotAverageNclip3`, aby uniknąć konfliktu z biblioteką PsyZ. Test wykonuje jej oryginalny control flow dla obu orientacji trójkąta: sprawdza screen XY, signed area, averaged depth, interpolation/flags oraz brak zapisu outputs przy backface rejection. Headless i ASan/UBSan oryginalnych źródeł przechodzą (PsyZ nieinstrumentowane). Integracja pełnego renderera i gry pozostaje niedokończona.

Oryginalny `src/main/anim/bone_rotations.c` kompiluje się jako host object i wykonuje w headless testach. Scratchpad kątów zastąpiono zwykłym buforem, a adresowanie próbek i override slots nie obcina już wskaźników. Usunięto wyłącznie PS1-offset assertions dla roboczych `RenderObjectEntity`, `RenderOrbitingEmitter` i callback `SceneFlagRecord`; sprawdzenia formatów modeli, animacji i pakietów pozostały. Fixture wykonuje oryginalne byte/halfword decoders, stałe i próbkowany kanał frame 1, translację, hit-body metadata oraz obrót override; build i ASan/UBSan przechodzą, PsyZ nieinstrumentowane. Moduł animacji jest jeszcze oddzielnym object target, ponieważ jego oryginalne input globals nie są zintegrowane z bootem gry. Test dostarcza input globals jako kontrolowany stan, nie jako implementację gameplay. Nie oznacza to działającej gry ani kompletnego testu wszystkich animacji z płyt.

Oryginalny dekoder animacji jest teraz bezpośrednio linkowany do `parasite-eve-pc`. `Scene_CheckFlagBits` przeniesiono bez zmiany treści z pliku animacji do istniejącego `boot/init.c`, gdzie znajduje się oryginalny input; usunięto testowe definicje globals pada z fixture animacji. Nie dodano nowej implementacji inputu. `original-game-animation-assets` wykonuje oryginalny dekoder na danych z Disc 1: 9042 strumienie, 374431 klatek, 10064925 macierzy kości. Wyniki obrotów i translacji porównuje z wcześniejszym dekoderem testowym; to dowód różnicowy zgodności tych wyników, nie dowód gameplay ani wszystkich override variants. Pełny corpus przechodzi ASan/UBSan dla kodu gry i testowego I/O/reference; PsyZ pozostaje nieinstrumentowane. Original boot/input oraz frame integration nadal wymagają pracy.

Oryginalne `Render_TransformVertices`, `Render_CopyFrameData` i `Render_CopyFrameDataDouble` przeniesiono z `render/object.c` do `render/bone_transforms.c`, aby oddzielnie kompilować istniejące procedury bez zależności draw/menu. Nie dodano nowego interfejsu ani algorytmu transformacji. Scratchpad origin/stos zastąpiono zwykłą pamięcią; wspólny stos mieści wszystkie głębokości u16 command count. Procedury są bezpośrednio linkowane do programu. Headless fixture sprawdza push/pop, root/part translation, hit-centre transforms oraz inherited modes 1/3/4 i flag 0x400. Pięć testów oryginalnego kodu (z strict Disc 1 animation corpus) oraz ASan/UBSan transform fixture przechodzą; PsyZ nieinstrumentowane. Pełna projekcja modeli, GPU packet submission, oryginalna frame loop i gameplay pozostają nieukończone.

Oryginalny `Render_TransformMorphVertices` przeniesiono do `render/vertex_projection.c` i bezpośrednio linkuje się do programu; scratchpad macierzy zastąpiono lokalnym workspace. Oryginalne shared work buffers otrzymały zwykłą pamięć z miejscem na u16 vertex count, output bias i RTPT tail. Test wykrył trzy-slotowe przesunięcie zapisu: potwierdzono je bezpośrednio w dostarczonym EXE USA (0x8003AEB8/BC inkrementują output pointers o 12 przed SWC2 od 0x8003AEC0). Zachowano oryginalną kolejność, zamiast dostosowywać ją do wcześniejszego `pc/projection.c`. Headless fixture sprawdza RTPT tails, nakładanie stores kolejnych części, przesunięcie, depth i hidden parts; ASan/UBSan kodu gry przechodzi, PsyZ nieinstrumentowane. Aliasowanie pozostałych oryginalnych symboli work buffers, pełny model corpus oraz GPU/render frame integration nadal wymagają pracy.

Aliasy `D_800B1640`, `D_800B1644` i `D_800A636C` wskazują wspólną pamięć work buffers (+2/+3/+3 words), bez osobnych alokacji. Poprawiono także obcinanie alias pointer w istniejącym `render/object.c`. `original-game-projection-assets` wywołuje oryginalny renderer projekcji na 878 modelach pokojowych Disc 1 (236410 vertex slots), z kontrolowanymi identity/translation poses i synthetic viewport. Porównuje screen/depth w authored part order, uwzględniając output bias, ukryte części i nakładanie RTPT tail stores. Corpus przechodzi ASan/UBSan kodu gry, I/O i reference helper; PsyZ nieinstrumentowane. Nie dowodzi to poprawności wszystkich animowanych poses, actual camera integration, światła/pakietów GPU ani gameplay; te elementy pozostają wymagane.

Oryginalne `Render_TransformSkinnedVertices` i `Render_SetupBoneTransforms` przeniesiono do istniejącego `render/vertex_projection.c`, zamieniając jedynie scratchpad na lokalne workspaces i bezpośrednie istniejące makra GTE. Obie procedury linkują się do programu. Wektor kotwicy i składowa Y współdzielą pamięć jak oryginalne symbole (0x8009CD98/9A); initial zero vector potwierdzono z danych EXE. Headless fixture sprawdza anchor cylinder/body/target centres, projekcję origin i target, headerless inherited values, radius projection i odtworzenie table_value2c po chwilowej zmianie. Build i ASan/UBSan procedur przechodzą; PsyZ nieinstrumentowane. Actual camera/frame integration i GPU submission nadal są niedokończone.

Oryginalne `Entity_RollbackPositionHierarchy` i `Entity_CopyParentPosition` wydzielono z `entity/frame_update.c` do `entity/hierarchy.c` bez zmiany algorytmu, razem z prawdziwym global head aktorów. Procedury linkują się do programu. Usunięto PS1-offset assertions wyłącznie dla runtime `FieldActor` i `FieldActorState`, których wskaźniki są teraz host-width; packed scalar status checks pozostają. Headless fixture sprawdza wielopoziomowy rollback, siblings, floor mirror, root-only flag, attachment flags i oryginalną list order (child-before-parent reads old parent position), także pustą listę. ASan/UBSan przechodzą dla oryginalnego kodu, PsyZ nieinstrumentowane. Pełny original Entity_FrameUpdate/AdvanceAnim, action/audio dependencies i original boot/input nadal wymagają integracji.

Oryginalny `Scene_CheckFlagBits` i prawdziwe pad globals przeniesiono z `boot/init.c` do `scene/input_rules.c`, bez zmiany algorytmu. Kod linkuje się do programu; testuje maski held/pressed/released, fallback, continuation i callback zmieniający mode bez przekierowania bieżącego record walk. ASan/UBSan przechodzą dla oryginalnego kodu, PsyZ nieinstrumentowane. Usunięto pozostałą duplikację definicji actor-list head w `frame_update.c`. Deklaracja `Anim_BuildRotationMatrices` i wywołania mają teraz rzeczywistą sygnaturę void/3 argumenty; czwarty historyczny argument nie był odczytywany przez definicję. Oryginalne sampling/remapping pada, script callbacks, movement integration i pełny Entity_FrameUpdate nadal wymagają uruchomienia.

Runtime widoki `BattleEntity`/`FieldActor` mają teraz zgodne native pointer fields: previous link, trzy script-list pointers i locals, frame callback, collision-face pointers i model allocation word. `Combatant`/`EnemyCombatant` zachowują relacyjne layout checks z `FieldActorState`; player action-mode bytes uwzględniają wzrost miejsca zajmowanego przez wspólny action-state pointer, a ukryte pointer slots enemy view są jawne. Wyłącznie stałe PS1 runtime-offset checks zastąpiono zgodnością widoków; scalar/serialized checks pozostają. `BattleTargetWords` przechowuje pełny host pointer, a queue-address view ma uintptr_t. Fixture oraz ASan/UBSan potwierdzają zgodność layoutów, zachowanie pełnych actor/parent/model pointers i działanie oryginalnej hierarchii. To nie dowód działania walki: istniejące raw offsets/strides i lokalne 32-bit kopie w pozostałym kodzie nadal wymagają portowania. Audyt oryginalnych jednostek gry bez psyq: 16/103 przechodzi syntax check; pełny build modułów frame/script nadal blokują runtime layouts EntityActionTable, Pe1GameState i PM oraz dalsze zależności.

Wspólne runtime nagłówki `EntityActionTable`, `Pe1GameState` i PM przyjmują host pointer width. Tabela akcji nadal ma 48 wpisów, PM siedem callbacków oraz dwa banki po jedenaście slotów; assertions opisują liczby elementów i wzajemne layouty, zamiast PS1 byte strides. Fixed scene-audio i fade command layouts mają osobne aktywne checks. Oryginalny `boot/frame_loop.c` wybiera ordering table przez rzeczywisty slot index, a `cdrom/track_read.c` wybiera odpowiedni bank-work pointer bez przesuwania całej struktury gry; usunięto nieużywany Pe1GameStateAddress union. Header/layout fixture i ASan/UBSan przechodzą, ale pełne moduły boot/CD nadal nie kompilują się jako native targets i nie zostały wykonane. Compile probe `Entity_FrameUpdate` przechodzi te nagłówki i zatrzymuje się teraz na `RenderShadowActor` runtime layout. Dalsze raw offsets w main/overlays oraz original frame integration pozostają wymagane.

`RenderShadowActor` używa teraz canonical `FieldActor`, zamiast oddzielnego sparse record z PS1 padding. Oryginalny `Render_DrawActorShadow` odwołuje się do właściwych pól runtime aktora/render object, a world coordinates pobiera przez fixed-point helper. `allocation_block`/`shadow_quads` współdzielą pełny host pointer; matrix index/intensity są jawne w aktorze i odpowiadającym BattleEntity view. Oryginalny loader zapisuje adres alokacji przez uintptr_t; free-function argument ma tę samą szerokość. Test layout/alias/parametrów oraz ASan/UBSan przechodzą. Sam shadow renderer i allocator nie są jeszcze native targets i nie były wykonane; ich GPU/storage dependencies oraz pool strides nadal wymagają portowania. Compile probe Entity_FrameUpdate przechodzi shadow view i zatrzymuje się na kolejnych historycznych Pe1GameState assertions w scene_assets.h.

Oryginalne `Entity_ResetAllPools` i `Entity_InitFreePool` działają na typed array 14 native FieldActor records, bez stałego 0x280 stride i 32-bit pool addresses. Udostępniono prawdziwe storage dla linker-only globals: actor pool, allocation table, 512 script variables, 64 node flags i native game state. Zachowano oryginalną kolejność resetów; istniejący node-flags loop nadal zapisuje pierwszy slot, jak w źródle (pełny reset flag wymaga osobnej weryfikacji, nie jest tu deklarowany). Oryginalny allocator wydzielono z contact pass do `entity/block_alloc.c`, zachowując 16 slots, 0x47E0 blocks, selection/skip order i 1/2/4/8 sizing. Adresy są uintptr_t, iteracja tabeli używa rzeczywistych elementów, braku backing memory i run poza tabelę nie dereferencjonuje. Headless fixture i ASan/UBSan sprawdzają free-list stride/count, variable clear, mode flags, allocation/exhaustion/free/reuse oraz no-buffer failure. Procedury linkują się do programu; production memory partition, full actor create/contact/script/frame integration nadal wymagają pracy.

Oryginalny `script/node_pool.c` jest teraz native target i linkuje się do programu: init, free-list, alloc i garbage collection pozostają oryginalnymi procedurami. Reset zeruje całe 72 native records zamiast jedenastu 32-bit words na rekord; operand storage ma 16 words. Zapewniono prawdziwe linker-only node/operand globals. Allocation przy pustej liście zwraca NULL zamiast dereferencji NULL; obsługa tego wyniku w wszystkich wywołujących nadal wymaga audytu. Relokacja i flush scene maps zostały przeniesione bez zmiany treści do `script/reloc_block.c` i nadal nie są native target. Test i ASan/UBSan obejmują wszystkie 72 allocations, insertion prev/next, exhaustion, head/middle/tail collection, actor-death collection i u16 sequence wrap. Sygnatury init w oryginalnym boot poprawiono na rzeczywiste void functions. Interpreter i full scene/frame integration nadal są niedokończone.

Oryginalny `Script_RunQueue` wydzielono z `flow_opcodes.c` do `script/queue.c` i kompiluje się jako osobny object target. Zachowano dispatch/yield/node-delay ordering i dekodowanie operand modes; actor locals używają rzeczywistego `pad_0AC`, player/current entity aliases są bezpośrednimi oryginalnymi globals, odczyt flags nie narusza scalar aliasing. Test headless oraz ASan/UBSan sprawdzają literal/local/global/operand/node-flag arguments, drugi modes word po piątym argumencie, cursor writeback, yield, delay, dead-node skip i player-only gating. Handler table w tym teście jest jawnie synthetic fixture; runtime interpreter nie jest jeszcze linkowany do głównej gry, bo właściwa pełna tabela oryginalnych opcode handlers i dependencies nadal wymagają portowania. To nie dowód wykonania rzeczywistych scen ani zastępczy game VM.

`docs/SCRIPT_OPCODE_TABLE.json` zapisuje oryginalną tabelę USA 0x800910A0..0x80091464: 241 slots, 220 named handlers, 21 null slots, zero unresolved symbols. To katalog adresów/nazw, nie runtime implementation table. Oryginalny `Script_EvalExpr` przeniesiono z value_ops do expression.c, zachowując 24 operacje i existing argument record; jest linkowany do programu. Word add/sub/multiply/negate i masked MIPS shift counts nie powodują host signed-overflow UB; signed right shift ma jawny floor behavior. Testy obejmują overflow bit patterns, signed comparisons, nonzero division/remainder i fixed-point functions; arithmetic handler wywoływany jest też z oryginalnej kolejki przez jawny test-only ABI adapter. ASan/UBSan obu fixtures przechodzą. Divide-by-zero/invalid expression fault semantics nadal wymagają audytu. Wszystkie pozostałe oryginalne handlery, bezpieczna wspólna sygnatura dispatch i pełne runtime table binding nadal wymagają portowania.

Interpreter i portowane handlery mają wspólną sygnaturę `int(int **)`, bez testowego ABI adaptera dla wyrażeń. Oryginalne Script_AssignValue, Script_JumpToEntityOffset, Entity_SetCurrentNodeActive, Entity_MarkCurrentDead i Entity_MarkCurrentNodeFree wydzielono z flow_opcodes do basic_flow.c i linkują się do programu; zmiany ograniczają się do sygnatur/host addressing. `g_ScriptCursor` i `g_ScriptCursorAddress` ponownie współdzielą jedną native union, jak potwierdza oryginalny linker alias, zamiast niezależnych globals. Fixture wykonuje przypisanie→delay, jump i actor death przez oryginalną kolejkę oraz sprawdza pointer-width cursor alias. ASan/UBSan/function sanitizer przechodzą. Pełna tabela 220 handlerów nadal nie jest runtime-bound; interpreter pozostaje osobnym object target. Pozostałe handler signatures i historyczne 32-bit lokale adresowe w flow/node_actions nadal wymagają dostosowania.

Oryginalne Script_SpawnChildNode i Script_JumpIfZero są teraz native-linked. Child allocation zachowuje inline insertion za current node oraz event-node prepend do actor bucket 2; script addresses nie są obcinane do int. Pusta free list nie jest dereferencjonowana (pełny fault handling callers nadal pending). ScriptNode current integer view i next_value mieszczą native addresses, a adresowe cursor/stack lokale pozostałych source handlers otrzymały uintptr_t. Usunięto duplikaty node globals i stare cursor globals w script units; canonical storage pozostaje w node_pool/basic_flow. Test wykonuje oba spawn modes, inline child w tym samym interpreter pass oraz taken/not-taken conditional jump. ASan/UBSan/function sanitizer przechodzą dla node/queue fixtures. Pełna tabela handlerów, event scheduling i actual scene/campaign execution pozostają niedokończone; zmiany w jeszcze niekompilowanych handler modules nie są dowodem ich działania.

Cały obecny `src/main/entity/frame_update.c` kompiluje się do host object jako `parasite-eve-original-frame`, bez replacement stubs. Usunięto pozostałe stałe PS1 runtime-offset assertions Pe1GameState z scene_assets/menu_background; assertions serialized scene directory, bank/track/TIM records i GPU command formats pozostały. PM handler layout sprawdza canonical pm_types zamiast starego unload offset. Action-pointer rows w Scene_LoadMap mają native pointer elements, a Menu_RunFrameWithArg przyjmuje rzeczywisty void pointer; frame/battle calls nie obcinają adresu action record. Header/layout fixture przechodzi. Frame module nie jest jeszcze linkowany ani uruchomiony: unresolved dependencies obejmują original battle, scene loading, menu, PM i draw functions; raw offsets i game-global aliases nadal wymagają audytu. Kompilacja jest krokiem do original frame integration, nie dowodem gameplay.

Cały obecny `entity/allocation.c` (oryginalny contact pass) jest native-linked. Global mode scalar `g_GameStateFlags` i `g_GameStateBlock.flags` mają wspólne canonical storage; usunięto rozdzielone definicje z boot/draw oraz synthetic mode-flags definition z queue fixture. Shared window zachowuje 16-byte record prefix. Contact fixture wykonuje body task creation i duplicate suppression, skip flag, cylinder rollback, player/contact flags, nonplayer yaw restoration, battle per-part callbacks i parent-child skip. Callback argumenty są rzeczywistymi actor/part IDs. Przy pełnej node pool helper nie dereferencjonuje NULL; pełny fault/recovery audit jeszcze pending. ASan/UBSan/function sanitizer przechodzą dla contact fixture. Extreme-coordinate arithmetic, kompletna scena/frame integration i full campaign nadal są niezweryfikowane.

Oryginalne Entity_DispatchCallbacks, Entity_IntegratePositionFull i Conditional wydzielono z frame_update do entity/movement.c bez zmiany kolejności etapów. World word additions mają jawny wrap bez signed-overflow UB. Headless fixture wykonuje rzeczywisty Script_RunQueue i oryginalne assign/delay handlers w trzech actor buckets, potem input callback, gravity/acceleration/motion/delta. Sprawdza callback-before-input-before-motion ordering, pause 0x100 z zachowaniem callbacków i bez save/move, move lock, forced mode 17 oraz INT32 overflow. ASan/UBSan/function sanitizer przechodzą. Input rule table i opcode table pozostają kontrolowanymi fixtures; movement/queue są osobnymi object targets, nie runtime-linked z zastępczymi tabelami. Pełne authored rule/handler tables, scene scheduling i original frame execution nadal wymagają integracji.

Oryginalne Entity_SetActionMode, Entity_SetAction i Entity_AdvanceAnim wydzielono z actor_collision do entity/animation.c; Akao_LoadVoiceBank z asset/game_state do scene/actor_sound.c. Audio trigger korzysta z canonical BattleEntity/GameState pól zamiast sparse PS1 padding; jego rzeczywisty zwrot to int. Action rows korzystają bezpośrednio z native g_GameState.bank_rows (dawny D_800B0E98 alias), bez osobnego storage. Clock word add/stop-frame shift nie wywołują host signed-overflow UB. Headless fixture wykonuje action propagation do child, restart versus same-action preservation, parent-first clock, inherited frame, forward/reverse loop, stop clamp oraz static-player/dynamic-key sound triggers w obu kierunkach. ASan/UBSan/function sanitizer przechodzą. Positional sound endpoint i voice-entry data są jawnie fixtures; te moduły są osobnym object target, nie shipują stuba audio. Production audio routing, authored voice-entry storage/aliases i pełne scene/frame integration nadal są wymagane; bank selection poza udokumentowanymi groups 0..9 wymaga dalszego audytu.

Cały obecny `field/actor_collision.c` jest native-linked, bez zastępczych geometry functions. Relokacja zachowuje 40-byte serialized CollisionDatabaseFile i buduje header/region pointer table w zwykłej pamięci, zamiast wpisywać 64-bit adresy w disc bytes. Pozostałe triangle/vertex/region formats mają aktywne layout checks; pointer-word views są host-width. Room collision source wskazuje canonical bank_reset_948, D_8009D2E8 aliasuje prawdziwy move lock, a dwa floor-edge points/Z view współdzielą contiguous storage. Zapewniono visited bitmap i oryginalne neighbour sentinel values. Enemy runtime-slot assertions uwzględniają native alignment. Strict Disc 1 test i ASan/UBSan/function sanitizer relokują wszystkie 414 pokoje, sprawdzają counts, table/region pointers, visited-word conversion i idempotent immutable data. Pełny payload bounds/fault audit oraz zachowanie floor/wall/ramp traversal i all movement scenarios nadal wymagają testów; obecny dowód dotyczy relokacji, nie pełnej eksploracji.

## 1. Stan rzeczywisty i zakres wydania

Opcode 114 / SetRenderEntryEnabled zmienia tylko bit 2 native background flags, zachowując prędkości i pozostałe tryby. Real VM fixture ukrywa tile, wykonuje motion step na ukrytym wpisie, ponownie go pokazuje i potwierdza zmienioną draw position. Niezerowe enabled values są true; invalid index nie zmienia stanu. 35 headless testów i ASan/UBSan native scene/VM fixture przechodzą. Controller/group animation, GPU packets oraz pełna frame/campaign integration pozostają wymagane.

Scene dispatch obsługuje background opcodes 187/188/230: scroll flag 4 z unsigned >>8 speed conversion, parallax flag 8 ze wrapped `(0x10000-arg)>>8` oraz signed-halfword position/base reset z render refresh 0x80. Enable/disable zachowuje pozostałe flagi i nadal zapisuje prędkość; position reset nie usuwa fractions. Real VM fixture sprawdza setters, resulting motion tick, negative speed, disable, malformed index i halfword position truncation. 35 headless testów i ASan/UBSan native scene/VM fixture przechodzą. Group animation/control records, visibility commands, frame scheduling i GPU/reference rendering pozostają wymagane.

Scene prepare posiada teraz persistent background motion array, dekodowaną transakcyjnie z authored entries; close zwalnia ją przed room bytes. Background step commit stosuje całą partię dopiero po sukcesie i odtwarza scroll flag 0x80 saved-camera snapshot poza pause 0x104. Draw-list extraction używa runtime flags/positions/moduli bez modyfikacji source blob. Native scene fixture sprawdza dwa half-pixel ticks widoczne w draw coordinates, pause/snapshot oraz atomic invalid-modulus failure. 35 headless testów, ASan/UBSan native scene fixture i pełny native audit przechodzą. Script motion setters, group animation/control tables, automatic frame ordering i GPU/reference images pozostają wymagane.

Background motion decoder przenosi authored base/position/speed/fraction i modulus fields do native state. Step wykonuje original 8.8 signed remainder scroll, następnie parallax replacement względem camera origin; uint32 arithmetic odtwarza overflow przed arithmetic shift. Scene flags 0x104 zatrzymują update, zero selected modulus odrzucany jest przed mutacją. Fixture sprawdza fractional carry, negative remainder, parallax precedence, pause i overflowing products; real-data audit krokowuje wszystkie entries dla starting cameras. 35 headless testów oraz ASan/UBSan fixture i pełny native audit przechodzą. Persistent per-scene motion state, application to draw lists, script setters i automatic frame integration pozostają wymagane.

Scene background extraction aktualizuje original source-minus-camera display offsets względem 160/112 i generuje selected-group draw list. Halfword wrapping zachowuje negative offsets; frame-origin pozostaje oddzielny od current position. Display state commit następuje po udanej ekstrakcji, więc capacity failure zachowuje list/count/previous display. Camera fixture sprawdza offset arithmetic, native scene/VM fixture sprawdza one-pixel scroll i transactional failure. 35 headless testy i ASan/UBSan native scene fixture przechodzą. Automatyczna kolejność frame callbacks, background animation oraz GPU/reference rendering pozostają wymagane.

Background draw-list core filtruje enabled flag/group, stosuje unsigned halfword display offsets, original signed remainder/modulus wrapping, 16px tile screen clipping i original OT depth window 8..4088. Zachowuje entry/tile order oraz source texture metadata; invalid selected modulus/span/capacity nie mutuje output/count. Audyt all initial-camera groups generuje 180717 draw tiles. 35 headless testów oraz ASan/UBSan background fixture i pełny native audit przechodzą, z testami negative offset, wrap, hidden group i capacity failure. GPU packet linking/order, fade/shade, dynamic background animation i reference rendering nadal wymagają pracy.

`pc/background.c` dekoduje GeomState mesh entries (56 bytes), checked signed entry-relative position/texture spans i packed tile position, UV, CLUT/page oraz semi-transparency bit. Scene prepare transakcyjnie wiąże borrowed background view do owned room bytes. Audyt Disc 1 odczytuje 4511 entries i 362647 tiles ze wszystkich 414 geometry blobs; malformed span nie zmienia output/view. 35 headless testów oraz ASan/UBSan background fixture i pełny native audit przechodzą. Runtime scrolling/animation, group visibility, GPU packets/order i rendered background/reference images pozostają wymagane.

Compound opcode 237 przenosi potwierdzone subcommands 2100 (explicit no-op), 2101 (game flags 0x800), 2200 (projected player scroll z signed word duration/mode) i 2201 (wrapped signed-halfword scroll origin). Unknown subcommands pozostają jawnie unsupported. Real VM test sprawdza origin bez zmiany position, game/render flag separation, player fixture projection i missing-player failure. 34 headless testy i ASan/UBSan native camera-VM fixture przechodzą. Pozostałe compound actions, automatic player creation i pełny frame/background driver pozostają wymagane.

Nowy `camera-vm` integration test wykonuje real decoded program: scroll start, completion query, JumpIfZero, one-tick Wait i powrót do query. Kamera jest krokowana między VM ticks; linear i smooth wariant kończą task dopiero po transition completion, w trzech ticks/12 instructions. Completion flags nie są ustawiane ręcznie w teście. 34 headless testy oraz ASan/UBSan całego native camera-VM fixture przechodzą; PsyZ link pozostaje nieinstrumentowany. To dowód protokołu VM/camera, nie pełnej game frame loop: callbacks, movement, render order i campaign scenarios pozostają wymagane.

Scene camera dispatch obsługuje potwierdzone opcodes 3/70/71/72: explicit linear/smooth target bez viewport clamping, completion query `(render_flags&7)==4` oraz reset do projected player focus. Scroll-start zachowuje halfword target/duration, current start i elapsed=1; disabled renderer jest original no-op dla skryptu, zero duration jest błędem bez mutacji. Test scene dispatch wykonuje start→dwa steps→completion query i reset; missing player jest błędem, valid-player branch używa jawnego fixture przed niezaimplementowanym Spawn8. 33 headless testy i ASan/UBSan camera fixture/pełny native audit przechodzą. VM polling flow, continuous frame calls, compound scroll commands i background draw updates pozostają wymagane.

Camera focus transition projektuje integer part pozycji 16.16 przez actual view/H przy centre 160/112, odejmuje wrapped focus height, dodaje signed viewport half sizes i clampuje target. Zachowuje start, elapsed=1, duration -1→30 oraz original mode masks (w tym zachowanie smooth bit dla mode -1). Scroll enable flags są osobne od render transition flags; scene startup inicjuje scroll flags=0x70 i focus height=0x180 jak InitViewState. Scene API wybiera live actor slot. Test obejmuje clamped target, negative focus height, disable/zero duration i rzeczywistą room scene; 33 headless testy oraz ASan/UBSan wrapper fixture i pełny native audit przechodzą. Script scroll/wait commands, continuous focus driver i background draw updates pozostają wymagane.

Camera transition step przenosi original modes 1/2/3, linear signed-halfword interpolation oraz cosine smoothing. Elapsed start/target/duration i screen offsets zachowują halfword wrap; mode 1 przechodzi w 2, a zakończenie ustawia original 0x84/0x80 flags po elapsed>duration. Disabled flag zwraca -20 bez mutacji; zero duration jest odrzucane zamiast division fault. Scene udostępnia explicit step. Fixture sprawdza midpoint/endpoint, mode completion, smoothing, screen offsets i signed boundary wrap; 33 headless testy oraz ASan/UBSan fixture i pełny native audit przechodzą. Start transition z projected actor focus, script wait/scroll commands i frame-driver wywołania nadal wymagają integracji.

Camera startup przenosi Render_DrawMeshList centre/origin/bounds i display translation: signed min/max sums dzielone przez dwa z truncation toward zero, source X/Y z GeomState +2C/+2E oraz wrapped halfword output offsets względem centre 160/112. Scene prepare inicjuje PeCameraScroll; select group nadal tylko zmienia view/H jak original LoadGeomState. Audyt inicjuje wszystkie 509 groups, fixture sprawdza ujemne odd sums i ekstremalne bounds. 33 headless testy oraz ASan/UBSan fixture i pełny native audit przechodzą. Background sprite/mesh packets, per-frame scroll offsets, actor focus i transitions pozostają wymagane.

Scena przełącza checked camera group i oznacza render refresh bit 0x80 jak Render_LoadGeomState; combined dispatch obsługuje potwierdzony opcode 130. ClampCameraPosition stosuje signed-halfword X/Y i selected viewport bounds wyłącznie przy render flag 0x40, zachowując pozycję przy wyłączonej fladze. Invalid group/operand nie zmienia aktywnej kamery. Testy sprawdzają low-half wrapping, min/max, disabled gating i scene dispatch selection; 33 headless testy oraz ASan/UBSan camera fixture i pełny native audit przechodzą. Actor focus projection, scroll-origin/screen-offset updates, transitions i dynamic bounds nadal wymagają integracji.

`pc/camera.c` dekoduje owned room taskAux GeomState: checked declared size, viewport offset +1C i group count halfword +0A. Wszystkie 414 payloads Disc 1 potwierdzają count*52 aż do końca bloba; 509 viewport groups dekoduje matrix rotation/word translation, projection distance, dimensions i signed bounds. Scena przechowuje bank view i group zero transakcyjnie. Audyt neutral room models używa teraz rzeczywistego first-camera matrix/H, z początkowym screen centre 160/112; external animation audit pozostaje synthetic view. 33 testy headless oraz ASan/UBSan camera fixture i pełnego native audytu przechodzą. Camera group switching, scrolling/focus/transition, depth cue, background geometry i reference images nadal wymagają implementacji/weryfikacji.

RTPT tails odtwarzają teraz original raw packed reads: ostatnia trójka może zawierać następne vertices lub bytes z colour section. Preflight wymaga całych 22 bytes w owned model blob; brak danych zachowuje output. Overlapping stores do declared vertex slots zachowują original part order; stores poza declared array są pomijane, ponieważ checked model primitive references nie mogą ich użyć. `written` liczy logical visible vertices, nie padding writes. Fixture wykazuje wpływ colour bytes na shared flags, overlap i truncation. Synthetic-view projection przechodzi wszystkie 878 neutral room models oraz 2630446 vertices w 5901 external animation frames. 32 testy headless, ASan/UBSan wrapper fixture i pełny native data audit przechodzą; PsyZ pozostaje nieinstrumentowaną biblioteką. Actual game cameras, draw order, lighting/packets i reference images nadal wymagają pracy.

Projection obsługuje teraz PsyZ RTPT triplets: SXY0–2 i SZ1–3 w FIFO order, wspólny accumulated FLAG oraz final IR0. Model batch używa RTPT dla pełnych trójek, zamiast trzech niezależnych RTPS. Fixture porównuje screen/depth z pojedynczymi projekcjami, flag union i final interpolation; model fixture wykrywa shared batch flags. 32 headless testy i ASan/UBSan native wrapper przechodzą. Audyt źródłowych room records wykazuje 8126 visible parts z count%3!=0: obecne tails używają RTPS, a original padded/overlapping RTPT reads nadal wymagają analizy i weryfikacji. Pełna zgodność tail flags, game camera, depth sorting i rendering pozostają nieudowodnione.

`pc/projection.c` używa headless PsyZ GTE RTPS z jawnie ustawionymi rotation/translation, raw 16.16 screen offsets, H, DQA/DQB. Wynik zachowuje SXY saturation, SZ depth, IR0 interpolation i FLAG; nie wymaga InitVideo ani okna. Visible model batch komponuje view/pose przez MAC translation, zachowuje hidden slots i output przy błędzie. Golden fixture sprawdza reciprocal rounding (199/139), near/behind depth, screen clipping i IR0; 32 testy headless przechodzą. Native wrapper/model batch przechodzi ASan/UBSan, przypięta biblioteka PsyZ nie jest instrumentowana tym fixture. Actual game cameras, RTPT batch flags, visibility/depth sorting, packets/GPU i reference rendered images nadal wymagają implementacji/weryfikacji.

Visible part vertex transforms łączą sprawdzane global vertex ranges z pose matrices w original part order. Wynik udostępnia RT MAC words i saturated IR coordinates; hidden/unreferenced slots zachowują poprzedni output, błędy capacity/pose/reference zachowują całą partię. Audyt przelicza 2630446 wierzchołków w 5901 external animation frames i 231890 visible vertices w neutral poses 878 room models. 31 testów headless oraz ASan/UBSan matrix fixture i pełnego audytu przechodzą. Ten etap nie projektuje na ekran: view composition, perspective/depth, lighting/packets, GPU draw i reference images pozostają wymagane.

Entity texture loader przenosi primary bank area+3 oraz day+0x26 upload tylko dla dni 2–6; pozostałe known days używają primary-only jak source gating. CPU VRAM staging zapewnia atomowość obu banków i zachowanie stats przy błędzie drugiego. Public blob upload zachowuje existing primary/secondary-relative layouts i preflight całej partii. Test 40 area/day selections wykonuje 570 uploads / 727210 words; forced second-bank failure nie zmienia VRAM. 31 testów headless oraz ASan/UBSan VRAM fixture i pełny audit przechodzą. Automatic loading stages, GPU VRAM synchronization, textures-to-model/reference image validation i complete Spawn8 pozostają wymagane.

Scena posiada teraz także area action-bank bytes. Explicit loader stosuje original `(entity_id-10)/2*8+day+0x16` dla known entity IDs 10–13/days 1–8, zachowuje primary root i odtwarza room→primary→area view layering przed wymianą. Successful selection zapisuje observable entity ID/day; primary replacement czyści area metadata/views. Aktywne bindings blokują hot reload. Test przechodzi 16 area banks, sprawdza root preservation/failure i używa owned override animation po zamknięciu disc source. 31 headless testów oraz pełny ASan/UBSan audit przechodzą. Automatyczny story selection, day textures, streams, runtime rebind i kompletny Spawn8 pozostają wymagane.

Scena posiada primary entity-bank bytes i explicit loader dla oryginalnych area IDs 10–14 (sector bank area+8). Checked bank views aktualizowane są dopiero po poprawnym odczycie; wymiana przed actor binding odtwarza room overrides, aby nie zostawić pointers do zwolnionego poprzedniego banku. Aktywne modele/animacje blokują wymianę do implementacji hot reload. Test ładuje wszystkie pięć primary banks do Tutorial scene, wiąże model/action z native actor i oblicza kolejną pozę po zamknięciu disc source; invalid area/active binding zachowuje poprzedni bank. 31 testów headless oraz pełny ASan/UBSan audit przechodzą. Automatyczny wybór area/day, TIM uploads, area-action overrides, hot reload i kompletny Spawn8 nadal wymagają pracy.

Actor opcodes 101/102/223/224 zerują motion+acceleration, liczą motion X/Z z yaw i move_factor/move_speed oraz włączają/wyłączają gravity bit 2. Velocity zachowuje player multiplier pięć, signed 16.16 floor multiplication, word-wrap negation i istniejące motion Y; ClearMotion nie usuwa delta/gravity. Testy obejmują player/nonplayer, quarter/diagonal angles, fractional negative rounding, INT_MIN i gravity gating. 31 testów headless oraz ASan/UBSan aktora przechodzą. Oryginalne input callbacks i pełny collision/movement driver pozostają wymagane; sam velocity opcode nie oznacza sterowanej eksploracji.

Actor dispatch przenosi potwierdzone opcodes 27/77/78/93: move_speed z arithmetic shift >>4, raw anim_step, u16 frame seek clamp do last frame oraz raw move_factor. Nazwany w referencji AnimSpeed opcode 78 faktycznie ustawia klatkę. Player scale aktualizuje camera focus height przez wrapped multiply/shift, negative rounding correction i signed-halfword storage; pola pozostają jawne, bez UB signed overflow. Test sprawdza ujemne i skrajne wartości, camera truncation, frame clamp i brak mutacji przy missing operand. 31 testów headless i ASan/UBSan aktora przechodzą. Player camera integration, movement callbacks i pełny frame driver pozostają wymagane.

Actor dispatch obsługuje potwierdzone opcodes 11/12 Set/GetEntityVec3: position, saved position, motion, acceleration, gravity, signed-halfword rotation i delta. Set position stosuje FindFloor, zapisuje wynik do rollback base i oznacza player render flag 0x80; collision context wiązany jest do owned sceny po jej przeniesieniu. Unknown mode pozostaje original no-op, brak argumentów/kontekstu nie zmienia aktora. Test roundtrip wszystkich wektorów obejmuje skrajne wartości, signed rotation, floor adjustment i rzeczywisty VM getter. 31 testów headless oraz ASan/UBSan aktora przechodzą. Frame motion/input callbacks, global render-flag integration, wall/contact response i kompletny Spawn8 pozostają wymagane.

Scene dispatch obsługuje opcode 17 / Script_DispatchCmd z oryginalnej tabeli: all-mask held/pressed/released queries oraz najwyższy-bit held counter. PeInput zlicza 32 wrapped u32 counters, zerując released keys; input state przechodzi przez udaną zmianę sceny. Counter query zachowuje CLZ writeback do argumentu maski, z wyjątkiem high-bit mask; nieznany mode jest original no-op. Zero mask counter odrzucany jest jako oryginalny błędny index -1. Nowy test wykonuje rzeczywisty VM, sprawdza counter/operand mutation/high-bit/unsupported/error; 31 headless testów i ASan/UBSan input/VM fixture przechodzą. Remapping/locks, movement callbacks i pełna tick integration pozostają wymagane.

Actor collection stosuje teraz oryginalne direct-parent deletion: usuwa aktora przy własnej fladze 0x10 lub fladze bezpośredniego rodzica. Preflight sprawdza parent index/ID; taski są zwalniane także dla child bez własnej death flag, a jego zachowane flags nie otrzymują 0x10. Dlatego grandchild takiego child może przeżyć, jak w oryginalnym control flow; nie zastępujemy tego recursive cascade. Test obejmuje parent/child removal, grandchild survival, task counts, detach i stale-ID failure bez mutacji. 30 testów headless oraz ASan/UBSan aktora przechodzą. Rozwiązanie surviving references do usuniętych rodziców, attachment/animation state i pełna frame integration nadal wymagają pracy; obecne hierarchy guards odrzucają takie cele podczas transformacji.

Collision rollback primitive odtwarza Entity_RollbackPositionHierarchy: root odzyskuje saved position/floor mirror i flagę 0x40000, potem przywracane są wszystkie siblings na każdym poziomie ścieżki od root do wywołującego. Rotacja/motion pozostają bez zmian. Iteracyjna bounded ścieżka zastępuje rekurencję; list/hierarchy/stale-ID preflight zapobiega częściowym zmianom przy błędzie. Test obejmuje trzy poziomy, sibling rollback, root-only flag, rotation preservation i failure. 30 testów headless oraz ASan/UBSan aktora przechodzą. Wywoływanie z właściwej detekcji ścian/kontaktów, floor response i pełny physics driver nadal wymagają pracy.

Native parent links używają slot index i field-ID guard, aby recycled slot nie stał się przypadkowym rodzicem. SetParent odrzuca cykle i błędne cele przed mutacją. CopyParentPosition waliduje całą listę/hierarchię, potem kopiuje position/rotation przy fladze 0x400000 w oryginalnej actor-list order. Nie wykonuje topological reorder: child-before-parent otrzymuje dotychczasową pozycję rodzica. Test sprawdza gating, trzy poziomy/order, detach, cycle i stale ID bez częściowej zmiany pozycji. 30 testów headless i ASan/UBSan aktora przechodzą. Parent animation propagation, attachment transforms, collision rollback, deletion cascade oraz wywołanie w pełnej pętli pozostają wymagane.

Natywny PeInput przechowuje held/pressed/released z original changed-bit edge logic; scena udostępnia `pe_scene_set_input` bez SDL. Bounded Scene_CheckFlagBits rules zachowują all/any masks, press-before-release priority, unconditional callbacks, sticky triggered/continuation i fallback; callback zmiana mode nie przekierowuje bieżącej listy. Missing terminator/index odrzucany jest przed callbackami. 30 testów headless i ASan/UBSan input fixture przechodzą; scene test potwierdza press/release injection. Dekodowanie oryginalnych rule tables, właściwe movement callbacks, input lock/remapping i wywołania w pełnym frame driverze pozostają wymagane. Sam injection nie dowodzi sterowanej rozgrywki.

Actor motion core zapisuje poprzednią pozycję/rotację przed callback phase i stosuje grawitację warunkowaną flagą 2, acceleration, motion i delta w oryginalnej kolejności, przez jawne unsigned word wrap. Spawn zeruje pola, zachowując początkową gravity_y=0x4000. Testy obejmują gravity gating, overflow, cumulative steps i 100000 deterministycznych skrajnych wejść z niezależnym 64-bit modular arithmetic oracle. 29 testów headless i ASan/UBSan aktora przechodzą. Te primitives nie stanowią jeszcze kompletnego ruchu: callbacks/input rules, conditional freeze w driverze, floor/wall/contact response, rollback/parent hierarchy i wywołania w pełnej pętli pozostają wymagane.

Event scheduler obsługuje teraz stable pointer targets zamiast wymagania contiguous PeScriptActor array. Native actor adapter zbiera live actors w list order, waliduje linkage/cycles/live count i dopiero wtedy dostarcza kolejkę; duplicate/null targets są odrzucane bez mutacji. Taski zachowują rzeczywiste borrowed actor pointers, trigger value i argument. Scena udostępnia explicit event delivery; testy obejmują native receiver, corrupt list, duplicate target, queue preservation oraz New Game scene queue. 29 testów headless i ASan/UBSan actor/scheduler fixtures przechodzą. Automatyczne wywołanie w pełnej kolejności frame callbacks, physics i render loop nadal wymaga integracji.

Scena posiada teraz native PeActors pool zamiast osobnej tablicy PeScriptActor oraz owned scene audio. Initial map descriptors tworzą stabilne slots/task references; close zwalnia actor models i muzykę przed room bytes. Scheduler wrapper zachowuje własny context, a jego combined dispatch używa przeniesionej sceny i kieruje komendy do aktorów/audio. `pe_scene_actor_scripts` wykonuje task buckets jednego aktora z katalogiem pożyczonym wyłącznie na czas wywołania. Testy rzeczywistych ticków New Game/Tutorial zachowują jawne unsupported errors; Tutorial uruchamia owned muzykę i dochodzi do Spawn8. 29 testów headless oraz pełny ASan/UBSan audit przechodzą, w tym failure rollback i close po zamknięciu obrazu. To jeszcze nie pełna pętla klatki: event delivery, callback/physics order, model bank selection, parent state, synthesis, rendering i complete Spawn8 pozostają wymagane.

Actor dispatch obsługuje potwierdzone w oryginalnej tabeli opcodes 46/47/48. 46 wykonuje forced action restart i czyści pause flag; 47 maskuje argument do u16, clampuje target do last frame i ustawia stop flag; 48 retry zachowuje PC do integer-frame match, po czym yield przesuwa PC i czeka jeden tick. Actor wybierany jest przez stabilny script pointer z live pool. Test rzeczywistego VM wykonuje cały ciąg wraz z animation advances oraz błędy operand/unknown opcode; 29 testów headless i ASan/UBSan aktora przechodzą. Parent action propagation jest jawnie nieobsługiwane; podłączenie combined dispatch w pełnej pętli scen i kompletny Spawn8 pozostają wymagane.

Actor world pose buduje macierz z signed integer position pobranej z 16.16, oryginalnego RotMatrix (inna kolejność niż bone YXZ) i signed-halfword move_speed scale przez GTE-style composition. Flagi 0x10040 zachowują poprzednią world matrix; pose failure zachowuje także world state. Native XYZ i YXZ zgadzają się z PsyZ dla 65536 signed-angle przypadków; fixture sprawdza ujemną/ułamkową pozycję, obrót o ćwierć obrotu, half scale, flag preservation i failure. 29 testów headless oraz ASan/UBSan aktora przechodzą. Scene-loop calls, parent transforms, view/projection, alternate skin modes i rendered reference validation pozostają wymagane.

Natywny aktor posiada własny PeActorModel: source/pose matrices, dwa overrides, hit volumes i body values. Bind kopiuje checked action views i inicjuje macierze identity, po udanej alokacji zastępuje poprzedni model; dane modelu/akcji pozostają borrowed od sceny/banku. SetAction wybiera sprawdzony slot, pose stosuje bieżącą integer frame i jawny base matrix. Błąd nie zmienia poprzednich sources/pose/volumes/body. Collection zwalnia model przed recyklingiem aktora. Test obejmuje bind/failure/action/advance/pose/volume/delete; rzeczywista ścieżka aktora przechodzi wszystkie 5901 klatek 225 external actions i zgadza się z direct pose arithmetic. 29 testów headless oraz ASan/UBSan fixture i pełnego audytu przechodzą. Scene bank selection/ownership, world transform construction, parent propagation, physical movement, packet generation i kompletny Spawn8 pozostają wymagane.

Owned sector-bank loader odczytuje granice oryginalnego PE.IMG, rozróżnia asset/room directory layouts i zachowuje poprzedni blob przy błędzie I/O/header/index. External entity views instalują primary root w slot zero lub tylko area action overrides z zachowaniem root; source IDs indeksują group zero, bez używania room group byte. Audyt Disc 1 obejmuje pięć primary models (banki 18–22) i 16 area overrides (23–38): 225 supplied actions, 5901 klatek i 173941 pose matrices. Area poses sprawdzane są z bazowymi modelami 18/20; to dowód kompatybilności danych, nie kompletnego runtime selection. 29 testów headless oraz ASan/UBSan fixture i pełnego audytu przechodzą. Wybór banków według sceny/day, owned lifetime w scenie, TIM uploads i actor model allocation pozostają wymagane.

`pe_animation_pose` łączy klatkę animacji, ordered overrides, persistent source matrices i command hierarchy modelu. Nieanimowane części zachowują dotychczasowe macierze; błąd frame/capacity/bone count nie zmienia sources, pose ani written. Audyt wszystkich klatek 7126 strumieni z lokalnymi model roots Disc 1 wylicza 7105103 pose matrices, zero incompatible streams. Pozostałe 1916 strumieni nie ma local root i wymaga external entity banks; to jawny brak integracji, nie zaliczona zawartość. 29 testów headless oraz ASan/UBSan fixture i pełny audyt przechodzą. Actor model allocation, world/view transforms, timing w pętli scen i reference rendered/animated poses pozostają wymagane.

Lokalny zegar animacji aktora zachowuje oryginalny 16.16 advance, previous frame, paused/stop flags, forward wrap z usunięciem części ułamkowej i backward wrap z jej zachowaniem. 100000 deterministycznych przypadków zgadza się z oryginalnym control flow. Aktor wiąże borrowed animation, rozróżnia zmianę akcji od forced restart, zachowuje stop frame i czyści stop flag jak Entity_SetAction/Mode. 29 testów headless oraz ASan/UBSan animacji/aktorów przechodzą. Parent/action propagation, voice-bank callback, zero-frame action binding, model allocation i wywołania z pełnej pętli scen nadal wymagają implementacji; inherited mode nie jest zastępowany lokalnym advance.

Macierze kości powstają teraz z kątów animacji przez native fixed-point YXZ z matematyczną quarter-wave table, bez runtime floating point. Porównanie z PsyZ obejmuje 65536 signed-angle przypadków i zachowanie translacji. Dwie kolejno stosowane korekty zachowują replace-before-add, halfword wrap i pomijanie index zero; błędny index/capacity nie mutuje output. Root dostaje translację klatki, pozostałe kości zachowują poprzednie translacje. Audyt 374431 klatek Disc 1 wylicza 10064925 macierzy kości. 29 testów headless, ASan/UBSan fixture i pełny audyt przechodzą. Wiązanie animacji z aktorem/modelem, timing/interpolacja i reference animated poses nadal wymagają implementacji.

Domyślny tryb pracy i odbioru portu to headless, bez widocznego okna i bez ingerencji użytkownika. Każdy podsystem musi udostępniać sterowanie stanem, inputem, czasem i RNG oraz obserwację stanu, obrazu, audio i błędów. Runner ma umożliwiać samodzielne uruchamianie scenariuszy, zapis/odczyt checkpointów i reprodukcję awarii. Save editor pozostaje osobnym projektem Gemini; może dostarczać fixtures, ale nie jest zależnością automatycznych testów portu. Obecny runner steruje boot/title; pełne sterowanie gameplay nadal wymaga implementacji.

`pc/animation.c` dekoduje kanały translacji i rotacji z oryginalnych strumieni byte/halfword, zachowując signed translacje, constant channels, stride i skalę kątów. Błędy zakresów zachowują output. Audyt odczytuje wszystkie 374431 zapisane klatki z 9042 strumieni lokacji Disc 1 (1675 halfword). Wszystkie 29 testów headless przechodzą; fixture animacji i pełny audyt danych przechodzą ASan/UBSan. Budowa macierzy z kątów, overrides, timing/interpolacja i weryfikacja rzeczywistych animated poses pozostają wymagane.

Command-mode hit volumes dekodują bounds i transformują eligible centres przez pose matrices z saturated RT coordinates. Metadata zachowuje part order, unscaled radius i bounds attributes; centres używają command order. Hidden/disabled entries są wyłączone, capacity/incomplete-pose error zachowuje output. Neutral-input audyt Disc 1 wylicza 4300 volumes w 878 modelach. 28 headless testów oraz ASan/UBSan fixture i pełnego scene/model audit przechodzi. Animated/alternate modes, scaling/contact integration i reference gameplay validation pozostają wymagane.

Porównanie PsyZ obejmuje teraz także matrix composition: trzy rotation columns względem RTIR i translation względem RT. Point transform i composition zgadzają się dla 10000 deterministycznych skrajnych przypadków, headless. 28 testów z Disc 1 przechodzi. Dowód dotyczy tych operacji arytmetycznych; animated poses i reference images pozostają nieweryfikowane.

Natywna transformacja punktu udostępnia pełne MAC words i saturated IR halfwords operacji RT. Headless `matrix-psyz` porównuje obie wartości z GTE przypiętego forka PsyZ dla 10000 deterministycznych skrajnych wejść; wszystkie są zgodne. Native matrix fixture przechodzi ASan/UBSan, a 28 testów headless z Disc 1 przechodzi. Hit-volume integration, view/perspective projection, lighting i referencyjne obrazy gry pozostają wymagane.

`pc/matrix.c` wykonuje command-mode poses w C: 12-bit fractional rotation, signed-halfword input, saturated rotation i wrapped word translation. Push/pop używa zwykłej pamięci; child translation pochodzi z Z części, root z source matrix. Błąd komendy/capacity zachowuje output. Audyt wylicza 17748 neutral-input matrices dla 878 modeli. 27 testów headless z Disc 1 oraz ASan/UBSan matrix fixture i pełnego audytu przechodzi. Animated inputs, alternate animation modes, hit-volume/view transforms i reference pose/image validation pozostają wymagane.

Model parser dekoduje matrix commands (part/push/pop), odrzuca stack underflow, błędne signed part indices i output-buffer overflow oraz zapisuje stack depth. Audyt Disc 1 waliduje 24942 komendy, 17748 matrix outputs w 878 modelach; maksymalna głębokość wynosi 4. Test sprawdza rodzaje, referencje i zachowanie output przy błędzie. 26 testów headless oraz ASan/UBSan fixture i pełnego audytu przechodzi. Egzekucja fixed-point transforms i rendering wyników pozostają wymagane.

Model accessors dekodują signed vertex positions, raw normal/color words, part visibility/ranges/translations i cztery klasy primitive descriptors. Open sprawdza vertex references trójkątów/quadów; accessor zachowuje output przy błędnym indeksie lub zmienionych malformed data. Audyt Disc 1 dekoduje 236410 vertices i 346985 descriptors w 878 modelach. 26 testów headless oraz ASan/UBSan model fixture i pełnego scene/model audit przechodzi. Lighting, transforms, matrix commands i rendering pozostają wymagane; decoded geometry nie dowodzi wyrenderowanej gry.

Model parser sprawdza quad/triangle texture spans i dekoduje UV pairs, CLUT oraz page words. Audyt Disc 1 odczytuje 338857 texture records w 878 modelach lokacji. Test obejmuje oba layouts, dokładne bit values, truncation i brak mutacji output przy błędnym dostępie. 26 testów headless z Disc 1 i ASan/UBSan model fixture oraz całego audytu scen/modeli przechodzi. Matrix commands, packet generation, animation i rendered image validation pozostają wymagane.

`pc/model.c` dekoduje RenderObjectHeader i sprawdza sekcje parts/vertices/colors/primitive descriptors/bounds oraz zaokrąglony span matrix commands. Waliduje się 878 model prefixes Disc 1 z 236410 vertices i 346985 descriptors. Nowy 26. test sprawdza offsets, zakresy części, truncation i zachowanie starego widoku przy błędzie. 26 testów headless z Disc 1 oraz ASan/UBSan model fixture i pełnego audytu przechodzi. Matrix execution, textures/packets, animation i rendering pozostają wymagane; prefix validation nie oznacza rendered model validation.

`pc/room_banks.c` wiąże sprawdzane room overrides do dziesięciu model root slots i dziesięciu grup po 48 animation slots. Zachowuje kolejność/last-write, pożycza owned room bytes sceny i nie niszczy starego widoku przy błędzie. Walidują się wszystkie 414 lokacje Disc 1: 878 root records, 9042 animation-row records. Nowy 25. test sprawdza granice slot/group/payload i duplicate overrides; headless suite oraz ASan/UBSan fixture i pełnego audytu scen przechodzą. External entity banks, dekodowanie modeli/animacji i renderowanie pozostają wymagane.

Natywny aktor stosuje FindFloor do Y i obu ID powierzchni; bypass/no-match zachowuje poprzednie wartości, a flaga preserve-Y jest respektowana. Scena posiada dane kolizji transakcyjnie, z zachowaniem starej geometrii przy nieudanym przejściu. Headless boot emituje `room.collision.ready`, wymagane w replay New Game i Tutorial. 24 testy z Disc 1 oraz ASan/UBSan aktorów i scen/kolizji przechodzą. Pełne spawn/model setup, ruch fizyczny i gameplay collision pozostają nieukończone.

31 braków centroid hit jest sklasyfikowane: M90 pomija trójkąty 0–30 w początkowych listach regionów (region 0 pusty, region 1 zawiera pozostałe 75). Oryginalny lookup szuka tylko wpisanych faces. Audyt sprawdza dokładnie to pominięcie i zero niewyjaśnionych braków; fixture potwierdza, że sama geometria bez referencji regionu nie daje floor hit. 24 testy headless z Disc 1 i ASan/UBSan collision fixture przechodzą. To wyjaśnia początkowy audyt danych, nie runtime geometry changes ani gameplay collision.

Ładowanie kolizji wskazuje rzeczywisty short record taskData pokoju. Dane Disc 1 skorygowały nagłówek: floor triangle count to halfword +04, plane count +06, a +08 opisuje bitmapę traversal. Dekodują się wszystkie 414 baz (300 flat, 114 sloped), 423 regiony i 16858 trójkątów. Próby integer centroid znajdują floor dla 16827 punktów; 31 braków trafienia pozostaje jawne i nie jest dowodem gameplay collision. 24 testy headless z Disc 1 oraz ASan/UBSan fixture i całego audytu room/collision przechodzą. Integracja aktorów i referencyjne porównanie geometrii pozostają wymagane.

`pc/collision.c` posiada sprawdzaną bazę kolizji z offsetami względnymi i oryginalne wyszukiwanie pierwszego podłoża dla regionów płaskich/nachylonych. Plane height zachowuje wrap 16.16; flagi bypass/preserve-Y są obsłużone. Nowy 24. test fixture sprawdza ujemne/ułamkowe wysokości, kolejność regionów, złe referencje, transakcyjne ładowanie i niezależność danych. 24 testy headless z Disc 1 oraz ASan/UBSan collision przechodzą. Wybór rzeczywistego assetu kolizji pokoju, referencyjne dane geometrii, integracja aktorów i ściany nadal wymagają pracy; fixture nie dowodzi kolizji gry.

`pc/collision_math.c` przenosi oryginalny test punktu w trójkącie podłoża: parity crossings, półotwarte granice i zawijane signed iloczyny 32-bitowe. Nowy 23. test sprawdza kolejność wierzchołków, jednoznaczność wspólnych krawędzi, degenerację i skrajne współrzędne. 23 testy headless z Disc 1 oraz ASan/UBSan collision math przechodzą. Dekodowanie bazy kolizji, wysokość płaskiego/nachylonego podłoża i integracja aktorów nadal wymagają implementacji; sam prymityw nie oznacza działającej kolizji.

Odroczona kolekcja aktorów zwalnia taski przed recyklingiem slotu, wypina head/middle/tail, czyści flagę przejściową i zwalnia blokady ruchu gracza jak w oryginale. Reużyty slot dostaje nowe ID. Test obejmuje deletion, pełną/pustą listę, reuse i uszkodzone linki; 22 testy headless z Disc 1 oraz ASan/UBSan aktorów przechodzą. Hierarchie parent, modele i pełna egzekucja scen nadal wymagają implementacji.

`pc/actors.c` implementuje rdzeń puli 14 aktorów ze stabilnymi adresami skryptów, wstawianiem po aktorze wywołującym/na początku listy, ID per room, pozycją i początkową grawitacją/ruchem. Tworzenie wiąże task typu; type 0 ustawia player. Nowy 22. test `actors` obejmuje kolejność, ID, defaults, borrowed task pointers oraz wyczerpanie obu pul bez utraty serial/list state. 22 testy headless z Disc 1 przechodzą; actor test przechodzi ASan/UBSan. Pełny opcode 8 pozostaje jawnie nieobsługiwany do implementacji modeli i wyszukiwania podłoża; rdzeń nie oznacza fizycznej symulacji ani działającej sceny.

Audio D0/D3/D6 ustawiają globalną skalę tempo/pitch jako maskowany bajt w 16.16 i anulują odpowiednie czasy slide. D6 stosuje obie zmiany. Test sprawdza rozdzielne/łączne wartości, truncation i rzeczywisty operand D0 Tutorial; task dochodzi do actor opcode 8 przy `0x19e8`. 21 testów headless z Disc 1 i ASan/UBSan scen/audio przechodzi. Zastosowanie globali w pełnym harmonogramie sterownika audio i syntezie pozostaje wymagane.

Audio CD stosuje natychmiastową głośność banku wybranego przez cached key/handle, zeruje czas slide, oznacza aktywne głosy i zapisuje bajt głośności/flagę slotu. Test rozróżnia 7-bit gain sterownika od pełnego bajtu slotu i sprawdza brak efektu dla nieznanego klucza. Tutorial dochodzi do audio D0 przy `0x19c0`. 21 testów headless z Disc 1 i ASan/UBSan audytu scen/audio przechodzi. Pełne odtwarzanie i egzekucja scen pozostają wymagane.

Opcode 28 kolejkuje zdarzenia skryptowe, mimo nazwy SFX w referencji. Kolejka zachowuje 28 wpisów, u8/u16 argumenty i ID nadawcy. Scheduler dostarcza je przez type/sub-ID lub selector aktora, tworzy taski grupy 2 z flagą 4, argumentem i trigger value oraz sprawdza cursor/pojemność przed alokacją. Test obejmuje wielu odbiorców, selector i brak częściowego dostarczenia przy przepełnieniu pod ASan/UBSan. ID aktorów i sekwencja tasków sceny zaczynają się od 1 jak w Draw_InitPipeline. New Game dochodzi do następnej komendy audio przy `0x394`. Dostarczanie zdarzeń w pełnej pętli scen i fizyczna symulacja aktorów pozostają wymagane.

Opcode 136 czyści bit `0x40` we wspólnym trybie fade ekranu. Nazwa handlera w referencji mylnie sugeruje zmianę flag aktora. Test VM sprawdza zachowanie pozostałych bitów i actor flags; rzeczywisty New Game wykonuje zmianę przed następnym brakującym opcode. 21 testów headless z Disc 1 i ASan/UBSan testu VM przechodzi. Zastosowanie stanu fade do renderowania scen pozostaje nieukończone.

Opcodes 170/171 ustawiają/czyszczą `0x2000` w osobnym natywnym `game_flags` odpowiadającym `g_GameState.flags`, a nie globalnym flagom skryptowym 81–83. Audio może wiązać ten wspólny stan. Test VM sprawdza zachowanie pozostałych bitów i oddzielenie obu pól; rzeczywisty New Game wykonuje reset→set i dochodzi do brakującego opcode 136 przy `0xb4`. 21 testów headless z Disc 1 przechodzi; VM i audyt scen/audio przechodzą ASan/UBSan.

Test wykonuje teraz rzeczywiste początkowe ticki tasków Tutorial/New Game z dispatch audio. Tutorial uruchamia muzykę i zatrzymuje się na audio CD przy `0x19a0`; New Game resetuje audio i zatrzymuje się na actor opcode 170 przy `0xac`. To jawne dowody braków integracji scen, nie poprawnego przejścia gameplay. Test brakującego klucza ponownie dekoduje zmieniony literal przed dispatch. 21 testów headless z Disc 1 i ASan/UBSan audytu scen przechodzi.

Dispatch audio obsługuje świeży start głównego banku C8 z rzeczywistej komendy Tutorial M353: klucz rekordu, własne dane muzyki, wynik ID banku zapisany przez operand oraz slot/volume/flags. Powtórzenie już grającego klucza zachowuje player i wynik; nieobecny klucz jest oryginalnym no-op. Test sprawdza rzeczywiste dekodowane operandy. Restart cached/secondary banków i kolejność kolejki przy tym samym ID pozostają jawnie nieobsługiwane; pełna pętla scen i synteza nadal wymagają implementacji.

Dispatch `pc/scene_audio.c` obsługuje actor opcode 234 / audio D9: czyści oba zestawy kluczy/ID banków, maskę flag `0xf0` i wywołuje F0 dołączonych playerów. Test wykonuje rzeczywistą komendę M431 po zamknięciu Disc 1, sprawdza stan slotów/flag i zatrzymuje własną muzykę Tutorial po trzech kolejnych tickach. Pozostałe komendy skryptowe audio, pełna egzekucja scen i słyszalny wynik nadal pozostają nieukończone.

Player banku głównego obsługuje F0 używane przez reset audio sceny: key-off dla 24 głosów, release rate 5 z zachowanym trybem release oraz przejście do stop po trzech należnych tickach muzycznych. Bank nieaktywny pozostaje bez zmian. Test playera sprawdza czas i zachowany control pod ASan/UBSan; 21 testów przechodzi headless z Disc 1. Bank wtórny, dispatch resetu skryptowego i rzeczywisty decay obwiedni w syntezie nadal wymagają integracji.

`pc/music.c` ładuje muzykę główną po kluczu rekordu sceny, transakcyjnie. Właściciel kopiuje sekwencję, resident samples i grammar, ładuje wskazany bank próbek i uruchamia player pod stabilnym adresem. Błędny klucz zachowuje poprzednią muzykę. Test Disc 1 wykonuje 4096 ticków po zwolnieniu pokoju/banku wejściowego i zamknięciu obrazu płyty. Dispatch komendy skryptowej, reset pozycyjnego audio, nested banks i słyszalne odtwarzanie nadal wymagają integracji.

DE/DF konfigurują slide głębokości volume i auxiliary LFO. Kroki głosu przesuwają zawijane target i przeliczają modulację między krokami przebiegu bez zużywania próbek. Test jednoczesnych malejących slide sprawdza signed wynik, zachowanie cursor oraz zakończenie; przechodzi ASan/UBSan. 21 testów przechodzi headless z Disc 1. Pełny harmonogram sterownika głosów i synteza nadal wymagają integracji i referencyjnych testów audio.

`PE_AKAO_AUDIT_TICKS` ustawia okno audytu room/audio od 4 do 65536 ticków (domyślnie 4096). Rozszerzony audyt Disc 1: 38651480 ticków muzycznych, 17315934 zdarzenia nut, dziesięć sekwencji zakończonych, 589 nadal aktywnych i zero błędów komend/stanu. Aktywne pętle nie są uznane za ukończone utwory. Harmonogram aktualizacji głosów/LFO, synteza oraz porównanie referencyjnego audio pozostają wymagane.

DD ustawia slide głębokości pitch-LFO (zero czasu oznacza 256). Aktualizacja głosu przesuwa zawijany target, przelicza głębokość i ponownie odczytuje bieżący przebieg bez przesuwania cursor między krokami fazy. Test malejącego slide, resamplingu i końca przechodzi ASan/UBSan; 21 testów przechodzi headless z Disc 1. Audyt 4096 ticków: jedna sekwencja zakończona, 598 nadal aktywnych, zero brakujących komend, 2449892 ticki i 1079209 zdarzeń nut. To ograniczony audyt sekwencera, bez harmonogramu kroków głosów, syntezy i dowodów pełnych utworów.

B4/B5/B6 obsługują konfigurację, głębokość i reset pitch-LFO. Obliczenia zachowują dwa tryby skalowania, low-word pitch przy inicjalizacji, opóźnienie, obejście przy key-on oraz oryginalne podwojenie nieujemnego wyniku. Testy obu trybów, czasu zero=256, signed wartości i resetu przechodzą ASan/UBSan; 21 testów przechodzi headless z Disc 1. Audyt 4096 ticków trafia teraz na DD w jednej sekwencji; 598 pozostaje aktywnych. Slide głębokości, harmonogram/zastosowanie głosów i referencyjna synteza nadal wymagają implementacji.

CC ustawia oryginalną fazę frazy dla legato: pierwsza nuta żąda key-on, kolejne zachowują ciągłość głosu. CD odpowiada pustemu handlerowi oryginału i nie ma operandów ani efektów. Test timing sprawdza fazę i key-on oraz przechodzi ASan/UBSan. Audyt nadal jawnie zgłasza dalsze brakujące komendy; słyszalne legato wymaga integracji syntezy i porównania referencyjnego.

Dłuższy audyt muzyki Disc 1 uruchamia każdą pełną sekwencję przez maksymalnie 4096 wymuszonych ticków muzycznych. Wynik: 2447792 ticki, 1077926 zdarzeń nut, 589 sekwencji nadal aktywnych, dziesięć błędów brakujących komend (B4: jedna, CC: dziewięć), zero zakończonych sekwencji w tym oknie. Audyt ujawnia dalsze braki; nie potwierdza pełnych utworów, syntezy, referencyjnego audio ani sterownika głosów. Wyniki i histogram pozostają jawne w logu `room-assets`.

B8/B9/BA konfigurują opóźnienie, przebieg, głębokość i reset volume-LFO kanału głównego. Osobny krok głosu zachowuje gating opóźnienia, obejście przy key-on i signed arytmetykę fixed-point z wrap. Nowe nuty restartują przebieg i zerują bieżącą wartość. Testy obejmują fazę, wartości dodatnie/ujemne, reset i key-on oraz przechodzą ASan/UBSan. Slide głębokości, pitch-LFO, harmonogram sterownika głosów i słyszalna synteza pozostają wymagane.

BC ustawia czas, selektor i fazę pomocniczego LFO. Osobny krok sterownika głosu odczytuje przebieg i liczy signed modulację z oryginalnym zaokrągleniem arithmetic shift. Nuty restartują przebieg, tie go zachowuje; nowe nuty zerują także offset pitch-slide. Test obejmuje fazę, wartości ujemne, restart, błędne cursor i brak mutacji przy błędzie, również pod ASan/UBSan. Harmonogram sterownika głosów i zastosowanie modulacji w syntezie pozostają wymagane. Audyt osiąga 8869 początkowych zdarzeń nut i 12 brakujących komend volume-LFO; 588 prefiksów sekwencji przechodzi cztery wymuszone ticki, a 11 trafia na brakującą komendę. Nie jest to dowód pełnych utworów ani słyszalnej modulacji.

Katalog scen posiada teraz 16 offsetów przebiegów LFO i 708 słów próbek, dekodowanych z danych EXE do własnej pamięci C. Odczyt próbki obsługuje signed wartości i względne znaczniki pętli z kontrolą granic, bez wskaźników PS1. Audyt Disc 1 wykonuje po 1000 kroków każdego selektora. `akao-lfo-data` jest 21. testem; sprawdza błędne skoki i zachowanie cursor/output przy błędzie. Dane i odczyt przebiegów są gotowe; aktywna modulacja kanałów oraz synteza pozostają do wdrożenia.

FC/0A wybiera instrument ze stałym adresem początku próbki `0x1010`, zachowując jego loop, ADSR, przesunięcie banku i reguły przeliczenia pitch z A1. Test ADSR obejmuje release override, aktywne głosy, alternatywny bank oraz ucięty operand i przechodzi ASan/UBSan. Jest to obsługa stanu sekwencera; synteza i dowody pełnych utworów nadal pozostają wymagane.

A4 ustawia signed przesunięcie pitch i czas przejścia (zero oznacza 256 ticków). Obsługa nuty oblicza krok ze zawijanego różnicowego pitch 16.16; ticki przesuwają offset i oznaczają zmiany pitch. Testy timing sprawdzają ruch w górę/w dół i kodowanie czasu, również pod ASan/UBSan. Jest to stan sekwencera; zastosowanie do fizycznych głosów oraz synteza pozostają do wdrożenia.

Aktualizacja FC/14: wybór instrumentu zależny od nuty używa offsetu względnego, sprawdzanej tabeli rekordów i oryginalnej histerezy dla nut rosnących/malejących. Rekord nadpisuje attack/sustain/release; legato nie wywołuje ponownego wyboru. `akao-note-range` jest 20. testem i obejmuje progi, sąsiedni instrument, ADSR, alternatywny bank, ujemny offset i uszkodzone dane. Wszystkie 20 testów przechodzi headless z Disc 1. Audyt osiąga 8648 początkowych zdarzeń nut, dwa stop i 233 niezaimplementowane prefiksy kanałów; 397 prefiksów sekwencji przechodzi cztery wymuszone ticki, a 202 trafia na brakującą komendę. Pełne utwory, synteza oraz rozgrywka nadal wymagają implementacji i dowodów.

Komendy BD/BE pomocniczego LFO zapisują głębokość i resetują bieżącą modulację wraz z flagą aktywności. Zachowują pitch nuty, a reset zachowuje ustawioną głębokość i oznacza zmianę głośności. Test kanałów obejmuje te rozróżnienia i ucięty operand BD; 19 testów headless z Disc 1 oraz ASan/UBSan testu kanałów przechodzi. Aktywne przebiegi LFO pozostają jawnie nieobsługiwane. Wyniki audytu prefiksów są takie same jak w poniższej aktualizacji ADSR/metrum.

Aktualny stan: 19 testów CTest przechodzi headless z danymi Disc 1. Nowy `akao-adsr` sprawdza zapisy parametrów i trybów ADSR, maski zmian oraz różnice między resetem całego instrumentu i samego release. FC/15 ustawia metrum, a FC/16 licznik taktów we wspólnym stanie banku; aliasy struktur w kodzie referencyjnym nie oznaczają tu alokacji głosu. Testy ADSR i kanałów przechodzą też ASan/UBSan. Audyt osiąga 7987 początkowych zdarzeń nut, dwa stop i 894 niezaimplementowane prefiksy kanałów. Cztery wymuszone ticki kończą się dla 266 prefiksów sekwencji, a 333 trafiają na brakującą komendę. Nie jest to dowód odtworzenia pełnych utworów ani syntezy obwiedni ADSR. Pełna rozgrywka pozostaje do wdrożenia. Poniższe aktualizacje opisują kolejne historyczne etapy i ich ówczesne wyniki.

Aktualizacja drum-kit: `akao-drum` jest 18. testem. FC/04..05 ustawia/usuwa kit przez sprawdzany offset do rekordów sześciobajtowych. Nuty wybierają instrument bez ratio z A1, stosują pitch rekordu, gain, pan i reverb oraz offset alternatywnego banku. Testy obejmują selekcję, wejście/wyjście, zachowane melodic bookkeeping i złe/przycięte offsety. Najnowszy audyt osiąga 7303 początkowe zdarzenia nut, dwa stop i 1578 niezaimplementowanych prefiksów. Wszystkie pełne sekwencje nadal wymagają pozostałych komend, alokacji/zapisu głosów i syntezy; nie są uznane za poprawnie odtworzone.

Aktualizacja audio: `akao-sequence` w CTest sprawdza maski i offsety kanałów oraz uszkodzone dane. `room-assets` kataloguje 600 rekordów muzycznych Disc 1: 599 pełnych sekwencji z poprawnymi granicami i jeden jawnie nierozpoznany krótki rekord ID 0. Ten nierozpoznany rekord nie jest uznany za obsłużoną zawartość. `akao-channel` wykonuje podstawowe komendy A0, A2, A3, A5, A6, A7, A8 i AA, a także pętle C8/C9/CA oraz skoki FC/06, FC/08 i FC/09. Testy sprawdzają zagnieżdżenia, cykliczny stos czterech wpisów, 256 powtórzeń, odroczony key-off, signed offsety oraz błędne/nieskończone skoki. Audyt pierwszych prefiksów kanałów osiąga 1055 surowych nut i dwa zakończenia; 7826 kanałów zgłasza brakujący opcode. Czas/pitch nut, instrumenty, pozostały przepływ/LFO, alokacja głosów, synteza i pełne odtwarzanie pozostają do wdrożenia. Łącznie CTest rejestruje obecnie 13 testów; zielony test oczekiwanej diagnostyki brakujących komend nie potwierdza obsługi tych kanałów.

Katalog natywny odczytuje również 250 zakresów banków audio z tabeli EXE. Wszystkie 51 różnych banków próbek wskazanych przez rekordy muzyczne pokojów Disc 1 przechodzi odczyt z PE.IMG i upload z relokacją adresów instrumentów do wybranego banku SPU. Testy sprawdzają zachowanie źródła i poprzedniego stanu po błędzie. Automatyczny dobór banku z aktywnych głosów, komendy skryptowego audio, sekwencer i synteza pozostają do wdrożenia; sam upload tych banków nie jest dowodem odtwarzania muzyki.

Wspólny `PeAkaoControl` obsługuje komendy FC/00..03 (tempo i master-volume set/slide), warunek FC/07, akumulator tempa, wymuszone ticki oraz liczniki taktu/miary. Testy obejmują współdzielenie kontekstu, oba wyniki warunku, opadające i kwantyzowane slide'y, czas 256 ticków i skalowanie tempa. Te operacje nie są jeszcze połączone z odliczaniem nut, zegarem SPU i mixerem; pełny sekwencer pozostaje nieukończony.

`akao-timing` (14. test CTest) sprawdza oryginalny podgląd następnej nuty, osobny od wykonawcy komend, wraz z jego różnicami w wirtualnym przechodzeniu pętli. `PeScene` przechowuje odczytane z EXE tablice długości komend i nut. Helpers obliczają długość/bramkę nuty, odliczają pojedynczy tick muzyczny, uruchamiają następną nutę i zatrzaskują żądanie key-off; testy obejmują granice czasu, tie/rest, override, u16 wrap, błędne dane i limit podglądu. Obliczanie długości przechodzi dla 1055 osiągalnych początkowych nut Disc 1. Połączenie tych helpers w pełny driver odtwarzania, pitch, instrumenty, alokacja głosów i synteza pozostają do wdrożenia. Te testy nie są referencyjnymi porównaniami wyjścia audio.

`akao-player` dodaje 15. test CTest. Natywny driver czasu łączy akumulator banku, odliczanie aktywnych kanałów w kolejności głosów, komendy, slides/meter oraz zatrzaskiwane key-off. Test dwukanałowy sprawdza kolejność zmian wspólnego tempa, bramki, zakończenie banku i zachowanie stanu przy błędnym starcie. Audyt czterech wymuszonych ticków nadal zatrzymuje wszystkie 599 pełnych sekwencji Disc 1 na niezaimplementowanym opcode — żadna nie jest oznaczona jako odtwarzana poprawnie. Rejestry SPU, alokacja głosów, nakładające się banki, synteza i połączenie z komendami audio scen pozostają do wdrożenia.

Kanał wykonuje także A9 (expression slide), AB (pan slide) i FC/12 (volume slide). Aktualizacja parametrów następuje po komendach bieżącego ticku, jak w `Spu_UpdateVoiceRegisters`. Testy sprawdzają spadki, czas 256 ticków, signed delty zapisane jako bity u16, maski zmian `7F00`/`FF00`/`FFE00000` i brak anulowania wcześniejszego slide'u przez A3. Flagowanie zmian parametrów nie jest jeszcze zapisem rejestrów ani syntezą wyjścia SPU.

`akao-instrument` dodaje 16. test CTest. A1 odczytuje instrumenty z rzeczywistego banku SPU: adresy start/loop, ADSR, zachowany release override, przesunięcie alternatywnego banku i pitch ratio z wrap mnożenia 32-bitowego. Tablica pitch-period jest aliasem pól instrumentów, nie osobnym plikiem. C2/C3 i stop aktualizują żądany reverb/reserved-voice mask bez zapisywania sprzętowych rejestrów. Najnowszy audyt osiąga 5036 surowych początkowych nut i dwa stop; 3845 kanałów zgłasza pozostały opcode. Histogram obejmuje numery i liczbę wystąpień brakujących komend. Wszystkie 599 pełnych sekwencji nadal trafia na brakującą komendę w teście czterech wymuszonych ticków. Generowanie pitch nut, rejestry głosów, synteza i referencyjne audio pozostają nieukończone.

`akao-pitch` dodaje 17. test. Normalne nuty mają pitch z tabel instrumentów, przesunięcia oktawy oraz signed transpose/detune (C0/C1, D8/D9), z wrap iloczynu 32-bitowego i unsigned shift jak w oryginale. Tie zachowuje pitch, rest nie generuje key-on; kanał ze związanym bankiem stosuje te operacje w ticku muzycznym. Testy obejmują ujemne wartości, przesunięcia, alias pitch table, instrumenty nieobecne i jawne odrzucenie drum-kit/portamento/LFO/pending-pitch mode. Najnowszy audyt obsługuje 5434 początkowe zdarzenia raw note/tie/rest; 3447 prefiksów pozostaje niezaimplementowanych, a wszystkie pełne sekwencje nadal zatrzymują się na brakującej komendzie. Alokacja/zapis głosów SPU, synteza i referencyjne porównania audio pozostają wymagane.

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

## Integracja oryginalnego audio: stan bieżący

Smoke test ujawnił brak dźwięków menu: `pc/title.c` nie wywołuje komend AKAO, a strumień intro jest zamykany przed menu. Oryginalne `akao/sequencer.c` i `akao/spu_transfer.c` kompilują się jako oddzielny object target, jeszcze bez runtime linkage. Native pointer layout banku i 48 ścieżek ma zgodne widoki timera: pola po sequence pointer oraz timing counters nie używają już stałych offsetów PS1. Test `original-game-akao-layout` i ASan/UBSan sprawdzają współdzielenie stanu oraz zachowanie pełnych wskaźników; format instrumentu 64 bytes i prefiks pliku 4 bytes pozostają niezmienione. To nie jest dowód odtwarzania audio. Voice driver/bank nadal wymagają usunięcia obcinania adresów i raw strides, oryginalne MMIO SPU wymaga podłączenia backendu, a timer, kolejka komend i menu wymagają integracji oraz testów rzeczywistego PCM.

Oryginalne `Akao_LookupPitchPeriod`, `SeqOp_LoadInstrumentFromIndex` i `SeqOp_SetTempo` wykonują się w izolowanym headless teście z rzeczywistym object sekwencera. Adresowanie tabel używa native pointers zamiast `int`, a detune/shift zachowuje 32-bit word wrap bez signed overflow. Test sprawdza 256 instrumentów, zakres 256 nut i 16 wartości detune, ADSR/update flags i dwubajtowy odczyt tempa z pełnym kursorem. CTest oraz ASan/UBSan/function sanitizer przechodzą. Linker usuwa nieużywane procedury zamiast dodawać testowe audio stubs; nie dowodzi to działania pełnego sekwencera ani SPU. Sterownik, próbki, timer i dźwięki menu pozostają niezintegrowane.

Oryginalny `akao/voice_bank.c` dołącza do object target po dodaniu brakujących deklaracji publicznego API reverb. `Akao_LoadSamplePairFromIndex` i jego indexed-stream callers przekazują pełne native pointers. Test wykonuje 1024 indeksy obu próbek, masked IDs, sentinel `0xFFFF` i poprawny zerowy offset; CTest oraz ASan/UBSan/function przechodzą. Bank backup kopiuje native sizeof, a oryginalna pętla inicjalizacji ścieżek używa sizeof(AkaoTrack) zamiast 284 bytes. Te dwie ścieżki nie mają jeszcze pełnego runtime dowodu; inne raw command adresy/default nested stream nadal wymagają audytu. Sam object build nie oznacza sprawnego audio.

Wszystkie cztery oryginalne jednostki AKAO kompilują się teraz w object target: sekwencer, transfer, bank i voice driver. Driver potrzebuje tylko deklaracji CD audio mixing, wydzielonej z nagłówka obejmującego GPU/DMA/drive; publiczne CdMix/CdlATV pozostają tym samym API. Dwa istniejące testy AKAO przechodzą po dodaniu drivera, ale usuwają jego nieużywane procedury podczas linkowania i nie dowodzą poprawności sprzętowego wyjścia. Driver nadal ma bezpośrednie MMIO, ostrzeżenia ABI i niezintegrowane global state/dispatch tables. Kolejka oraz relative/default nested commands nadal wymagają dostosowania wskaźników.

Komenda efektu `0x24` kopiuje teraz native sequence pointer do kolejki, zamiast tylko niskiego 32-bit word. Test wykonuje oryginalne `Akao_Cmd_24`, validator, enqueue i message dispatch: 1025 komend obejmuje dwukrotne zawinięcie identyfikatora 0x400..0x5FF, masks parametrów, pointer preservation, odblokowanie kolejki i reset count. Para komend `0xDA` zachowuje kolejność D2/D6 i parametry. Dispatch fixture rejestruje żądania; nie odtwarza audio. PsyZ nie udostępnia jeszcze SpuSetReverbModeType, więc testowy endpoint przerywa test przy jakimkolwiek takim wywołaniu. CTest przechodzi; ASan/UBSan/function przed rozszerzeniem dispatch fixture sprawdził enqueue i pointer path, PsyZ nieinstrumentowane. Queue capacity/reentrancy/fault audit, oryginalna tabela handlerów i rzeczywisty PCM nadal pozostają wymagane.

Native message views dla value, bank pitch/slide/range, global param/slide/range i global pitch/slide/range uwzględniają pointer-width argument slot kolejki. Static assertions porównują argument/bank/target offsets do canonical AkaoQueueEntry zamiast PS1 constants. Test enqueue→oryginalny Seq_SetCurrentTrackField56 potwierdza odczyt właściwego low halfword i dodatkowo sprawdza byte/halfword/word views pozostałych komend. CTest oraz ASan/UBSan/function przechodzą dla całego bieżącego fixture (w tym effect dispatch); PsyZ nieinstrumentowane. Raw int-pointer handlers i relative/default nested commands pozostają nieprzeniesione; brak pełnej tabeli dispatch oraz SPU runtime nadal uniemożliwia uznanie audio za działające.

Trzynaście original bank handlers dla manage voices oraz immediate/slides volume/pan/pitch odczytuje canonical AkaoQueueEntry zamiast int[1..4] i byte offsets 4/8/C/10. Sygnatury mają wspólny message pointer; mask filtering i original loops pozostają. Fixture wykonuje trzy masked immediate procedures dla 12 kanałów, obu trybów wyboru (key-on mask/key-off ID), aktywnych/nieaktywnych głosów i low-byte versus 7-bit parametrów; sprawdza target/pan/pitch i dirty flags. CTest oraz ASan/UBSan/function przechodzą. Slides, manage/all-voice paths obecnie mają dowód kompilacji, nie pełnego zachowania. Pozostają relative/default stream addressing, dispatch catalog/globals i rzeczywisty SPU/PCM.

Alias views bank→track nie używają już niezgodnych native track offsets: playback selection zapisuje canonical bank_id, track34 mask zapisuje bank key_off_dirty_mask, stream-pair/read64 opcodes zapisują ticks_per_beat/beats_per_measure/beat/tick/measure. StopActiveVoices odzyskuje track z offsetof(flags), nie 0x38. Test wykonuje original rhythm decoders/cursor, primary/secondary mask selection i zatrzymanie 12 głosów z protected 0x02000000 mask; sprawdza active/pending-keyoff i flags. CTest oraz ASan/UBSan/function przechodzą, PsyZ nieinstrumentowane. Playback selection nadal nie ma pełnego dowodu (wymaga start/rebase engine i rzeczywistych sekwencji); audio runtime pozostaje nieukończone.

Original Akao_UpdateVoiceMask restore/rebase używa pointer-width modular address delta zamiast int i odejmowania wskaźników różnych allocations. Native backup bank/24 tracks jest odtwarzany przez oryginalne CopyWords; test potwierdza przeniesienie pc/branch/four call-stack pointers aktywnych głosów, duration increments, default program/durations nieaktywnych i zachowanie volume. Seq_MarkDirtyTracks oraz Spu_MarkActiveVoicesDirty używają offsetof-derived field pointer/native stride zamiast 0xF4/0x11C. Rebase fixture, CTest i ASan/UBSan/function przechodzą (PsyZ nieinstrumentowane). Dowód dotyczy syntetycznych danych dla oryginalnej procedury; native dirty-loop behavior, real sequence corpus i pełne odtwarzanie nadal wymagają weryfikacji.

Original Akao_InitVoices przyjmuje typed AkaoTrack pointer zamiast byte buffer; callers nie rzutują native track arrays na char. Test wykonuje Akao_InitVoiceState→Akao_SetNotePitch→SeqOp_SetVoiceInstrument oraz oba primary/secondary init wrappers dla 48 ścieżek, matching/nonmatching bank IDs i reset-all. Sprawdza program pointer, release ADSR, durations, instrument fields/dirty flags, zachowanie assigned voice i przywrócenie current bank. CTest i ASan/UBSan/function przechodzą (PsyZ nieinstrumentowane). Initialize procedures mają dowód izolowanego zachowania, nie odtwarzania sekwencji/timera ani faktycznego PCM.

Original Akao_StepSequencerVoice start używa typed track cursor zamiast byte-pointer i bias 0x116; decoding/order/defaults pozostają. Headless fixture uruchamia sparse channel mask 0/2/23 z header pending/yield masks i relative stream offsets, również paused mode. Sprawdza native PC, assigned voice sentinel, durations/pan, active/pending-restore, tempo/accumulator i allocation mask. CTest i ASan/UBSan/function przechodzą (PsyZ nieinstrumentowane). To syntetyczny nagłówek; real corpus parsing, bytecode execution/tick/SPU i PCM nie mają jeszcze pełnego dowodu.

Strict `original-game-akao-assets` wykonuje original Akao_StepSequencerVoice na prawdziwych nagłówkach Disc 1: 414 pokojów, 599 sekwencji, 8883 aktywne kanały. Test-only parser/Disc I/O ustala oczekiwane offsets/masks; original procedure ustawia runtime PC, masks i inactive defaults. Snapshot potwierdza immutable sequence data. CTest i ASan/UBSan/function pełnego corpus przechodzą (PsyZ nieinstrumentowane). Jeden short record jest jawnie policzony i niewykonany; jego runtime handling pozostaje wymagany. Test dowodzi start/header addressing, nie wykonania całego bytecode, piosenek, SPU ani PCM. Bez obrazu test zwraca SKIP 77, a PE_REQUIRE_GAME_ASSETS=1 wymusza failure; SKIP nie jest dowodem ukończenia.

AkaoScriptState jest teraz aliasem canonical native AkaoTrack, nie częściową strukturą z PS1 padding. Cztery original handlers zapisują canonical pitch/volume/aux LFO fields oraz branch/volume i high-half expression value. Usunięto historyczny opis, że pc/akao_channel jest właściwym runtime. Fixture wykonuje trzy slides z duration zero→256, dodatnim/ujemnym delta oraz signed backward branch ze saved high-half; sprawdza cursor i niezmienione targets. Trzy AKAO CTests z wymaganym Disc 1 i ASan/UBSan/function fixture przechodzą (PsyZ nieinstrumentowane). Pełna opcode tabela, interpreter timing/SPU i rzeczywiste audio pozostają wymagane.

Original voice-driver Akao_SetVoiceKeyOff wykonuje w headless fixture 16 kolejnych aktualizacji wszystkich trzech LFO na dodatniej/ujemnej fali z relative loop marker. Test sprawdza zachowanie native waveform pointers, pitch/volume/aux values, wynikowy pitch i dirty flags; nie implementuje zastępczego LFO. CTest i ASan/UBSan/function przechodzą (PsyZ nieinstrumentowane). Dowód obejmuje tę procedurę na kontrolowanym waveform; real waveform corpus, key-on variant, timing gates/stereo i PCM nadal pozostają wymagane.

LFO fixture obejmuje teraz original Akao_SetVoiceKeyOn i KeyOff oraz ujemne expression. Sterownik robi left shifts LFO products i previous-voice pitch w unsigned word przed signed arithmetic shift, zachowując PS1 wrap bez signed-left-shift UB. Test potwierdza positive/negative waveform, loop marker, output pitch i dirty flags w obu procedurach; signed expression daje poprawny opposite volume LFO. CTest i ASan/UBSan/function przechodzą (PsyZ nieinstrumentowane). Pozostałe dwa miejsca shift w wcześniejszych driver routines oraz previous-voice branch mają obecnie tylko dowód kompilacji; nie rozszerzamy tego wyniku na pełny driver/audio.

`docs/AKAO_OPCODE_TABLE.json` kataloguje oryginalne USA tables: main 0x8009C8F0, opcode A0..FF (96 entries), extended 0x8009CCF0 (32 entries), 90 różnych source procedures, zero unresolved names. Pole lookahead_skip jest wartością oryginalnej tablicy skanera, nie liczbą argumentów handlera (main skip zawiera opcode, extended scanner już konsumuje FC). `python3 tests/verify_akao_catalog.py <SLUS_006.62>` sprawdza wszystkie adresy/skip values w prywatnym EXE, zakres sekcji oraz obecność oryginalnych definicji w źródłach; wykonane na lokalnym Disc 1 przechodzi. Katalog nie zawiera kodu/data payload gry i nie jest jeszcze runtime dispatch table. Native ABI handlerów, ich kolejność/semantyka i full bytecode/tick execution pozostają wymagane.

Osiem original opcode handlers (JumpRelative, StreamPair, ReadTrack64U16, Skip2 i cztery oryginalne no-op) ma canonical void(AkaoTrack*,u32) ABI. Skip2 aktualizuje pełny PC zamiast low word; JumpRelative jawnie odczytuje signed displacement i dodaje go po dwóch bajtach, usuwając unsequenced pointer increments w dawnym READ_S16. Fixture wywołuje no-op/skip/backward jump przez AkaoCommandHandler bez cast i sprawdza native cursor; rhythm handlers zachowują poprzedni dowód. CTest oraz ASan/UBSan/function przechodzą. Pozostałe sygnatury i runtime tables nie są jeszcze ujednolicone/podłączone; te no-op są zachowanymi procedurami gry, nie zastępnikami brakujących efektów.

Kolejne 53 original handlers bez bezpośrednich callers mają canonical void(AkaoTrack*,u32) i deklaracje w note_step.h; argument mask jest jawnie niewykorzystywany, bodies zachowano. Fixture wywołuje ReadExpression/ReadDetune/SetPanpot/ReadVolume/SetReleaseRate/ClearFlag3 przez canonical function pointers i sprawdza signed parameters, byte wrap, PC, ADSR i flags. CTest oraz ASan/UBSan/function przechodzą. Pozostałe 47 zmienionych bodies mają dotychczas tylko build i wcześniejsze dowody tam, gdzie opisane; nie jest to test wszystkich 90 procedur. Cała tabela i wykonanie songs/SPU nadal wymagają integracji.

Wszystkie 90 original opcode procedures mają teraz zgodny void(AkaoTrack*,u32) ABI. Dostosowano ostatnie 12 sygnatur i bezpośredni caller ReadAdsrDecayRate; SetTrack5AValue nie deklaruje już nieistniejącego return value. `src/main/akao/opcode_dispatch.c` odtwarza 128 wpisów obu tablic wskazując bezpośrednio original functions, bez casts/adapters. Object target kompiluje tabelę. Verifier porównuje każdy native dispatch entry z katalogiem i private USA EXE; trzy AKAO CTests z wymaganym Disc 1 przechodzą po zmianie. Test linker usuwa nieużywaną pełną tabelę, więc pełne powiązanie handler dependencies i wykonanie bytecode przez tę tabelę nie mają jeszcze runtime dowodu. Global state, grammar tables, nested streams, SPU i PCM pozostają niezintegrowane.

Original symbol map potwierdza g_AkaoPitchPeriodTable = g_AkaoInstrumentTable+0x10 oraz voice bank aliases: primary B8AC0, secondary BA560 (+24*284), nested SFX BC000 (+48*284), 12 slots. Sequencer lookup/PlayNote używa teraz canonical instrument.pitch, bez niezależnej pitch table ani cross-record word indexing. Headers utrzymują jeden native pool 60 tracks, z secondary/channel pointer aliases; testowe oddzielne arrays usunięto. Trzy strict Disc 1 CTests i ASan/UBSan/function procedural fixture przechodzą; sanitizer nie linkuje nieużywanej kompletnej dispatch table, PsyZ nieinstrumentowane. Canonical timer-track/update-slot aliases oraz runtime-owned storage nadal wymagają audytu/integracji; pełnego audio nie uznajemy za działające.

Timer/driver/sequencer track views mają jedno canonical storage AkaoTickTrack[60]; primary, secondary, SFX i update-slot aliases odnoszą się do niego. Original map potwierdza current-bank alias g_AkaoCurTrack=0x8009D2C8: oba typed pointer views współdzielą native union, zamiast oddzielnych globals. Test przełącza bank przez oba widoki oraz sprawdza shared flags/update_flags dla wszystkich 60 rekordów i offsets banków. Trzy AKAO CTests z Disc 1 oraz ASan/UBSan/function procedural fixture przechodzą (bez nieużywanej pełnej dispatch table; PsyZ nieinstrumentowane). Storage nadal jest zdefiniowane w test fixture, nie w połączonym runtime; dalsze global alias/dispatch/timer/SPU integration pozostają wymagane.

Pełna tabela opcode dispatch jest teraz rzeczywiście linkowana i wywoływana przez original Akao_StepSampleLoader w headless fixture. Program A3(volume)→C0(expression)→8F(rest)→A0(stop) sprawdza PC, parametry, key-off i active masks. Brakujące driver globals są explicit input state w fixture; brak zastępczych handlerów. Nested source word names współdzielą canonical descriptor, zgodnie z original symbol aliases B89D0..E0. Note loader używa named bank fields zamiast words[14], które po native pointer expansion wskazywało złe pole. Trzy strict Disc 1 CTests i ASan/UBSan/function (teraz z całą dispatch table) przechodzą; PsyZ nieinstrumentowane. Test wykonuje tylko wskazane opcode path; pełne sequence execution, real grammar/state loading, timer, nested streams i SPU/PCM pozostają wymagane.

Original Akao_StepSampleLoader przechodzi fixture note→tie→rest→stop dla parent BGM i child SFX, przez pełną tablicę opcode. Test sprawdza octave/pitch lookup 4096, current note 72, duration/gate 48/46, brak retrigger przy tie, key-on/key-off, primary versus SFX allocation masks i stop behavior. CTest i ASan/UBSan/function pełnego original dispatch przechodzą (PsyZ nieinstrumentowane). Wyzerowanie note_length między wywołaniami jest jawne fixture input zamiast symulowania działającego timera; dowód nie obejmuje timing drivera, rzeczywistych songs ani PCM.

Original full-dispatch fixture wykonuje C8→rest→C9 dla 3 oraz zero→256 powtórzeń, potem A0; sprawdza pointer stosu, liczniki, pop i koniec banku. Extended opcode 8/9 testuje taken/not-taken signed backward branch oraz conditional pop. CTest z timeout 15s i ASan/UBSan/function przechodzą (PsyZ nieinstrumentowane). Lookahead skip C8=1 jest jawnym fixture input zgodnym z katalogiem; nie udajemy załadowania całej grammar table. Full songs, wszystkie nested/conditional variants, timer oraz PCM pozostają wymagane.

`original-game-akao-prefix-assets` ładuje real grammar/note lengths/LFO z Disc 1 i instrument tables z resident + właściwego audio banku (test-only sample decoder), po czym wykonuje original start i Akao_StepSampleLoader do pierwszej nuty/stopu dla 599 sekwencji/8883 kanałów z 414 pokojów. Sprawdza cursor bounds i immutable sequence data. CTest strict assets oraz ASan/UBSan/function z halt_on_error=1 przechodzą dla pełnego prefix corpus; PsyZ nieinstrumentowane. Trace opcjonalny PE_AKAO_PREFIX_TRACE identyfikuje map/record/voice. Jeden short record pozostaje jawnie niewykonany. Wynik nie dowodzi dalszych nut, pełnych utworów, timera, voice allocation, SPU czy PCM; corpus nie jest automatycznym dowodem całego audio.

Original Akao_WriteVoiceParam ma teraz ścisłą C11 sygnaturę (int,AkaoVoiceParams*); usunięto K&R definition i ignorowane trailing flags z sześciu callers. Błędny int* call używa canonical voice-parameter view. Static assertions porównują flags/start/pitch/volume offsets widoku do AkaoTrack. Headless typed-pointer test wykonuje zero-dirty-flags path bez dotykania MMIO; CTest i ASan/UBSan/function przechodzą. Nie dowodzi to działającego zapisu parametrów SPU. Próba linkowania pełnego original Akao_Tick wykazała 32 brakujące symbole (log diagnostyczny /tmp/pe-akao-tick-link.log); global state i registers/timer integration nadal wymagają pracy, a bezpośrednie 1F80 MMIO nadal nie jest native-safe.

`src/main/akao/state.c` jest właścicielem native original instrument/voice/bank/backup storage, current bank pointer, masks, selected IDs, nested descriptor, playback mode, global slides/accumulators, SFX tempo/tick, envelope slots i SPU common attributes. Testowe duplikaty tych globals usunięto. Original map potwierdza byte aliases PitchScale=PitchDepth+2 i TempoScale=TempoScaleAccum+2; canonical headers zachowują shared bytes, fixture sprawdza read/write bez naruszania pozostałych bitów. CTest oraz ASan/UBSan/function z halt_on_error=1 przechodzą dla procedural fixture i 599-sequence prefix corpus (PsyZ nieinstrumentowane). Moduł stanu jest w original AKAO object target, nie połączonym main runtime; grammar/sample inputs, queue/dispatch message table, CD volume alias, platform registers/endpoints i timer integration nadal pozostają wymagane. Sama własność stanu nie oznacza działania audio.

CdInputVolume i CdInputVolumeAccum współdzielą native union, zgodnie z original addresses D2B6/D2B4. Górny halfword używany przez Seq_ApplyGlobalPitch nie jest już oddzielnym globalem; state.c ma canonical storage. Headless test zapisuje przez oba widoki i potwierdza zachowanie części ułamkowej. CTest i ASan/UBSan/function z halt_on_error=1 przechodzą (PsyZ nieinstrumentowane). To poprawność aliasu; actual common-volume SPU output i slides pozostają wymagane.

Original Spu_StepReverbLoad wykonuje w fixture 12 kolejnych calls, dowodząc four-tick divider, dwóch steps tempo/global pitch depth oraz przeciwnych pitch slides primary/secondary. Test sprawdza frozen results po counter expiry, current-bank restore i primary/secondary dirty flags; używa canonical native storage. Pointer-punning AkaoBankCursor zastąpiono bezpośrednim current-bank reload, a accumulator additions zachowują unsigned word wrap bez signed overflow. CTest i ASan/UBSan/function z halt_on_error=1 przechodzą (PsyZ nieinstrumentowane). CD volume SPU calls i SFX per-voice slides pozostają poza tym dowodem; pełny Akao_Tick i PCM nadal niezintegrowane.

Original AkaoSpuVoice register writers adresują native PsyZ _spu_RXX zamiast 1F801C00/1F800000; zachowano mask/shift/ADSR bit composition i volatile halfword operations. Voice mask aliases również odnoszą się do PsyZ key-on/off, FM/noise/reverb words. Test rzeczywistego Akao_WriteVoiceParam sprawdza full dirty parameters dla wszystkich 24 głosów: volume masks, pitch, start/repeat >>3, ADSR low/high i clear flags; mask writes sprawdza low/high words. Cztery strict AKAO CTests i ASan/UBSan/function przechodzą (PsyZ nieinstrumentowane). To native register-map integration, nie synteza SPU/PCM; envelope/reverb endpoints, samples/backend clock i main audio nadal wymagają implementacji.

Fork khasinski/parasite-eve-psyz ma opublikowany commit c15c13f implementujący native SpuGetVoiceEnvelope i SpuSetReverbModeDepth poprzez SPUR/SPUW; submodule przypięto do niego. Previous mask-map integration omijało key-on/off side effects: original mask writers teraz wywołują Psyz_SpuWrite, który uruchamia/zwalnia backend voices. PsyZ ma już backend syntezy i offline PullSamples; jego PCM fidelity nadal wymaga weryfikacji. Voice-envelope ABI dopasowano do native SDK. Fixture sprawdza envelope reads wszystkich 24 voices i signed reverb-depth registers. Pełny original Akao_Tick po raz pierwszy linkuje się i wykonuje: forced beats zmniejszają gate/note counters i dochodzą do A0 stop, current bank pozostaje poprawny. Cztery strict AKAO CTests oraz ASan/UBSan/function original sources przechodzą (PsyZ nieinstrumentowane). Dowód ticka używa voice sentinel 24 i nie jest testem fizycznej alokacji lub PCM; music corpus pełnych ticków, audio waveform/reference i main integration pozostają wymagane. Missing reverb-mode-type test trap nadal jest fixture-only.

Procedural fixture generuje teraz actual stereo PCM przez Psyz_SpuPullSamples po original Akao_WriteVoiceParam i Spu_WriteKeyOn/Off. Własna kontrolowana looping ADPCM próbka (nie asset gry) sprawdza 512 frames ciszy przed key-on, nonzero output z proporcją stereo 2:1, release do ciszy oraz byte-identical output po reset/repeat. Headless bez SDL audio device; CTest i ASan/UBSan/function original callers przechodzą, backend PsyZ nadal nieinstrumentowany. To dowód rzeczywistej syntezy i poprawności event routing dla jednego controlled voice, nie full SPU fidelity ani odtwarzanie soundtrack/menu. Actual voice allocation, asset samples, reverb/noise/FM, tick-to-sample cadence, reference waveform i main menu integration pozostają wymagane.

## Źródła techniczne do implementacji bramek

Audyt bieżącego projektu: `pc/CMakeLists.txt`, `pc/tests/test_disc.c`, `pc/tests/test_boot_assets.c`, `.github/workflows/pc.yml`, `docs/ARCHITECTURE.md`, `docs/ROOMS.md` i źródła `src/`.

[CTest SKIP_RETURN_CODE](https://cmake.org/cmake/help/latest/prop_test/SKIP_RETURN_CODE.html) opisuje istniejące pomijanie testu kodem 77. Docelowa bramka wymaganego zakresu musi je traktować jako brak dowodu. [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) służy do wykrywania błędów pamięci, a [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) do wykrywania wyścigów; wybieramy obsługiwane przez narzędzie konfiguracje zamiast deklarować jeden identyczny tryb na wszystkich systemach.

### Original AKAO reverb mode integration (2026-10-14)

PsyZ fork c05349b implements SpuSetReverbModeType using its existing
SpuSetReverbModeParam implementation. Original Seq_GetGlobalPitch reads the
backend reverb state rather than a separate fixture variable. The abort-only
test endpoint is removed. All ten modes, clear-work-area requests, mode
readback, reset depths, echo/delay defaults and invalid mode rejection pass
in the headless original instrument test. The SDK reverb attribute mask is
native pointer width; using the original 32-bit layout corrupts memory on
64-bit hosts and has been corrected for this API. All four original AKAO
CTest entries pass with required Disc 1 assets. This does not yet connect
menu sounds or music to the frontend; full campaign integration remains open.

### Original sound initialization and native common attributes (2026-10-14)

SpuCommonAttr now matches PsyZ's native-width mask. A regression first
failed on the master-volume register with the PS1 layout; it now passes
through SpuSetCommonAttr, including deterministic nonzero stereo PCM.
Seq_ApplyGlobalPitch is exercised against actual CD-volume registers and
checked not to alter master volume. The original Spu_InitVoiceState now
links and runs headlessly: master/CD levels, enabled reverb mode 4, queue
reset and all 60 channel assignments are checked. Its missing voice-volume
helper is linked from the existing original s_svva.c with the public SDK's
signed-short signature; no replacement initialization logic was added.
The busy flag now belongs to original AKAO runtime state.

All four original AKAO tests pass with required Disc 1 assets. Instrument
test passes ASan/UBSan/function instrumentation of original source and test
code (the linked PsyZ archive is still uninstrumented). This is initialization
and PCM evidence, not frontend menu audio or full-game completion.

### Original sample upload completion (2026-10-14)

Original Spu_UploadSampleBlock now runs against native SPU RAM in the
instrument test. Before the fix the nonblocking transfer left status 1: PsyZ
copied bytes but omitted the registered completion callback, so the original
blocking transfer would spin indefinitely. Fork c0275f5 delivers that callback
after its synchronous copy, preserving the no-callback kernel event path.
The regression checks completion, exact 16-byte ADPCM RAM contents, the
64-byte instrument record and a second blocking upload. All four original
AKAO tests pass with required Disc 1 assets. The transfer status is owned by
original runtime state. This does not yet integrate frontend menu audio.

### Original streaming upload on Disc 1 banks (2026-10-14)

The private prefix test now executes original Spu_SetStreamModeA and
Akao_StepNoteSequencer for every playable music record instead of copying
fixture-decoded instruments into runtime state. It compares all 256 instrument
records and the uploaded SPU RAM range against the independent fixture reader,
then executes the original sequence prefix. AkaoInstrumentAddress.word now
uses uintptr_t so byte offsets cannot truncate/carry-wrap native pointers.
Stream-upload mode/address/remaining counters belong to runtime state.

Required Disc 1 run passes: 414 rooms, 599 sequences, 8883 channel prefixes;
one unresolved short record remains unexecuted. All four AKAO CTest entries
pass. The asset run also passes ASan/UBSan/function on original/test sources
with the uninstrumented PsyZ archive. This proves sample transfer and first
note/stop prefixes, not complete music playback or audible frontend menus.

### Original tick-to-PCM corpus smoke (2026-10-14)

New original-game-akao-tick-assets test loads the original four-byte default
voice program and 256-entry pan table from the supplied USA executable.
It runs original initialization, streaming sample upload and sequence start,
then 240 Akao_Tick calls with 44100 stereo PCM frames per music record.
Fractional samples per 240 Hz tick are carried by integer frame boundaries.
Resident sample RAM is fixture-loaded; music-bank RAM comes from the original
streaming procedure, not the fixture decoder. All 599 playable records in
414 rooms complete this one-second smoke; the known short record is still
unresolved. Across the corpus 51,102,257 channel samples are nonzero.

Five required-asset CTest entries pass. Independent repeated process and
ASan/UBSan/function-instrumented original/test source runs produce identical
little-endian PCM FNV-1a hash 25836548e09d8a00. PsyZ remains uninstrumented.
This is an early playback smoke, not full-song fidelity evidence: no original
hardware PCM reference comparison, full duration, menu integration or campaign
completion is claimed.

### Native sound-effect command 0x24 (2026-10-14)

The original USA dispatch table identifies 0x24 as Seq_StartRelativeNestedStream.
That handler now consumes AkaoQueueEntry directly, keeping its relative-stream
table pointer native-width and materializing the original scalar nested-source
fields after decoding both offsets. The obsolete pointer-in-u32 command view
is removed. The regression executes Akao_Cmd_24, original enqueue and queue
dispatch into that handler, checks both allocated effect channels, native
script pointers, generated sound identifier, pan and volume, and checks absent
stream sentinels. Instrument test passes ASan/UBSan/function; the other four
AKAO asset/layout tests also pass. This still uses controlled effect data;
menu asset lookup and frontend playback are not yet integrated.

### Original menu helpers through real assets to PCM (2026-10-14)

Original menu sound helpers and Akao_SendTableCommand are moved unchanged
to akao/menu_sound.c; the original three keyed lookup macro instantiations
are moved unchanged to cdrom/asset_lookup.c so they can link independently
of unrelated game/CD procedures. No replacement menu/audio logic was added.
Disc 1 sector bank 7 contains all four menu keys 044C..044F. The new required-
asset menu test loads that blob and executes each original helper through
lookup, command enqueue, original 0x24 dispatch, tick and SPU PCM synthesis.
Confirm/cancel/move/error produce respectively 30172/23184/7834/41810 nonzero
channel samples in a one-second window each. Resident samples are still loaded
by the test fixture reader; runtime audio integration remains incomplete.

Sanitizers exposed a signed left-shift overflow in the original pitch-slide
setup. Unsigned 32-bit shifts now preserve its machine-word arithmetic. The
menu test passes ASan/UBSan/function on original/test code (PsyZ uninstrumented),
and all six AKAO CTest entries pass. The frontend still needs to retain an
audio stream and invoke these original helpers when processing menu input.

### Title menu now uses original AKAO audio (2026-10-14)

The running parasite-eve-pc executable now links the original AKAO object
modules. The title frontend loads the original grammar, pan/default/LFO tables,
sector-bank 7 menu sequences and uploads resident bank 6 using original
Spu_UploadSampleBlock. Menu input calls original confirm/move/cancel helpers,
then original queue dispatch. A 240 Hz main-thread timer drives Akao_Tick and
44100 Hz SPU PCM into an SDL S16 stereo stream. SDL does not mutate sequencer
state on its audio thread. Selection leaves 500 ms for confirmation feedback
before the existing scene preparation transition.

Native asset/queue storage is in src/main/akao/assets.c. Only the 0x24 message
handler is connected in the frontend dispatch table; the full gameplay audio
message catalog, indexed effect sources and scene audio lifetime still need
integration. The title does not use the abandoned pc/akao_player implementation
for its menu sounds. This is not a claim that gameplay itself runs.

New boot-menu-audio replay drives confirm, cursor movement and cancel in the
actual headless SDL frontend and requires nonzero PCM submission after each
input. An independent run of the main executable passed the same replay.
All nine targeted tests pass with required Disc 1 assets: six original AKAO
tests plus menu-audio, Tutorial and New Game boot replays. The latter two
prove the existing asset-preparation transitions, not playable scenes.

### Original floor query native indexing (2026-10-14)

Entity_FindFloor now indexes relocated region pointers directly instead of
adding floor_index*4 to a 32-bit address. Both flat and sloped paths use the
native table. Negative flat heights and plane-D subtraction preserve 32-bit
word arithmetic without signed-shift/subtraction undefined behavior.
New original-game-floor test executes the original query for a second region,
negative flat height, miss, flags 2/80, and sloped height with integer and
fractional X/Z coordinates. ASan/UBSan/function pass for this original-source
test; PsyZ remains uninstrumented. The 414-room immutable collision relocation
test and actual headless menu-audio replay also pass. Corpus-wide floor query
comparisons and movement integration remain pending; scenes are not yet playable.

### Disc 1 floor-query corpus comparison (2026-10-14)

The original collision asset test now compares Entity_FindFloor with the
independent byte-decoding test oracle over every triangle's three vertices
and centroid, including fractional coordinates and normal/preserve-Y/bypass
flags. All 414 rooms pass: 202296 queries, 99614 hits, 40274 sloped hits.
It checks selected face, mirror pointer, height and unchanged state on misses,
as well as immutable source bytes. The oracle is test-only.

UBSan found that real sloped region records can be halfword-aligned. Their
opaque four-byte prefix is now two u16 fields rather than a u32, preserving
the on-disc layout while allowing its actual alignment. The complete corpus
passes ASan/UBSan/function on original/test sources; PsyZ is uninstrumented.
The standalone floor test also passes. This is not proof of playable scenes:
scene frame execution, scripts and player control still require integration.

### Original actor model setup executable on PC (2026-10-14)

First-location integration is blocked by original scene/frame dependencies,
including actor model setup and GPU submission. Render_SetupEntityPrims,
Anim_SetInterpRate and Render_InitPrimBlock are moved unchanged from object.c
to render/model_setup.c and compile as an original-source object target.
This isolates existing procedures, without replacement model/render logic.

New required-asset test runs original model setup on writable copies of all
878 Disc 1 models. It verifies native section pointers, bounded animation/
matrix storage, identity matrices, both primitive buffers, command/length
bytes, blend kinds and UV/CLUT/page values against the independent model
reader. This uses setup=1 and initCount=0; texture relocation variants are not
yet covered. Original/test-source ASan/UBSan/function passes with PsyZ
uninstrumented, and the existing projection corpus test passes.
The target is not yet in the live scene: GPU submission, scene loading,
script/frame integration and playable actor control remain required.

### Native model packets and original ordering-table insertion (2026-10-14)

The four runtime model packet classes now use PsyZ native pointer/length
headers. Their GPU command payloads remain 32-bit, and the test asserts ABI
size/coordinate offsets against actual PsyZ POLY_GT4/GT3/G4/G3 and OT_TYPE.
Original Render_DrawTexturedQuads moves to model_draw.c with native links and
a local NCLIP result replacing scratchpad address 1F800000. Its winding and
depth selection remain unchanged. Scene_LoadMap allocation sizes now account
for native packets rather than PS1 primitive-block bytes. Other old renderer
packet classes still need porting before complete scene drawing can run.

The model corpus test performs original setup, projection and insertion into
SDK ClearOTagR ordering tables for both draw slots: 878 models, 301224 visible
packets, bounded complete native chains. ASan/UBSan/function passes for
original/test sources; PsyZ remains uninstrumented. Full build passes, as do
model setup, existing projection corpus and headless menu-audio tests.
This verifies packet construction and ordering, not GPU rasterization or an
on-screen actor. Actual GPU submission and scene frame integration remain open.

### Original model GPU execution — 2026-10-14

`original-game-model-gpu` now submits the original setup/projection/face-ordering
packets to PsyZ's SDL GPU backend and reads native VRAM back with `StoreImage`.
No display environment or VSync is submitted, so the backend window remains
hidden. This requires a functioning host GPU backend; missing disc data remains
a skip unless `PE_REQUIRE_GAME_ASSETS=1` is set.

Disc 1 corpus evidence: all 878 models produced visible pixels (2,239,579 total),
301,224 face packets were linked across both draw slots, and each model's two
slots produced identical readback. Both setup and GPU CTests passed on Metal.
The test supplies synthetic white VRAM and neutral vertex colours: it verifies
actual geometry packet execution, not room texture placement, original lighting,
or visual fidelity against PS1 captures. No playable scene is implied. Original
lighting, room texture uploads, actor/script/input frame integration remain next.

### Original model lighting on native packets — 2026-10-14

Moved the original `Render_DrawObject` and `Render_UpdateClutTable` procedures
from `render/object.c` into `render/model_lighting.c`, built with the native model
renderer. The lighting procedure now uses a local colour word and matrix instead
of PS1 scratchpad addresses. Colour packet updates now use native structure sizes
and full pointer links instead of PS1 byte strides and 24-bit address masks.
The NCCT lighting and original per-vertex colour selection remain unchanged.

The GPU corpus test now reads the 240 normal vectors at executable address
0x80091A58 and runs original lighting and colour packet updates before submission.
All 878 models still produce visible pixels; both slots match (2,239,490 lit
pixels total). It regenerates projection before each slot because original
projection and lighting intentionally reuse the same work arrays. Setup/GPU
CTest checks and full build pass. ASan/UBSan/function instrumentation of original
and test sources also passes the GPU corpus (PsyZ/SDL archives uninstrumented).

The fixture still supplies identity lighting matrices, ambient shade 64 and white
texture VRAM. This is original lighting execution evidence, not validation of
room light parameters or original textured appearance. Scene integration remains
incomplete; this does not make New Game playable yet.

### Original room texture uploads through GPU — 2026-10-14

Moved the unchanged `Draw_LoadTimAsset` procedure from `cdrom/scene_assets.c`
to `render/texture_upload.c`, using PsyZ's native GPU declaration and explicit
payload casts. The original primary-image height rule and secondary offsets
relative to the primary image are preserved. No replacement upload algorithm
was added to the runtime.

`original-game-model-room-gpu` executes this procedure for scratch and texture
banks in all 414 populated Disc 1 rooms: 9,017 uploads. Full 1024x512 VRAM readback
matches the independent byte-oriented `pc/vram.c` oracle after each room's batch.
The model test then uses that room VRAM, original lighting and original geometry
packets. 732/878 models produce visible pixels in the fixture (1,542,996 pixels);
both draw slots match for every model. Room VRAM is restored before each draw to
prevent feedback from the test framebuffer. The existing white-texture corpus
still requires all 878 models to produce visible geometry.

Full build and all three model CTests pass. ASan/UBSan/function instrumented
original/test code also passes the room-GPU test; PsyZ/SDL archives remain
uninstrumented. These are hidden-window Metal runs, not software-only rendering.

This does not certify final model appearance: the fixture still uses a fixed
camera and light matrices, skips actor-specific texture relocation and resident
entity-bank selection, and 146 models have no visible pixels with just their room
banks. Those models remain unverified for textured appearance, not waived. The
actual scene/frame loop and controls still need integration before a location is
playable.

### Original actor draw entry point — 2026-10-14

The native model target now compiles the remaining original `render/object.c`,
including `Render_DrawEntity`, animation draw setup, fades and texture-page
updates. `Render_InitRoomPrimState` moved unchanged from `camera.c` to the linked
model lighting unit. The two height-effect routines use local scratch storage;
`Render_DrawObjectVariant` no longer truncates part/bounds pointers to 32 bits or
left-shifts a negative signed height. An unused legacy GPU header was removed.

GPU corpus tests now call `Render_InitRoomPrimState` and `Render_DrawEntity`, the
same actor draw entry used by `Entity_FrameUpdate`. Fixture lighting remains
explicit (shade 64, positive/negative Y contributions 24/12, white room tint), not
actual room/script state. Slot 0 uses ordinary lighting; slot 1 additionally runs
both height effects at limit 32767 and must produce the same pixels. This covers
their native pointer/scratch traversal, not clipping/tinting below that limit.
The fixture reads the original texture-page delta table from the executable.

All 878 white-texture models render (2,239,298 pixels). The instrumented room-GPU
run retains complete VRAM equality for 9,017 uploads/414 rooms and 732 visible
models (1,518,643 pixels), with identical draw slots. Full build and ASan/UBSan/
function checks pass; PsyZ/SDL archives are not instrumented. Other fade/animation
branches are compiled and linked but not yet comprehensively exercised.

`Entity_FrameUpdate` still requires real battle/menu dispatch, script batches,
actor callbacks, camera, shadows and scene-state wiring. No runtime stubs were
introduced to hide those dependencies, and New Game is still not playable.

### Native original camera view and transitions — 2026-10-14

Moved the original view initialization, clamping/bounds, follow projection,
scroll and transition procedures from `render/camera.c` to `camera_view.c`.
A complete `GeomScrollRecord` now backs the camera rather than a cast over a
single scalar. Its paired-word view has explicit native alignment; header
assertions check native runtime sizes while serialized viewport layout remains
52 bytes. Geometry offset arithmetic now uses uintptr_t. `Render_SetViewport`
accepts the caller's actual fixed-point s32[3], extracts high halves by shifts,
and has a consistent int return declaration. GTE scratch vectors are initialized.
Camera flags and screen-X writes in these procedures access the real record.

`original-game-camera` loads all 509 viewports across the 414 populated Disc 1
rooms. Linear and cosine transitions (30 frames each) agree with the independent
byte-reader/reference for 30,540 frames, including flags and screen offsets.
Clamp and bounds save/restore checks preserve the original geometry blob;
follow projection stays within each viewport and world-target scroll defaults
to 30 frames. Native word/field alias and matrix-pointer checks pass. Full build,
camera/model-setup CTests, and ASan/UBSan/function camera tests pass; PsyZ is not
instrumented. These tests are headless and do not initialize a GPU window.

Remaining integration: other legacy scalar aliases (including overlay reads of
D_800BCF88) have not all been unified with this record. Native GeomEntry runtime
pointer fields are not a serialized room-entry layout; mesh loading still needs
its own relocation work. The camera target is not yet wired into the running
scene, and no whole-frame/playable-location claim follows from these tests.

### Shared camera state in original callers — 2026-10-14

Removed the remaining original-source references/declarations for the detached
camera flags (including D_800BCF88), screen-X, group, focus height, digital/analog
base angles, basis matrix, mesh tint and ordering-depth scalar aliases. Their
callers now address fields in the single native GeomScrollRecord directly:
camera, mesh rendering, script commands, player movement, entity frames, menu
and grab/cruiser effects. This avoids separate globals silently diverging when
those units are linked. Flags retain the original signed word type used by
script pointer writes; mesh tint retains its signed-byte behavior. Entity focus
height conversion uses unsigned word arithmetic to preserve wrapping without
left-shifting a negative signed value.

Camera tests additionally verify initialization of the actual record's focus,
tint, group, angles and ordering depth after poisoning all storage. The 509-view,
30,540-frame corpus still passes under ASan/UBSan/function instrumentation of
original/test sources; GPU model regression and full build pass. Some changed
legacy mesh/overlay/script units are not yet part of a linked native scene, so
this is not execution evidence for those paths. Room light tint and projection/
view-slot aliases still need integration; the full scene remains incomplete.

### Original player input and native atan ABI — 2026-10-14

Moved the five original idle/walk/run/facing handlers from player_movement.c
(the unrelated TIM helpers stay there) into player_input.c and linked them with
original action selection/animation. The original ApplyMatrixLV SDK procedure is
also linked. Input handlers use the canonical player pointer and shared camera
basis/angles. Negative sine/cosine scaling uses multiplication rather than an
undefined signed left shift. A call without a directional digital bit now has
a zero-initialized motion vector instead of reading indeterminate stack data.

Digital walk/run tests pass for all four axes using the observed retail speeds,
and verify original action selection. Idle, no-input and Confusion reversal also
pass. Analog left initially failed: the game's int/int ratan2 declaration called
PsyZ's LP64 long/long function, turning a negative axis into a positive 64-bit
value. Our PsyZ fork commit bf37e56 makes the native signature signed int/int,
matching PS1 coordinate width, and adds SDK signature/axis regression cases.
The root end-to-end analog tests now pass for all four axes. The fork is pushed
to khasinski/parasite-eve-psyz and pinned by the root submodule.

Full build, original movement/action-animation CTests and ASan/UBSan/function
instrumented game/test sources pass. PsyZ archive remains uninstrumented; its
new standalone SDK test suite was not run here. Existing positional-audio test
recording remains a test fixture, not a runtime replacement. SDL input sampling,
original input-rule selection, movement/collision integration and scene rendering
are still not wired together in a playable location. Battle movement variants,
diagonals and arbitrary camera rotations require further integration evidence.

### Original input rules drive position integration — 2026-10-14

Added the native data definition of the original 50-slot USA player input rule
table (0x800943C0..0x80094487), preserving all null slots, shared record lists,
masks, ordering and fallbacks from 0x800942F0 onward. This binds the original
movement handlers rather than supplying a test-only input callback. Callback
types now match BattleEntity/int-pointer functions; the turn-in-place handler
accepts and ignores the second dispatcher argument, avoiding an incompatible
function-pointer call on native C.

`original-game-player-input-assets` reads the executable from the disc and checks
all 50 table slots plus every record's masks, flags and mapped callback identity.
The integrated test calls original Entity_IntegratePositionFull: held Up selects
walk/action 22 and moves by the original fixed-point speed, Up+bit 1 selects
run/action 23, and release selects idle/action 21 without further displacement.
It uses real action selection and original rule filtering/movement integration.

Full build and all four input-rule/movement/action/input-assets CTests pass;
ASan/UBSan/function instrumentation of game/test sources passes the disc-backed
integrated test. PsyZ is uninstrumented. The script handler table remains a test
fixture with no scripts attached in this movement scenario, and animation data
is controlled fixture data. There is no full script/collision/SDL-input/render
scene loop yet. Native player-rule data is a separate object target for upcoming
runtime linkage, not a parallel gameplay implementation.

### Raw pad packet to original actor movement — 2026-10-14

Moved original flag-table initialization and Field_HandleStateTransition into
scene/pad_input.c. Original remapping, menu masks, held counters, analog thresholds,
combo sequence and press/release calculation remain intact. PS1 connection/mode/
actuator negotiation is replaced by the host packet's receive-status byte:
nonzero signals disconnect, valid packets reconnect immediately, and subsequent
disconnected frames clear stale inputs. The original first-disconnect pause event
is preserved. SDL packet production is still pending.

Original MenuWidget_FindByModeAndSelectedBase now links from widget_lookup.c;
the active-list head has one definition. Runtime MenuWidgetNode PS1 byte-offset
asserts were removed (native pointer layout); serialized descriptor/layout asserts
remain. The rest of the widget implementation, including legacy navigation views,
still needs native layout migration and is not claimed functional by this change.

Testing exposed unimplemented GTE data registers 30/31 in PsyZ: every original
button-map entry was being stored at the wrong index. Own-fork commit 010004d
implements LZCS/LZCR leading-sign count and read-only result behavior, with axis
edge register tests. It is pushed and pinned. The original map builder now works
without replacement button logic.

The integrated actor test now supplies raw digital pad packets for walk/run/
release, then runs original sampling, rule selection, animation selection and
position integration. It also checks analog field/menu thresholds, repeat-frame
edges, disconnect/reconnect and register edge cases. All 50 executable rule slots
still match. Full build, five targeted input/movement/camera CTests and game/test
ASan/UBSan/function checks pass (PsyZ uninstrumented; standalone SDK suite not run).
These are injected packets, not evidence of live SDL controls or a playable scene.

### SDL keyboard packet input — 2026-10-14

Added host-only SDL keyboard packet production in pc/host_pad.c. It maintains
per-scancode held state, so releasing W while Up is still held does not release
the shared PS1 button. Focus loss/quit clears all held state; events while
unfocused are ignored, and focus regain cannot resurrect stale presses. It
publishes a ready digital packet and centered axes, leaving game remapping,
thresholds and movement to the original code. Current bindings are arrows/WASD,
Shift/Z (Circle), X/Space (Cross), C (Triangle), V (Square), Enter/Escape (Start),
Tab (Select), Q/E (L1/R1). Gamepad devices and configurable mappings remain pending.

The runtime title event loop feeds this packet and runs original pad sampling;
its existing title controls/audio remain in place. The pad units now have their
own object target shared by main and boot/input tests. Replay keydown injection
also supplies scancode/modifier/down fields, matching the input boundary.

The integrated movement test initializes SDL events only, pushes real SDL events
through its queue, and reaches original walk/run/release position updates without
a window. Shared-key aliases and focus loss/regain pass. Full build, original
actor/input-assets tests, menu-audio replay CTest, and game/test ASan/UBSan/function
checks pass; SDL/PsyZ archives uninstrumented. The actual main executable also
passed the headless menu-audio replay before the scancode injection refinement;
the boot replay CTest passed after it. This is input wiring and regression
coverage, not a playable location or end-to-end scene evidence.

### Original floor movement and boundary rollback corpus — 2026-10-14

Extended the existing 414-room collision corpus beyond FindFloor: each eligible
triangle-centroid hit now runs original Entity_ResolvePosition and four one-unit
steps through Entity_UpdateAndRender, with both zero and 32-unit cylinder radii.
The test checks resolved height, unchanged source records, a valid containing
triangle after accepted movement, and exact last-position restoration on rejected
movement. This is the non-player path (no player ramp sliding).

The new path exposed signed-shift UB in boundary visited bits, negative floor
heights and edge-coordinate fixed-point conversion. These now use unsigned word
shifts while preserving the PS1 bit patterns. It also exposed a null neighbouring
face on a zero-radius boundary crossing (first reproduced in map 24). The caller
now reverts to the last valid position if the original neighbour walk returns no
face, rather than dereferencing null. This is a defensive native behavior for an
unreachable crossing, not a claim of PS1 trace equivalence for that edge case.

All 134,616 movement trials pass, including 3,227 rollbacks. The prior 202,296
floor queries / 99,614 hits / 40,274 sloped hits remain intact. Full build and
floor/collision/movement/input regressions pass; ASan/UBSan/function instrumented
game/test sources also pass the entire corpus (PsyZ uninstrumented). Player ramp
sliding, larger moves, authored actor radii and collision connected to the running
SDL scene remain to be verified. No full playable-location claim is made.

### Player ramp sliding uses canonical actor — 2026-10-14

The floor-motion corpus now selects the actor as the player as well as testing
NPC movement. This exposed a separate D_8009D254 pointer definition inside the
collision unit: setting canonical g_PlayerEntity had not activated any player
collision branches. Removed that duplicate storage; original collision/ramp/wall
response procedures now reference the same canonical player used by input and
movement. Other legacy battle units still declaring the old symbol need migration
when linked; no second storage is retained to conceal this integration issue.

Player cases execute two consecutive movement frames while retaining the original
edge-contact state between frames, exercising the initial slide and the next
frame's blocked-edge checks. Across 414 rooms, radii 0/32 and four directions:
403,848 total player/NPC steps, 3,559 rollbacks and 5,270 player edge-contact frames.
Accepted moves remain in their returned floor triangle; rejected moves restore
all three saved coordinates. The earlier floor-query corpus remains unchanged.
Full build and collision/floor/movement/input regressions pass. ASan/UBSan/function
instrumented game/test code passes the expanded corpus; PsyZ uninstrumented.
This covers small local steps and player ramp sliding, not wall polygons, large
moves, collision fidelity against PS1 traces, or the full interactive scene.

### Original wall response: native arithmetic check

The original `Geo_FindNearestEdge` and `Entity_SlideOnWall` now multiply
signed normalized components by 16 instead of left-shifting negative values.
Removed an unused 32-bit pointer union and obsolete matching-only commentary
from the wall routine. No replacement collision solver was introduced.
`original-game-collision-assets` now also exercises the canonical player wall
response against all four sides of a rectangle, checking clearance and preserved
tangential movement (within 1/256 world unit of the requested step).
Full build and collision-assets/floor/movement CTests passed. ASan/UBSan/function
checks passed for game/test sources, including 403848 floor movement steps across
414 Disc 1 rooms; the PsyZ archive remains uninstrumented. Wall tests are synthetic,
not coverage of every scripted room polygon or proof of PS1 trace equivalence.
Playable scene integration remains incomplete.

### Original actor cleanup and owned process cleanup

`Entity_CollectGarbage` now checks whether the new list head exists before
clearing its previous link. The new `original-game-actor-lifecycle` test reproduced
a crash before this fix when deleting the last live actor. It exercises head,
middle, tail and last deletion, player pointer/movement-lock cleanup, actual
model block release, restoration of all 14 free actor records, and empty-list
collection. Owned tasks in both PM banks are removed by the original
`Scene_FreeEntityTable` / `Pm_Stop`, using a fixture handler that counts stops.
The renderer cleanup endpoint is the original `Render_ReturnTrue`.

Process dispatch now includes an opaque field-task binding declaration instead
of depending on the unported field-effect renderer payload layouts. The test
links only the exercised procedures; unused scene loading, PM send/state APIs
and effect handlers are not runtime-integrated by this change. In particular,
legacy integer-carried result pointers in PM send/state remain to be ported.
No runtime replacement handlers were added. Full build and actor-lifecycle,
pools and hierarchy CTests passed, as did ASan/UBSan/function instrumentation
of game/test sources (PsyZ/SDL archives uninstrumented). This is lifecycle
coverage, not evidence of a running full scene.

### Native PM command/result addresses

`Pm_SendCmd` payload arguments and `Pm_SetGetState` value/address argument now
use `intptr_t`, as does `PmSendCallback`. Numeric game values remain signed
32-bit values at their sources; result destinations remain `int *`. The four
original PM script wrappers moved unchanged in behavior from system_opcodes.c
to script/pm_commands.c, with explicit native address casts. The battle actor
address sent to the impact task also no longer narrows to int.

The lifecycle test now calls the original script wrappers and PM dispatcher
with real native stack addresses, verifies all three header result writes,
state set/get, rejected state values, invalid slot errors, negative numeric
payloads, and command-114 dispatch via handler 85. Full build, five related
CTests and game/test ASan/UBSan/function checks passed. Task callbacks in this
test are fixture callbacks; the actual effect-handler registry and its callback
signatures still require integration against the native `PmSendCallback` type.
The previously recorded integer-result-pointer issue is fixed for these PM APIs;
this does not imply the effect implementations or full scene runner are ready.

### Original PM per-frame passes

Moved the original `Pm_RunPrimary` and `Pm_RunBatch` procedures out of the TIM
loader into pm/frame.c so they can link independently of disc/texture loading.
No scheduling algorithm changed. The lifecycle integration test now exercises
both eleven-slot banks through the real `Pm_Start`/`Pm_Exec`, including callback
order, initialized-to-running transitions and tick increments. It verifies
uninitialized, normal, menu-paused and player-only flags: start callbacks still
run when execution is paused, while player-only mode executes only eligible
primary effect commands. Deferred deletion still takes two execution passes.

Callbacks also record the real PsyZ GTE offset registers, proving the original
camera functions select the room offset for start callbacks and restore
(160,112) for execute callbacks. Their declarations now consistently match the
original integer return type. Full build, three related CTests and ASan/UBSan/
function checks on game/test sources passed; PsyZ/SDL archives uninstrumented.
Command-specific handlers remain fixtures here; full effect execution and
Entity_FrameUpdate runtime integration are not established by this test.

### Shared native PM tables and scene initialization

The original three PM pointers now alias the members of one native `g_PmTables`
record. This preserves the PS1 shared-storage relationship: initialization through
`g_PmTables.primary/secondary` is immediately visible through `g_PmSlotTable` and
`g_PmSlotTable2`; handler writes also share the same record. Duplicate extern
views were removed. Original scene initialization was extracted from
Asset_LoadTimTextures to `Pm_InitScene`, compiled in the original game target;
the loader calls it at the same point and with unchanged flag gating.

The lifecycle test initializes actual native slot banks from a synthetic scene
record, checks all 22 header resets and the original handler-ID filtering,
verifies flag changes, and allocates through the original `Pm_AllocSlot` to prove
it reads those same banks. A second initialization preserves occupied slots and
does not rerun handlers. Full build, lifecycle/pools/hierarchy CTests and game/test
ASan/UBSan/function checks passed (PsyZ/SDL archives uninstrumented). The fixture
registry is not the final room handler registry; scene execution remains pending.

### Actor contact arithmetic on native C

The original Scene_UpdateEntityPositions contact pass now explicitly preserves
MIPS modulo-2^32 square/sum, movement-difference and dot-product arithmetic before
signed comparisons. UBSan reproduced signed overflow in the body distance sum
before the fix. A 49-pair synthetic test spanning signed 16-bit coordinate limits
compares the original contact-task decisions with a wide-integer calculation
that explicitly applies the original signed-word comparison. Existing body,
cylinder rollback, part callback and parent exclusion cases remain covered.
Full build and contact/hierarchy/lifecycle CTests passed, along with game/test
ASan/UBSan/function checks (PsyZ archive uninstrumented). This preserves original
word arithmetic rather than changing collision rules to an unbounded-distance
solver. It is not proof of all campaign actor interactions or frame integration.

### Original area entity texture state machine

`Scene_LoadEntityTextures` and `Scene_UpdateBgDraw` now compile independently in
scene/entity_textures.c. Their original state machine is retained. The model
setup result uses an actual native pointer instead of casting an int array to
s8 **, and background animation binding uses the canonical scene_object field
rather than a detached linker-era global.

The new original-game-entity-textures test links the original loader, renderer
and camera. Its synthetic bank and scripted I/O boundary verify day-bank sector
selection, busy polling, read/poll retry transitions, bank-row and three stream
pointer relocation, stream index selection, delayed rebuild states, missing
player behavior and area gating. The test has no TIM uploads or model rebuild:
those branches and the background draw path still need integration coverage.
The I/O functions are test fixtures only; this target is not the native runtime
I/O binding. Full build, entity-textures/lifecycle/contact CTests and game/test
ASan/UBSan/function checks passed, with PsyZ/SDL archives uninstrumented.

### Native archive sector I/O used by the original entity loader

pc/host_disc.c binds an open CUE/BIN disc and PE.IMG extent to the original
CdRom_ReadSectorsFromLba/CdRom_PollReady API. Host reads are synchronous; the
original read-in-progress flags remain set until the completion poll. Busy
reads are rejected, archive bounds and arithmetic are checked before copying,
and unmounted reads fail. This is I/O only, with no rewritten loader logic.
Callers must allocate the complete requested sectors, including bank padding.

The new original-game-entity-texture-assets test drives the original loader
through four actual area/day banks (areas 10/12, days 1/7) from Disc 1, compares
the bytes with the existing archive reader, and checks the original relocated
bank pointers and completion flags. It also rejects reads before the archive,
oversized requests and unmounted reads without changing a sentinel byte.
ASan caught an undersized test allocation based on logical bank bytes instead
of full sectors; the corrected fixture allocates the entire sector range.
Full build, both entity-texture CTests and game/test ASan/UBSan/function checks
passed. PsyZ/SDL archives remain uninstrumented. This adapter is not yet mounted
by the main scene runtime; keyed stream reads, TIM-loading days, model rebuild,
Disc 2 selection and full gameplay integration remain outstanding.

### Real day TIM uploads through the original loader

The native CUE/BIN integration test now covers all days 1..7 for entity-bank
areas 10 and 12: 14 bank selections and 180 original TIM uploads. Days 2..6
traverse the original scratch read/poll/upload states before the entity-bank
read. After each selection the entire PsyZ VRAM is read back and compared with
the independent blob TIM decoder. The test uses a hidden Metal backend; it
never presents a window, but still requires GPU access.

Render_DrawWithAnim declarations/call sites now match the actual original
implementation (typed object, s16 frame, lighting pointer; no unused fifth
argument). Full build, both texture-loader CTests and ASan/UBSan/function checks
of game/test sources passed, with PsyZ/SDL archives uninstrumented. Real model
rebuild/animation attachment and mounting this path in the main game runtime
remain pending; these results establish loading and VRAM behavior only.

### Real resident scene-object rebuild and attachment

The Disc 1 entity-loader integration test now loads resident bank 12 and uses
the original keyed lookup for model 0xC4B5BA04. After each of the 14 area/day
loads, it runs the original delayed states 11, 12 and 13 with native-sized model
work memory. This executes Render_SetupEntityPrims, lighting initialization and
Render_DrawWithAnim, checking state completion and binding to player bone 18.
Both destination matrices must equal the controlled source pose. A subsequent
Scene_UpdateBgDraw follows a changed pose and restores the GTE screen offset.

The source player pose is a fixture, not a real Aya animation. The follow-up
sets variant_visible=0, so it verifies transformation/attachment but does not
claim a rendered background-object image. The real model bytes, loader and
render procedures are used without replacement game handlers. Full build,
both entity-texture CTests and game/test ASan/UBSan/function checks passed;
PsyZ/SDL archives uninstrumented. This covers the exploration model path;
battle stream selection/rebuild and full main scene integration remain pending.

### Attachment driven by real player model animations

The entity-loader asset test now replaces the controlled 19-matrix player pose
with real primary models from banks 18/20 and their area/day animation overrides.
The original Render_SetupEntityPrims builds the native player model; original
Anim_BuildRotationMatrices and Render_TransformVertices process every frame of
every available action, followed by original Scene_UpdateBgDraw. Both attached
object matrices must match player bone 18 on all 9591 exercised frames. Bank
read/TIM VRAM checks and fourteen delayed object rebuilds remain in the same run.

Full build, both entity-texture CTests and game/test ASan/UBSan/function checks
passed, with PsyZ/SDL archives uninstrumented. This replaces the earlier pose
fixture limitation, but not the remaining image/integration limitations:
variant_visible remains zero during attachment updates, action selection/timing
is driven by the corpus test, and the main scene loop is not running yet.

### Native original actor shadow packets

Render_DrawActorShadow now compiles independently in render/actor_shadow.c.
The original transform, shading and projection are preserved; its packet uses
a native PsyZ tag and links directly through native ordering-table pointers.
Scene_LoadMap reserves and skips two sizeof(RenderShadowQuad) packets instead
of the PS1-only 0x50 bytes, including its non-full allocation path. This avoids
placing model work over the enlarged native shadow packet prefix.

The new original-game-shadow test checks exact SDK POLY_FT4 layout, projected
corners, texture fields, ordinary/faded shading, one ordering link in each draw
slot, and disabled/invisible no-write paths. Full build, shadow/lifecycle/
hierarchy CTests and game/test ASan/UBSan/function checks passed; PsyZ archive
uninstrumented. It checks packet construction, not an actual shadow texture
image or complete scene draw. The Scene_LoadMap allocation change is compiled
but still awaits full actor-spawn integration coverage.

### Original Scene_LoadMap native model spawn coverage

The model-address table g_SceneMapPrimBaseTable now holds uintptr_t rather than
32-bit words. Render-buffer declarations now use the canonical typed scene
archive pointer. The original block allocator retains its sixteen slots and
1/2/4/8-block policy, with block bytes scaled by native pointer width. This is
required by native packet heads: map 23/group 2 used 174784 bytes while the old
maximum allocation reserved only 147200. The legacy display scratch reservation
now uses the same block-size constant; its full native arena setup is not yet
runtime-integrated.

The new original-game-scene-spawn test calls actual Scene_LoadMap for all 878
Disc 1 room models, with full and non-full allocation (1756 spawns). It verifies
native model pointers, actor-list insertion, model workspace bounds, native
shadow prefix, actor defaults and actual original Script_AllocNode results.
Full build, spawn/pools/lifecycle/shadow CTests and game/test ASan/UBSan/function
checks passed. PsyZ archive uninstrumented. Actors are tested as NPC type 1 with
no action rows; the player-equipment hook aborts if reached and is test-only.
Player spawn, animated spawn, multi-actor memory pressure and the complete
Scene_LoadMap runtime path remain unverified.

### Original stat-growth queries for player initialization

The four original stat query procedures moved from save/game_data.c to
aya/stat_queries.c, and the growth/level/EXP table accessors from text/tables.c
to aya/stat_tables.c. They compile in the original game target. Pointer lookup
now uses native table indexing instead of adding a truncated 32-bit address;
sublevel numerator arithmetic preserves 32-bit word multiplication without
left-shifting a potentially negative value. The original search, caps and
level/distance/overflow behavior remain intact. ResidentDataPack has native
storage for the original startup data.

original-game-stat-assets reads resident bank 2 directly from the supplied
Disc 1 archive. Across all seven growth tables it compares every value 0..32767
(229376 cases) with an independent linear interval calculation, including
sublevels, distance and overflow, plus nullable outputs and level-row clamping.
Full build, stat-assets/scene-spawn/entity-textures CTests and game/test
ASan/UBSan/function checks passed (PsyZ archive uninstrumented). This establishes
stat query prerequisites; equipment lists, complete stat recalculation and
Battle_InitEquipLists/player spawn are still pending.

### Original player stat recalculation and inventory capacity

Inv_RecalcSlotStats now compiles independently in aya/stat_recalc.c and reads
the canonical g_PlayerEntity instead of the detached D_8009D254 symbol. Its
original signed-halfword growth inputs and HP/stat/ATB mappings remain intact.
AyaSaveState and seven modifier words now have native storage. The original
capacity setter/getters moved to item/capacity.c, using the actual save-state
capacity and inventory fields rather than linker-relative address arithmetic.
Duplicate definitions of the active-list pointer, limit and bonus count were
removed so the migrated paths share one state.

The resident-data test exercises all 99 levels with three modifier sets (297
combinations), checking derived HP, existing-HP clamping/preservation, combat
stats, ATB parameters, displayed level and inventory capacity. It also checks
that an override list retains its own limit and recalculation without a player
does not change current HP. Full build, stat-assets/scene-spawn/pools CTests and
game/test ASan/UBSan/function checks passed (PsyZ archive uninstrumented).
Weapon/armor item decoding, modifier extraction and Battle_InitEquipLists are
still needed before the original player-spawn path can run.

### Shared Aya save/inventory storage and original list selection

AyaSaveState and InventoryRuntime now share one union-backed native allocation,
matching the original overlap at equipment selections and the 50 inventory IDs.
The independent character storage would otherwise let equipped slots and item
lists diverge between stat and inventory procedures. Layout assertions and
bidirectional writes in the test verify both equipped slots and all 50 IDs.
InventoryRuntime remains the original serialized layout; no save-format expansion
or gameplay-level inventory abstraction was introduced.

The original select/reset/override/query list functions compile independently
in item/list_selection.c. Active-list selection pointers, counts and the two
selection bitmaps have canonical native storage instead of duplicate tentative
definitions. Tests cover ordinary/override selection, capacities, bitmap banks,
reset and missing-override fallback alongside the real stat corpus. Full build,
stat-assets/scene-spawn CTests and game/test ASan/UBSan/function checks passed
(PsyZ archive uninstrumented). Item record lookup and weapon/armor derivation
remain prerequisites for complete player equipment initialization.

### Original item lookup and weapon parameter derivation

Original Item_LookupBaseData, Inv_LookupData and Inv_LookupActiveListData now
compile in item/data_lookup.c against shared character/inventory storage. The
migrated key-item lookup indexes nine actual records instead of the PS1 base
biased by -0x4000. Other legacy key-table consumers remain to migrate; startup
loading of these nine records is not yet in the main runtime. The test reads
them from the USA executable and covers base/equipment/key/invalid IDs and
active/override slot boundaries.

Inv_BuildWeaponList moved from battle/commands.c to item/weapon_stats.c without
changing its derivation rules. Its six native effect-count constants are checked
against the supplied executable. Tests exercise all 86 resident weapon records,
1201 range/stat values across the 999 cap, integer-square-root bounds, ammunition
and category fields, plus individual effect inputs (including half-attack and
hit-count/mode selections). Full build, stat-assets/scene-spawn CTests and
game/test ASan/UBSan/function checks passed (PsyZ archive uninstrumented).
This does not establish all effect combinations or attack gameplay. Armor
parameters/modifier extraction and complete player equipment initialization
remain pending.

### Original armor parameters and equipment stat modifiers

Inv_BuildArmorList and its integer-root/clamp helpers moved from aya/stats.c
into item/armor_stats.c, retaining the original derivation and effect rules.
BattleCmd_LoadWeaponModifiers moved from battle/commands.c and now reads the
canonical shared Aya equipped-armor slot instead of a detached legacy symbol.
Its original modifier reset, effect ordering, stat recalculation and active-list
restoration are preserved. This links the original armor calculations into the
native original-code target; it does not replace them with a host implementation.

Verification covers 51 real resident armor records, 1201 parameter inputs,
individual effects 0..17, overlapping effects, inventory bonuses, override-list
restoration and unequipped reset. Full build and the stat-assets, scene-spawn and
pool CTests passed. The stat test also passed ASan/UBSan/function instrumentation
with the actual Disc 1 data (PsyZ archive uninstrumented). These checks establish
parameter derivation, not armor behavior in a running battle. Complete original
Battle_InitEquipLists/player-spawn integration and playable scenes remain pending.

### Original equipment initialization and native battle-state prefix

Battle_InitEquipLists now compiles from battle/equipment_init.c, retaining its
original pointer setup and stat/weapon/armor call order. Its three output records
have canonical native storage. AyaBattleState's opaque prefix now follows the
actual Combatant action offset, with assertions for both equipment pointers and
total prefix size; fixed PS1 pointer offsets are not valid on a 64-bit host.
The legacy save-flow byte copies still require explicit serialized conversion
before that path is enabled. Initial battle defaults and Battle_SaveAyaState
are not yet integrated into startup.

The real resident-data test runs all 4386 weapon/armor pairs through this original
initializer and checks pointers, preserved prefix/tail bytes, weapon attack and
kind, armor third parameter and active inventory selection. This exercises the
initializer without a live player; existing stat-recalculation tests separately
cover a live player. The scene-spawn fixture now links this actual initializer
instead of its former abort hook, though its corpus still spawns NPCs only.
Full build, stat-assets/scene-spawn/pools CTests and stat ASan/UBSan/function
checks passed (PsyZ archive uninstrumented). Player spawn and playable scene
integration remain pending; this is not end-to-end gameplay evidence.

### Original saved battle-state reset and USA defaults

Battle_SaveAyaState now compiles alongside Battle_InitEquipLists. The original
copy/reset procedure is unchanged. Native initial records reproduce the USA
0x80010928/0x80010998 templates; designated word indices follow Combatant field
offsets across native padding, rather than copying PS1 bytes over host pointers.
The stat-assets test reads both source templates directly from the supplied USA
executable and compares every meaningful prefix byte, both null pointers and
the full action tail after poisoning and resetting the native records twice.

Full build, stat-assets/scene-spawn/pools CTests and the instrumented stat test
(ASan/UBSan/function; PsyZ archive uninstrumented) passed. Startup still needs to
call this original reset in the full player/scene initialization sequence; this
change alone does not initialize New Game inventory, implement save conversion
or establish a playable player spawn.

### Original player spawn with actual Aya model banks

The scene-spawn test now exercises the player branch of original Scene_LoadMap
using actual entity banks 18 and 20, their model roots and action 0x15 records.
It loads the real resident stat/item tables, resets battle state through the
original procedure, then checks full and non-full allocation (four player
spawns). Checks cover the shared player/list/state pointers, equipment storage,
initial action, script node/base, movement factor, render matrices and derived
maximum HP. The original model setup and initial animation/draw path execute.
PsyZ is initialized without presenting a window; no pixel comparison is claimed.

This passed alongside the existing 1756 NPC spawns and stat-assets/pool CTests,
and under game/test ASan/UBSan/function instrumentation (PsyZ uninstrumented).
The player fixture deliberately supplies one valid weapon and armor plus a
minimal script: it is not the actual New Game inventory or room script startup.
This establishes that the migrated original player-spawn dependencies work
together, not that scene initialization/frame progression or gameplay is ready.
The next integration gap is the original New Game/scene data setup and callbacks.

### Original inventory insertion prerequisite for New Game

Inv_AddItem, Inv_FindSlotByIndex, Inv_WriteSlotById and their original slot,
record and ammo helpers now compile from item/add.c. The algorithms retain
original equipment cloning, active-slot behavior and ammo pooling/capping.
g_InvSpecialItemRecords aliases the canonical g_KeyItemRecords: the historical
USA symbol is 0x800A1E64, also the actual base of the biased key-item table.
This prevents ammo writes from diverging from inventory lookup storage.

The stat-assets test inserts the original five starting IDs (0x44, 0x96, 0x3F,
1, 6), checks cloned equipment and pooled ammunition, then tests full inventory,
continued ammo pooling, the 999 cap, exhausted equipment records and direct
special-item insertion. Full build, stat-assets/scene-spawn/pools CTests and
ASan/UBSan/function stat checks passed (PsyZ uninstrumented). The fixture sets
up ammo capacity explicitly: complete Inv_InitNewGameInventory, preceding
special-record initialization, menu fade dependencies and tagged NG+ carryover
remain to integrate and verify.

### Original New Game inventory initialization and menu setup dependencies

Inv_InitNewGameInventory and its tagged-item carryover helper compile from
item/new_game.c. Growth initialization now addresses the canonical Aya stat
array; the original item order, HP/capacity/spell reset and tagged equipment
selection remain unchanged. Original Menu_ClampRange, Menu_SaveBgInitFade and
Menu_ComputeGammaLut compile independently with shared native state. Their
state declarations are separated from unported DRAWENV layout declarations;
no menu drawing replacement or stub was introduced.

Tests cover initial stats/items/ammo, equipped selections, one tagged weapon
and armor carried from special storage, menu range and the exact fade curve.
The actual-model player-spawn test now calls this original New Game initializer
instead of manually choosing equipment. Full build, stat-assets/scene-spawn/
pools CTests and both stat/spawn ASan/UBSan/function runs passed (PsyZ archive
uninstrumented). Earlier category-grid/special-record setup, metadata/name
initialization, complete NG+ combinations and startup/room scripts are still
pending; these fixtures do not prove complete New Game or menu rendering.

### Original category-grid and storage preparation

Inv_BuildItemGridFromCategory now compiles from item/category_init.c and uses
canonical Aya inventory, Wayne storage and special-storage arrays. Its original
category search and nine cyclic record copies are retained. Descending clears
use array indices to avoid forming a pointer before the start of an array.
The category base ID has one native definition.

New Game tests now call this original preparation, verify all nine records
against resident item data (including the original 999 ammo capacity), verify
both cleared storage arrays and override reset, then exercise original New Game
and tagged carryover. The real-model player-spawn test also uses this preparation
instead of leaving special records uninitialized. Full build, stat-assets and
scene-spawn CTests plus both ASan/UBSan/function runs passed (PsyZ archive
uninstrumented). Complete startup metadata, room scripts/callbacks and scene
frame integration remain pending; no full New Game or gameplay claim follows.

### Native relocation of original scene task blocks

Script_RelocBlock resolves the original base/count/offset table into a native
pointer view instead of overwriting asset offset words with truncated pointers.
The single active scene view is replaced on subsequent relocation; passing the
resolved view again is idempotent. Disk offsets remain 32-bit and unmodified.
The native view retains the original pointer-table indexing used by Scene_LoadMap.
Script_DrawSyncAndFlush retains its original descriptor iteration and now compiles
with the scene/frame target in script/spawn_scene.c.

All 414 real room task blocks were checked against the independent asset decoder:
base pointer, every script entry, repeat relocation and unchanged asset bytes.
The player-spawn fixture also calls the original sync/spawn loop for its full
allocation cases using a minimal one-player descriptor, real Aya model/action
banks and original New Game/equipment initialization. Full build, scene-spawn,
stat-assets and actor-lifecycle CTests passed, as did the updated spawn test under
ASan/UBSan/function instrumentation (PsyZ archive uninstrumented).

This is not execution of all room scripts: only relocation is covered for the
414-room corpus, and spawn-loop input remains a controlled descriptor. Complete
room startup, script dispatch/callbacks and malformed-block bounds validation
still need integration. The relocation routine expects a validated asset block.

### Consolidation back into existing modules (user correction)

Removed aya/stat_recalc.c and script/spawn_scene.c. Their implementations are
back in aya/stats.c and script/reloc_block.c respectively; both complete source
files compile. CMake uses function-level dead stripping for test links instead
of requiring source extraction. Unused entry points are not claimed as executed.
Compared the union of definitions immediately before consolidation with the
merged files: all 11 stats functions and both relocation/spawn functions remain.
This audit covers these merges only; earlier extracted armor/modifier functions
and the other modules still need restoration and a historical completeness audit.

The complete stats module now uses canonical Aya fields for the two scalar stat
queries and the canonical player pointer for equipment HP synchronization.
TextboxEntry's runtime assertions account for native pointer alignment; the
textbox header no longer imports an unused PS1 draw-environment dependency.
Full build and stat-assets/scene-spawn/actor-lifecycle CTests passed, as did
stat and spawn ASan/UBSan/function runs (PsyZ archive uninstrumented).

The pending real-map spawn test was corrected to compare player presence with
actual descriptors. New Game map 431 has one initial descriptor and no player
on that list; this test does not establish script-created Aya or playable startup.
The pc/ directory and other extraction files still exist. Their removal and
integration into existing original modules remain mandatory unfinished work.

### Restore resident-table functions to text/tables.c

The three Aya table accessors and resident-data storage are back in the existing
text/tables.c; aya/stat_tables.c was removed. Item_LookupBaseData also returned
to text/tables.c from item/data_lookup.c. The whole original tables module now
compiles, including its text, map-name, glyph and menu-descriptor accessors.
An unused psyq_gpu.h dependency was removed from render_prim.h; its declarations
only require the existing common, GTE and packet headers.

Comparing function-definition sets across all affected files before/after the
merge retained all 19 functions. Full build, stat-assets/scene-spawn/actor-lifecycle
CTests and the stat ASan/UBSan/function run passed (PsyZ uninstrumented). This
verifies the existing exercised table/stat paths, not every newly compiled text
or menu accessor. No new files were created. Other extractions and pc/ removal
remain unfinished requirements.

### Restore complete original allocation module

Entity_AllocBlock and Entity_FreeAllocationBlock, including native storage,
returned to entity/allocation.c; entity/block_alloc.c was removed. Compared
against allocation.c before extraction (4bbad127f parent): all four original
functions remain, with the previously verified signed-word helper added for
portable MIPS arithmetic. The complete original file compiles; no source slices
or conditional exclusions were introduced. Original-object compilation now
explicitly enables function/data sections (or MSVC /Gy /Gw) for whole-module
linking into individual tests; other platforms have not been validated here.

Full build and pools/contact/actor-lifecycle/scene-spawn CTests passed, followed
by contact and spawn ASan/UBSan/function runs (PsyZ archive uninstrumented).
No new files were added. Remaining extractions and the pc/ directory are still
pending consolidation/removal, not accepted as the final architecture.

### Restore shadows to the original mesh-list module

Render_DrawActorShadow returned to render/mesh_list.c and actor_shadow.c was
removed. The complete mesh-list file compiles; unused field-tile/task includes
were removed rather than importing unrelated legacy GPU and task-layout types.
Comparison with mesh_list.c before extraction (39da513f7 parent) confirms all
15 historical function definitions remain. No new files or source exclusions.

Full build, shadow and real-data camera CTests passed. The existing shadow test
also exercises the original Render_ClipPoint over 60,501 input pairs, and the
combined test passed ASan/UBSan/function instrumentation (PsyZ uninstrumented).
This is not validation of the entire renderer: sprite/mesh packet linking still
contains legacy 24-bit address assumptions, and those paths need native packet
repair and scene-level image evidence. Consolidation preserves them for work in
place; successful compilation is not counted as functional completion.

### Repair native mesh scrolling in place

Render_ScrollEntries now indexes the existing GeomEntryView array before using
its scroll member. Incrementing a GeomScrollEntry pointer used the old 56-byte
stride while native GeomEntryView occupies 64 bytes on this host, misaddressing
all entries after the first. Scroll/parallax calculations and pause flags are
unchanged. No functions moved and no files were added.

The existing shadow/mesh test checks three consecutive entries across fractional
positive/negative scrolling, a second update, camera-relative parallax, saved
camera coordinates and both pause flags. Full build, shadow and real-data camera
CTests and the mesh/shadow ASan/UBSan/function run passed (PsyZ uninstrumented).
Sprite packet linking and serialized-to-native geometry preparation remain
unfinished; these synthetic runtime entries are not scene-level image evidence.

### Original mesh fade counters share canonical state

The original fade-count/counter names now refer to the existing tint fields in
GeomScrollRecord, matching their shared USA addresses 0x800BCFFA/0x800BCFFB.
This removes unresolved detached linker-only storage from the mesh fade path.
No functions moved and no files were created.

The existing mesh/shadow test executes every frame for durations 2..255 across
three entries and both buffers, checking RGB values, inactive-buffer preservation,
completion flags/depth and canonical counter fields. It also tests both phases
of scene-load tint toggling. Full build, shadow/real-data camera CTests and
ASan/UBSan/function checks passed (PsyZ uninstrumented). Duration 0/1 behavior
and conversion of legacy sprite packets to native PsyZ packets remain pending;
this verifies original color/state calculations, not rendered fade images.

### Native room-sprite packets and links in original renderer modules

mesh_list.c now links sprite/page packets using full native addresses and
RenderNativeTag ordering entries instead of 24-bit masks and four-byte OT
strides. Self-relative position addressing uses uintptr_t. Existing room tile
and texture-page structs carry native tags; fade/tint loops now use the actual
sprite stride/color offsets. mesh_entry.c compiles as a complete original module
and builds both packet buffers with native pointer arithmetic. No files added
or functions extracted.

The existing mesh/shadow test verifies two sprites linked at one depth in both
buffers (page/sprite order, preserved command lengths, full pointers and screen
coordinates), then calls original Render_LoadMeshEntry and checks the resulting
work-buffer extent, native PsyZ SPRT_16 fields and both page buffers. Existing
scroll/fade/tint tests use the enlarged packet layout. Full build, shadow,
real-data camera and scene-spawn CTests and mesh/shadow ASan/UBSan/function checks
passed (PsyZ uninstrumented). This is packet/state evidence, not rendered scene
fidelity. Serialized geometry conversion, all wrapping/culling cases, glyph
consumers of the shared page type and full GPU image comparison remain pending.

### Restore the complete camera module

camera_view.c was merged back into existing render/camera.c and removed. All
25 historical function definitions from before extraction (fc15988f6 parent)
are retained. The complete module compiles without source exclusions; an unused
menu-background include was removed. No new files were created.

The real-data camera test now also calls original Render_LoadGeomState for all
509 viewports and verifies the loaded projection distance and matrix against
independently decoded room data. Existing checks cover 30,540 transition frames.
Full build, camera/actor-lifecycle/entity-texture-assets CTests and camera
ASan/UBSan/function instrumentation passed (PsyZ uninstrumented). Geometry-entry
conversion is still unresolved: viewport records remain serialized-compatible,
but the enlarged native mesh entries require separate preparation within the
existing loader. This change restores the whole camera module and validates
viewport loading, not complete room initialization or all animation commands.

### Exercise original geometry animation commands in place

The existing camera test now links canonical game-state storage and runs the
original geometry reset, group animation and control commands on two controls
and four native entries. It verifies group gating, visibility selection,
frame timing/wrap, duration writes, reverse-step selection, negative frame
encoding, stop-marker consumption and pause. Render_SetEntryLimit shifts an
unsigned word when encoding signed frame input, preserving PS1 bits without C
signed-left-shift undefined behavior. No files added or functions moved.

Full build, real-data camera/shadow/actor-lifecycle CTests and camera
ASan/UBSan/function checks passed (PsyZ uninstrumented), including existing
509-viewport/30,540-transition-frame coverage. This does not establish all
animation flag combinations or actual room animation playback; serialized
geometry preparation and full scene integration remain pending.


### Original armor/weapon module restoration

Restored Inv_BuildArmorList to src/main/aya/stats.c, and Inv_BuildWeaponList,
its effect data and BattleCmd_LoadWeaponModifiers to src/main/battle/commands.c.
Deleted item/armor_stats.c and item/weapon_stats.c; no new files were added.
The full commands module now compiles in the original object target. Definition
inventory retains all 35 names in the pre-merge union and all 22 historical
commands functions (fc07c19cd parent) and 13 historical stats functions
(53dcc5af2 parent), including local helpers. This establishes preservation of
entry points, not verified runtime behavior of every function.

Battle command payloads now retain native item pointers, including the producer
in item/actions.c. The four-entry pending-command stack owns actual array
storage and uses its own end, rather than assuming adjacency to modifier globals.
Inv_SetActiveList obtains the actor core through the native player pointer.
Existing stat-assets tests exercise twelve successive allocations/overflow shifts,
checking order, capacity and all three full pointers after each allocation.
Full build, real Disc 1 stat-assets and scene-spawn CTests pass. Stat-assets
also passes ASan/UBSan/function, including real weapon/armor/equipment data and
New Game cases. Full command execution, undo paths and unported callers remain
unverified; the historical no-prototype CommitAmmoAndUpdate call convention
still needs resolution. The pc/ directory and other extracted files still
require removal through restoration of original modules and platform integration.


### Restore complete save/game_data.c compilation

Returned the stat-query functions from aya/stat_queries.c and New Game inventory
initialization from item/new_game.c to their original save/game_data.c module;
both extraction files were deleted, without adding files. The shared SelectAya
helper is defined once. The definition inventory preserves all 42 names in the
pre-merge union. CMake now compiles the entire original game_data module.

Removed unused GPU/room/Bios includes and used the existing asset-table and
battle-command declarations. Runtime menu queue, memory-card port and SJIS
format-argument layout checks now account for native pointers. Serialized save
record assertions remain unchanged. New Game reads the first native growth-table
integer with an explicit u16 conversion rather than aliasing its first halfword.

Full build and real Disc 1 original-game-stat-assets and original-game-scene-spawn
pass. Stat-assets passes ASan/UBSan/function after the final change, covering
229376 growth queries, initial and tagged carryover inventory, real equipment
and derived stats. Other game_data entry points, memory-card I/O, save/Continue
and the full original New Game runtime remain unverified. Whole-module
compilation with dead stripping is not proof that every entry point is linked
or executed. Further extraction removal and elimination of pc/ remain required.


### Restore the complete active inventory module

Merged item/category_init.c, add.c, data_lookup.c, list_selection.c and capacity.c
back into their original item/active_list.c and deleted the five extracted
files. No new files were added. The complete active_list module now compiles in
the original object target. Function-name audit preserves all 63 pre-merge
names, and all 62 names from the module before the capacity extraction
(ec026934b parent); this includes static helpers and is not runtime coverage.

Preserved native storage and lookup corrections from the extracted code. Removed
local NULL redefinitions/undefinitions, replaced a truncated pointer bound in
Inv_ClearEquipFlagForKind, and changed script-variable indexing to native pointer
arithmetic in Inv_RebuildSelectionBitset. These latter two whole-module entry
points are compiled but have not yet been exercised with their full dependencies.

Full build and real Disc 1 stat-assets/scene-spawn CTests pass. Stat-assets also
passes ASan/UBSan/function, including initial and tagged carryover inventory,
item insertion capacity/ammunition boundaries, real weapon/armor combinations
and stat queries. Menu rendering, storage-list synchronization, random item
selection and the remaining active_list entry points still require integration
and execution tests. Full game and removal of pc/ remain incomplete.


### Restore full battle encounter module

Returned Battle_InitEquipLists, Battle_SaveAyaState and their native state and
USA default templates to battle/encounter.c, deleting battle/equipment_init.c.
CMake compiles the entire original encounter module, without adding files.
All 16 function definitions from the pre-merge union and the historical module
before equipment extraction (6efa75e3f parent) are retained. Battle gauge alias
assertions now reference the existing native page/sprite packet layout rather
than fixed PS1 offsets; this does not establish native HUD rendering support.

Full build, real Disc 1 stat-assets/scene-spawn CTests and stat-assets
ASan/UBSan/function pass, including USA executable default-state comparisons
and 4386 weapon/armor pairs. Encounter startup, enemy reset, battle phases,
rewards, death and HUD paths are now compiled together but remain unverified
at runtime. Unused functions are still dead-stripped from tests; this is not
proof of a functioning battle. Other extractions and pc/ removal remain open.


### Restore complete status panel module

Returned the fade initialization, gamma curve and clamp functions/storage to
menu/status_panels.c and deleted menu/fade_init.c. All 25 pre-merge function
names remain, and the whole original module compiles. No new files were added.
Fixed selected-widget pointer truncation and gave widget input callbacks their
node/buttons signature; remaining unported callback assignments need auditing
as their modules are brought into the build. Native widget field checks are
relative to pointer-bearing fields rather than fixed PS1 offsets.

DR_ENV now has PsyZ's native address/length prefix and native command words.
Independent compilation against both local and PsyZ headers verifies DR_ENV
size 136, DRAWENV size 168, packet offset 32 and RGB offset 25 on this 64-bit
host. This verifies layout only, not GPU execution or other platforms.
Existing stat-assets tests now cover all 65536 byte-valued gamma input/threshold
pairs and clamp inputs -1024..1024. Full build, real Disc 1 stat-assets,
scene-spawn, camera and shadow CTests pass; stat-assets ASan/UBSan/function
passes after the changes. Menu image transfer, visual fading, widget dispatch
and the full status renderer remain unverified. Other extractions and pc/
removal remain required.


### Restore fade declarations and verify original pixel processing

Merged menu_fade_state.h declarations back into the existing menu_background.h
and removed the extracted header. Updated the save module and existing stat
fixture to use the original header; no files added.

The existing stat-assets fixture now calls original Menu_SaveBgApplyFadeStep
on all 65536 PS1 pixel values, in eight legal 8192-pixel blocks, for every
entry of the default gamma curve and tint values 0, 72 and 255. An independently
rearranged channel formula checks output, transparent zero, preservation of
bit 15 and both native DRAWENV RGB fields. Forward completion reaches state 7;
backward completion reaches state 0 and restores the source pixels exactly.
The test restores the canonical background-buffer pointers before continuing.

Full build, Disc 1 stat-assets/scene-spawn CTests and stat-assets
ASan/UBSan/function pass. This validates the original CPU color transform and
completion boundaries, not StoreImage/LoadImage integration, an on-screen fade
or comparison against captured console frames. Full game, remaining extracted
files and removal of pc/ remain open.


### Scene loader consolidation prerequisite

Removed the extracted scene_entity_textures.h header, restoring its declarations
to existing scene_room_load.h and updating consumers. No new files added.
Full build and Disc 1 entity-textures, entity-texture-assets and scene-spawn
CTests pass. Source consolidation of entity_textures.c is still outstanding.

An attempted whole room_load.c compilation exposed an essential unresolved
format issue: SceneRoomRecord is serialized as 12 bytes but embeds a native
handler pointer in its trailing word. On 64-bit hosts this changes the record
stride; merely changing its size assertion would misread archives. Its two
handler installations also currently treat PS1 addresses as native callbacks.
The source merge was reverted pending an explicit mapping to original native
procedures. Boot-disc header dependencies additionally pull in PS1-only CD
runtime layouts; the file-image backend should replace the relevant dependencies.
Do not claim the full room loader or room script callbacks are functional.


### Disc 1 room handler descriptor audit

Clarification of the loader prerequisite: the trailing room-record address
points to a descriptor, not directly to a function. PM slots below 85 use a
seven-function PmCommand descriptor; slots at/above 85 select FieldFxTaskProgram
(table, initializer and script). Native binding must account for each descriptor's
contents and the owning overlay, including relocated copies of shared routines.
Different addresses alone do not establish different behavior.

The existing original_scene_spawn test now audits all 414 Disc 1 room blobs:
479 records comprise 298 PM descriptors and 181 field-effect programs, with 98
slot IDs and 212 distinct slot/address pairs. Every source offset is 8 and all
aligned descriptor spans (28/56 serialized bytes) lie inside their room at the
USA load base 0x8018efe8 (corrected by the subsequent descriptor-content audit).
The archive prefix is included at that base, not placed before it.
The test explicitly labels native dispatch unverified; no PS1 address is invoked.

Build, Disc 1 scene-spawn CTest and scene-spawn ASan/UBSan/function pass, retaining
all 414 relocation/immutable-asset checks and 1756 NPC plus four Aya spawn cases.
This is a data-format inventory and prerequisite for original module integration,
not implementation of native handler registration or completion of room loading.
No files were added. Next required work is resolving the descriptor function/data
references to the original linked overlay modules, then restoring room_load.c.


### Correct room descriptor base using actual callback contents

The previous span-only audit used a base eight bytes too low. Corrected the
room archive base to 0x8018efe8: the historical overlay extraction includes
the archive prefix. At the old base, the apparent first mirror method was
MirrorSelect and the last two words were data (0xf000, 0), despite passing bounds.
The actual M017 table at 0x8018fb44 contains MirrorNoOp0, MirrorReset,
MirrorSelect, MirrorTick, MirrorNoOp4, MirrorKill and MirrorNoOp6, matching
mirror_model.c and historical sym.room_m017_theater.txt at 1d20afe62.

The existing scene-spawn audit now checks all seven exact method addresses for
all seven mirror descriptors and validates all 2086 PM method addresses as
aligned and inside their owning room blob. This catches the old eight-byte
base error. Full build, Disc 1 scene-spawn and ASan/UBSan/function pass. These
checks identify serialized function references; no native PM registry or mirror
runtime has been implemented by this change. Binding additionally requires
correct typed callbacks, native actor/model structures and renderer integration.
No files added.


### Restore PM initialization to the complete TIM loader

Merged pm/scene_init.c back into asset/tim_textures.c, its original module,
and removed the extraction file. The canonical PM storage, InitHandler helper,
Pm_InitScene and Asset_LoadTimTextures are retained; the whole module now
compiles in the original object target. No new files added.

Extended existing original_entity_textures real-disc fixture to execute
Asset_LoadTimTextures from state 0 through the actual sector-bank-5 read,
poll and upload. It checks the scratch bytes against an independent bank load,
completion flags and the complete VRAM contents against pc/vram's byte decoder.
Disc 1 bank 5 contains one upload. This fixture uses an empty scene key table;
room-specific 0x73DECD80 keyed image uploads remain unverified. PM initialization
is exercised separately by original_actor_lifecycle (22 slots, handler init
selection and the once-only flag), not by a stub in this test.

Full build, actor-lifecycle, entity-textures and real entity-texture-assets
CTests pass. Entity-texture-assets ASan/UBSan/function passes with the common
bank plus 14 area/day banks, 180 existing TIM uploads, 14 model rebuilds and
9591 animation attachment frames. PsyZ itself is not instrumented. The native
room descriptor registry, complete room loader and removal of pc/ remain open.


### Verify original room-keyed TIM uploads across Disc 1

Extended existing original_entity_textures real-disc test to execute the full
Asset_LoadTimTextures read/poll/upload path for every populated Disc 1 room.
An independent byte-oriented test decoder scans the 12-byte keyed record table,
resolves each packed image's self-relative pixel source and checks bounds before
copying its pixels to expected VRAM. It overlays these images after the common
bank in original key order (0x73DECD80, increasing by four until the first miss).
The production path remains the original loader and original asset lookup.

All 414 rooms pass complete VRAM comparisons: 123 rooms contain 296 keyed images.
The other rooms exercise the missing-key termination path. The test also checks
completion flags, retains common bank, area/day bank and animation checks, and
uses actual file-image I/O. Build, entity-texture-assets CTest and
ASan/UBSan/function pass; PsyZ archives are not instrumented. The PM registry is
already-initialized fixture state here, so this does not prove native room
handlers, full scene initialization, presentation or gameplay. No files added.


### Original TIM loader I/O transition coverage

Expanded the existing synthetic entity-textures fixture to call the restored
Asset_LoadTimTextures function for all eight combinations of force, request bit
and pending-texture bit. Scripted read/poll outcomes exercise initial read
failure, repeated failure, successful submission, pending poll, poll failure,
resubmission and completion. Assertions check exact I/O counts/destination/LBA,
retention of the pending flag until completion, preservation of unrelated flags
and absence of extra reads after completion. This uses the original state
machine and original asset lookup; only the existing I/O boundary is scripted.
The synthetic archive has no images, complementing the real 414-room VRAM test.

Full build, original-game-entity-textures CTest and its ASan/UBSan/function
run pass. No new files or game implementation were added. This proves the
loader retry transitions, not recovery of every real device/file I/O failure,
full room initialization or completion of the port.


### Remove abandoned PC LFO implementation

Deleted pc/akao_lfo.c and its three pe_akao_channel_*_lfo_step declarations.
Reference audit found no production caller of those functions: the only calls
were in their own pc/tests/test_akao_lfo_data.c fixture. Removed that duplicate
implementation's tests and build dependencies; retained the independent waveform
binary-data decoding/invalid-offset checks, now including room.h directly.
No replacement gameplay implementation or new files were created.

Original AKAO modulation coverage remains in original_akao_instrument.c: the
actual key-on/key-off voice-driver paths check pitch, volume and auxiliary LFO
values, loop positions and parameter flags, plus original slide opcodes. This
is not a claim of exhaustive parity with every removed experimental assertion.
Full build and five CTests pass: original AKAO instrument, real assets, menu
assets, musical ticks and retained waveform decoder. Existing pc/background
and test_room_assets user changes were left untouched. Remaining PC channel,
player, music and scene-audio code is still referenced by the unfinished PC
scene implementation and must be replaced with original modules before removal.
The full pc/ directory removal and full-game audio integration remain open.


### Compile the original scene-audio track loader in place

Added the complete existing cdrom/track_read.c module to the original AKAO
object target. Replaced its legacy memcpy declaration with string.h and made
CdRangeAddress's integer view native-width to avoid truncating range pointers.
No functions were extracted and no files were added.

Existing original_akao_instrument now executes Akao_ResetPositionalState against
the canonical game-state layout and the real original command queue. It verifies
both slots' keys/banks reset to -1, clearing exactly flag bits 0xf0 and enqueueing
F0. Build, original AKAO instrument/assets/menu/tick CTests and the instrument
ASan/UBSan/function fixture pass. This tests reset and queueing, not completion
of the queued stop command or the track-loading state machine. SeekToTrack,
ReadAssetSectors and stream/sequence transfer paths are compiled together but
still need end-to-end file-image tests before replacing PC scene-audio calls.
PsyZ is not instrumented; range pointer arithmetic has not yet been exercised
through the complete audio range loader. Full audio and pc/ removal remain open.


### Original sector-to-SPU audio bank path

CdRom_ReadAssetSectors now passes its computed absolute chunk sector directly
to CdRom_ReadSectorsFromLba, allowing the existing CUE/BIN backend to serve the
original loader without drive negotiation. The original phases, chunk limits,
polling and SPU upload procedures remain in track_read.c. Restored missing
stream A counters and the 0x2000-byte stream instrument cache in original
akao/state.c (128 records; historical USA symbol 0x800B4900).

The existing AKAO tick-assets fixture now supplies the actual audio-sector
directory and executes the original sector loader with 33-sector chunks before
its instrument/SPU-RAM comparisons and 240 playback ticks. All 599 supported
sequence cases across 414 rooms pass, including 579 multi-chunk bank loads and
8883 channels. Comparison with the previously built whole-bank path gives the
same 51102257 nonzero PCM samples and hash 25836548e09d8a00. One short record
remains explicitly unexecuted, so this is not complete audio coverage.

Full build, original instrument/assets/menu/tick CTests and the complete
chunked tick-assets ASan/UBSan/function run pass. PsyZ archives are not
instrumented. Modes 1/2/3, SeekToTrack, scene command dispatch, XA and removal
of the temporary pc/host_disc backend still require integration. No new files.

Przywrócono `Entity_AdvanceAnim`, `Entity_SetActionMode` i `Entity_SetAction` do całego oryginalnego modułu `src/main/field/actor_collision.c`; usunięto wydzielony `src/main/entity/animation.c` i jego zależności CMake. Treść trzech procedur zachowano dokładnie wraz z dotychczasowymi poprawkami natywnymi. Pełny build oraz pięć testów original-game: movement, actor-animation, player-input-assets, floor i scene-spawn przechodzą; testy danych użyły rzeczywistego Disc 1 z wymaganymi assets (bez SKIP). To przywrócenie struktury oryginału, nie dowód pełnej integracji gry. `scene/actor_sound.c` nadal wymaga scalenia do `asset/game_state.c`, którego kompilację blokują między innymi nagłówki struktur fizycznego CD. Katalog `pc/` nadal pozostaje do usunięcia.

Przywrócono `Akao_LoadVoiceBank` bez zmiany treści do `src/main/asset/game_state.c` i usunięto wydzielony `scene/actor_sound.c`. Cały oryginalny moduł jest kompilowany w targetcie actor-animation; linker odrzuca niewywoływane sekcje, co nie dowodzi działania pozostałych funkcji. Historyczne sprawdzenia offsetów struktur roboczych CD/callbacków i fontów zawierających wskaźniki ograniczono do 32-bitowego ABI; sprawdzenia formatów danych pozostają. DrawFrameBuffer sprawdza relacje natywnych pól zamiast rozmiaru PS1; deklaracja strncmp używa size_t. Pełny build i pięć testów original-game (movement, actor-animation, player-input-assets, floor, scene-spawn) przechodzą z wymaganym rzeczywistym Disc 1, bez SKIP. Nadal wymagają portowania: montowanie obrazów zamiast fizycznego CD, reset stanu używający 49 słów od offsetu PS1 oraz układ buforów Boot_InitMemoryLayout. Samo udostępnienie nagłówków na 64 bitach nie portuje dawnego sterownika napędu.

`Asset_ResetGameState` w oryginalnym `asset/game_state.c` zeruje teraz cały natywny RenderObjectEntity i jego ośmiobajtową końcówkę według nazwanych pól, zamiast 49 słów pod offsetem PS1 0x14. Istniejący test actor-animation zatruwa stan przed resetem i sprawdza pełne wyzerowanie obiektu, wartości startowe audio i faz loadera oraz zachowanie LBA, wskaźników buforów i końcowego wpisu tabeli animacji. Test uruchomiony z poprzednią implementacją wykazał regresję (kod 53); po wymuszonym przebudowaniu poprawionej jednostki przechodzi. Pełny build i pięć testów original-game: movement, actor-animation, player-input-assets, floor, scene-spawn przechodzą z wymaganymi danymi Disc 1. Nie jest to jeszcze test całego Boot_InitGameState: oryginalny układ buforów i dalsza integracja startu pozostają do portowania.

Pakiety fade w oryginalnym Pe1GameState używają natywnych tagów PsyZ i natywnej szerokości komendy DR_TPAGE. Usunięto błędne wymaganie 4-bajtowego tagu PS1 dla tych nieserializowanych struktur roboczych. Istniejący test shadow porównuje rozmiary, offsety i szerokość komendy bezpośrednio z TILE/DR_TPAGE nagłówków PsyZ; test actor-animation sprawdza oryginalną inicjalizację długości, koloru, geometrii i komendy. Pełny build i siedem testów original-game (movement, actor-animation, player-input-assets, floor, shadow, scene-spawn, stat-assets) przechodzą z wymaganymi danymi Disc 1, bez SKIP. Testy nie uruchamiają jeszcze całego Boot_FinishFrame i nie dowodzą poprawności końcowego obrazu fade; pozostaje to wymaganiem integracji oryginalnej pętli klatki.

Oryginalny Boot_InitMemoryLayout rezerwuje teraz dwie tablice po 4096 natywnych RenderNativeTag oraz pełne 16 * ENTITY_ALLOCATION_BLOCK_SIZE dla alokatora aktorów. Tę samą korektę rozmiaru tablic zastosowano w istniejącym Draw_InitDisplay (pm/disc_tables.c); ten drugi moduł nie jest jeszcze w natywnym targetcie i jego wykonanie pozostaje niezweryfikowane. Istniejący test actor-animation wywołuje rzeczywisty Boot_InitMemoryLayout z pamięcią fixture, zapisuje graniczne wpisy obu tablic, zapełnia wszystkie 16 bloków oryginalnym Entity_AllocBlock, sprawdza granicę następnego bufora i odmowę siedemnastej alokacji oraz zachowanie wpisów OT. Pełny build i siedem testów original-game (movement, actor-animation, player-input-assets, floor, shadow, scene-spawn, stat-assets) przechodzą z wymaganym Disc 1. Test nie dowodzi kompletności produkcyjnego backing storage, aliasów pól globalnych, pozostałych obszarów pamięci ani działania całej sceny; te zależności nadal wymagają portowania.

Boot_InitMemoryLayout i istniejący Draw_InitDisplay zapisują dziewięć wskaźników buforów bezpośrednio do kanonicznych pól g_GameState, zamiast do historycznych osobnych symboli adresowych. Usunięto te deklaracje z boot_services.h i zastępcze zmienne z fixture. Test actor-animation wykonuje teraz cały oryginalny Boot_InitGameState (layout, następnie reset), sprawdza zachowanie rzeczywistych adresów obszarów i wyliczenie trzech wskaźników banku audio, a następnie nadal zapełnia oryginalnym alokatorem 16 bloków i sprawdza tablice OT. Pełny build i siedem testów original-game (movement, actor-animation, player-input-assets, floor, shadow, scene-spawn, stat-assets) przechodzą z wymaganymi danymi Disc 1. Jest to integracja tych dwóch procedur na pamięci fixture, nie produkcyjny boot pełnej gry; pozostałe aliasy buforów, backing storage i wykonanie Draw_InitDisplay nadal wymagają integracji.

Historyczne g_LoadedSceneAssetBlock i g_SceneLoadScratchBuffer są teraz adresami odpowiednich pól kanonicznego g_GameState, bez niezależnych zmiennych. Wskaźnik archiwum ma dwa widoki typowane w tej samej unii (SceneAssetView i SceneAssetBlob), a scratch ma typ bajtowy potrzebny loaderowi fontów. Usunięto zastępcze globalne tablice z fixture actor-animation i scene-spawn. Test bootu sprawdza teraz także rzeczywiste pola archiwum i scratch po Boot_InitGameState. Pełny build i osiem testów original-game (akao-instrument, akao-assets, akao-menu-assets, actor-animation, player-input-assets, entity-textures, entity-texture-assets, scene-spawn) przechodzą z wymaganym Disc 1, bez SKIP. Te wyniki potwierdzają spójność pamięci dla wykonywanych ścieżek; pełne sceny, nakładki i dawny osobny widok FontGlyphLoadState nadal wymagają integracji.

Usunięto historyczny widok FontGlyphLoadState i nagłówek disc_load_state.h: loader fontów, odczyty sceny, PM i video odwołują się do kanonicznych pól pe_image_base_lba oraz scene_load_scratch. Całe oryginalne render/font.c i render/font_load.c są kompilowane w parasite-eve-original. Istniejący test actor-animation wykonuje Render_LoadFontGlyph na syntetycznej tabeli 256 rekordów: porównuje wszystkie 328 bajtów każdego rekordu, zachowanie wspólnego codeCount oraz liczbę odczytów/pollowania po odmowie odczytu, pending i błędzie wymagającym ponowienia. Przechodzi pełny build i sześć testów original-game (actor-animation, player-input-assets, entity-textures, entity-texture-assets, scene-spawn, stat-assets) z wymaganym Disc 1. Test rekordów fontów używa kontrolowanego I/O; nie stanowi dowodu rzeczywistych danych fontów, algorytmu losowania ani całego interfejsu. Pozostałe procedury font.c i zmienione niekompilowane ścieżki PM/video nadal wymagają integracji i wykonania.

Oryginalny render/font.c przechowuje teraz rzeczywisty FontGlyphSelectionState, aktywną tabelę i 71-bajtową sekwencję kodów. Wartości początkowe zweryfikowano z dostarczonym EXE USA: selected=255, pozostałe pola stanu, 329 bajtów tabeli i 71 bajtów sekwencji są zerowe. Usunięto osobne symbole indeksu/wybranego slotu; wszystkie procedury korzystają bezpośrednio z pól stanu. Istniejący test actor-animation wykonuje wybór rekordu, wyszukiwanie kodów, ruch slotowy/grupowy, przejście do następnego/poprzedniego rekordu, granicę indeksu 70, konsumpcję flag i reset. Pełny build i cztery testy original-game (actor-animation, player-input-assets, scene-spawn, stat-assets) przechodzą z wymaganym Disc 1. Dane testu nawigacji są syntetyczne; oryginalne losowanie sekwencji i pełny interfejs z rzeczywistymi rekordami nadal nie są zweryfikowane.

Istniejący test actor-animation --assets wykonuje oryginalny Render_LoadFontGlyph na rzeczywistym zakresie PE.IMG wskazanym przez wpisy 79/80 tabeli z EXE USA: sektory 2205..2224, 115 rekordów. Testowy adapter I/O czyta z CUE/BIN do kanonicznego scene_load_scratch; niezależny odczyt danych porównuje wszystkie 328 bajtów każdego rekordu, 2723 sloty i 5750 zapytań Render_DrawTextDigit dla pięciu trybów i dziesięciu indeksów. Wszystkie wyniki przechodzą. Świeżo przebudowany test z ASan/UBSan/function również przechodzi cały --assets, w tym boot/reset, syntetyczne błędy I/O, nawigację i istniejące testy animacji/inputu; biblioteki PsyZ/SDL pozostają nieinstrumentowane. Pełny build przechodzi. Losowanie sekwencji i kompletny interfejs w uruchomionej scenie nadal nie są zweryfikowane.

Własny fork PsyZ implementuje BIOS-owy generator A(2Fh)/A(30h) w istniejącym libetc.c: 32-bitowy stan, mnożnik 0x41c64e6d, przyrost 0x3039, wynik 15-bitowy. Źródło algorytmu: https://psx-spx.consoledev.net/ps1/kernelbios/misc-functions/; dostarczony EXE USA pod adresami rand/srand 0x80071A54/64 zawiera wywołania tych dwóch usług BIOS. common.h przekierowuje rand/srand oryginalnego kodu do Psyz_Rand/Psyz_Srand, bez modyfikacji algorytmów gry i bez zastępowania symboli libc całego procesu. Psyz_RandState oraz seed pozwalają zapisać/odtworzyć pełny stan. Test sprawdza stały wektor, 50000 kroków dla pięciu seedów (również granice 32 bitów) i odtworzenie stanu. Na prawdziwych rekordach Disc 1 wykonuje Render_LoadFontGlyphAlt dla czterech seedów dwukrotnie, kontrolując sekwencję, stan RNG, zakresy, brak duplikatów i stałe pozycje. Pełny build i sześć testów original-game (math, movement, actor-animation, player-input-assets, scene-spawn, stat-assets) przechodzą. PsyZ commit 80f73b6. Pozostałe niewykonywane ścieżki walki/efektów korzystające z rand nadal wymagają pełnych testów integracji.

Cały oryginalny room_lib/mirror_model.c kompiluje się natywnie, bez wydzielania funkcji. Descriptor ma typ PmCommand i zgodne callbacki; kontroler używa PmSlotHeader oraz kanonicznego FieldActor/RenderObjectEntity zamiast niezależnych widoków z paddingiem PS1. Oryginalne pola właściciela odbicia i końców linii są nazwanymi widokami pamięci aktora. Zachowano obliczenia odbicia; wywołanie dekodera animacji ma jego rzeczywistą sygnaturę. Renderer używa lokalnego scratchpada, natywnych tagów i indeksów OT, odrzuca ujemną i nadmierną głębokość. Istniejący test shadow wywołuje siedem callbacków, wybór aktora z pominięciem zajętego, anulowanie, komendy linii, wiązanie, wyliczenie położenia/obrotu i stop. Sprawdza cztery klasy pakietów w obu slotach, pełne łącza wskaźnikowe, payload XY, backface oraz granice głębokości. Pełny build, 12 testów original-game (animation-assets, projection-assets, hierarchy, pools, model-setup, actor-animation, player-input-assets, actor-lifecycle, entity-textures, entity-texture-assets, shadow, scene-spawn) i świeży ASan/UBSan/function testu shadow przechodzą; PsyZ nieinstrumentowane. Nie wykonano jeszcze aktywnego MirrorTick na rzeczywistym animowanym modelu ani porównania obrazu siedmiu pokoi. Rejestracja descriptorów z adresów PS1 w Scene_LoadRoom pozostaje niezaimplementowana, room_load.c nadal nie jest w natywnym targetcie, a entity_textures.c pozostaje do scalenia.

Scalono Scene_LoadEntityTextures i Scene_UpdateBgDraw bez zmiany treści z wydzielonego scene/entity_textures.c do całego oryginalnego scene/room_load.c; wydzielony plik usunięto. Cały room_load.c jest kompilowany. SceneRoomRecord ma rzeczywisty 12-bajtowy format z u32 handlerAddress zamiast wskaźnika hosta. Scene_LoadRoom rozpoznaje descriptor lustra (slot 69, 0x8018FB44) w M017/M018/M021/M045/M102/M151/M319 i przypisuje natywny PmCommand; pozostałe descriptory jawnie zwracają -2 z diagnostyką, zanim zmienione zostaną banki sceny. Nie ma konwersji adresu MIPS na wywoływalny wskaźnik. Draw_InitPipeline ma zgodną deklarację i przerywa dalszą inicjalizację po błędzie loadera; przeszedł syntax check, lecz nie jest jeszcze wykonywany w natywnym runtime. Audyt danych scene-spawn potwierdza format rekordów oraz adres i whitelistę siedmiu pokojów. Pełny build i sześć testów original-game (model-setup, actor-lifecycle, entity-textures, entity-texture-assets, shadow, scene-spawn) przechodzą. ASan/UBSan/function scalanego loadera tekstur przechodzi 414 pokoi/296 obrazów, 14 banków/180 TIM/9591 klatek animacji; PsyZ nieinstrumentowane. Pełna Scene_LoadRoom wraz z rejestracją descriptora nie została jeszcze wykonana w teście; inne descriptory nadal są niezaimplementowane. To cofnięcie wydzielenia i częściowe wiązanie overlayu, nie działająca scena ani zakończenie portu.

Cały oryginalny cdrom/scene_assets.c kompiluje się w natywnym targetcie. CdRom_ReadSectorsFromLba/CdRom_ReadSectors/CdRom_PollReady korzystają z obrazu pliku i oryginalnych flag zajętości, bez negocjacji fizycznego napędu, kontroli pokrywy ani zakończenia procesu przy zmianie dysku. CdRom_BindImage wiąże istniejący otwarty obraz i zakres PE.IMG; offset sektorowy jest sprawdzany pod kątem przepełnienia. Usunięto pc/host_disc.c, pc/host_disc.h i powielający poll include/pe1/cd_poll.h. Czytnik ISO/CUE/BIN pc/disc.c nadal jest zależnością i pozostaje do integracji w oryginalnym kodzie/platformie. Akao_SetPos3D w tym module odczytuje flagę i tabelę bezpośrednio z g_GameState zamiast starego widoku AkaoRuntimeState. Testy odczytu obejmują rzeczywiste dane, niewyrównany bufor z wartownikami, bazę+offset, zajętość, poll, przepełnienie, brak bufora, zakres poza obrazem i odmontowanie. Pełny build i pięć testów original-game (akao-instrument, akao-assets, akao-menu-assets, akao-tick-assets, entity-texture-assets) przechodzą na Disc 1. To weryfikuje odczyt używany przez oryginalne loadery banków audio/tekstur; pełna Scene_LoadRoom, pozycjonowanie dźwięku i reszta modułu nie zostały jeszcze wykonane.


### Execute the original room loader and restore boot input rules

Scene_LoadRoom now has an execution fixture against Disc 1 for M017, M018,
M021, M045, M102, M151 and M319. It checks the original room-name codec,
file-image reads, mirror descriptor binding, native bank roots/rows and script
relocation without modifying serialized room bytes. M005 returns -2 for its
unimplemented descriptor before altering bank pointers. The fixture presets
cached audio IDs: cold bank loading and a playable scene are not established.
The original resident base32 alphabet was restored from USA EXE data.
The real-data texture fixture passes ASan/UBSan/function, including its existing
414-room texture corpus; PsyZ/SDL libraries are not instrumented.

Restored the full Scene_CheckFlagBits body and five input globals unchanged to
src/main/boot/init.c; deleted src/main/scene/input_rules.c. The entire original
boot module now compiles, using RECT for ClearImage and correctly typed pad
buffer arguments. Removed an unused card-internals include; FieldSfxScriptArgs
checks three native pointers, while scalar queue layout assertions remain.
Full build and five original-game tests (input-rules, actor-animation,
player-input-assets, entity-textures, entity-texture-assets) pass with required
Disc 1 assets. The input-rule fixture also passes ASan/UBSan/function.
Boot_InitSystem and the full boot sequence have not been executed by these
checks. Remaining extractions and the entire pc/ directory still require removal.


### Restore actor hierarchy to the complete original frame module

Deleted src/main/entity/hierarchy.c and restored Entity_RollbackPositionHierarchy,
Entity_CopyParentPosition and g_FieldActorListHead to src/main/entity/frame_update.c.
The complete restored text matches the former extraction exactly. The whole
frame module now belongs to parasite-eve-original; CMake no longer compiles a
separate hierarchy implementation or duplicates the frame module across targets.
Full build and six original-game tests (hierarchy, actor-animation,
actor-lifecycle, entity-texture-assets, shadow, scene-spawn) pass with required
Disc 1 assets. Hierarchy rollback, attachments and list ordering also pass
ASan/UBSan/function; PsyZ remains uninstrumented. This does not establish a
working full Entity_FrameUpdate game loop. No new files; remaining extractions
and removal of pc/ remain open.


### Restore movement, script relocation and PM passes to original modules

Deleted entity/movement.c, script/reloc_block.c and pm/frame.c. Restored all
movement/callback procedures to entity/frame_update.c, both relocation/scene
spawn procedures to script/node_pool.c and both PM frame passes to
asset/tim_textures.c, following their extraction history. Complete function
bodies compare unchanged; native word-wrap arithmetic and pointer relocation
remain intact. Removed redundant CMake object targets and their references.
The TIM scratch pointer now has an explicit SceneAssetBlob view.

Full build and eight original-game tests (hierarchy, nodes, movement,
actor-animation, actor-lifecycle, entity-textures, entity-texture-assets,
scene-spawn) pass with Disc 1 required. Fresh ASan/UBSan/function runs pass for
movement, actor-lifecycle (including the PM frame fixture) and the real texture
corpus/room-loader fixture. PsyZ/SDL are not instrumented. This establishes the
restored paths, not full scene/frame runtime integration or cold audio bank
loading. No new files; removal of pc/ and the remaining extractions is pending.


### Restore model setup, ordering and lighting to render/object.c

Deleted render/model_setup.c, render/model_draw.c and render/model_lighting.c.
Their complete bodies, native packet types/macros and lighting state now reside
in the original render/object.c. Extraction history confirms the original
location; function text was preserved, including native pointer arithmetic,
local scratch and lighting clamps. CMake compiles the whole object module.
No new files were added.

Full build and eight original-game checks pass with required Disc 1 assets:
model-setup, model-gpu, model-room-gpu, actor-lifecycle, entity-textures,
entity-texture-assets, shadow and scene-spawn. A fresh ASan/UBSan/function
model --gpu run also passes: 878 models, 301224 visible ordering packets,
2239298 visible pixels and identical results for both draw slots. PsyZ/SDL
libraries are not instrumented. This validates the model fixtures, not a
complete scene image or running gameplay. Other renderer extractions and the
entire pc/ directory remain to be removed.


### Restore bone transforms and projection to the original renderer

Deleted render/bone_transforms.c and render/vertex_projection.c. Their complete
functions and backing storage now reside in render/object.c, preserving all
existing native fixes. Only obsolete extraction comments changed. The entire
object module now compiles in parasite-eve-original, replacing the two extracted
sources without duplicate compilation. No new files.

Fresh ASan/UBSan/function checks pass for projection-assets (878 models,
236410 vertex slots) and model-setup --gpu (878 models, 301224 visible packets,
2239298 pixels, identical draw slots); PsyZ/SDL remain uninstrumented. Full
build passes. These are model-level results, not proof of complete scene output
or a playable game. Remaining extraction files and pc/ still require removal.

Ten original-game CTests also pass with required Disc 1 assets: animation,
animation-assets, projection, projection-assets, model-setup, model-gpu,
model-room-gpu, actor-animation, entity-texture-assets and shadow.


### Restore script interpreter and arithmetic to original opcode modules

Deleted script/basic_flow.c, script/queue.c and script/expression.c. Restored
all interpreter/basic-flow bodies and cursor storage to script/flow_opcodes.c,
and expression evaluation plus word-arithmetic helpers to script/value_ops.c.
Restored bodies compare unchanged. Both complete original modules now compile
in parasite-eve-original; removed the separate queue object target. Interpreter
layout assertions now check native cursor width and adjacent trigger/wait fields
rather than fixed PS1 runtime offsets.

Script_DispatchCmd in value_ops.c no longer narrows its mask operand pointer to
int before dereferencing it. The existing expression fixture covers stack
operands, the held-frame result and original mask mutation, missing input and
the high-bit special case. Full build and six CTests pass: nodes, queue,
expression, movement, actor-animation and scene-spawn, with required Disc 1
assets. Fresh ASan/UBSan/function runs pass for expression, queue and movement;
PsyZ is not instrumented. Other newly compiled opcode paths are not yet verified
by these fixtures, and this is not a complete scene script runtime. No new files;
remaining extractions and the entire pc/ directory remain to be removed.


### Restore PM script commands to system_opcodes.c

Deleted script/pm_commands.c and restored its complete wrappers and operand
record to script/system_opcodes.c, preserving their bodies exactly. The whole
system-opcode module now compiles in parasite-eve-original. Removed its stale
conflicting Render_SetScrollMode declaration in favor of the existing camera
header; the barcode encoder uses a char[8] buffer and scene flag pointers use
u32. Removed the redundant NULL macro. No new files.

Full build and six original-game tests pass: queue, expression, movement,
actor-animation, camera and actor-lifecycle (Disc 1 assets required where used).
A fresh ASan/UBSan/function actor-lifecycle run passes, including existing PM
script send/state checks; PsyZ is not instrumented. Other system-opcode paths
are compiled but not established as working by these checks. Full script runtime,
remaining extraction removal and elimination of pc/ remain incomplete.


### Restore pad sampling and player movement to original modules

Deleted scene/pad_input.c, scene/player_input.c and scene/player_input_rules.c.
The entire pad-sampling implementation and its backing globals return to
boot/init.c. Player movement handlers return to scene/player_movement.c;
the USA input tables now reside alongside those handlers in the same original
module. Full extracted bodies and table contents compare unchanged. Removed
the separate rules object target. The expression fixture now references the
real held-frame array instead of supplying duplicate storage. No new files.

Full build and six original-game tests pass with required Disc 1 assets where
applicable: akao-menu-assets, input-rules, expression, movement, actor-animation
and player-input-assets. Fresh ASan/UBSan/function actor-animation execution
passes digital/analog movement, action/clock/audio-trigger checks; PsyZ/SDL are
not instrumented. The remaining TIM helper procedures in player_movement.c
compile but are not verified by these movement checks. Full boot/game runtime,
remaining extraction removal and elimination of pc/ are still incomplete.


### Restore widget lookup and use native navigation records

Deleted menu/widget_lookup.c and restored its lookup function and active-list
head to the original menu/widget.c. The whole widget module now compiles in
place. MenuWidgetListNavigation is the canonical MenuWidgetNode, with shared
unions for the managed-list pointer, visible-row/draw-state fields and the
navigation flags stored in the disabled slot. This removes its fixed 0x34-byte
PS1 prefix and makes its update callback signature agree with the node. The
existing fixture verifies native list-pointer preservation and scrollbar
position/size computed by MenuWidget_EaseNodePosition. No new files.

Full build and four tests pass: akao-menu-assets, input-rules, actor-animation
and player-input-assets (Disc 1 required where used). Fresh ASan/UBSan/function
actor-animation execution also passes including the navigation layout check;
PsyZ/SDL remain uninstrumented. Other widget construction/drawing paths remain
unverified, with known legacy packet-pointer narrowing and old callback
prototypes still present. This is not a completed native menu renderer. Full
runtime integration and removal of pc/ remain incomplete.


### Consolidate original AKAO data in the existing sequencer module

Deleted akao/state.c and akao/opcode_dispatch.c. Native storage for the original
AKAO driver and both USA instruction dispatch tables now reside in the existing
akao/sequencer.c. Full definitions compare unchanged, preserving initializers,
array sizes and dispatch order. No new files or gameplay implementation.

Full build passes. Fresh ASan/UBSan/function runs pass for the original complete
tick/headless PCM fixture and menu-assets: commands 44c/44d/44e/44f produce
30172/23184/7834/41810 nonzero samples respectively. PsyZ is uninstrumented and
its DMA callback remains unimplemented; these tests do not establish complete
audio/runtime integration. Remaining extraction/data files and all of pc/ still
require removal.

All six original-game-akao CTests pass with required Disc 1 assets: layout,
instrument, assets, menu-assets, tick-assets and prefix-assets.


### Restore menu sound procedures to their original modules

Deleted akao/menu_sound.c. The four Menu_Play*Sound functions return to
aya/stats.c, and Akao_SendTableCommand returns to cdrom/scene_assets.c, matching
their extraction history. Complete bodies compare unchanged. Frontend audio
targets now link the complete original scene/archive object; the audio fixture
compiles the complete original stats module. No new files.

Full build and five original-game tests pass with required Disc 1 assets:
akao-instrument, akao-assets, akao-menu-assets, actor-animation and stat-assets.
A fresh ASan/UBSan/function menu-assets run produces the same nonzero PCM counts
for 44c/44d/44e/44f: 30172/23184/7834/41810. PsyZ is uninstrumented, including its
unimplemented DMA callback. This verifies those original sound paths, not all
game audio. Remaining extractions/data files, full runtime integration and
elimination of pc/ remain incomplete.


### Use original driver storage in both runtime and audio tests

Deleted akao/assets.c and placed its unchanged asset/queue/global definitions
in the existing akao/voice_driver.c. Removed the fixture's duplicate definitions;
it now supplies input buffers and tables to production storage. It checks the
production relative-message handler before installing test handlers. No new
files; the still-partial production message-handler table is unchanged.

Full build and all six original-game-akao CTests pass with Disc 1 required.
The 240-tick corpus retains 579 multi-chunk bank loads, 51102257 nonzero PCM
samples and hash 25836548e09d8a00. Fresh ASan/UBSan/function instrument and
menu-assets runs pass; the four menu sample counts remain unchanged. PsyZ is
uninstrumented. This does not establish all audio commands or integration with
the full game. Remaining asset-lookup/texture extractions and the pc/ directory
still require removal.


### Restore asset lookup and TIM upload to the original archive module

Deleted cdrom/asset_lookup.c and render/texture_upload.c. All keyed lookup
procedures and Draw_LoadTimAsset return to cdrom/scene_assets.c. Complete bodies
are preserved except pixel-pointer casts now match the original header's void*
LoadImage argument. Removed the obsolete model-setup object target; its consumers
use the complete original archive module.

The scripted retry/poll fixture compiles that same whole module with only its
real read/poll symbols renamed at compile time, keeping them distinct from the
fixture doubles. Production code has no conditional test path or new I/O layer.
The real-data fixture uses the ordinary archive object and file-image reads.

Full build passes. Fresh ASan/UBSan/function real-data execution passes the
seven cached-audio mirror room loads, unsupported M005 rejection, full VRAM
comparisons across 414 rooms, 14 area/day banks and 9591 attachment frames.
PsyZ/SDL remain uninstrumented. These are not complete playable scene tests.
No added source files remain under src/main relative to commit eadcfc73f;
this structural comparison does not establish functional completeness. The
pc/ directory and its dependencies still require removal.

Ten CTests pass with required Disc 1 assets: AKAO instrument/assets/menu-assets,
model setup/GPU/room-GPU, actor lifecycle, synthetic/real entity textures and
scene spawn.


### Remove the separate PC keyboard adapter

Deleted pc/host_pad.c and pc/host_pad.h. Their platform packet producer now
belongs to the existing boot/init.c beside the original controller decoder,
exposed as Boot_ResetPadInput/Boot_HandlePadEvent in the existing pad header.
Keyboard mappings, multiple-key held state, focus-loss clearing, disconnected
second port and centered analog axes are preserved. Frontend and existing
fixture call these endpoints directly. SDL include paths apply only to the boot
source, without changing definitions on the other original modules. No new files.

Full build and six tests pass with required Disc 1 assets where applicable:
original input-rules, actor-animation, player-input-assets and the legacy
boot-menu-audio, boot-replay, boot-newgame checks. Fresh ASan/UBSan/function
actor-animation passes digital/analog movement and SDL event/focus transitions.
PsyZ/SDL are uninstrumented. Legacy boot checks do not prove original scene
runtime or gameplay. Other pc/ code, including the parallel scene/audio engine,
still requires removal and original runtime integration.


### Give the original widget pool native backing storage

menu/widget.c now owns its 24 MenuWidgetNode records. The historical sentinel
name aliases element 23, and both original initializers use the canonical pool
base. MenuWidget_AllocNode retains its free/active-list and child-slot order,
with ordinary indexed child clearing instead of constructing pointers before
the children array. Its definition is externally callable under C11. No new
files or replacement allocator.

The existing actor-animation fixture executes all 24 original allocations,
checks native record addresses, active-list order, cleared child slots and the
four owner links, then checks reinitialization and the terminal null link.
Full build, three input/actor CTests (Disc 1 required where applicable) and fresh
ASan/UBSan/function actor-animation pass; PsyZ/SDL remain uninstrumented.
Recursive release, saved layout backing, allocation failure behavior and full
menu rendering still require work. Full game integration and pc/ removal remain
incomplete.


### Execute original recursive widget release and saved layouts

menu/widget.c now owns the original 72 four-byte saved cursor/scroll records,
matching the existing 0x120 reset and 0x48 save bound. Both reset paths use
memset with the actual array size instead of a historical bzero declaration.
The original recursive release algorithm remains unchanged. No new files.

The existing fixture resets all 72 records and checks the eight unsaved-column
sentinels, deletes a root with four attached children while 19 other nodes
remain, checks current-node fallback and parent/reciprocal-link cleanup, verifies
saved cursor/scroll values, reuses the five released nodes in original order,
and frees everything with an exact 24-record uniqueness check. Full build and
the actor-animation fixture pass. Failure/exhaustion handling and full menu
rendering remain unverified; full runtime integration and pc/ removal remain open.

The expanded fixture also passes fresh ASan/UBSan/function checks; PsyZ/SDL
are not instrumented.


### Back the original menu cursor stack and execute tree drawing

The historical USA symbol map before eadcfc73f places cursor-stack bottom/top
at 0x800A2270/0x800A22B0: eight coordinate pairs. Existing draw code also uses
0x30 bytes beginning at the top address for vertex scratch. menu/widget.c now
owns one contiguous 28-int array preserving both regions; the top name aliases
element 16. This replaces linker-only storage without altering Draw_PushPrimToList.
No new files.

The existing fixture executes the original traversal on a three-level tree with
four draw callbacks. Callbacks deliberately change both cursor coordinates;
checks confirm preorder, parent-relative positions, final cursor/stack restoration
and untouched adjacent vertex scratch. Full build and fresh ordinary plus
ASan/UBSan/function actor-animation runs pass. PsyZ/SDL remain uninstrumented.
This tests traversal, not GPU menu packets, text or complete menu visuals. Full
renderer integration, game runtime and pc/ removal remain incomplete.


### Execute the original menu event queue and callback dispatch

The complete menu/input_queue.c now compiles in place. It owns its original
20-entry native event pool, references existing shared draw/input globals and
exports enqueue/dequeue normally under C11. Field-pad bit remapping uses unsigned
word shifts. Dispatch handles a missing current widget and skips absent update
callbacks while preserving child-to-parent propagation. No new files.

The existing fixture fills all 20 entries, verifies full-queue drop behavior,
masked removal/reuse and FIFO order, then sends press/repeat/release events
through child and parent callbacks and checks exact flag translation. Empty
current-widget dispatch executes without dereferencing NULL. Full build, four
original/legacy input and boot tests, and fresh ASan/UBSan/function fixture pass;
PsyZ/SDL remain uninstrumented. Draw/present routines in the same module are
compiled but unverified and retain legacy pointer assumptions. No claim of full
menu visuals or original game runtime; pc/ removal remains pending.


### Verify keyboard-to-original-menu dispatch end to end

Expanded the existing actor-animation fixture to feed SDL events through
Boot_HandlePadEvent, Field_HandleStateTransition and
MenuInput_DispatchQueuedEvents into a real widget update callback. It verifies
idle activation, immediate Up (0x1000), no early repeats, the first repeat on
the ninth held update (0x21000), ignored direction release, X press (0x20) and
release (0x10000), V/high-bit input (0x80), and clearing held state on focus loss.
The original pad decoder deliberately swaps raw 0x2000/0x4000 before field
mapping, so Z produces menu 0x40 and X produces 0x20; no gameplay mapping was
changed to accommodate the test. No new files or replacement runtime logic.

The rebuilt ordinary fixture passes. This establishes the input path through
an update callback, not complete rendered menus or gameplay. Full runtime and
removal of pc/ remain incomplete.

Fresh ASan/UBSan/function execution also passes; PsyZ/SDL libraries are not
instrumented.


### Execute cold room audio loading through original archive and SPU code

The real-data entity-texture fixture now links the complete original AKAO
modules instead of aborting at CdRom_ReadAssetSectors. It binds the real audio
sector directory and initializes the PsyZ SPU backend without installing the
unintegrated original timer bootstrap. For M017, M018, M021, M045, M102, M151
and M319 it begins with empty stream/sample IDs and executes Scene_LoadRoom,
including kind-2 stream-bank and kind-1 sample-bank uploads.

An independent raw-bank decoder checks payload extents, every uploaded instrument
record (including stream address relocation), final cache IDs, completed read
phase and the entire 512 KiB SPU RAM. Each supported room is loaded again with
cached IDs and sentinel-filled SPU memory to prove the cache-hit path performs
no upload. M005 still rejects its unimplemented native descriptor before audio
loading. All room bank payloads in this fixture fit the original 33-sector
scratch capacity; it does not establish larger-bank kind-1/2 chunk behavior.

Full build and four original-game tests pass with required Disc 1 assets:
entity-textures, entity-texture-assets, akao-assets and akao-menu-assets. The
existing 414-room VRAM and 14-area animation/texture corpus remains included.
No new files or replacement game code. This establishes integrated loading,
not starting scene audio, a running scene frame or playable gameplay. Full
runtime and removal of pc/ remain incomplete.

Fresh ASan/UBSan/function execution of the expanded real-data fixture passes.
PsyZ/SDL are uninstrumented, and the backend still reports an unimplemented
DMA callback.


### Compile the original scene pipeline with canonical lighting storage

The complete boot/frame_loop.c and draw/pipeline.c now compile in the original
native target. Removed their duplicate frame-counter/dispatch-token definitions.
A single RenderLightingState backing record in frame_loop.c owns the direction
matrix, color matrix and three per-light colors. Existing matrix/interior names
are field aliases, replacing the fixtures' independent matrices and unresolved
interior symbols. Pipeline color setup passes the canonical record with the
actual original function signatures. No new files or replacement pipeline.

The model fixture calls the original Boot_SetBackColor and Boot_SetLightColor:
it checks all nine direction cells, untouched neighboring storage after back
color setup, all three RGB matrix columns and per-light bytes, plus interior
alias identity. Full build and five tests pass with required Disc 1 assets:
model-setup, model-gpu, actor-lifecycle, entity-texture-assets and scene-spawn.
Fresh ASan/UBSan/function --gpu execution passes the lighting checks and 878-model
corpus (301224 packets, 2239298 visible pixels, identical draw slots). PsyZ/SDL
are not instrumented. Full Draw_InitPipeline/Boot_FinishFrame execution remains
unverified; runtime integration and pc/ removal remain incomplete.


### Submit original double-buffered frames through native ordering tables

The complete draw/frame.c and pm/disc_tables.c now compile in the original
target. Draw_RenderFrame uses canonical g_GameState.shared_buffers rather than
the removed fixed-offset RenderFrameState view. Its final OT entry is native
entry 4095 instead of byte offset 0x3ffc. The frame module owns the active slot
and both display/draw environments; fixtures no longer supply duplicate slots.
The draw-status queries use normal C11 definitions and native storage. Removed
three pointer-to-int conversions at the existing sector-read calls in the disc
module; other initialization paths remain unverified. No new files.

The GPU fixture sends a tile at OT entry 2048 through the original frame
function for each draw slot, verifies slot switching and exact red/green pixels
in separate VRAM regions. Full build and five tests pass with required Disc 1:
model-gpu, model-room-gpu, entity-texture-assets, shadow and scene-spawn. Fresh
ASan/UBSan/function --gpu execution passes, including the 878-model corpus;
PsyZ/SDL are not instrumented.

Known presentation fidelity limits: PsyZ reports VSync(n>1) as not fully
implemented. An RGB=248 solid tile produced 15-bit channel value 30 in this
backend; the buffer-routing test uses RGB=255 endpoints and does not establish
intermediate color conversion fidelity. This discrepancy still needs investigation
in our PsyZ fork. Movie/save-background branch presentation and full frame-loop
execution are unverified. Complete game integration and pc/ removal remain open.


### Correct non-dithered RGB5 conversion in our PsyZ fork

Fork commit 31befd2 (pushed to khasinski/parasite-eve-psyz/main) quantizes
non-dithered fragment colors during rasterization. Previously an untextured
8-bit color remained normalized in the render texture, and readback rounded
its scaled value to five bits (RGB=248 became 30 instead of 31). The shader now
truncates each channel to five bits before storage. The GL source and generated
Metal/SPIR-V/DXIL fragment variants are updated; no new repository files.

The original-frame fixture now renders ramps covering all 256 inputs of each
RGB channel into both draw buffers and compares VRAM with independent integer
right-shift results. It failed against the prior fork and passes with this
change. Metal model-gpu, model-room-gpu and entity-texture-assets pass with Disc 1
required. Fresh ASan/UBSan/function --gpu execution passes the ramps and 878-model
corpus (301224 packets, 2239015 visible pixels, identical slots). The pixel count
changes with corrected quantization. PsyZ/SDL are not instrumented.

GLSL 330 core and GLES 300 syntax checks pass. SPIR-V and Metal variants were
compiled with glslang/spirv-cross; DXIL was compiled using Microsoft's release
DXC in a Linux container. Their runtime evidence is limited to Metal: the macOS
build explicitly rejects sdl3_gl, and Vulkan/Direct3D were not run. The normal
Metal build configuration was restored. Dithering/blending fidelity, mask-bit
behavior, VSync(n>1), full scene runtime and removal of pc/ remain incomplete.


### Synchronize original frame intervals in our PsyZ fork

Fork commit f14e6c0 changes VSync(n>1) from one present plus n synthetic kernel
notifications to counter-based waiting relative to the last blocking call,
following the SDK implementation in decomp/src/libetc/vsync.c. Each observed
blank advances pad sampling, the callback and kernel service; an already elapsed
interval still waits for the next blank. Queries preserve the interval. SDL
limitless mode advances a logical counter per explicit present, and pacing-mode
switches keep it continuous. Mode-1 documentation now correctly identifies the
horizontal-line query. No new files.

The existing GPU fixture checks nonblocking queries, exact 2/4-blank increments
and callback counts, late synchronization after three independent presents,
and continuity when returning to normal pacing. Original frame rendering and
all-channel RGB5 checks remain included. Full build and four tests pass with
Disc 1 required: model-gpu, entity-texture-assets, boot-replay and boot-newgame.
The two legacy boot tests do not establish original scene gameplay.

Fresh ASan/UBSan/function execution passes with libapi.c and sdl3_gpu.c (including
sdl3_common.h) instrumented alongside the original game test sources. Remaining
Psyz/SDL objects are not instrumented. The 878-model corpus still reports
301224 packets and 2239015 visible pixels with identical draw slots. Hardware
scanline precision, asynchronous blanks during unsynchronized CPU work and
other backends remain unverified. Full game runtime and removal of pc/ remain
incomplete.


### Bind loaded room geometry to the canonical camera state

The historical USA map places g_GeomState at 0x800B1624 and g_GameState at
0x800B0CD8: exactly the existing taskAux/bank_reset_94c slot. Pe1GameState now
provides a typed geometry_state view of that slot, and g_GeomState aliases it.
Scene_LoadRoom writes the typed field; removed three independent fixture globals.
The real loader now supplies the same geometry pointer the camera reads.

The full render/frame_update.c compiles in the original target and owns the
nine-word camera workspace. The historical projection-distance symbol is an
alias of word 8 (USA 0x800B8A18 vs matrix 0x800B89F8). Removed the fixtures' separate
arrays, including an undersized eight-word buffer. No new files.

The real-room fixture checks the taskAux payload address and loads every camera
view in the seven supported mirror rooms into the actual workspace. Distances,
nine rotation cells and three translations match independent serialized reads;
room bytes remain unchanged. Full build, five original tests (camera,
entity-textures, entity-texture-assets, shadow, scene-spawn) and fresh
ASan/UBSan/function real-data execution pass with required Disc 1 assets. PsyZ/SDL
are not instrumented. Render_PrepareFrame as a whole is compiled but not executed
by these checks: native mesh-entry relocation and full scene/frame runtime are
still incomplete. Removal of pc/ remains pending.


### Native geometry in the original room loader and removal of pc/disc

Scene_LoadRoom now expands the 56-byte serialized mesh records into native
GeomEntry records in owned memory, preserving the original raw room bytes,
view/control arrays and relative position/texture references. GeomScrollEntry
uses the same native stride as GeomEntry. The complete original
Render_PrepareFrame executes with canonical game state and focus storage; no
new source files or replacement frame preparation were introduced.

The real-data fixture verifies 190 entries in M017, M018, M021, M045, M102, M151
and M319. It compares all sprite/tpage packets in both buffers against the
serialized positions, UV, CLUT, page and transparency values, with a guard
after the 597184 total packet bytes. An invalid geometry offset in a temporary
image is rejected with -3 before bank/geometry state replacement; reloading
the original image succeeds. Rendering those packets to reference background
images, production packet-arena capacity/lifetime and other room descriptors
remain unverified.

Deleted pc/disc.c and pc/disc.h. Image I/O is integrated into the existing
cdrom/scene_assets.c and its original cdrom.h header, used by the original
sector reader and by native image consumers. The build links the whole original
archive module instead of compiling a separate platform reader. Existing
fixture read/poll and sound-request doubles remain isolated through renamed
symbols in the existing test object. No parallel gameplay engine was moved.

Full build and ten CTests pass with Disc 1 assets required: disc-reader, original
actor-animation, camera, entity-textures, entity-texture-assets, shadow,
scene-spawn, boot-menu-audio, boot-replay and boot-newgame. Fresh
ASan/UBSan/function runs pass for the disc fixture (ISO, raw BIN, CUE, extent
boundaries and malformed input) and the real entity-texture fixture, including
the original archive and room modules. PsyZ/SDL archives were not instrumented
in these runs. The legacy boot tests do not prove original gameplay.

The rest of pc/, including scene/script/audio/frontend implementations and
their test dependencies, still requires elimination. Full original scene/frame
integration and the complete game remain unfinished.


### Remove the redundant asset-source wrapper

Deleted pc/assets.c and pc/assets.h without relocating their API or adding
files. The existing frontend opens both PeDisc images directly through the
original archive module, closes both on startup failure and keeps both open
through boot. Consumers retain separate image and file-extent pairs.

The original image reader now rejects reads after close before calling stdio
with a null stream. The existing disc fixture alternates payload and raw-sector
reads from two simultaneously mounted synthetic images with identical file
names but different bytes; closing source 0 leaves source 1 readable. Closed
payload/raw/directory reads and out-of-extent reads fail. Full build, five
CTests (disc-reader, original entity-texture assets and three boot checks) pass
with Disc 1 assets required. After the final fixture additions, disc-reader
and a fresh ASan/UBSan/function build of the original image module and fixture
pass again. A headless executable invocation with a valid Disc 1 and missing
Disc 2 exits with code 1 before boot.

This does not prove real Disc 2 contents or automatic selection by gameplay: the
frontend still boots from the first image. Original Asset_OpenPeImage, source
selection in original loaders and the rest of pc/ remain to be integrated or
removed. Full original gameplay remains incomplete.


### Connect original archive discovery to native startup

Asset_OpenPeImage remains in the complete original asset/game_state.c. Its
physical-drive status, queue and seek loops now use ISO9660 lookups in the
borrowed image already owned by startup. The original marker paths and PE.IMG
path are resident data in that same module. A discovered archive is bound to
the original sector reader. No new files or extracted implementation. Native
boot now calls this original procedure and unmounts the borrowed binding before
its caller closes the stream. The whole original asset/player module object is
linked into the executable and boot fixtures.

Fixed canonical aliases: g_PeImageBaseLba is g_GameState.pe_image_base_lba and
g_PeImageMountFlags is g_GameState.display_list_modes. The latter corresponds
to original address 800B0DCD minus game-state base 800B0CD8 = F5, and the base
LBA address 800B0DD8 is offset 100. Removed the separate texture-fixture base
variable and stale header declarations. This prevents archive discovery and
original loaders from updating different storage.

The real asset fixture now calls original Asset_OpenPeImage instead of manually
binding its catalog extent. It checks successful Disc 1 marker/extent discovery,
exact alias addresses, -1 for absent/closed images and -2 for an empty directory.
Original room, common/entity texture, audio-bank and geometry checks then use
that discovered binding. Full build and eight CTests pass with Disc 1 required:
akao-assets, actor-animation, entity-textures, entity-texture-assets, disc-reader
and the three boot checks. Fresh ASan/UBSan/function validation of the real
entity-texture fixture passes with original discovery/archive/room code
instrumented; PsyZ/SDL archives are uninstrumented.

Actual Disc 2 contents, selection of its original offset tables, replacement of
remaining disc-notice logic and full scene/game integration remain incomplete.
The remaining pc/ runtime still requires elimination.


### Repair the original base entity-bank loader in place

Scene_LoadEntityTexture in the complete scene/room_load.c no longer computes
a bank-row destination by modifying the low 32 bits of a Pe1GameState pointer
and adding id*4. The original flattened byte id now indexes native bank_rows
by group/column, preserving its meaning with eight-byte pointers. Removed the
unused matching-only stack reserve and stale matching comment. No new files,
extracted routines or replacement loader.

The real Disc 1 fixture executes the original read/poll/upload/bank-binding
phases for all five base area banks (10..14), compares the loaded bytes and
83 model/action row references to independent serialized records, and compares
all VRAM words after 15 real TIM uploads. The existing synthetic fixture checks
ids 0, 47, 48 and 255 across native group boundaries, root binding and the
original Aya action-step gate before texture upload. Aya action step is a
controlled fixture input; original battle integration is not established.

Full build and four CTests pass: original entity-textures, entity-texture-assets,
scene-spawn and actor-animation, with Disc 1 required. Fresh
ASan/UBSan/function runs pass for both synthetic and real entity fixtures with
original loader/archive sources instrumented. PsyZ/SDL archives remain
uninstrumented. Full Scene_InitEntityPlayer, gameplay runtime and elimination
of the remaining pc/ code are still incomplete.


### Execute full original scene-player initialization with native storage

Scene_InitEntityPlayer remains in the complete original room_load.c. Its model
packet buffer now starts after two native RenderShadowQuad records, matching
the existing original actor loader, instead of after the PS1-only 0x50 bytes.
The original action selection, texture coordinates, lighting, animation and
state transitions are retained. No new files or replacement initializer.

Canonical game-state aliases now cover the original flags word, story flags,
pending/current day, loaded area and background cache byte. Removed the separate
current-day fixture storage and stale scalar declarations. This lets original
scene initialization trigger base-bank loading and clear its loading flags in
the same state read by the other original modules.

The real fixture executes all normal initialization phases 32..39 for areas 10
and 12, days 1..7 (14 combinations), using original sector reads, base/day
loaders, Entity_SetActionMode, Render_SetupEntityPrims, lighting and
Render_DrawWithAnim. Completion takes 11 or 12 calls. It checks model/action
references, canonical area/day/loading flags, packet-buffer start and guards
around both native shadow records and the full two-buffer packet/matrix arena.
It subsequently runs the existing scene attachment and 9591-frame corpus.
Battle step remains a controlled fixture input, not a replacement gameplay
implementation.

Full build and eight CTests pass with Disc 1 required: movement, actor-animation,
entity-textures, entity-texture-assets, scene-spawn and three legacy boot checks.
Fresh ASan/UBSan/function real-data validation passes with the original
initializer, loaders, action and renderer code instrumented; PsyZ/SDL archives
are uninstrumented. Resume-mode initialization, full scene scheduling, gameplay
and removal of the remaining pc/ runtime remain incomplete.


### Verify original loading for every currently bindable Disc 1 room

A direct audit of the serialized room directory shows that New Game room M431
has no room-specific PM/effect descriptor entries. It therefore already uses
the original native loader without an additional overlay binding; the missing
descriptor restriction does not apply to that startup room. M001 and M002
also have no such entries. No placeholder handler or new implementation was
added to make these rooms pass.

Expanded the existing real-data fixture from seven mirror rooms to all 229
Disc 1 rooms without descriptor entries plus the seven bound mirror rooms.
It executes complete original Scene_LoadRoom cold and cached loading, validates
script/bank relocation and immutable serialized room data, compares the full
SPU RAM and loaded instrument records, runs original Render_PrepareFrame and
compares all native sprite/tpage payloads in both buffers with guards. It also
checks every camera viewport using the production GTE scratch array.

The verified corpus is 236 rooms, 2763 native geometry entries and 23350656
packet bytes; M431 is required explicitly and has one geometry entry with
33600 packet bytes. M005 remains an explicit unsupported-descriptor failure.
The other 178 of 414 nonempty Disc 1 rooms still require static native PM/effect
bindings; this corpus does not count them as loaded or replace their behavior.

Full build and three CTests pass with Disc 1 required: entity-textures,
entity-texture-assets and scene-spawn. Fresh ASan/UBSan/function validation
passes for the expanded real-data corpus with original loader/render/action
sources instrumented; PsyZ/SDL archives remain uninstrumented. Packet preparation
and loading are not proof of rendered background fidelity, executed room
scripts or playable New Game. Full original pipeline/frame integration and
elimination of the remaining pc/ runtime remain open.


### Return original room I/O errors instead of drive retry loops

Scene_LoadRoom now validates the USA room index and nonempty range before
addressing its 450-entry table, checks the complete LBA range in 64-bit
arithmetic, and returns -1 on failed scratch/texture/room reads or polls. The
original range order and interleaved TIM uploads remain unchanged. Physical
drive retry jumps and busy retry loops were removed from these three reads;
native image failures can be observed and recovered by the caller. No new files
or replacement loader.

Draw_InitPipeline now returns the original room-load status, and Boot_RunFrame
returns before its gameplay loop when pipeline initialization fails. Both whole
original modules compile with the corrected prototype. Full pipeline execution
and its other loading helpers are not covered by this fixture; their remaining
retry/error paths still require work.

The real fixture restricts the borrowed archive extent at each of the three
M017 read boundaries, checks immediate -1 and retained geometry/bank pointers,
then successfully reloads the complete archive after every failure. It also
rejects room IDs 0, 451 and the empty final slot 450, an overflowing base LBA
and an unmounted image. This is not a general rollback guarantee for partially
read buffers or uploaded VRAM.

Full build and three CTests pass with Disc 1 required: entity-textures,
entity-texture-assets and scene-spawn. Fresh ASan/UBSan/function validation
passes for these errors and the complete 236-room corpus; PsyZ/SDL archives
remain uninstrumented. The original complete game loop, remaining helper error
propagation and removal of pc/ remain incomplete.


### Propagate original entity texture/model loading failures

The complete original Scene_LoadEntityTexture and Scene_LoadEntityTextures
now return -1 for failed texture/model reads and completion polls instead of
reporting those failures as pending work. Read failures retain their request
phase; poll failures restore the matching request phase. Genuine pending polls
still return 1. Successful original loading/upload/action behavior is retained.
Scene_InitEntityPlayer propagates failures without advancing its phase, and
Draw_InitPipeline checks both entity loaders before continuing scene setup.
All changes remain in existing original modules; no new files or abstraction.

The existing synthetic fixture checks read failure, pending poll, failed poll
and the retained retry phase for both base and day texture/model requests, plus
error propagation at player-init phases 36 and 38. Real-data checks restrict
the borrowed archive extent for both reads of all five base banks, daily TIM
reads for days 2..6 in areas 10/12, and all 14 day-model banks. Each request
fails before its next phase; restoring the complete archive permits normal
loading and existing VRAM/model/animation verification.

Full build and three CTests pass with Disc 1 required: entity-textures,
entity-texture-assets and scene-spawn. Fresh ASan/UBSan/function runs pass for
both synthetic and real fixtures, including the 236-room and 14 player-init
corpora. Original loaders and player initialization are instrumented; PsyZ/SDL
archives are not. Full pipeline execution, audio-streamer error paths, battle
callers of player initialization and the complete original game remain
unverified/incomplete. Removal of the remaining pc/ runtime remains required.


### Compile the whole original room actor-class library in place

The complete 53-function actor_classes.c is now part of parasite-eve-original;
no functions were extracted and no files added. Removed its physical scratchpad
addresses, pointer-to-int scratch arithmetic and fixed-offset accesses to the
active BattleEntity. Matrix/coordinate accesses now use named native fields.
Configuration arguments carrying pointers use intptr_t. Native callback types
carry RoomEnt context, and original A/B/C update methods explicitly pass their
object to the callback. Owner-busy checks also receive their explicit context.

Pointer-bearing runtime geometry/context/emitter layout assertions now account
for native pointer width; scalar serialized-record checks remain unchanged.
The whole actor-class source passes C11 syntax checking without diagnostics and
the full build passes. These changes do not establish native compatibility of
the remaining RoomLink/RoomRenderNode views, private-state allocation sizes or
field-engine task-owner overlays. No additional room handler was registered.

The existing actor-animation fixture executes the three original A/B/C update
methods and verifies callback identity/context and mutation of the same object.
Three CTests pass with Disc 1 required: actor-animation, entity-texture-assets
and scene-spawn; actor-animation passes again after final type cleanup. Fresh
ASan/UBSan/function validation of the actor fixture passes with original source
instrumented; PsyZ/SDL archives are not. Motion/state-machine/GTE branches in
this newly compiled module were not exercised by that callback fixture.

Next integration must reconcile those actor/owner views and allocation sizes
before binding the class tables to real room descriptors. M005 and the other
unsupported PM/effect rooms, the complete original game runtime and removal
of the remaining pc/ code remain unfinished.


### Reconcile original room actor views with canonical native actors

Repaired the existing RoomLink, RoomRenderNode and FieldActorNode views in
room_lib.h. RoomLink now includes all canonical pointer-bearing actor members
through its native RenderObjectEntity prefix, rather than treating list/script/
callback pointers as fixed PS1 padding. The existing model translation, matrix
array and render-flag names alias their actual render-object members in the
same storage. RoomRenderNode's action-state pointer and start byte follow the
canonical FieldActorState layout; the actor-list node uses native state, next
and previous pointers. No new files or runtime abstraction.

Compile-time checks compare every exposed actor field, native prefix size and
nested model/state aliases with FieldActor/FieldActorState. The existing fixture
reads and writes positions, animation halves, motion, parent/list pointers,
model matrices/flags and actor-state fields through the repaired views. It
executes original B/C release methods with a real canonical actor/state, checks
the action-state byte and completion signal, and verifies the rest of the actor
record remains unchanged. The original A/B/C callback-context checks remain.

Full build and three CTests pass with Disc 1 required: actor-animation,
entity-texture-assets and scene-spawn. Fresh ASan/UBSan/function actor-fixture
validation passes with original class code instrumented; PsyZ/SDL archives
are not. This does not prove motion/configuration branches or the field-engine
task-owner/private-state allocation layout. Those must still be reconciled
before static room class registration. No additional PM/effect room bindings
were enabled; complete gameplay and removal of pc/ remain incomplete.


### Execute original owner and callback-list helpers with native task storage

The complete original fieldfx/vector.c and fieldfx/task_list.c now compile in
parasite-eve-original. Linked animation looping reads canonical FieldActor
fields, with the existing repeat/restart algorithm retained; its controller
prefix accounts for both native callback/signal pointers. Compile-time checks
prove RoomEnt.link and t19 alias the owner actor and busy flags consumed by
FieldFx_IsOwnerBusy. No new files or extracted routines.

Runtime task/program/slot assertions now account for native pointer width;
the script-address union uses intptr_t. Callback-list entries use C11 flexible
arrays with aligned payloads. Original list construction/allocation/acquisition
now uses native header sizes and aligned strides. Constructor failures reject
negative sizes/counts, integer size overflow and misaligned output before
writing the list. Allocation uses a typed callback pointer. The original list
walkers retain update/draw order, completion counts, age restoration and
first-free reuse; their globals have canonical storage in the same module.

The existing fixture exercises owner busy/one-shot flags and all child wait
phases, finite/infinite/disabled animation repeats and the pre-terminal frame.
It executes real callback-list allocation, three pointer-bearing payloads,
update/draw callbacks, completion and reuse, verifies alignment/ages/arguments,
checks the arena guard and malformed constructor requests. No replacement
field-effect gameplay was introduced.

Full build and three CTests pass with Disc 1 required: actor-animation,
entity-texture-assets and scene-spawn. Fresh ASan/UBSan/function actor-fixture
validation passes with original owner/vector/list/class code instrumented;
PsyZ/SDL archives are not. Full generic task spawning, arena wrapping/capacity,
program-table relocation, script execution and remaining effect callback
signatures still require integration and verification before enabling room
handlers. The complete game and elimination of pc/ remain unfinished.


### Compile original effect geometry and execute the real task cycle

The entire original fieldfx/geometry.c now compiles in parasite-eve-original;
no functions were extracted or files added. Replaced incompatible legacy libc
declarations with native standard declarations and removed pointer truncation
in the palette lookup. The task spawner retains the eight-slot scan and ring
wrap policy with native payload alignment, validates IDs/table callbacks and
capacity, and releases the selected slot/restores cursor bookkeeping when
initialization fails. Callback-list construction rejects insufficient remaining
space in the current task arena before writes. Task initialization clears both
start/end pointers while retaining its bound program.

Fixed the view-matrix alias: g_ViewMatrixSlot and its D_800BCFA4 spelling refer
to the canonical g_GeomScroll matrix pointer, not separate fixture variables.
The historical flags base BCF88 and matrix BCFA4 differ by 1C, matching the
original scroll record. Removed duplicate matrix fixture storage and the actor
fixture's separate scroll record; that fixture links the whole original camera
module. Existing mirror code imports the complete geometry state for access.

The existing actor fixture executes actual InitTaskContext, SpawnTask,
UpdateTasks and DrawTasks with observing callbacks: script variable assignment,
spawn, timer wait and stop, update/draw ages, completion, eight-slot exhaustion,
invalid IDs, oversize payload, native alignment, arena wrap and failed callback
allocation rollback. Guards outside the owner storage remain intact. No
replacement effect gameplay is used in production.

Full build and four CTests pass with Disc 1 required: actor-animation, camera,
entity-texture-assets and shadow. Fresh ASan/UBSan/function actor-fixture
validation passes with the original task engine, helpers and camera sources
instrumented; PsyZ/SDL archives are not. These tests do not execute GPU drawing
primitives in geometry.c. Its PS1 packet layouts/linking, script opcode 3
address semantics, complete effect callback signatures and native program-table
bindings still require work. No new room effect handlers were enabled. Full
gameplay and elimination of the remaining pc/ runtime remain incomplete.


### Audit retail effect scripts and reject invalid native pointer jumps

Disassembly of the supplied USA executable confirms opcode 3's positive
countdown branch: 800D42EC jumps to 800D432C, its delay slot sign-extends the
16-bit operand, and 800D432C/4334 subtract 6 and store it as the script PC.
A halfword cannot preserve a native script pointer. The existing interpreter
now returns -1 on that branch rather than forging a host address; its
nonpositive countdown/yield behavior remains unchanged. Variable accesses in
opcodes 1, 3 and variable-wait opcode 4 reject indices outside the seven-word
array. This does not implement a guessed relative jump.

The existing real scene-spawn fixture now audits all 181 room effect-program
scripts in Disc 1, reading their descriptor script address directly from the
serialized 56-byte records. All 1188 instructions are bounded, end in opcode
0, have valid variable/task indices and contain no opcode 3. The corpus is
required, not skipped. This is data inspection, not execution of those effects
or proof about Disc 2/resident program scripts.

The actor fixture verifies invalid variable indices, rejection of the positive
opcode 3 branch without replacing the native PC, and the original terminating
countdown behavior. Full build and two CTests pass with Disc 1 required:
actor-animation and scene-spawn. Fresh ASan/UBSan/function runs pass for both
fixtures with original interpreter/source code instrumented; PsyZ/SDL archives
are uninstrumented. GPU effect packet porting, native descriptor bindings,
complete gameplay and removal of pc/ remain incomplete.


Native field-effect packet headers now use the existing RenderNativeTag with
full host addresses and lengths. Existing packet structures, scroll-state views,
OT strides and geometry.c linking were corrected in place; GPU payload words
remain unchanged. The existing actor fixture checks packet chains in both slots,
four blend modes, semitransparency, mode packet size and surrounding guards.
The two point/glow paths also use native OT strides. No new files were added.
This is packet-layout and link verification, not evidence of rendered effect
fidelity or complete room-effect bindings. The pc/ runtime still has to be
replaced by the whole original modules and removed.


The remaining five direct effect OT insertions (textured quad, glow and ribbon)
and the textured-strip helper now index native tags instead of four-byte words
or single bytes. No AddPrim calls with legacy OT strides remain in geometry.c.
The existing actor fixture executes the original FieldFx_DrawScreenTint for all
4096 depths, both slots and four blend modes plus opaque mode: full pointer
chains, command words, RGB scaling, dimensions, consumed bytes and guards are
checked. Depth 4096 does not alter the OT. The strip helper is checked against
the same native table at every valid depth. Full build and four related CTests
pass with Disc 1 required. A freshly rebuilt actor fixture passes
ASan/UBSan/function with original source instrumented; PsyZ/SDL archives are
uninstrumented. These CPU packet checks do not establish GPU image fidelity or
complete effect callbacks. Whole-game integration and deletion of pc/ remain
required.


D_800B0E38 now aliases the canonical GameState ten-slot buffer region through an
existing RenderBufferPrefix union view. Original ordering tables, six temporary
resource pointers and the packet pair share their original storage; no separate
production buffer state or fixture copies remain. Static offsets and a test
cover all ten native slots in both directions. Five existing fixtures now use
the canonical state when executing original renderer/effect methods. Full build
and nine related CTests pass, including original model GPU and room GPU paths,
with Disc 1 required. Fresh actor ASan/UBSan/function passes with original
source instrumented and PsyZ/SDL archives uninstrumented. Boot allocation,
buffer lifetime across complete scene transitions, real effect descriptors,
full game-loop integration and deletion of pc/ remain incomplete.


The remaining original buffer names now alias canonical GameState slots:
OT at USA 800B0E38/3C, scratch at 800B0E4C, front/back at 800B0E50/54 and
animation packets at 800B0E58/5C, corroborated by the historical symbol map.
Entity allocation no longer owns separate scratch storage. Original boot
initialization now sets the actual shared renderer/effect/allocation buffers;
its existing fixture checks both OTs, packet slots and all allocation bounds.
All changes are in existing files. Full build and ten related CTests pass with
Disc 1 required, including pools, actor lifecycle, original GPU model/room paths
and room/entity assets. This does not yet prove full transition lifetimes or
complete authored effect execution. Production boot-memory backing, integration
of remaining original modules and removal of pc/ remain required.


The whole original fieldfx/model.c now compiles in parasite-eve-original.
Its physical scratchpad access is replaced in place by a local workspace;
native OT indexing and backing for original model parameters/color backup are
provided in the existing module. Packet insertion rejects depths outside the
4096-entry table. The texture setter agrees with the existing int API.
The actor fixture executes original FieldFx_ModelDraw for flat/shaded triangles
and quads together, in both slots and both transparency modes. It checks all
four native lengths/codes, full pointer chain, one occupied OT depth, consumed
packet bytes and surrounding guards. Full build and four related CTests pass
with Disc 1 required; fresh actor ASan/UBSan/function passes with original source
instrumented and PsyZ/SDL archives uninstrumented. This fixture establishes CPU
packet generation, not rendered effect-image fidelity. Real model descriptors,
color/UV controllers, culling variants, full scene integration and removal of
pc/ remain incomplete.


The whole original fieldfx/sprite_render.c now compiles in the native target.
Its two physical scratchpad views share ordinary module-owned union storage,
retaining shared work-area semantics without dangling local pointers. Original
sprite state and cell corners are backed in the same module; the diagnostic
string matches the supplied USA EXE. Standard C library declarations replace
the BIOS declarations. Shaded-quad/ring OT helpers index native tags, and ring
draw modes consume sizeof(RenderTintMode), rather than eight bytes.
The existing actor fixture executes original oriented-sprite draw in both slots,
all four texture presets, both color depths and five blend modes, checking cell
geometry, RGB, UV, texture/CLUT, command, length, pointer chain and buffer guards.
Full build and four related CTests pass with Disc 1 required. Fresh actor
ASan/UBSan/function passes with original source instrumented and PsyZ/SDL
archives uninstrumented. Other draws in this module, authored layer constants,
out-of-range depths, GPU image fidelity, complete scene integration and removal
of pc/ remain unverified or incomplete; compiling whole modules is not proof of
full gameplay.


Original ring generation/draw now has an executed native packet fixture:
four segments, both slots, both placement modes and five blend modes. It
checks builder coordinates, every mode/quad link, command/length/RGB words,
the final OT head, consumed arena bytes and guard bytes. Colour payload copies
use memcpy with initialized ignored padding instead of unaligned word casts.
The entire original fieldfx/collision.c compiles to supply its actual colour
scaler. Triangle scratch uses ordinary module storage, and player triangle/quad
queries read canonical BattleEntity position fields instead of a sparse PS1
view. The obsolete unused view was removed. A 441-position fixture checks that
player triangle/quad queries agree with their original point-query counterparts;
this is coordinate-path equivalence, not an independent collision oracle.
Full build and six related CTests pass with Disc 1 required. Fresh actor
ASan/UBSan/function passes for the tested paths (original source instrumented,
PsyZ/SDL archives uninstrumented). Other collision functions and out-of-range
sprite/ring OT depths still need verification/repair. Actual GPU ring images,
authored descriptor bindings, whole-game integration and deletion of pc/ remain
incomplete. No new repository files were added.


Removed pc/sine_table.h and the independent XYZ/YXZ rotation calculations from
pc/matrix.c. Existing temporary consumers now call PsyZ RotMatrix/RotMatrixYXZ;
the remaining matrix entry points only adapt angle/output storage and preserve
translation. No replacement table or new source file was introduced. CMake
links PsyZ for every existing matrix consumer, including its isolated fixtures.
Full build and eleven related tests pass. The 65536-angle fixture now checks
adapter outputs and translation preservation against direct PsyZ calls; it is
not an independent proof of PsyZ rotation correctness. Its fresh
ASan/UBSan/function run passes with temporary caller/fixture code instrumented
and PsyZ/SDL archives uninstrumented. Original transforms, animation and actor
paths also pass their existing tests. The remaining pc/matrix.c composition,
point transform and model walkers are still temporary duplicate logic and must
be replaced by whole original module integration; this deletion does not prove
complete gameplay or removal of the entire pc/ directory.


The entire original fieldfx/glow.c now compiles in the native target. Its
fourteen resident layer spin/scale records and original zero-initialized sprite
parameter storage are backed in that same module. The existing actor fixture
reads SLUS_006.62 directly from the supplied CUE/BIN and compares all fourteen
compiled records to the original executable bytes using its load-base header.
It executes the three paired flare draw variants and the four-layer variant
through original composition, scale and sprite methods in both slots, checking
packet count, unique native chains, commands/lengths, layer brightness and
arena guards. Sprite input records are controlled fixtures; authored script
setup and full effect lifecycle are not yet bound. Full build and four related
CTests pass with Disc 1 required, and fresh actor ASan/UBSan/function passes
with original source instrumented and PsyZ/SDL archives uninstrumented.
Remaining glow callbacks, GPU image fidelity, authored task tables, full scene
integration and deletion of pc/ remain incomplete. No new source files were
added or functions extracted.


The whole original fieldfx/object_script.c now compiles in the native target.
Slot address arithmetic uses uintptr_t; FieldFxObject owns its complete 0x80C
slot-data array instead of a one-byte placeholder. Original script globals and
USA diagnostic strings are backed in that module. Halt/free checks read named
slot_count/halted fields instead of aliasing a different word structure; the
unused word-view type was removed. Standard C library declarations replace
BIOS declarations. The existing actor fixture executes GetSlot, RunScript,
Register, UpdateSlots and FreeAllSlots with native pointer-bearing payloads,
checking original callback order/count, init/draw/update/free, wait/end PCs,
context/register/variable views, one-live-slot halt and object guards.
Handlers and command tables are controlled fixtures, not authored effect
bindings. Full build and four related CTests pass with Disc 1 required; fresh
actor ASan/UBSan/function passes with original source instrumented and PsyZ/SDL
archives uninstrumented. Actor-owned dispatch gates, malformed commands,
allocation bounds/alignment/wrap across authored sizes, original handler-table
ABI, full effect lifecycle integration and deletion of pc/ remain incomplete.
No functions were extracted and no new files were added.


The original GlowEffect now has its native init/draw/update lists, 25-word
command script, seven slot sizes, action scales and flare/halo offsets backed
in glow.c. Existing callbacks were modified in place to the same three-void-
pointer/int-return ABI used by FieldFxHandler; missing ignored returns are
explicit zero. No wrapper handlers or extracted functions were added. The
shared paired-layer template uses that ABI too; four-layer drawing remains a
direct method. The previously unnamed update entry at USA 800C8CBC is confirmed
by disassembly to be the ground fade/expansion method (brightness -= 8,
scale += 0x78, teardown below 0x14). Original ApplyMatrixSV (whole mtx_06.c)
is compiled for the initializers. Effect texture pointers alias canonical
GameState startup_resource_tables at USA 800B0E18/1C instead of separate state.

The actor fixture compares resident script, slot sizes, action scales and all
flare/halo offsets to the supplied USA EXE in addition to layer constants.
It plants the authored script and executes actual init/draw/update tables for
160 ticks with a controlled actor, player action 1, matrix state and two trigger
requests, followed by stop. All six handler IDs execute and slots finish; both
packet buffers are used and object guards remain intact. Texture page 0 is
already marked resident in this fixture, so actual texture loading is excluded.
Full build and four related CTests pass with Disc 1 required. Fresh actor
ASan/UBSan/function passes with original source instrumented and PsyZ/SDL
archives uninstrumented. Real actor ownership, all player actions, texture
uploads, room descriptor/class/spawn ABI integration, other authored effects,
GPU image fidelity, full gameplay and deletion of pc/ remain incomplete.


Original Spawn6 interfaces now carry the owner as intptr_t rather than int.
The eight field effect entry points, common room macro and matching declarations
were updated in place to avoid narrowing host addresses. FieldFx_Spawn6 has a
complete six-argument prototype; its four meaningful arguments retain the
original owner/unused/index/value behavior and the final two remain ignored.
It rejects null owners and indices outside the nine-variable file before writes.
The authored glow fixture now calls its actual GlowEffect_Spawn6 entry point.
An additional existing-fixture check calls all four glow entry points through
matching native function-pointer types, writes all nine variables with signed
extreme values, verifies all other state bytes and guards, and checks invalid
common-function inputs preserve state. Full build and four related CTests pass
with Disc 1 required; fresh actor ASan/UBSan/function passes with original source
instrumented and PsyZ/SDL archives uninstrumented. Burst methods and other room
Spawn6 consumers have consistent declarations but their full modules/class
bindings remain unverified. This is not proof of descriptor/class-table ABI,
full gameplay or removal of pc/. No new files or extracted functions.


Original object-script slot allocation now checks available context, the
64-slot bound, free-slot state, data cursor range and reservation size before
writes. Oversized data reservations do not call init callbacks. FreeSlot ignores
out-of-range/already-free slots instead of decrementing the live count again.
The original >=0x80C wrap policy is preserved, including wrapping back onto
existing data; this does not add an overlapping-allocation policy or padding
that could change authored behavior. The existing actor fixture covers all 64
slots and the full-table register signal, exact full-area writes, wrap at the
end, invalid cursor/reservation/slot inputs and repeated frees with object guards.
The authored 160-tick GlowEffect cycle still passes. Full build and four related
CTests pass with Disc 1 required, plus fresh actor ASan/UBSan/function with
original source instrumented and PsyZ/SDL archives uninstrumented. Unknown
handler-list lengths/IDs, all authored allocation lifetimes and native alignment
requirements, complete descriptor integration, full gameplay and deletion of
pc/ remain incomplete. No new files or extracted functions.


GlowEffectB and GlowEffectC now use their original native init/draw/update lists,
slot sizes, authored scripts, action scales and flare offsets, backed in the
existing glow.c. Their existing leaf callbacks use the same three-pointer/int
ABI in place, with explicit ignored return values; no wrapper functions or new
files were introduced. The fixture reads the additional resident data directly
from SLUS_006.62 inside the supplied USA CUE/BIN and compares the compiled bytes.
The authored lifecycle fixture now runs three glow variants across action
indices 1..7, 160 ticks each (21 controlled cycles), including start/retrigger,
all respective handler IDs, native drawing to both packet slots, stop and final
slot release. Controlled actor/player/matrix inputs and resident texture-page
status remain fixtures; this does not establish scene ownership or GPU pixels.
Full build and four related CTests pass with Disc 1 required. Fresh actor
ASan/UBSan/function passes with original source instrumented and PsyZ/SDL
archives uninstrumented. Four-layer authored callback tables, other actions,
texture uploads, class/descriptor integration, full gameplay and deletion of
pc/ remain incomplete.


FourLayerGlow now has its original init/draw/update lists, slot sizes, script,
action scales, point/flare/halo offsets and sprite rotation backed in glow.c.
Existing callbacks, including the four-layer draw, use the native three-pointer
handler ABI in place with explicit ignored returns. The previously unnamed
update entry at USA 800CBBF0 is confirmed by disassembly as twin-glow fading
(brightness -= 8, scale += 0x3C, teardown below 0x14). No new wrappers or files.
The resident-data fixture compares these additional compiled records to the
supplied USA EXE. Authored lifecycle coverage is now four variants x action
indices 1..7 x 160 ticks = 28 controlled cycles (4480 ticks), including start,
retrigger, all respective handler IDs, native packet generation to both slots,
stop and release. Full build and four related CTests pass with Disc 1 required,
plus fresh actor ASan/UBSan/function with original source instrumented and
PsyZ/SDL archives uninstrumented. Texture residency, actor/player/matrix state
remain controlled fixture inputs. This does not yet verify texture uploads,
rendered effect pixels, production native object allocation/class sizes,
actor-owned dispatch gates, descriptor integration, full gameplay or deletion
of pc/.


Primary process slots now include native union views of the existing
FieldFxObject and FieldFxTaskOwner records, retaining the original PM header
and legacy payload view. On this ARM64 build the former slot was 2576 bytes,
while scripts need 2592 and task owners need 2688; the slot now has capacity
for both (2688 bytes), with canonical command/kind/count/owner alias assertions.
All eleven primary-bank strides and the secondary-bank boundary use sizeof
native slots. The existing lifecycle fixture executes actual GetSlot and task
context reset in each primary record and writes both complete payload areas,
checking every byte outside that slot, including all neighbours/secondary bank
and guards. Unused transitive GPU declarations were removed from two existing
sprite-record headers so the shared PM types can coexist with native SDK users.
No new record implementation, files or extracted functions.

The whole original pm/process.c is now in parasite-eve-original; its separate
fixture source entry was removed. Standard libc declarations replace BIOS
ones, and the obsolete critical-section/instruction-cache-flush sequence is
removed for statically linked native room code. Full build and six related
CTests pass with Disc 1 required. Fresh actor-animation and actor-lifecycle
ASan/UBSan/function runs pass with original source instrumented and PsyZ/SDL
archives uninstrumented. Original room descriptor bindings beyond mirrors,
effect PM callback ABI/class tables, on-demand commands 108..114 and their I/O
failure behavior, production allocation lifetimes, full gameplay and deletion
of pc/ remain incomplete. This fixes native pool capacity; it does not claim
that the complete process-manager/game loop is integrated.


Scene_LoadRoomAssets in the original process manager now returns -13 for an
on-demand task-bank read/poll error instead of retrying indefinitely. Empty or
reversed sector bounds, null destination and a 64-bit LBA extent exceeding
2^32 are rejected before I/O. Registry installation/ready flag/slot allocation
remain after successful reads only. The existing actor fixture covers commands
108..114 with read rejection and a pending poll followed by failure, plus invalid
extents and other negative read statuses, checking exact call counts, unchanged
program registry and no loaded flag. The still-unbound program labels/buffer
are controlled fixture addresses; no successful load or native program binding
is claimed. The missing task-program diagnostic now has its original USA
string backed in task_list.c. Full build and five related CTests pass with Disc 1
required, plus fresh actor ASan/UBSan/function with original source instrumented
and PsyZ/SDL archives uninstrumented. Native programs 108..114, their successful
load/recovery, callers beyond the tested error path, complete scene/gameplay
integration and deletion of pc/ remain incomplete. No new files or extracted
functions.


Original Asset_LoadTimTextures now rejects read errors and invalid/null/overflow
extents in phase 0x34. A poll error returns -1 and resets to phase 0x34 so recovery
requires a fresh read; positive pending polls still preserve phase 0x35.
Scene_LoadRoomAssets waits for this native image-loader state machine to finish
before looking up/allocating the process, returning -14 on texture failure.
The existing actor fixture checks read failure, pending-then-error propagation,
other negative read statuses, empty/reversed/overflow bounds, null scratch and
recovery with a valid empty upload bank. Recovery clears the load flag/state and
then reports the expected missing PM handler, without executing a fake handler
or task program. It does not prove uploads of nonempty authored texture banks.
Full build and five related CTests pass with Disc 1 required; fresh actor
ASan/UBSan/function passes with original source instrumented and PsyZ/SDL
archives uninstrumented. Real bank upload/recovery, all callers, native effect
class/descriptor integration, full gameplay and deletion of pc/ remain
incomplete. No new files or extracted functions.


Real Asset_LoadTimTextures failure/recovery is now exercised against the USA
common bank (one actual upload), using the original image backend and GPU VRAM
readback. Restricting the mounted PE.IMG extent rejects the bank read without
changing scratch bytes or VRAM. A deliberate backend error injected between a
successful read and completion makes the loader reject polling, reset to read
phase and avoid uploading the valid-but-uncommitted scratch. A fresh file-backed
read is then required and its upload matches the full VRAM reference. This
extends the existing real TIM fixture, which also compares all 414 room-bank
loads and 296 keyed images in 123 rooms. Full build and three related CTests
pass with Disc 1 required. A freshly compiled real entity-assets fixture passes
ASan/UBSan/function with original/test source instrumented and PsyZ/SDL archives
uninstrumented. This proves the tested TIM upload data and error protocol, not
rendered effect/gameplay fidelity or complete PM/descriptor integration. Full
gameplay and deletion of pc/ remain incomplete. No new source files or game
functions were added.


All four original glow classes now have native PmCommand seven-method tables
backed in glow.c. Their existing methods were modified in place to match init,
initialize, send, start, execute, stop and unload callback signatures exactly;
no adapter functions or relocated game methods were added. Send preserves full
native owner/result-word widths and explicitly stores the original 32-bit script
variable value. Close writes the named PM state field. Original USA class
records at 800E086C/09D4/0ABC/0BCC confirm the same seven method ordering.

The authored fixture now runs the 28 glow/action cycles in actual native
PmPrimarySlot records through Pm_SendCmd, Pm_Start and Pm_Exec, with init/unload
and state-driven close/release. It also restarts each variant and tests Pm_Stop
ownership rejection and owner-authorized stop. Handler row 7, actor/player/
matrix state and texture residency are controlled fixture inputs; this does not
bind the real resident/room descriptor IDs. Full build and five related CTests
pass with Disc 1 required; fresh actor ASan/UBSan/function passes with original
source instrumented and PsyZ/SDL archives uninstrumented. Actual descriptor
registration/factory allocation, actor-owned gates, GPU pixels, full gameplay
and deletion of pc/ remain incomplete.


The original PM handler-array symbol now has native storage in the existing
asset/tim_textures.c, and g_PmTables starts with that registry. USA executable
records confirm resident IDs 0..3 map to the four original glow classes;
compiled native entries use those exact IDs. Other unbound entries remain null,
not fabricated handlers. The resident fixture compares the first four original
pointer words directly to the supplied EXE. The authored cycles now create
actual native PM slots via Scene_LoadRoomAssets with those resident IDs, a live
actor state and real class constructor; no manually chosen handler row or direct
constructor call. The owner action-state transition is also checked. Init,
send/start/execute, natural close/release and owner-checked forced stop still
pass for four variants x seven actions. Texture/scene-ready flags and actor/
matrix state remain controlled fixture inputs.
Full build and five related CTests pass with Disc 1 required, plus fresh actor
ASan/UBSan/function with original source instrumented and PsyZ/SDL archives
uninstrumented. Actor gates with other core flags, remaining resident handlers,
room-local descriptor bindings, live scene inputs, GPU effect pixels, full
gameplay and deletion of pc/ remain incomplete. No new files or extracted
functions.


The original actor-owned effect termination gate in FieldFx_UpdateSlots now
selects native named fields instead of byte offsets 0x1C + selector*0x10 from
the PS1 state layout. Its eight byte views map to count_limit, progress_step,
transition_timers[0], flags, number_desc_1[4], substate, reserved74[8] and
script_value8c; the original flag mask/zero-byte predicate remain. Inactive masks
skip the read. The existing fixture tests all selectors with mask off/on and
zero/nonzero values, plus the action-state 1-to-2 transition. The substate test
checks the original low-byte-of-pointer representation; its meaning for real
actor data remains unverified. Full build and five related CTests pass with
Disc 1 required, plus fresh actor ASan/UBSan/function with original source
instrumented and PsyZ/SDL archives uninstrumented. Real actor-state semantics
for all selectors, room descriptors, complete scene/gameplay integration and
deletion of pc/ remain incomplete. No new files or extracted functions.


The entire original fieldfx/burst.c now compiles in the native target. Pointer-
bearing game_audio_state.h assertions describe native pointer offsets instead
of fixed PS1 offsets; no substitute audio-state backing was introduced.
PointSprite's original six init/draw/update callbacks and seven class methods
were normalized in place to the existing native dispatch ABI, with its actual
22-word script, seven slot sizes and sprite-state storage backed in burst.c.
The shared scatter callback declaration agrees with its handler ABI. Resident
PM ID 7 now points to the original PointSprite class. The unnamed USA update
entry 800CCA40 is confirmed by disassembly as ground-ring fading/expansion
(scale -= 2, extent += 0x28, teardown below 0x1E).

The existing factory/lifecycle fixture now covers five resident effects x action
indices 1..7 x 160 ticks (35 cycles), plus restarts/owner-checked stops. PointSprite
uses its original one-shot trigger and all six handler IDs; action 3's reduced
radial/scattered counts (8/4 versus 16/16) are checked. The source executable's
resident registry words, PointSprite script and sizes are compared to the
supplied disc. Full build and five related CTests pass with Disc 1 required;
fresh actor ASan/UBSan/function passes with original source instrumented and
PsyZ/SDL archives uninstrumented. Other burst classes/audio-state routing,
texture residency, scene descriptors, live inputs, GPU effect pixels, full
gameplay and removal of pc/ remain incomplete. No new files, wrapper handlers
or extracted functions.


Burst target lists reveal that the original effect variable file holds both
scalar words and addresses: three target initializers dereference VarPtr(1) as
FieldActor**. FieldFxState.vars and VarPtr now use intptr_t, and Spawn6's value
argument carries a native word. Existing normalized class methods no longer
narrow that value before storing it. Matching room/module declarations and the
common spawn macro were updated in place, including the two consumers that
retain a variable-file pointer. Numeric registers remain the original int
file; their existing script operations have not been redesigned.
The existing spawn fixture now checks native target-list round trips through
five real class entry points, dereferences the received list, and verifies all
nine variable slots with signed numeric extremes. Existing authored cycles and
native PM slot-capacity checks still pass. Full build and five related CTests
pass with Disc 1 required, plus fresh actor ASan/UBSan/function with original
source instrumented and PsyZ/SDL archives uninstrumented. Remaining burst
callback/class bindings, audio-state alias/routing, all uncompiled room consumers,
numeric-register pointer transport/arithmetic beyond tested scripts, scene
integration, full gameplay and removal of pc/ remain incomplete. No new files
or extracted functions.
