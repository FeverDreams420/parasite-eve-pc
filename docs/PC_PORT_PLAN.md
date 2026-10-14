# Plan implementacji pełnego portu Parasite Eve na PC

Plan obejmuje całą grę, nie tylko boot i filmy. Celem jest ukończony natywny projekt C11: pełna kampania, oba dyski, wszystkie lokacje i sceny, eksploracja, walka ATB, PE, przeciwnicy, przedmioty, ekwipunek, menu, audio, zapis/odczyt, Tutorial oraz tryby i zawartość dodatkowa. Każda funkcja i każdy element zawartości mają otrzymać automatyczny test oraz mierzalny warunek odbioru.

„Perfekcyjny” oznacza pełny uzgodniony zakres, poprawne zachowanie, brak znanych defektów i komplet wymaganych dowodów automatycznych. Sam build, procent pokrycia kodu, ekran menu albo liczba przetestowanych plików nie dowodzą ukończenia gry. Nie można udowodnić braku błędów dla wszystkich możliwych kombinacji stanu i sprzętu; dlatego jawnie definiujemy katalog funkcji, niezmienniki, warianty, obsługiwane platformy i testy błędów. Żadna pozycja zakresu nie może zniknąć z raportu przez pominięcie testu.

Byte match, odbudowa EXE PS1, historyczne kompilatory, kolejność instrukcji i adresy kodu nie są wymagane. Oryginał jest wzorcem zachowania i źródłem danych; port ma normalne typy, wskaźniki, moduły i build C. Poprawiamy błędy pamięci i przenośności. Zmiany balansu i tempa pozostają osobnymi, opcjonalnymi funkcjami.

## Obowiązujący kierunek portowania

Uruchamiamy oryginalny kod `src/main/` i `src/overlays/` na PC. Modyfikujemy go w miejscu, usuwając zależności sprzętowe i dostosowując pamięć oraz wskaźniki; zachowujemy logikę, kolejność aktualizacji i zachowanie gry. Nie budujemy równoległego silnika ani nowych warstw abstrakcji. PsyZ zapewnia istniejące API platformowe.

Aktualne wiążące wymaganie użytkownika: żadnych nowych plików ani wydzielania funkcji. Naprawiamy całe istniejące moduły w miejscu. Dotychczasowe wydzielenia należy scalić z powrotem, zachowując poprawki i sprawdzając kompletność funkcji względem oryginalnych modułów. Katalog `pc/` ma zostać usunięty: nie jest docelowym miejscem ani logiki gry, ani osobnej implementacji platformowej. Potrzebne dostosowania platformy należy integrować w istniejących modułach oryginału i PsyZ. Automatyczna weryfikacja pozostaje wymagana; jej zależności od `pc/` również wymagają przebudowy. Historyczne wpisy poniżej opisujące wydzielanie funkcji lub dopuszczające `pc/` nie stanowią zgody na kontynuowanie tego podejścia.

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
