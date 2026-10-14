# Parasite Eve PC

A native C11 PC port of the North American PlayStation release of Parasite Eve
(`SLUS-006.62`), using [our PsyZ source fork](https://github.com/khasinski/parasite-eve-psyz) for the
PlayStation runtime APIs.

The code was imported from
[parasite-eve-decomp](https://github.com/khasinski/parasite-eve-decomp) through
revision `1d20afe62`. From this point this repository develops independently
as a native C project. Byte matching, retail executable checksums, original
compiler output, fixed code addresses and PS1 binary builds are no longer
acceptance requirements. Changes are evaluated by behavior, portability,
readability and native builds.

The native executable boots through the opening movie to the original title
screen and main menu. MDEC video and stereo XA audio are decoded directly from
a user-supplied Disc 1 CUE/BIN; menu artwork comes from `PE.IMG`.

Gameplay is not playable yet. The game sources under `src/` are retained for
porting and are not yet linked into that executable. Remaining MIPS/GTE
instructions, hardware access, address aliases and 32-bit pointer assumptions
need C implementations or PsyZ adapters before the affected modules can join
the host build.

## Build and run

Use a C11 compiler, CMake 3.21 or newer, Git, pkg-config and FFmpeg development
libraries (`libavformat`, `libavcodec`, `libavutil`, `libswscale`, `libswresample`).
On macOS install them with `brew install ffmpeg pkg-config`. PsyZ and SDL are source
submodules; no historical PS1 compiler, Sony SDK, Python packages or extracted
game executable is needed for this build.

```sh
git clone https://github.com/khasinski/parasite-eve-pc
cd parasite-eve-pc
git submodule update --init third_party/psyz third_party/psyz/external/SDL
cmake -S . -B build
cmake --build build --parallel
./build/pc/parasite-eve-pc --disc "/path/to/Parasite Eve (Disc 1).bin"
```

`--discs "/path/to/Disc 1.cue" "/path/to/Disc 2.cue"` mounts both sources
throughout boot. `pc/assets.h` keeps source provenance for identically named
files and allows reads from either image without a disc-swap operation. CUE
support currently covers a single BINARY MODE2/2352 track, relative quoted
FILE paths and INDEX 01 offsets. Other layouts fail explicitly. Disc role/version
validation and the game-wide asset catalog are still pending; this is not yet
proof of gameplay or correct Disc 2 content. `--disc` remains a boot diagnostic.

Interactive mode requires a graphical session and an audio device; automated
boot tests can use `--headless`. You can set
`PE_DISC_IMAGE` instead of passing `--disc`; `make run` uses that environment
variable. Enter/Space skips the intro and opens the title options. Arrows or
W/S change the selection, Enter confirms, and Escape returns to the title
prompt. Continue remains disabled until native save support exists. Selecting
New Game or Tutorial loads the original starting room data (M431 or M353)
and applies its scratch/texture-bank uploads to CPU-visible VRAM, then exits;
script entry/actor catalogs and the startup SPU sample bank are prepared in
`PeScene`, but scene execution and gameplay are not integrated.

`make ci` builds and runs the asset-free disc-reader, VRAM, script-format,
script-runtime, script-scheduler, SPU-bank, AKAO-sequence, AKAO-channel,
AKAO-timing, AKAO-player, AKAO-instrument, AKAO-pitch and automation tests. The
room-assets, boot-assets, boot-replay and boot-newgame tests skip when no image
is supplied. To verify complete movie decoding, room directory bounds and
menu input with your own image:

```sh
PE_TEST_DISC="/path/to/Parasite Eve (Disc 1).bin" ctest --test-dir build --output-on-failure
```

The optional PsyZ window test is `./build/pc/parasite-eve-pc --smoke-test`.
A direct `cmake -S pc -B pc/build` build remains supported.

## Layout

- `pc/` — native executable and PsyZ integration.
- `src/main/` — game logic to port; `psyq/` holds legacy runtime implementations
  whose gameplay-relevant behavior guides native replacement or removal.
- `src/overlays/` — room, scene and effect modules to register natively.
- `include/` — game types, data formats and legacy platform interfaces.
- `third_party/psyz/` — our maintained PsyZ source fork and its SDL dependency.
- `docs/` — native architecture, conventions, port plan and data-format notes.

See [CONTRIBUTING.md](CONTRIBUTING.md) and
[the port plan](docs/PC_PORT_PLAN.md) for the development workflow.

This repository contains source only. Disc images, game assets, FMV, memory
cards and proprietary SDK files are excluded. Running disc-backed game
features will require the user's own legal copy of the game.

## Diagnostic commands

`--inspect-disc`, `--decode-intro`, `--play-intro` and `--menu` each take a
Disc 1 BIN path. They inspect boot assets, decode without presentation, play
only the intro, or start directly at the title menu. Normal boot uses `--disc`.

An ordinary 2048-byte ISO preserves data files but loses the XA sectors needed
for movie audio. Use the original single-track MODE2/2352 BIN for full boot.
No game assets are copied into this repository.

The full-game roadmap favors gameplay fidelity and native simplification: PS1
APIs can be removed, room modules linked statically, and scratchpad work moved
to normal buffers. Both user-supplied CUE/BIN images will form one logical asset
set, without a physical drive or manual disc swapping. That combined layer is
planned; the current boot command still takes Disc 1 only. Hi-res uses PsyZ's
existing rendering support during game integration; high FPS is a later stage.

## Headless automation

The boot frontend can run without a visible window or audio device, using a
software renderer and dummy audio. Replays react to actual presented states,
not timed key presses, and write machine-readable JSONL events:

```sh
./build/pc/parasite-eve-pc --headless \
  --replay pc/tests/replays/boot-tutorial.replay \
  --trace build/boot.jsonl --replay-timeout-ms 5000 \
  --disc "/path/to/Parasite Eve (Disc 1).bin"
```

Each replay line is `event expected_integer action`. Actions currently cover
the title inputs (`return`, `space`, `escape`, `up`, `down`, `z`), `close`, and
`assert`. A mismatched value, incomplete replay, trace failure or timeout returns
failure. Trace values include the displayed movie-frame index, menu selection
and exit result; timestamps stay monotonic through SDL shutdown.

This interface currently covers boot/title state. Complete game-state snapshots,
input actions, RNG/clock control, scenario checkpoints and save fixture loading
will be added with `game_core`; the gameplay port remains incomplete.
`--headless` does not emulate the PsyZ GPU smoke test.

For mandatory asset tests, set both `PE_TEST_DISC` and
`PE_REQUIRE_GAME_ASSETS=1` before running CTest. Missing data then fails rather
than silently skipping tests that require game assets.

`room-assets` walks all 450 USA Disc 1 room table slots, including the final
zero slot: 414 contain loadable scratch/texture/room blobs and 36 are empty. It checks directory bounds and
transactional loading and texture upload bounds for both initial banks
(62,002,432 uploaded 16-bit words across the 414 rooms). `vram` checks upload
order, secondary image offsets, zero-height rules and atomic rejection of bad
batches without game data. This does not verify all payload semantics, render
rooms, execute scripts or prove that maps are playable. The asset-free reader
test also checks malformed room headers/TIM tables without disc images.

`pc/script.c` preserves self-relative script offsets and decodes the original
operand modes, including the tail word for arguments after the fifth. Disc 1
tests validate 2,411 entry offsets and 594 initial actor descriptors, and decode
each initial actor's first command. These checks do not traverse every branch
or execute scripts. The active starting actors of M431/M353 begin with opcode
234 (`Akao_ProcessCommand`), whose native execution remains pending.

`pc/spu_bank.c` uploads the original AKAO sample-bank format to CPU-visible
512 KiB SPU RAM and decodes instrument addresses, ADSR fields and pitches.
The startup bank comes from the executable's PE.IMG range table, with 28,176
sample bytes and 256 instrument records. Uploads and scene preparation reject
bad data before replacing existing state. Room changes preserve existing SPU
and VRAM contents before texture uploads. This is resource preparation;
sequence execution, synthesis, mixing and room-specific audio-bank changes
remain pending. `spu-bank` tests malformed/truncated banks and partial uploads,
while `room-assets` tests real startup data and scene ownership/rollback.

`pc/script_vm.c` executes one native script task with shared variable/operand
banks and writable literal operands. It supports assignments, all 24 expression
operations, conditional/unconditional jumps, wait ticks, event cursors, scene
flags, node metadata and defined
no-ops. Signed comparisons and fixed-point operations preserve the original
word semantics without C overflow/shift undefined behavior. Unsupported
opcodes and instruction-limit exhaustion return explicit errors. Tests execute
both event branches of the complete actual M431 entry-zero task; the active scene
actor still fails explicitly at AKAO opcode 234. The task runtime is compiled
into the native target but is not yet driven by the frontend's scene loop.
Subsystem dispatch supports continue, yield, retry and explicit errors, with
operand reads/writes through the original binding. Tests check live operands
between retry ticks and the original battle/actor pause gates. Audio/render
handlers, physical actor creation and full scenes remain pending;
these tests do not prove campaign execution.

`pc/script_scheduler.c` implements the original 72-node pool, three actor task
lists, child spawning, 16-bit sequence IDs and deferred collection. Tasks share
actor locals/flags/event cursors rather than copying them into independent
nodes. Tests cover insertion order within a tick, actor deletion after callback
processing, pool exhaustion/reuse and invalid links. `PeScene` owns the initial
script actor bindings and scheduler, and preserves script banks across room
changes. The frontend prepares these bindings but does not yet tick the scene;
models, movement, collision and rendering still need integration. The real
initial M431 actor reports missing AKAO opcode 234 through the scheduler.

`pc/akao_sequence.c` reads room music records and validates AKAO sequence
headers, 24-voice masks and channel offsets relative to the word following each
offset. Disc 1 contains 600 room music records: 599 validate as full sequences;
one ID-zero short record remains explicitly unresolved and has no parsed
sequence. This is metadata/offset validation, not sequence command execution
or audio output. The unresolved record is retained for further porting.

The native catalog also decodes the 250 audio-bank ranges from the supplied
executable. `pe_audio_bank_read` reads complete banks from PE.IMG with bounds
checks. `pe_spu_bank_upload_rebased` preserves the source bytes while relocating
instrument start/loop addresses and uploading to a selected SPU/instrument
bank. Tests load all 51 distinct sample banks referenced by Disc 1 room music
records. Automatic selection between active voice banks and live playback are
still pending; these tests upload data without synthesizing it.

`pc/akao_channel.c` executes eight primary-channel command types through the
next raw note/tie/rest byte: stop, next-note length, volume, octave set/up/down,
expression (or drum volume) and pan. Tests preserve the original signed
expression shift, pan wrapping and octave retention, and reject missing
opcodes/truncated operands explicitly. The first-prefix audit of Disc 1 music
reaches 1,055 raw notes and two stops; 7,826 other channels report an unsupported
command. Finite/nested loops C8/C9, repeating CA with deferred key-off, signed
relative jumps FC/06 and repeat-count branches FC/08/09 now preserve the
original four-slot ring stack and zero-as-256 repeat count. Tests also check
invalid targets and infinite-loop limits. Note timing/pitch, instrument
commands, remaining flow/LFOs, SPU allocation and
synthesis are still pending. Passing parser/command tests is not music playback.

Channels share `PeAkaoControl` for FC/00..03 tempo/master-volume set/slide
commands and the FC/07 condition branch. The native control helpers preserve
the scaled tempo accumulator, forced musical ticks, slide truncation and
beat/measure counters. The caller must run channel processing between the due
check and slide/meter update, as in the original timer. These helpers are not
connected to the complete playback driver or mixer; sequence playback is incomplete.

`pc/akao_timing.c` ports the original next-note lookahead separately from
command execution, including its different virtual loop behavior. Skip and
duration tables are decoded from the supplied executable and retained by
`PeScene`. Note/gate lengths preserve tie/rest rules, overrides and 16-bit
wrapping. A musical-tick helper advances the channel countdown, starts the next
note and latches key-off requests. Tests check exact tick boundaries and apply
length/lookahead processing to the 1,055 reachable initial notes of Disc 1.
Pitch, instrument/voice allocation, full-song scheduling and synthesized audio
remain pending; these are timing-state tests, not playback comparisons.

`pc/akao_player.c` combines the bank accumulator, active-channel countdowns,
ordered command execution, slides/meter and latched key-off events in one
headless timing driver. Tests cover two channels changing shared tempo in the
same tick, gate boundaries, bank completion and failed-start rollback. The
four-forced-tick audit still reports an unsupported opcode for all 599 full
Disc 1 sequences; none is claimed playable. SPU registers/voice allocation,
overlapping banks, waveform synthesis and integration with scene audio commands
remain pending. Sequence bytes are borrowed and must outlive the player.

Channel slides now execute after the current musical tick's commands:
A9 changes expression, AB changes pan and FC/12 changes channel volume. The
native state preserves 16-bit signed-delta wrapping, zero-as-256 durations and
the original change masks for pending volume-register updates. Tests also
preserve A3's behavior of changing volume without cancelling a running slide.
This models parameter changes; it does not yet apply SPU registers or produce
audio samples.

`pc/akao_instrument.c` implements A1 instrument changes against the loaded
SPU bank: sample/loop addresses, ADSR with release overrides, alternate-bank
offsets and the original wrapped pitch rescaling when the voice is unallocated.
The pitch-period table is an alias of each instrument's pitch array, not a
separate asset. C2/C3 update the bank's requested reverb mask and release
reserved voices; stop clears these requests before clearing channel flags.
The latest prefix audit reaches 5,036 raw notes and two stops, with 3,845
unsupported channel prefixes. Four forced ticks still fail on a missing opcode
for every full sequence. Note pitch generation, voice registers and audio
synthesis remain pending; these tests validate state, not audible playback.

`pc/akao_pitch.c` calculates normal melodic pitch from instrument semitone
tables, octave shifts, signed transpose and detune. It preserves the wrapped
32-bit detune product and unsigned shift before the final 16-bit pitch. C0/C1
and D8/D9 set/adjust transpose and detune; ties preserve pitch and rests avoid
key-on. Bound channels now apply pitch handling during musical ticks.
LFO modes remain explicit
errors. The current audit handles 5,434 initial raw note/tie/rest events and
reports 3,447 unsupported prefixes; every full sequence still hits missing
commands. SPU voice allocation/register application and audible synthesis remain
pending.

Drum-kit FC/04..05 now uses checked offsets to six-byte sample records. Each
key selects its instrument without A1 pitch rescaling, then applies the record's
pitch, gain, pan and reverb, including the alternate sample bank. `akao-drum`
tests kit entry/exit, preserved melodic bookkeeping, malformed offsets and the
no-rescale distinction. The current audit reaches 7,303 initial note events;
1,578 prefixes and all complete sequences still hit unimplemented commands.
This is state execution, not synthesized playback.

AKAO ADSR commands now update attack, decay, sustain and release parameters
and their dirty masks. Full instrument reset and release-only reset preserve
their distinct original semantics; `akao-adsr` verifies these operations.
FC/15 configures musical meter and FC/16 sets the measure counter. These
commands address shared bank state, despite misleading track aliases in the
reference code. All 19 CTest tests pass headless with Disc 1 data; ADSR and
channel tests also pass AddressSanitizer and UndefinedBehaviorSanitizer.
The latest audit reaches 7,987 initial note events, two stops and 894 unsupported
channel prefixes. 266 sequence prefixes complete four forced timer ticks;
333 encounter missing commands. This limited audit does not prove full songs,
ADSR envelope synthesis or gameplay, which remain unfinished.

BD writes the auxiliary LFO depth without changing note pitch; BE clears its
current modulation value and active flag while preserving the configured depth.
Channel tests verify these distinctions, volume dirty flags and truncated
operands. Active LFO waveform execution remains unsupported and explicit.

FC/14 enables note-range instrument selection through a checked relative
table offset. Ascending and descending notes use the original range thresholds
and neighbouring-instrument hysteresis; selected records override attack,
sustain and release parameters. `akao-note-range` verifies table bounds,
signed offsets, bank relocation, ADSR overrides and legato suppression.
All 20 CTest tests pass headless with Disc 1 data; note-range tests and the
Disc 1 room/audio audit also pass ASan/UBSan. The latest audit reaches
8,648 initial note events, two stops and 233 unsupported channel prefixes;
397 sequence prefixes complete four forced ticks and 202 encounter missing
commands. Full songs, synthesis and gameplay remain unfinished.

A4 configures a signed semitone pitch transition with zero duration meaning
256 ticks. Note processing calculates the original wrapped 16.16 difference
and signed step; musical ticks advance the offset and pitch dirty flag.
Timing tests verify ascending/descending transitions and the zero-duration
encoding under ASan/UBSan. These are sequencer state operations; physical
voice application and synthesis remain pending.
All 20 CTest tests pass headless with Disc 1. The updated initial-prefix audit
reaches 8,649 note events and 232 unsupported commands; 398 sequence prefixes
complete four forced ticks and 201 encounter missing commands.

FC/0A applies the original fixed sample start (`0x1010`) while retaining the
selected instrument's loop, ADSR, bank offset and A1 pitch-rescaling rules.
Tests cover release overrides, active voices, alternate-bank selection and
truncated operands under ASan/UBSan. This command updates sequencer state;
sample playback and complete songs still require synthesis and validation.
All 20 tests pass headless with Disc 1. The initial-prefix audit now reaches
8,718 note events and 163 unsupported commands (B8/BC LFO setup); four forced
ticks complete for 467 sequence prefixes, while 132 hit missing commands.

The room catalog now owns the original 16 LFO waveform offsets and 708 signed
sample words decoded from executable data. Native waveform traversal follows
checked relative loop markers without retaining console pointers. All 16
selectors complete 1,000 traversal steps in the Disc 1 audit; `akao-lfo-data`
tests signed samples, bad jumps and failure without cursor/output mutation.
Waveform data and traversal are available, but active channel modulation and
synthesis remain pending.

BC now initializes auxiliary LFO duration, selector and phase. Its separate
voice-driver step samples the waveform and calculates signed modulation with
the original arithmetic shift; notes restart the waveform and ties preserve
it. Tests verify phase spacing, negative rounding, restart, malformed cursors
and unchanged state on failed reads under ASan/UBSan. New notes also clear the
pitch-slide offset as in the original. Voice-driver scheduling and applying
modulation to synthesized audio remain pending. The limited Disc 1 audit now
reaches 8,869 initial note events and 12 unsupported volume-LFO commands;
588 sequence prefixes complete four forced musical ticks and 11 hit missing
commands. These results do not validate full songs or audible modulation.

B8/B9/BA configure primary-channel volume LFO delay, waveform, depth and reset.
The separate voice step preserves delay gating, key-on bypass and signed,
wrapped fixed-point volume arithmetic. New notes restart the waveform and
clear its current value. Tests cover delay/phase spacing, positive/negative
modulation, reset and key-on behavior under ASan/UBSan. Depth slides, pitch
LFO, voice-driver scheduling and audible synthesis remain pending.

The Disc 1 data audit now runs every full sequence for up to 4,096 forced
musical ticks, beyond the initial prefix. It records 2,447,792 ticks and
1,077,926 note events: 589 sequences remain active at the limit, while ten
hit unsupported commands (one B4 and nine CC). No sequence stops within this
window. This diagnostic deliberately reports missing work; a passing data
audit does not prove full songs or audio fidelity. Voice-driver modulation,
register application and synthesis are not exercised by these musical ticks.

CC sets the original phrase phase used for legato: first-note key-on is
preserved and subsequent notes can avoid retriggering. CD is the original
empty handler and consumes no operands. Timing tests verify both commands
and key-on behavior under ASan/UBSan. The extended audit continues to report
later missing commands explicitly; these changes do not prove audible legato.

B4/B5/B6 now initialize, adjust and reset pitch LFO. Depth calculation retains
both original scaling modes and the low-word scale used at initialization.
The separate voice step preserves delay gating, key-on bypass, signed samples
and the original doubling of nonnegative results. Tests cover both depth
modes, zero-as-256 duration, delay, signed output and reset under ASan/UBSan.
All 21 tests pass headless with Disc 1. Its 4,096-tick audit now reaches DD in
one sequence; 598 remain active at the limit. Depth slides, complete voice
scheduling/application and synthesized reference audio remain unfinished.

DD now configures pitch-LFO depth slides, including zero duration as 256.
Voice updates advance the wrapped depth target, recalculate scale and resample
the pending waveform value without advancing its cursor between phase steps.
Tests verify descending slides, resampling and completion under ASan/UBSan.
All 21 tests pass headless with Disc 1. The 4,096-tick musical audit now reports
one stopped sequence, 598 still active and no missing commands (2,449,892
ticks and 1,079,209 note events). This bounded sequencer audit does not exercise
the voice-step schedule or prove full songs, synthesized audio or gameplay.

Set `PE_AKAO_AUDIT_TICKS` to a value from 4 to 65,536 to extend the room/audio
audit (default 4,096). The 65,536-tick Disc 1 run executes 38,651,480 musical
ticks and 17,315,934 note events, with ten sequences stopped, 589 still active
and no command/state errors. This remains a bounded sequencer test: active
loops are not declared completed, and LFO voice updates, synthesis and audio
comparison are still required.
All 21 tests pass headless with the extended setting; the same 65,536-tick
room/audio audit also passes ASan/UBSan with identical counts.

DE/DF now configure volume and auxiliary LFO depth slides. Voice updates
advance their wrapped targets and resample modulation between waveform steps
without consuming a sample. Tests verify simultaneous descending slides,
signed values, unchanged waveform cursors and completion under ASan/UBSan.
All 21 tests pass headless with Disc 1. These operations still await the full
voice-driver schedule and synthesis integration before audible validation.

`pc/music.c` adds transactional primary music loading by the room record key.
The owner copies sequence bytes, resident samples and decoded grammar, uploads
the referenced sample bank and starts its native player at a stable address.
It survives room/disc/resident-bank destruction; a failed key lookup preserves
the previous music. The Disc 1 test executes 4,096 further ticks after closing
and freeing those sources. Script command dispatch, positional audio reset,
nested banks and audible playback remain to be integrated.
The test obtains the actual requested music key from Tutorial M353's initial
script command. All 21 tests pass headless; the ownership/data audit also
passes ASan/UBSan.

The primary player now implements F0 release behavior used by scene audio
reset: request key-off for all 24 voices, set release rate 5 while preserving
release mode, and route channels to the original stop behavior after three
due musical ticks. Inactive banks remain unchanged. Player tests verify this
timing and retained control state under ASan/UBSan; all 21 tests pass headless
with Disc 1. Secondary banks, script reset dispatch and actual envelope decay
in synthesized audio remain to be integrated.

`pc/scene_audio.c` dispatches actor opcode 234 / audio D9 to native reset:
clear both slot key pairs and bank IDs, clear the original `0xf0` state bits,
and request F0 release on attached players. The data test dispatches the real
New Game M431 command after closing Disc 1, verifies slot/flag state and lets
the owned Tutorial music stop after three further ticks. Other script audio
commands, full scene execution and audible output remain unfinished.
The full 21-test headless suite passes with Disc 1; the scene/audio data test
also passes ASan/UBSan, including unchanged state for unsupported dispatch.

Scene audio dispatch now handles a fresh primary C8 start from the original
Tutorial M353 command: resolve the key, load owned music, return its bank ID
through the decoded result operand and register slot/volume flags. Repeating
an already-playing key preserves player/result state; a missing key follows
the original no-op path. Tests use actual decoded operands and verify these
cases. Cached/secondary restarts and same-ID queue ordering remain explicit
unsupported paths, alongside full scene execution and synthesized playback.

The scene data test now routes the real initial Tutorial/New Game task ticks
through audio dispatch, rather than invoking only the handler. Tutorial starts
music and then reports pending script audio CD at `0x19a0`; New Game resets
audio and reports actor opcode 170 at `0xac`. These expected diagnostics expose
unfinished scene integration. Missing-key tests re-decode modified literal
operands before dispatch. All 21 tests pass headless with Disc 1; this scene
audit also passes ASan/UBSan. No full scene/gameplay completion is claimed.

Actor opcodes 170/171 now set/clear game-state flag `0x2000`, using a separate
native word from script global flags (81–83). Scene audio can bind this shared
game-state word instead of keeping a separate copy. Tests verify preservation
of unrelated flags and the reset→set sequence from the real New Game script,
which now reaches unsupported opcode 136 at `0xb4`. All 21 headless tests pass
with Disc 1; VM and scene/audio data tests also pass ASan/UBSan.

Actor opcode 136 clears bit `0x40` of the shared screen fade mode. Despite its
reference handler name, it does not modify actor flags. VM tests verify the
other fade bits and actor flags are preserved; the real New Game task confirms
the state change before reporting the next unsupported opcode. All 21 tests
pass headless with Disc 1 and VM tests pass ASan/UBSan. Applying fade state to
scene rendering remains unfinished.

Actor opcode 28 queues script events, despite its reference SFX name. The
native queue retains the original 28-entry limit and byte/halfword arguments,
plus the sender's actor ID. Scheduler draining matches type/sub-ID or an actor
selector, creates group-2 tasks with event argument/value and flag 4, and checks
cursors and capacity before allocating. Tests cover multiple receivers,
selector routing and exhaustion without partial delivery under ASan/UBSan.
Room actor IDs and task sequences now start at one as in Draw_InitPipeline.
The New Game task reaches the next unsupported audio command at `0x394`;
scene-loop event delivery and physical actor simulation remain unfinished.

Scene audio CD now applies immediate bank volume selected through the cached
key/handle. It clears the volume-slide duration, marks active voices dirty and
registers the byte-sized slot volume/flag. Tests distinguish the 7-bit control
gain from the retained slot byte and verify missing-key no-op behavior. The
real Tutorial task now reaches audio D0 at `0x19c0`. All 21 tests pass headless
with Disc 1 and the scene/audio audit passes ASan/UBSan; full playback and
scene execution remain unfinished.

Scene audio D0/D3/D6 now set global tempo/pitch scales as byte-masked 16.16
values and cancel their corresponding slide durations. D6 applies both
immediate changes. Tests verify independent and combined state, truncation and
the actual Tutorial D0 operand; its task now reaches actor opcode 8 at `0x19e8`.
All 21 tests pass headless with Disc 1 and the scene/audio data test passes
ASan/UBSan. Applying these globals in the full audio-driver schedule and
synthesis remains pending.

`pc/actors.c` provides the original 14-slot actor/script allocation core with
stable addresses, insertion after the spawning actor (or at the list head),
per-room IDs and initial position/gravity/movement state. Creation binds the
type's initial script task; player selection follows type zero. `actors`
tests insertion order, IDs, defaults, task borrowing, pool exhaustion and
failure when script capacity is exhausted. All 22 tests pass headless with
Disc 1; actor tests also pass ASan/UBSan. Complete opcode 8 remains unsupported
until model allocation and floor lookup are implemented; this core alone is
not a physical actor simulation or playable scene.

Actor collection now releases script tasks before recycling deleted slots,
unlinks head/middle/tail entries, clears the transient actor flag and releases
the player's movement locks as in the original. Slot reuse assigns a new ID.
Tests cover deferred deletion, complete/empty lists, reuse and corrupt links;
all 22 tests pass headless with Disc 1, and actor tests pass ASan/UBSan.
Parent hierarchies, model resources and complete scene execution remain pending.

`pc/collision_math.c` ports the original floor triangle containment test:
parity crossings, half-open edges and wrapped signed 32-bit products.
`collision-math` verifies winding permutations, single ownership of shared
edges, degenerate triangles and extreme coordinates. All 23 tests pass
headless with Disc 1; collision math also passes ASan/UBSan. Room collision
database decoding, flat/sloped floor-height lookup and actor integration
remain unfinished; this primitive does not establish working collision.

`pc/collision.c` adds an owned, checked relative collision database and the
original first-match floor lookup for flat and sloped regions. Plane height
uses wrapped 16.16 products; flags retain bypass/preserve-Y semantics. Fixture
tests verify negative/fractional heights, region order, malformed references,
transactional loading and independence from source bytes. All 24 tests pass
headless with Disc 1; collision tests also pass ASan/UBSan. Selecting the actual
room collision asset and comparing game geometry, actor integration and wall
traversal remain pending, so these fixtures do not prove game collision.

Room collision loading now resolves the actual taskData short record. Real
Disc 1 data corrected the header interpretation: floor triangle count is the
halfword at +04, plane count at +06; +08 describes the traversal bitmap.
All 414 databases decode (300 flat, 114 sloped), with 423 regions and 16,858
floor triangles. Integer centroid probes find floor for 16,827 points; the
31 misses remain visible and do not count as validated gameplay collision.
All 24 tests pass headless with Disc 1; fixtures and the full room/collision
audit pass ASan/UBSan. Actor integration and reference geometry checks remain
unfinished.

`pc/room_banks.c` binds checked room overrides into the original ten model
root slots and ten groups of 48 animation slots. It keeps record order and
last-write behavior, borrowing the scene's owned room bytes; failed binding
preserves the previous view. All 414 Disc 1 rooms validate, containing 878
root records and 9,042 animation-row records. `room-banks` tests slot/group
bounds, payload bounds and duplicate overrides. All 25 tests pass headless;
the binding fixture and full scene data audit pass ASan/UBSan. External entity
banks, model/animation decoding and rendering remain unfinished.

The 31 centroid no-matches are now classified: M90's triangles 0–30 are
absent from its initial floor-region lists (region zero is empty; region one
lists the other 75 faces). The original lookup only searches listed faces.
The audit checks this exact omission and reports zero unexplained misses;
a fixture confirms that retained triangle geometry without a region reference
does not produce a floor hit. All 24 tests pass headless with Disc 1 and the
collision fixture passes ASan/UBSan. This explains the initial data audit;
runtime geometry changes and gameplay collision remain unverified.

Native actors now apply FindFloor results to Y and both collision-face IDs,
preserving old values on bypass/no match and respecting preserve-Y flags.
Scenes own collision data transactionally alongside their room resources;
failed transitions retain the previous geometry. Headless boot emits
`room.collision.ready`, asserted by both New Game and Tutorial replays.
All 24 tests pass with Disc 1; actor and scene/collision tests pass ASan/UBSan.
Complete spawn/model setup, physical movement and gameplay collision remain
unfinished.
All 21 CTest tests pass headless with Disc 1 data; waveform traversal tests and
the full room/audio data audit also pass ASan/UBSan.
