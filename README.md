
EdiZon

￼

A save dumper (future editor) for Horizon by thomasnet and WerWolv

Credits

3096 for save dumping/injecting

Bernardo Giordano for some Checkpoint code

Reswitched for the Homebrew Launcher GUI code

Images

￼

￼

Save editor config format

To expanding the functionality of the save file editor, .json files get used to specify the offset and address of spessific values inside the save file. These files look like follows:

{
 "saveFilePaths" : [ "/" ],
 "files" : "File\\d+\\.bin",
 "filetype": "bin",
 "items": [
 {
   "name" : "Integer Value",
   "offsetAddress" : "DEAD",
   "address" : "BEEF",
   "widget" : {
    "type" : "int",
    "minValue" : 0,
    "maxValue" : 9999
   }
 },
 {
   "name" : "Boolean Value",
   "offsetAddress" : "CAFE",
   "address" : "BABE",
   "widget" : {
    "type" : "bool",
    "onValue" : 1,
    "offValue" : 0
   }
 }
 ]
}



•  saveFilePaths : The folder paths in which the save files are located. Multiple locations may be set. Use '/' to target the root dir.
•  validSaveFileNames : How the save file names are formatted. This is a RegEx string.
•  filetype : Defines what save file parser will be used. Supported values are: bin 
•  items : Inside this array, every new element will become a new widget inside the editor.
•  name : The name of the widget. This is the text which will be displayed next to the value on the widget.
•  offsetAddress : Most save files load dynamic offsets from static locations within the save file. Imagine the savefile as an array of values. First we take offsetAddress and look up the value at that offsets ( saveFile[offsetAddress] ). Then we use the address found at this offset to find final value ( saveFile[saveFile[offsetAddress]] ). If no dynamic offsets are used, set this to 0. Note: This is a hexadecimal number in a string.
•  address : Additional offset which gets added to the dynamic offset. Note: This is a hexadecimal number in a string.
•  widget : These are the widget settings.
•  type : May be either boolean or integer. Boolean creates an ON/OFF switch where as integer creates a numbox. All values are uint16_t.
•  minValue : Spessific for integer types. This is the minimum value the address may have before it wraps around.
•  maxValue : Spessific for integer types. This is the maximum value the address may have before it wraps around.
•  onValue : Spessific for boolean types. This is the value the address will have if the switch is set to ON.
•  offValue : Spessific for boolean types. This is the value the address will have if the switch is set to OFF.

The finished files will get placed on your SD card within the sdmc:/EdiZon/editor folder and named as the titleID of the game they are made for, for example sdmc:/EdiZon/editor/01007EF00011E000.json . All characters are required to be in caps except the .json ending.