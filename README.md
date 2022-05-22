<p>
<img src="assets/playgb-logo-2x.png?raw=true" width="200">
</p>

## PlayGB

A Game Boy emulator for Playdate. PlayGB is based on [Peanut-GB](https://github.com/deltabeard/Peanut-GB), a header-only C Gameboy emulator by [deltabeard](https://github.com/deltabeard).

## Installing

<a href="https://github.com/risolvipro/PlayGB/releases/latest"><img src="assets/playdate-badge-download.png?raw=true" width="200"></a>

* Download the zip from the [latest release](https://github.com/risolvipro/PlayGB/releases/latest).
* Copy the pdx through the [online sideload](https://play.date/account/sideload/) or USB.
* Place the ROMs in `/Data/*.playgb/games/`. Filename must end with `.gb` or `.gbc`
* To access Data folder, connect Playdate to a computer and  press `LEFT` + `MENU` + `LOCK` in the homescreen.

## Notes

* Use the crank to press Start or Select.
* To save a game you have to use the in-game save option. Game is automatically saved when changing ROMs or quitting the app. After a crash, a new `(recovery).sav` file is created. Save files are stored in `/Data/*.playgb/saves/`
* Audio is disabled by default. You can optionally enable it from the ROMs list screen

## Implementation

PlayGB uses a slightly modified version of Peanut-GB which supports partial screen update.