# EdiZon SE

This fork is based on the solid foundation of EdiZon. The changes made are on the game memory hack aspect. 

I have added bookmark functionality, completed the range search feature and enhanced the speed to enable search for small value which generates lots of hits.
Bookmark automatically adjust relative to main and heap so static location relative to main and heap will continue to work for different game run. You can also extract the memory location pointed by cheat code to bookmark to explore/edit the ram around there. Able to rebase first offset of cheat code for game version change that only affects the first offset. Pointer search result is added back into bookmark. Bookmark with pointer chain attached will dynamically adjust the address on the bookmark list.

Work in progress : Faster pointer search, pause/resume of search, examination of intermidiate result, change search parameters mid search. 

PS: In app help on button currently don't show hint what right stick down and left stick down do. Right stick down relates to memory edit and Left stick down relates to pointer search. Just try it to figure it out. New key combo ZL+Y to get into pointer search setup.

Here are the steps to do pointer search. 
1. Clear previous search result if any.
2. Perform a search of type pointer when the game is in the state you want. (Press Lstick down will quickly setup the search). The result of this search is used for subsequent pointer search. Very important to get it right to find the pointer chains.
3. Select the bookmark with address you want to find the pointer chain that targets it. 
4. Press Lstick down will start the search with default configuration. 
5. Or press LZ+Y to go setting page for the search parameters. Move the cursor to the position you want to modify and press L or R to increment or decrement the value. Some value cannot be modified at the moment. Press + to start the search.
6. Results are appended to the bookmark list. Test them to see the validity of the chain when the game state change, especially after relaunching the game. When satisfied that a good bookmark entry is found press Y while highlighting the bookmark to add it to the cheat code file.
7. Pointer search can be paused by holddown ZL+B. Normal functionality of the app is all available while pointer search is pause. 
8. Start pointer search again to resume (step 4 or 5), in resume case the currently search parameters are used for future iteration of the loop, intermediate targets that are queued up for processing is preserved. Very important to ensure memory search results is valid before continuing the pointer chain search, repeat step 2 if necessary. To abort the search and start from scratch press ZL+"+". 


Original functinality of Edizon on game save is available when launched without a game running. 

  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/icon.jpg"><br />
      <a href="https://github.com/WerWolv/EdiZon/releases/latest"><img src="https://img.shields.io/github/downloads/WerWolv/EdiZon/total.svg" alt="Latest Release" /></a>
    <a href="https://discord.gg/qyA38T8"><img src="https://discordapp.com/api/guilds/465980502206054400/embed.png" alt="Discord Server" /></a>
    <a href="https://travis-ci.com/WerWolv/EdiZon"><img src="https://travis-ci.com/WerWolv/EdiZon.svg?branch=master" alt="Build Status" /></a>
  </p>
  
A Homebrew save file dumper, injector and on-console editor for Horizon, the OS of the Nintendo Switch. 
Please note if you are using Atmosphere 0.10.0+, you **must** use the snapshot version of EdiZon. Older versions of EdiZon do **not** work on the latest Atmosphere.

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
