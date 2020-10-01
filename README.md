# EdiZon SE

This fork is based on the foundation of EdiZon 3.1 nightly. The changes made are on the game memory hack aspect. 

Here are the added features: 
1. Range search.
2. Compare with previous value search. 
3. Bookmark memory location found. 
4. Speed enhancement to make small integer value in first search practicle.
5. Bookmark adjust to changing main and heap start address on subsequent launch of the game. Bookmark with pointer chain attached updates memory address dynamically when the chain is able to resolve into a valid memory address.
6. Extract memory address from dmnt cheat code and add it to bookmark for exploration of the memory location. 
7. Rebase feature to extract potential pointer chain form dmnt cheat code made for previous version of the game. 
8. In app pointer chain search for address on bookmark. 
9. Export dump to PC app (forked from pointersearcher 0.4) for more powerful pointer chain search. 
10. Import PC app search result for validation and testing. 
11. Create dmnt cheat code from pointer chain found.
12. Ability to detach dmnt from game process.
13. Adding/Removing conditional button to cheat code.

PS: Please refer to https://github.com/tomvita/EdiZon-SE/wiki for instructions on how to use the app. 

To establish common base for support please start from a clean boot with the latest atmosphere and only Sigpatches needed to run the game and latest releases from https://github.com/tomvita. Please state the url you downloaded from. No extra software unless it is related to the topic underdiscussoin.

Original functinality of Edizon on game save is available when launched without a game running. 

  <p align="center"><img src="https://user-images.githubusercontent.com/68505331/94226638-aa5aad00-ff2a-11ea-8b39-151c41bbc774.jpg"><br />
      <a href="https://github.com/tomvita/EdiZon-SE/releases/latest"><img src="https://img.shields.io/github/downloads/tomvita/EdiZon-SE/total.svg" alt="Latest Release" /></a>
    
    
  </p>

The following is the original readme the first two part is mostly unmodified except now you only see the save game functionality if you enter when no game is running and only the last game is display if there was a last game. To see all the games enter into "cheat" when no game is running and the next time EdiZon SE is launched all the game save will appear. 

# Overview
  EdiZon consists of 3 different main functionalities.
  - **Save file management**
    - Extraction of game saves.
    - Injection of extracted game saves (Your own and your friends save files).
    - Uploading of savefiles directly to https://anonfile.com.
    - Batch extraction of all save files of all games on the system.
  - **Save file editing**
    - Easy to use, scriptable and easily expandable on-console save editing.
      - Lua and Python script support.
    - Built-in save editor updater.
  - **On-the-fly memory editing**
    - Cheat Engine like RAM editing.
    - Freezing of values in RAM via Atmosphère's cheat module.
    - Interface for loading, managing and updating Atmosphère cheats.

  All packed into one easy to use and easy to install Homebrew.

# Images
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/assets/main_menu.jpg"></p>
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/assets/save_editor_1.jpg"></p>
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/assets/save_editor_2.jpg"></p>
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/assets/ram_editor.jpg"></p>

# Save editor Config and Script files

  To download working Editor Config and Editor Script files, visit [this repository](https://github.com/WerWolv/EdiZon_ConfigsAndScripts/tree/master)

  Check out our [Wiki page](https://github.com/WerWolv/EdiZon/wiki) for more information on how to build your own Editor Config and Editor Script files.

# How to install

  1. Download the latest release from the [GitHub release page](https://github.com/WerWolv/EdiZon/releases/latest).
  2. Unpack the downloaded zip file, put the files on your Nintendo Switch's SD card and let the folders merge.
  3. Use a free open source CFW like [Atmosphère](https://github.com/Atmosphere-NX/Atmosphere) to launch the hbmenu and start EdiZon from there.
     1. If you want to use the cheat manager you absolutely have to use [Atmosphère](https://github.com/Atmosphere-NX/Atmosphere) as only their cheats are supported.
     2. For the best experience, open the `/atmosphere/system_settings.ini` file and change `dmnt_cheats_enabled_by_default = u8!0x1` to `dmnt_cheats_enabled_by_default = u8!0x0`.


# How to compile

  1. Clone the EdiZon repo to your computer using `git clone https://github.com/WerWolv/EdiZon`.
  2. Download and install devkitA64. It comes bundled with the [devkitPro](https://devkitpro.org) toolchain.
  3. Use the pacman package manager that comes with devkitPro to download and install libNX, portlibs (`switch-portlibs`) and freetype2 (`switch-freetype`).
  4. The rest of the compilation works using the `make` command.

# Discord

  For support with the usage of EdiZon or the creation of save editor configs and scripts, feel free to join the EdiZon server on Discord: https://discord.gg/qyA38T8

# Credits

  Thanks to...

  - [devkitPro](https://devkitpro.org) for their amazing toolchain!
  - [3096](https://github.com/3096) for [save dumping/injecting](https://github.com/3096/nut)
  - [Bernardo Giordano](https://github.com/BernardoGiordano) for some code from [Checkpoint](https://github.com/BernardoGiordano/Checkpoint).
  - [SwitchBrew](https://switchbrew.org/) for the [Homebrew Launcher](https://github.com/switchbrew/nx-hbmenu) GUI and shared font code.
  - [thomasnet-mc](https://github.com/thomasnet-mc/) for most of the save backup and restore code and the updater script.
  - [trueicecold](https://github.com/trueicecold) for batch backups and the editable-only mode.
  - [onepiecefreak](https://github.com/onepiecefreak3) for the edizon debugger and LOTS of reviewing implementations.
  - [Jojo](https://github.com/drdrjojo) for the Travis CI configuration and the config creator.
  - [Ac_K](https://github.com/AcK77) for help with the server side update scripts and the EdiZon save website.
  - [jakibaki](https://github.com/jakibaki) for her massive help with the implementation of RAM editing and sys-netcheat which was used as inspiration.
  - [SciresM](https://github.com/SciresM) for the aarch64 hardware accelerated SHA256 code, his implementation of the Atmosphère cheat engine and his support during development.
  - **kardch** for the beautiful current icon.
  - **bernv3** for the beautiful old icon.
  - **All config creators** for bringing this project to life!

  <br>

  - [nlohmann](https://github.com/nlohmann) for his great json library.
  - [Martin J. Fiedler](https://svn.emphy.de/nanojpeg/trunk/nanojpeg/nanojpeg.c) for the nanojpeg JPEG decoding library.
  - [Lua](https://www.lua.org/) for their scripting language.
  - [Python](https://www.python.org/) and [nx-python](https://github.com/nx-python) for their scripting language respectively their python port to the switch.


  <br>
  <p align="center"><img src="https://www.lua.org/images/logo.gif">
  <img src="https://upload.wikimedia.org/wikipedia/commons/c/c3/Python-logo-notext.svg"><p>
