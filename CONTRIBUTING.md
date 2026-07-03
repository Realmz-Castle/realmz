# Contributing to Realmz

Thank you for your interest in helping to preserve Fantasoft's Realmz for future generations of classic RPG enthusiasts! Before submitting a PR or a feature request issue, please read below about the project's goals and scope. If you have any questions, or if you just want to hang out, we'd love to see you in our channel [#realmz-classic-port](https://discord.gg/Q3wZEyCjUQ) on the [Realmz Castle Discord server](https://discord.gg/XCEgbDkZkE).

## Project Goals

With Tim Phillip's blessing, we've been able to make the source code for Realmz available under a [Creative Commons BY-NC-SA license](https://creativecommons.org/licenses/by-nc-sa/4.0/). This means that you can build, distribute, fork, and modify the game, _provided_ that you share your modified version under the same license, with an attribution to Tim as the original creator, and for _non-commercial purposes only_. Other than that, you are free to make Realmz your own.

Given this freedom granted to everyone, that means that this project repository must play a special role as an archive and definitive source for anyone seeking to play and build upon the classic Realmz experience from the 1990s. To that end, _the maintainers of this repository will only approve and merge PRs that align with that role_. Our primary goals are **preservation and authenticity** – anything beyond that is best left to custom forks, which you are welcomed and encouraged to create. The more versions of Realmz that exist, the better chance that it can reach a wider audience and delight people for years to come!

### Examples of Changes Considered In Scope

Considering the above project goals, some examples of proposed changes that would most likely be approved:

- Porting the game to more platforms (Linux, WebAssembly, mobile, etc.)
- Restoring original functionality (such as music playback)
- Compensating for modern systems (high-resolution mode for high-DPI displays, gamma or color balancing)
- Changes that improve code readability, such as named constants (e.g. [#183](https://github.com/Realmz-Castle/realmz/issues/183))
- Fixing crashes
- Graphics changes that bring the UI closer to that of the last officially released versions for Macintosh and Windows
- Improvements to the non-original code (all of the C++/Objective-C code)
- Game logic fixes **that restore the clear original intent**. See [#180](https://github.com/Realmz-Castle/realmz/issues/180) for further discussion. For example:
  - Fixing inconsistent use of monster, PC, or item attributes (e.g. [#192](https://github.com/Realmz-Castle/realmz/pull/192))
  - Fixing obvious programming errors (e.g. [#188](https://github.com/Realmz-Castle/realmz/pull/188))
  - Fixing logic to match the described behavior from item, class, monster, or scenario data, or the manual

### Examples of Changes Considered Out of Scope

Here's a non-exhaustive list of example changes that would not be approved and should be made in a forked version of the repo:

- Augmenting the scenario engine (supporting new APs, new encounter types, or battle setups)
- Improving the interface (mouse wheel support, continuous scrolling in shop/trade, new shortcuts or buttons)
- Logic changes that seek to rebalance the game
- Multiplayer support
- Logic fixes for which the original intent is not clear

## Guidelines for Submitting a Pull Request

If you have a proposed set of changes, please fork the repo, push your changes to a feature branch on your fork, then [create a pull request](https://github.com/Realmz-Castle/realmz/compare). Some general guidelines:

- Please note that if your PR is accepted and merged, _your contribution will assume the same [license](https://github.com/Realmz-Castle/realmz#License-1-ov-file) as the repo_ so that it can be distributed freely along with the rest of the game. **By submitting a PR for review, you acknowledge this and grant permission to the project maintainers to do so**.
- Include a detailed PR description that outlines what the changes do and why they advance the goals of the project.
- If the PR addresses an issue, please link to the issue in either the PR title or description.
- AI-assisted code is allowed, however, you must first review it for conciseness and correctness. The maintainers reserve the right to peremptorily reject low-quality or overly verbose generated PRs.
- Any changes made to the original Realmz C code in the `src/realmz_orig` directory must be indicated by surrounding them with comments in the following format. An [example](https://github.com/Realmz-Castle/realmz/blob/fc143ecb7d54b1f7be3ff7e714fea450297b8bb9/src/realmz_orig/warn.c#L61).

```
    /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
     * NOTE(<your username>): <explanation of the changes>
     */
    ...
    /* *** END CHANGES *** */
```
