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
