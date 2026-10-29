# HD backgrounds — work in progress

This branch prototypes optional replacement backgrounds in the original room
renderer. Enable a locally generated pack with `PE_HD_BACKGROUNDS=/path/to/pack`.
Missing assets retain original rendering. The experimental atlas path currently
requires SDL_GPU; this is not a release feature.

The Day 1 study covers M001–M037: 33 maps with 2D layers, 43 views and 62 original
animation controllers. Original tile coverage, transparency, depth, timing,
scripts, native VRAM and audio remain authoritative. Atlas storage is recycled
at room transitions.

Offline study scripts are retained under `docs/assets/hd-poc/` and
`docs/assets/hd-day1/`. They require local extracted inputs, generated references,
model weights and recorded replay fixtures; some paths are still workstation
specific. Images, packs, disc data and private test captures are deliberately
excluded from Git. The private checkpoint includes these files and their
provenance; the scripts alone are not a downloadable HD pack.

The current M037 approach keeps one cabin/mirror frame and reconstructs street
and mirror-reflection motion from all 19 original phases. Yellow road markings
are source driven. Static pillar layers use their own original material rather
than phase-zero street colors. This prevents background lights from becoming
part of the car silhouette. M027 uses source-guided movement of one HD water
material. These remain visual research candidates.

Local validation reached Day 2 through 59 checkpoints with identical original
state, native VRAM and continuous PCM. That route visits 25 of the 33 maps;
optional maps such as M037 require separate packed-composition and offscreen
preview checks. A visual review is still necessary: numeric parity does not
prove artistic fidelity.

## Lossless PNG distribution packs

The SDL_GPU atlas loader accepts PNG and BMP. The game prefers `<key>.png`, then
tries `<key>.bmp`, retaining native rendering if neither atlas loads. PNG changes
storage only: decoded pixels, native reference colors, coverage and shader input
remain the same. GPU texture memory is unchanged.

After assembling and reviewing the BMP working pack, create a separate distribution
pack with Python and Pillow:

```
python docs/assets/hd-poc/compress_pack.py SOURCE DESTINATION --version 0.1
```

The converter checks source hashes and compares every decoded PNG pixel with its
source. It records file hashes, decoded-pixel hashes and compression totals in the
manifest. An existing destination is rejected; a failed conversion removes its
staging directory. Original BMP working packs and reviewed correction overrides
remain intact. The current combined 4069-atlas pack shrinks from 8,962,308,558 to
1,375,341,132 image bytes (about 85% smaller), with identical decoded pixels.

Offline converter regression: `python tests/hd_pack_png.py` (requires Pillow).
The PsyZ GPU regression compares PNG/BMP rendered pixels and native VRAM; the
original-game HD integration test can use either pack format through
`PE_TEST_HD_BACKGROUNDS`.
