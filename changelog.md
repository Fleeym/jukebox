# Changelog

All notable changes to this mod will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- Saved manifest now stores filenames, making it easier to port Jukebox data across systems
- Skip index downloading if there's no new version available
- Load indexes from local cache if fetching them fails as a failsafe
- Add "Clear caches" button in the settings

### Changed

- Internal code cleanup
- Improve index loading UX in the NONG popup
- Use "Keep a Changelog" format for the changelog

## [3.7.1] - 2026-09-04

### Added

- Configurable timeout for non-download web requests
- Log some FMOD related issues in the console

### Changed

- Update `about.md` Discord invite link
- Temporarily disable autocomplete metadata, as it is quite broken

### Fixed

- Fix 'You selected a directory' bug

## [3.7.0] - 2026-09-04

### Added

- Simple NONG search (thanks to @MalikHw for the implementation)

### Changed

- Small refactor (thanks to @slideglide)
  - Migrate to Geode components for text, backgrounds and disk I/O
  - Optimize string operations
  - Optimize data saving
  - Other smaller optimizations
  - Performance impact should be negligible, but Geode's components are easier
    to use
- Bump Geode to v5.10.1

### Fixed

- Fix back button in the NONG list being placed wherever it vibed that day

## [3.6.2] - 2026-04-05

### Fixed

- Fix index songs not loading on newly initialized song IDs

## [3.6.1] - 2026-04-02

### Changed

- Try to print more useful information on internal cURL errors
- Optimize index caching a little

## [3.6.0] - 2026-03-06

### Added

