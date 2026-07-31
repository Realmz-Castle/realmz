## [v8.1.0-beta4](https://github.com/Realmz-Castle/realmz/releases/tag/v8.1.0-beta4)

- stop character hiding on empty slot click by @chromancer in https://github.com/Realmz-Castle/realmz/pull/226
- Change TickCount() to wall clock rather than clock() by @chromancer in https://github.com/Realmz-Castle/realmz/pull/-29
- icons restored to Mac menus and dropdowns by @chromancer in https://github.com/Realmz-Castle/realmz/pull/230
- enable PC magic resistance by @chromancer in https://github.com/Realmz-Castle/realmz/pull/231
- add item marks & remove sync-on-change hangs by @chromancer in https://github.com/Realmz-Castle/realmz/pull/235
- menucontroller gets automatic memory management back by @chromancer in https://github.com/Realmz-Castle/realmz/pull/-42
- doc: add contribution guidance by @jpetrie in https://github.com/Realmz-Castle/realmz/pull/233
- bump SDL to 3.4.10 by @chromancer in https://github.com/Realmz-Castle/realmz/pull/250
- Use the Control key as Command on Windows by @afkelsall in https://github.com/Realmz-Castle/realmz/pull/245
- Fix total party level handling. by @jpetrie in https://github.com/Realmz-Castle/realmz/pull/239
- fix: skip incorrectly-sized character files during party selection by @jpetrie in https://github.com/Realmz-Castle/-ealmz/pull/232
- Fix Open Lock spell chance by @iSynic in https://github.com/Realmz-Castle/realmz/pull/252
- Honor disabled Windows menu state by @iSynic in https://github.com/Realmz-Castle/realmz/pull/267
- Apply weapon magic plus to player melee to-hit by @afkelsall in https://github.com/Realmz-Castle/realmz/pull/268
- Load saved games in windows from the original retail Realmz on MacOS 9 by @afkelsall in https://github.com/-ealmz-Castle/realmz/pull/241
- Fix shop scrollbars and live thumb drag by @afkelsall in https://github.com/Realmz-Castle/realmz/pull/243
- Port menu by @chromancer in https://github.com/Realmz-Castle/realmz/pull/256
- Use the original Black Chancery font and baseline-anchor pen text by @afkelsall in https://github.com/Realmz-Castle/-ealmz/pull/251
- reinstate "movie" scroll and respect alpha by @chromancer in https://github.com/Realmz-Castle/realmz/pull/257
- stop two more crashes in party select by @chromancer in https://github.com/Realmz-Castle/realmz/pull/258
- Fix Drop All Equipment inventory handling by @iSynic in https://github.com/Realmz-Castle/realmz/pull/282
- Fix fumbled item drops by @iSynic in https://github.com/Realmz-Castle/realmz/pull/270
- Update changelog for beta3. by @jpetrie in https://github.com/Realmz-Castle/realmz/pull/273
- Automatically copy resource file dependencies. by @jpetrie in https://github.com/Realmz-Castle/realmz/pull/272
- Add Contribution Guide by @danapplegate in https://github.com/Realmz-Castle/realmz/pull/283
- Honor synchronous SndPlay requests by @iSynic in https://github.com/Realmz-Castle/realmz/pull/253
- Support legacy scenario resource sidecars by @iSynic in https://github.com/Realmz-Castle/realmz/pull/281
- Fix class 6 spells like Cosmic Blast always being resisted by @afkelsall in https://github.com/Realmz-Castle/realmz/-ull/276
- fix monster DRV and immunity indexing in savevs by @chromancer in https://github.com/Realmz-Castle/realmz/pull/288
- Fix non-functional volume/speed menus on Windows by @afkelsall in https://github.com/Realmz-Castle/realmz/pull/236
- Show the dragged item icon in the shop by @afkelsall in https://github.com/Realmz-Castle/realmz/pull/244
- Fix crash plotting cicn icons that lack a monochrome bitmap by @afkelsall in https://github.com/Realmz-Castle/realmz/-ull/266
- calculate all equipment bonuses correctly by @chromancer in https://github.com/Realmz-Castle/realmz/pull/290
- fix attack & defense bonus in UI by @chromancer in https://github.com/Realmz-Castle/realmz/pull/291
- Fix spell info lights by @chromancer in https://github.com/Realmz-Castle/realmz/pull/297
- Fix on-portrait anims (spells, pots, selection) by adding missing leftshifts by @chromancer in https://github.com/-ealmz-Castle/realmz/pull/295
- Fix combat crash from out-of-range body id in centerfield by @afkelsall in https://github.com/Realmz-Castle/realmz/-ull/277
- Gray out disabled items in Windows popup menus by @afkelsall in https://github.com/Realmz-Castle/realmz/pull/264
- Draw dBoxProc dialog frames by @iSynic in https://github.com/Realmz-Castle/realmz/pull/279
- Handle missing resources consistently by @iSynic in https://github.com/Realmz-Castle/realmz/pull/285
- Fix EDIT_TEXT caret rendering by @iSynic in https://github.com/Realmz-Castle/realmz/pull/298
- Fix the post-fight treasure screen by @afkelsall in https://github.com/Realmz-Castle/realmz/pull/263
- Restore 3D dungeon view presentation by @iSynic in https://github.com/Realmz-Castle/realmz/pull/303
- add marks to Mac popups by @chromancer in https://github.com/Realmz-Castle/realmz/pull/309

## [v8.1.0-beta3](https://github.com/Realmz-Castle/realmz/releases/tag/v8.1.0-beta3)

- Throttle key repeats to avoid sluggish gameplay. by @jpetrie in #201
- Configure Windows Installer Build Target by @danapplegate in #205
- fix multiple issues; closes #116 #151 #153 #154 #166 #203 by @fuzziqersoftware in #207
- Use userdata directory for preferences file by @danapplegate in #204
- fix MCSync getting slower after playing for a while by @fuzziqersoftware in #209
- eliminate Data CD file by @fuzziqersoftware in #208
- fix location of type indicators on spell selection window by @fuzziqersoftware in #215
- update Sword Lands scenarios to latest versions by @fuzziqersoftware in #216
- implement menu item enabled state in popups by @fuzziqersoftware in #217
- fix long vs. int32_t in save file format by @fuzziqersoftware in #214
- use enums for character, monster, and party conditions; closes #210, closes #212 by @fuzziqersoftware in #213
- fix multiple minor/rare issues by @fuzziqersoftware in #218
- save storeditems flag in save file by @fuzziqersoftware in #220
- fix fumble command edge case in battle by @fuzziqersoftware in #221
- Fix Windows Cross-Compilation Build by @danapplegate in #222
- fix hang in Name modal on Generate New Character by @chromancer in #224
- Show bundled characters in party select, fix resulting Data CD crash by @chromancer in #225

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
