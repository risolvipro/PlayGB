<p>
<img src="assets/playgb-logo-2x.png?raw=true" width="200">
</p>

## PlayGB

A Game Boy emulator for Playdate. PlayGB is based on [Peanut-GB](https://github.com/deltabeard/Peanut-GB), a header-only C Gameboy emulator by [deltabeard](https://github.com/deltabeard).

## Installing

<a href="https://github.com/risolvipro/PlayGB/releases/latest"><img src="assets/playdate-badge-download.png?raw=true" width="200"></a>

* Download the zip from the [latest release](https://github.com/risolvipro/PlayGB/releases/latest).
* Copy the pdx through the [Web sideload](https://play.date/account/sideload/) or USB.
* Launch the app.
* Connect Playdate to a computer, press and hold `LEFT` + `MENU` + `LOCK` at the same time in the homescreen. Or go to Settings > System > Reboot to Data Disk.
* Place the ROMs in the app data folder, the folder name depends on the sideload method.
    * For Web sideload: `/Data/user.*.com.risolvipro.playgb/games/`
    * For USB: `/Data/com.risolvipro.playgb/games/`
* Filenames must end with `.gb` or `.gbc`

## Notes

* Use the crank to press Start or Select.
* To save a game you have to use the save option inside that game. A sav file is automatically created when changing ROMs or quitting the app. After a crash, a new `(recovery).sav` file is created. Save files are stored in `/Data/*.playgb/saves/`
* Audio is disabled by default. You can optionally enable it from the library screen

## Implementation

PlayGB uses a slightly modified version of Peanut-GB which supports partial screen update.