- Support for most song formats (except .opus, which sadly isn't usable by GD)

### Changed

- Optimize 0.0B fix a lot
- Optimize autocomplete metadata parsing
- Bump Geode to v5.3.0

### Fixed

- Fix a crash that sometimes happened when loading a custom level screen

## [3.5.1] - 2026-03-02

### Fixed

- Fix featured song effect positioning

## [3.5.0] - 2026-02-23

### Changed

- Port to Geometry Dash 2.2081 and Geode v5.0.0
- Optimizations to song downloads and metadata fetching
- Optimizations to memory operations
- Updated Song File Hub discord link

### Fixed

- Fix outdated tutorial
- Fix NONG list lines dissapearing after state update
- Fix "Delete all NONGs" button not updating UI properly
- Fix editor touch issues

## [3.4.0] - 2025-06-28

### Added

- "Paste" button in `NongAddPopup` for mobile platforms
- Support for patchless (JIT-less) platforms

### Fixed

- Fix macOS file picker (thanks calum12345)
- Fix downloaded songs not being set as active if the NONG popup was closed before the download was finished

## [3.3.0] - 2025-06-22

### Added

- Implement a "verified" song system, helping with finding which NONG is needed for a certain level
- Log a warning when default indexes provided by Jukebox are disabled

### Changed

- Use the more performant `SimpleAxisLayout` for the interface
- Rewrite index JSON storage

### Fixed

- Fix `NongAddPopup` not checking file extension as lowercase
- Fix path not being set when editing a NONG

## [3.2.1] - 2025-05-03

### Added

- Experimental iOS support!

## [3.2.0] - 2025-04-20

### Added

- Configurable request timeout for downloads

### Changed

- Improve 0.0B fix, taken out of Experimental settings

### Removed

- Remove usage of a custom label for the song ID / size label

### Fixed

- Remove all usage of std::stoi, fixing some crashes in the process
- Fix undownloaded songs not being marked as downloaded when switching to a NONG

## [3.1.1] - 2025-03-22

### Changed

- Do some internal refactoring of the mod, shouldn't break anything
- Bump to Geode v4.3.1

### Removed

- Remove the api key from mod.json as the mod doesn't expose an api anymore

### Fixed

- Fix a crash that can happen when Show Audio Assets by Taswert is enabled
- Fix Overcharged Main Menu showing NONG names for main levels

## [3.1.0] - 2025-01-26

### Added

- Better error for failed downloads with status code 502
- Add MIT License to the mod

### Changed

- Remove original Discord server button and replace it with Song File Hub
- Make newly downloaded songs automatically set as active
- Bump to Geode v4.2.0

## [3.0.3] - 2025-01-06

### Changed

- Simplify default song fixing - less bugs!
- Rewrite index loading - optimizations and bug fixes
- Let `matjson` handle json file reading completely

### Fixed

- Fix a typo in the mod settings
- Fix 2 rare crashes caused by pointer shennanigans
- Fix an incompatibility with Improved Song Browser by alphalaneous

## [3.0.2] - 2024-12-20

### Changed

- Bump Geode to v4.1.1

### Fixed

- Fix file paths not being read correctly when adding NONGs manually
- Fix a crash that sometimes happened when loading levels with official songs
- Fix a crash related to having Windows in a different language than English
- Fix a misstype in the "song refetched" popup
- Fix local song editing not actually editing a path
- Fix IndexManager loading the wrong indexes for hosted
- Fix a typo in the mod settings
- Fix manually added hosted songs not downloading
- Fix UI not being updated correctly when changing NONGs on official songs

## [3.0.1] - 2024-11-19

### Fixed

- Fix index songs not appearing on first launch when migrating from v2

## [3.0.0] - 2024-11-19

### Added

- Support for Geometry Dash 2.2074
- Song File Hub support back!
- Merge features with Auto Nong (the index system) - thanks to Flafy for helping merge the mods and agreeing to help develop the mod
- MacOS support - thanks to hiimjustin for porting the mod

### Changed

- Small tweaks to the NONG list UI
- Small UX improvements to the NONG list - the list doesn't scroll to the top for any action
- Change the way songs are stored internally, old data will migrate over
- Lots of miscellaneous fixes and improvements

## [2.11.0] - 2024-06-30

### Added

- Metadata parsing for autocompletion of song info (experimental)
- "song offset" option for importing nongs (thanks Flafy)

### Changed

- Small visual improvements to the app song popup
- Bump to Geode `v3.1.1`

### Fixed

- Fix "Level name" doing absolutely nothing in the add popup

## [2.10.1] - 2024-06-20

### Removed

- Remove debug logs

### Fixed

- Fix visual bugs

## [2.10.0] - 2024-06-15

### Added

- 2.206 support

### Changed

- Redesign the NONG UI
- Bump to Geode v3.0.0-beta.1
- Whatever other migrations were needed in the process

### Fixed

- Make a fix for non-ascii user dirs on Windows
- Fix crash on editor settings (thanks Flafy)
- Fix delete song for default songs (thanks Flafy)
- Fix deleting songs not actually working (thanks Flafy)

## [2.9.0] - 2024-05-31

### Added

- `jukebox::getActiveNong`, `jukebox::deleteNong` and `jukebox::getDefaultNong` to the API (thanks Flaafy!)
- A `Ref` `CustomSongWidget` param in `jukebox::setActiveNong` for updating the UI (thanks Flaafy!)

### Changed

- Bump Geode version to v2.0.0-beta.27

## [2.8.0] - 2024-05-18

### Added

- Support for main level songs

## [2.7.1] - 2024-04-11

### Fixed

- Fix bugs

## [2.7.0] - 2024-04-09

### Added

- Song ID label to the multiple song list of the nong menu

### Fixed

- Fix a bug that made the nong menu inaccessible on levels with multiple songs, out of which one was invalid (not found on NG) - go play Golden Hope 8)

## [2.6.0] - 2024-04-04

### Added

