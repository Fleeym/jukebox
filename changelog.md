# Changelog

## v2.2.0

 * Removed Song File Hub integration

## v2.1.5

 * Reenable manual song add on Android

## v2.1.4

 * Temporarily disable manual song add on Android
 * Fix touch priority issues in the song list

## v2.1.3

 * Fix buttons not working in the song list

## v2.1.2

 * Fix crashes on Android
 * Fix manual song add on Android

## v2.1.1

 * Fix the Robtop Music Library being... a little weird
 * Fix the random Error text that would appear in the song widget sometimes
 * 0.0B fix now accounts for songs that are included with the game (GD/Resources/songs folder)
 * Fix some editor song select issues
 * Android support (experimental)

## v2.1.0

 * Correctly disable nongs for levels that have robtop levels
 * Fix a crash that happened when entering a level with an invalid song id
 * Store level name separate from song name (and display it in the song list)
 * Store song data as minified json
 * Fix 0.0B on multi asset levels (experimental)

## v2.0.1

 * Fix a crash that happens when entering a level with song info data not fetched

## v2.0.0

 * Add 2.2 support
 * Add support for levels with multiple songs (experimental)
 * Optimizations and bug fixes
 * Rebranding!

This release is only available on Windows, next one should be available on Android too. (Sorry android fellas)

## v1.2.3

 * Fix crash when adding a NONG manually for the first time

## v1.2.2

 * Fix crashes on Android

## v1.2.1

 * Increased the Z Layer for the NONG popup

## v1.2.0

 * Replace old popup with a layer that fits the game more
 * Recompile for Android NDK r26b

## v1.1.1

 * Add experimental Android support

## v1.1.0

 * Changed the song size label to show N/A instead of 0.00MB for songs that are missing their file
 * Added a setting that prevents mashup downloading from Song File Hub
 * Added a "Remove All" button to the NONG list
 * Fixed aspect ratio issues in the popups
 * Copy locally added nongs to the mod storage instead of using the file provided by the user
 * Created a manifest system to track JSON structure updates
 * Added a button that opens the settings page in the nong popup

## v1.0.6

 * Fixed a bug that prevented saving songs to disk if the Song File Hub name contained Unicode characters

## v1.0.4

 * Switched to using the new API for Song File Hub
 * Gave the add song popup some elasticity

## v1.0.3
 
 * Removed all filters for the song file picker so that MacOS can actually use it.

## v1.0.2

 * Fixed a crash that occured by pressing ESC while downloading a NONG
 * Disable NONGd for levels that use Robtop level songs
 * Update json impl to match the new json library version API
 * Add a small indicator to the song label to hint that you can click it
 * Fixed text inputs for Geode v1.0.0-beta.14

## v1.0.1

 * Mod can now build on MacOS

## v1.0.0

 * Implement async file downloads
 * Fix some crashes
 * Only download one song at a time instead of downloading all of them
 * Fix size label showing 0.00mb for undownloaded newgrounds songs
 * Reduce calls to updateSongObject

## v1.0.0-beta.3

 * Fix the invalid SFH download popup being positioned weirdly
 * Update the Custom Song Widget on every nong update
 * Use layouts for list cells
 * Try to fix unicode not being parsed correctly
 * Fix nongd folder not being created properly on some occasions

## v1.0.0-beta.1

 * Initial version