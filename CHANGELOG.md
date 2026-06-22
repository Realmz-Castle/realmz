## Unreleased

- Clean up the item-grab sparkle on the post-fight treasure screen. The sparkle erases itself with XOR ovals that no longer cancel exactly here (the port's ovals are antialiased), leaving stray coloured pixels in the emptied cell, including past the right edge of the grid where the sparkle spills beyond its cell. The area it reaches is now repainted from the item list afterward so it no longer leaves leftovers behind.
- Stop dropping clicks made in quick succession on the treasure screen. The event loop no longer pauses between clicks while items are still queued, and grabbing items in quick succession skips the per-item pickup flourish, so rapid clicking is no longer held up waiting for each animation to finish.
- Stop the text blip from sounding when the cursor leaves the treasure area in the larger window layouts. The item preview drawn while hovering an item was clearing the flag that keeps that sound quiet during the treasure screen.

## [v8.1.0-beta2](https://github.com/Realmz-Castle/realmz/releases/tag/v8.1.0-beta2)

- Add CMake presets for macOS by @jpetrie in #167
- fix multiple text rendering issues; closes #164 #165 by @fuzziqersoftware in #170
- fix window ordering during 3D dungeon battles; closes #104 by @fuzziqersoftware in #173
- Fix some warnings by @jpetrie in #174
- Install runtime dylibs in bundle, resolves #171 by @danapplegate in #175
- Check the right condition when finding secrets. by @jpetrie in #186
- support menu keyboard shortcuts on macOS; ref #151 by @fuzziqersoftware in #182
- Use open versions of Chicago and Geneva fonts by @danapplegate in #189
- Update README Instructions by @danapplegate in #181
- Do not double-apply special attacks by @danapplegate in #188
- Fix some misaligned evil monster checks by @danapplegate in #191
- Fix misaligned reptilian monster check by @danapplegate in #192
- Fix some more misaligned attack checks by @danapplegate in #194
- Fix immunity checks when a monster attacks another monster by @danapplegate in #195
- Apply base to-hit and damage bonuses before aging. by @jpetrie in #197
- Fix Windows Mouse Click Offset by @danapplegate in #150
- implement volume menu; closes #160 by @fuzziqersoftware in #196

## [v8.1.0-beta](https://github.com/Realmz-Castle/realmz/releases/tag/v8.1.0-beta)

- Initial release of the Realmz Classic project
- Implement Classic MacOS system functionality with SDL3-backed replacement code
- CMake based build system
- Native resource fork management provided by [ResourceDASM](https://github.com/fuzziqersoftware/resource_dasm)
- See sections labeled "CHANGED FROM ORIGINAL IMPLEMENTATION" for detailed changes required
- [Full changelog](https://github.com/Realmz-Castle/realmz/releases/tag/v8.1.0-beta)