- Expose a minimal API for interacting with NONG data (Thanks to [Flafy](https://github.com/FlafyDev))

## [2.5.2] - 2024-02-29

### Fixed

- Fix dailies and weeklies setting their song name to "Unknown"

## [2.5.1] - 2024-02-25

### Fixed

- Fix fixDefault running for every single CustomSongWidget instance

## [2.5.0] - 2024-02-24

### Added

- Button for joining the Discord server

### Changed

- Make file IO error messages more descriptive

### Fixed

- Fix the download slider being made invisible

## [2.4.0] - 2024-02-07

### Added

- Backup invalid JSONs

### Fixed

- Fix a bug that caused defaultValid to be undefined when writing to JSON
- Fix crashing if JSON is invalid

## [2.3.0] - 2024-02-06

### Changed

- Redesign NongDropdownLayer and NongAddPopup, using the new initAnchored
- Use more layouts in the UI

### Fixed

- Fix a bug that made the download bar visible after fixing the default song

## [2.2.5] - 2024-02-05

### Added

- A way to refetch default song info if it somehow is broken

### Fixed

- Prevent CustomSongWidget update after adding a new song
- Bugfixes

## [2.2.4] - 2024-02-05

### Fixed

- Try and fix crash on startup from invalid JSON

## [2.2.3] - 2024-02-04

Cocos2dx reference release.

### Changed

- Write JSON on DataSaved
- Move initial JSON read to on_mod(Loaded)

### Removed

- Remove permission check for Android

### Fixed

- Fix NongAddPopup filters
- Fix random letter generator sometimes getting \0 as a character

## [2.2.2] - 2024-02-04

### Changed

- Actually bump mod.json version

## [2.2.1] - 2024-02-04

### Added

- Copy the song ID to the clipboard when opening Song File Hub
- A label with the chosen file path in the add popup
- Ask for permissions on Android before trying to pick a song

### Fixed

- Fixed the "Unknown" bug
- Try and fix songs that have been broken by said "Unknown" bug

## [2.2.0] - 2024-01-29

### Removed

- Removed Song File Hub integration

## [2.1.5] - 2024-01-27

### Changed

- Reenable manual song add on Android

## [2.1.4] - 2024-01-26

### Changed

- Temporarily disable manual song add on Android

### Fixed

- Fix touch priority issues in the song list

## [2.1.3] - 2024-01-24

### Fixed

- Fix buttons not working in the song list

## [2.1.2] - 2024-01-24

### Fixed

- Fix crashes on Android
- Fix manual song add on Android

## [2.1.1] - 2024-01-23

### Added

- Android support (experimental)

### Fixed

- Fix the Robtop Music Library being... a little weird
- Fix the random Error text that would appear in the song widget sometimes
- 0.0B fix now accounts for songs that are included with the game (GD/Resources/songs folder)
- Fix some editor song select issues

## [2.1.0] - 2024-01-22

### Changed

- Store level name separate from song name (and display it in the song list)
- Store song data as minified json

### Fixed

- Correctly disable nongs for levels that have robtop levels
- Fix a crash that happened when entering a level with an invalid song id
- Fix 0.0B on multi asset levels (experimental)

## [2.0.1] - 2024-01-22

### Fixed

- Fix a crash that happens when entering a level with song info data not fetched

## [2.0.0] - 2024-01-22

Rebranding! This release is only available on Windows, next one should be available on Android too. (Sorry android fellas)

### Added

- 2.2 support
- Support for levels with multiple songs (experimental)

### Changed

- Optimizations and bug fixes

## [1.2.3] - 2023-11-02

### Fixed

- Fix crash when adding a NONG manually for the first time

## [1.2.2] - 2023-10-23

### Fixed

- Fix crashes on Android

## [1.2.1] - 2023-10-23

### Changed

- Increased the Z Layer for the NONG popup

## [1.2.0] - 2023-10-19

### Changed

- Replace old popup with a layer that fits the game more
- Recompile for Android NDK r26b

## [1.1.1] - 2023-10-12

### Added

- Experimental Android support

## [1.1.0] - 2023-10-04

### Added

- A setting that prevents mashup downloading from Song File Hub
- A "Remove All" button to the NONG list
- Created a manifest system to track JSON structure updates
- A button that opens the settings page in the nong popup

### Changed

- Changed the song size label to show N/A instead of 0.00MB for songs that are missing their file
- Copy locally added nongs to the mod storage instead of using the file provided by the user

### Fixed

- Fixed aspect ratio issues in the popups

## [1.0.6] - 2023-09-08

### Fixed

- Fixed a bug that prevented saving songs to disk if the Song File Hub name contained Unicode characters

## [1.0.4] - 2023-09-07

### Changed

- Switched to using the new API for Song File Hub
- Gave the add song popup some elasticity

## [1.0.3] - 2023-06-30

### Removed

- Removed all filters for the song file picker so that MacOS can actually use it.

## [1.0.2] - 2023-04-12

### Added

- A small indicator to the song label to hint that you can click it

### Changed

- Disable NONGd for levels that use Robtop level songs
- Update json impl to match the new json library version API

### Fixed

- Fixed a crash that occured by pressing ESC while downloading a NONG
- Fixed text inputs for Geode v1.0.0-beta.14

## [1.0.1] - 2023-04-09

### Added

- Mod can now build on MacOS

## [1.0.0] - 2023-04-09

### Added

- Implement async file downloads

### Changed

- Only download one song at a time instead of downloading all of them
- Reduce calls to updateSongObject

### Fixed

- Fix some crashes
- Fix size label showing 0.00mb for undownloaded newgrounds songs

## [1.0.0-beta.3] - 2023-04-05

### Changed

- Update the Custom Song Widget on every nong update
- Use layouts for list cells

### Fixed

- Fix the invalid SFH download popup being positioned weirdly
- Try to fix unicode not being parsed correctly
- Fix nongd folder not being created properly on some occasions

## [1.0.0-beta.1] - 2023-04-05

### Added

- Initial version

[Unreleased]: https://github.com/Fleeym/jukebox/compare/v3.7.1...HEAD
[3.7.1]: https://github.com/Fleeym/jukebox/compare/v3.7.0...v3.7.1
[3.7.0]: https://github.com/Fleeym/jukebox/compare/v3.6.2...v3.7.0
[3.6.2]: https://github.com/Fleeym/jukebox/compare/v3.6.1...v3.6.2
[3.6.1]: https://github.com/Fleeym/jukebox/compare/v3.6.0...v3.6.1
[3.6.0]: https://github.com/Fleeym/jukebox/compare/v3.5.1...v3.6.0
[3.5.1]: https://github.com/Fleeym/jukebox/compare/v3.5.0...v3.5.1
[3.5.0]: https://github.com/Fleeym/jukebox/compare/v3.4.0...v3.5.0
[3.4.0]: https://github.com/Fleeym/jukebox/compare/v3.3.0...v3.4.0
[3.3.0]: https://github.com/Fleeym/jukebox/compare/v3.2.1...v3.3.0
[3.2.1]: https://github.com/Fleeym/jukebox/compare/v3.2.0...v3.2.1
[3.2.0]: https://github.com/Fleeym/jukebox/compare/v3.1.1...v3.2.0
[3.1.1]: https://github.com/Fleeym/jukebox/compare/v3.1.0...v3.1.1
[3.1.0]: https://github.com/Fleeym/jukebox/compare/v3.0.3...v3.1.0
[3.0.3]: https://github.com/Fleeym/jukebox/compare/v3.0.2...v3.0.3
[3.0.2]: https://github.com/Fleeym/jukebox/compare/v3.0.1...v3.0.2
[3.0.1]: https://github.com/Fleeym/jukebox/compare/v3.0.0...v3.0.1
[3.0.0]: https://github.com/Fleeym/jukebox/compare/v2.11.0...v3.0.0
[2.11.0]: https://github.com/Fleeym/jukebox/compare/v2.10.1...v2.11.0
[2.10.1]: https://github.com/Fleeym/jukebox/compare/v2.10.0...v2.10.1
[2.10.0]: https://github.com/Fleeym/jukebox/compare/v2.9.0...v2.10.0
[2.9.0]: https://github.com/Fleeym/jukebox/compare/v2.8.0...v2.9.0
[2.8.0]: https://github.com/Fleeym/jukebox/compare/v2.7.1...v2.8.0
[2.7.1]: https://github.com/Fleeym/jukebox/compare/v2.7.0...v2.7.1
[2.7.0]: https://github.com/Fleeym/jukebox/compare/v2.6.0...v2.7.0
[2.6.0]: https://github.com/Fleeym/jukebox/compare/v2.5.2...v2.6.0
[2.5.2]: https://github.com/Fleeym/jukebox/compare/v2.5.1...v2.5.2
[2.5.1]: https://github.com/Fleeym/jukebox/compare/v2.5.0...v2.5.1
[2.5.0]: https://github.com/Fleeym/jukebox/compare/v2.4.0...v2.5.0
[2.4.0]: https://github.com/Fleeym/jukebox/compare/v2.3.0...v2.4.0
[2.3.0]: https://github.com/Fleeym/jukebox/compare/v2.2.5...v2.3.0
[2.2.5]: https://github.com/Fleeym/jukebox/compare/v2.2.4...v2.2.5
[2.2.4]: https://github.com/Fleeym/jukebox/compare/v2.2.3...v2.2.4
[2.2.3]: https://github.com/Fleeym/jukebox/compare/v2.2.2...v2.2.3
[2.2.2]: https://github.com/Fleeym/jukebox/compare/v2.2.1...v2.2.2
[2.2.1]: https://github.com/Fleeym/jukebox/compare/v2.2.0...v2.2.1
[2.2.0]: https://github.com/Fleeym/jukebox/releases/tag/v2.2.0
