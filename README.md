# Jukebox

<img src="./logo.png" alt="NONGD logo" />

The Jukebox Mod is a song manager for Geometry Dash. The primary goal is simplifying the process of swapping Newgrounds songs with any NONGs.

## What is a NONG, anyway?

NONG stands for **Not On NewGrounds**. Basically, it means any song that is not on Newgrounds that was replaced manually through the game files. 

NONGs have always been a hassle to manage, because some level creators use popular Newgrounds song IDs and replace them with a NONG. So you have to swap those song files around quite a bit if you play a level with the Newgrounds song and a level with a NONG song.

## Start your jukebox!

The Jukebox Mod makes the process of managing your songs a breeze. You can download your NONGs using your method of choice. A recommandation of mine is [yt-dlp](https://github.com/yt-dlp/yt-dlp), a CLI application. After getting your MP3 file, you can enter a song and author name, for easier management.

> Note that Jukebox copies imported MP3 files in the storage location designated by Geode. You can open this folder from ingame.

Alternatively, you can download songs from **Song File Hub**. I have added a button that opens their site in your browser.

## So, how do I begin?

You can open up the Jukebox menu form any Level page. Just click on the song name, and either a song list (if the level has multiple songs), or the song management screen (if the level only uses 1 song) will open. From here, you can add, remove and swap songs.

## Reporting bugs

You can report bugs on the [Jukebox Discord server](https://discord.gg/SFE7qxYFyU)

## The API

As of v2.6.0, Jukebox includes an experimental API to interface with installed NONGs. This API is very barebones at the monent and subject to changes. Any updates will be announced on the discord server!

At its base, the API uses the **SongInfo** struct for storing song data. You can add / set an active NONG by passing instances of this struct. Note that for setting an active NONG, the path passed inside the object has to match a path stored for the song ID.

Here is a small example:

```cpp

#include <fleym.nongd/jukebox.hpp>

void my_cool_function() {
    // I want to create a new NONG for song ID 100

    SongInfo my_nong;
    my_nong.authorName = "Artist";
    my_nong.songName = "My Cool NONG!";
    my_nong.path = "ENTER_ABSOLUTE_FILEPATH_TO_THE_MP3";

    jukebox::addNong(my_nong, 100);

    // To set it as active, we need to call setActiveSong
    jukebox::setActiveSong(my_nong, 100);
}

```

Thanks to [Flafy](https://github.com/FlafyDev) for starting work on the API!

## Credits

- The Geode team, for creating such an amazing toolkit