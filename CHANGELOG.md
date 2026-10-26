# Changelog

## Unreleased

- Added optional 16:9 widescreen as the third Aspect ratio choice in Graphics settings (off by default). Rooms show more of their background where it is wider than the original screen, and the view stops at the background's edge at a room's end; rooms with only a 4:3 background keep even black bars, and movies stay 4:3.

## 0.4

- Press Start during a story movie to pause it and choose whether to skip. Skipping also stops music started for that movie.
- Added optional 4x SSAA antialiasing and bilinear filtering for model textures in Graphics settings. Both are off by default.
- Fixed text and image shimmering when the game switches framebuffers, at all internal resolutions.
- Fixed geometry correction and model filtering having no visible effect during gameplay.

## 0.3-alpha

- Moved the settings menu (`Esc`) to an in-window overlay rendered directly over the main game window instead of opening a separate window.
- Added 3D Geometry Correction (`PGXP`) for higher internal resolutions (`2x`–`8x`), eliminating PS1 vertex snapping and polygon wobbling while preserving seamless animated character models without jitter or seams.
- Fixed 2D sprite UV sampling at higher internal resolutions (`2x`–`8x`) so small menu fonts, save/load screen digits (`Level`, `HP`, `Time`), and HUD elements render cleanly without texel artifacts.

## 0.2-alpha

- Fixed streamed effects selecting the wrong instrument samples.
- Fixed the sound of music, footsteps, gunfire and other effects: corrected PS1 sample interpolation and the timing of sound playback.
- Restored the PS1 reverb filter for more faithful ambience and instrument sound.
- Fixed sound envelopes, pitch modulation, noise and mixing at high volumes.
- Fixed a menu selection bug.

## 0.1-alpha

- First public desktop release for macOS, Linux and Windows.
- Disc image selection, automatic disc switching, graphics, sound and control settings.
- Save editor included with the Mac download and available separately for Windows and Linux ARM64.
