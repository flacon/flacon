Cue-Sheet Syntax
================


[CATALOG](#catalog) | [CDTEXTFILE](#cdtextfile) | [FILE](#file) | [FLAGS](#flags) | [INDEX](#index) | [ISRC](#isrc) | [PERFORMER](#performer) | [POSTGAP](#postgap) | [PREGAP](#pregap) | [REM](#rem) | [SONGWRITER](#songwriter) | [TITLE](#title) |  [TRACK](#track)



CATALOG
-------

**Description:**
This command is used to specify the disc's "Media Catalog Number". It will typically be used only when mastering a CDROM for commercial production.

**Syntax:**
```
CATALOG [media-catalog-number]
```

**Examples:**
```
CATALOG 1234567890123
CATALOG 8340218374610
```

**Rules:**
The catalog number must be 13 digits long and is encoded according to UPC/EAN rules. This command can appear only once in the CUE SHEET file (it will usually be the first line, but this is not mandatory).




CDTEXTFILE
----------

**Description:**
This command is used to specify the name of the file that contains the encoded CD-TEXT information for the disc. This command is only used with files that were either created with the graphical CD-TEXT editor or generated automatically by the software when copying a CD-TEXT enhanced disc.

**Syntax:**
```
CDTEXTFILE [filename]
```

**Parameters:**
  * **filename** – Filename (can include device/directory). If the filename contains any spaces, then it must be enclosed in quotation marks.

**Examples:**
```
CDTEXTFILE C:\TEST\DISC.CDT
CDTEXTFILE "C:\LONG FILENAME.CDT"
```

**Rules:**
If your recorder does not support CD-TEXT, then this command will be ignored.




FILE
----

**Description:**
This command is used to specify a data/audio file that will be written to the recorder.

**Syntax:**
```
FILE [filename] [filetype]
```

**Parameters:**
 * **filename** – Filename (can include device/directory). If the filename contains any spaces, then it must be enclosed in quotation marks.
 * **filetype** – Filetype The following filetypes are allowed…
   * *BINARY* – Intel binary file (least significant byte first)
   * *MOTOROLA* – Motorola binary file (most significant byte first)
   * *AIFF* – Audio AIFF file
   * *WAVE* – Audio WAVE file
   * *MP3* – Audio MP3 file

**Note:**
All audio files (WAVE, AIFF, and MP3) must be in 44.1KHz 16-bit stereo format.

**Examples:**
```
FILE "C:\DATA\TRACK1.ISO" BINARY
FILE "C:\MUSIC\TRACK2.WAV" WAVE
FILE "C:\MUSIC\LONG FILENAME.MP3″ MP3
```

**Rules:**
FILE commands must appear before any other command except CATALOG. This rule contradicts the examples on this site: Note.

**Note:**
For AUDIO files, if the length of the data within the file is not an exact multiple of the CDROM sector size (2352 bytes), then the last sector will be padded with zeros when it is recorded to the blank disc.



FLAGS
-----

**Description:**
This command is used to set special subcode flags within a track. These flags are rarely used on any discs made today.

**Syntax:**
```
FLAGS [flags]
```

**Parameters:**

 * **flags** – Specifies one or more track flags. The following flags are allowed…
   * *DCP* – Digital copy permitted
   * *4CH* – Four channel audio
   * *PRE* – Pre-emphasis enabled (audio tracks only)
   * *SCMS* – Serial copy management system (not supported by all recorders)

**Example:**
```
FLAGS DCP
FLAGS 4CH PRE
```

**Rules:**
The FLAGS command must appear after a TRACK command, but before any INDEX commands. Only one FLAGS command is allowed per track.

**Note:**
There is a fourth subcode flag called "DATA" which is set for all non-audio tracks. This flag is set automatically based on the datatype of the track.



INDEX
-----

**Description:**
This command is used to specify indexes (or subindexes) within a track.

**Syntax:**
```
INDEX [number] [mm:ss:ff]
```

**Parameters:**
  * **number** – Index number (0-99)
  * **mm:ss:ff** – Starting time in minutes, seconds, and frames (75 frames/second).

**Note:**
All times are relative to the beginning of the current file.

**Example:**
```
INDEX 01 00:00:00
INDEX 05 02:34:50
```

**Rules:**
All index numbers must be between 0 and 99 inclusive. The first index must be 0 or 1 with all other indexes being sequential to the first one. The first index of a file must start at 00:00:00.

*INDEX 0* Specifies the starting time of the track "pregap".

*INDEX 1* Specifies the starting time of the track data. This is the only index that is stored in the disc’s table-of-contents.

*INDEX > 1* Specifies a subindex within a track.



ISRC
----

**Description:**
This command is used to specify a track’s "International Standard Recording Code" (ISRC). It will typically be used only when mastering a CD for commercial disc production.

**Syntax:**
```
ISRC (code)
```

**Example:**
```
ISRC ABCDE1234567
```

**Rules:**
The ISRC must be 12 characters in length. The first five characters are alphanumeric, but the last seven are numeric only. If it is used, the ISRC command must be specified after a TRACK command, but before any INDEX commands.



PERFORMER
---------

**Description:**
This command is used to specify the name of a perfomer for a CD-TEXT enhanced disc.

**Syntax:**
```
PERFORMER [performer-string]
```

**Parameters:**
  * **performer-string** – Name of performer. If the string contains any spaces, then it must be enclosed in quotation marks. Strings should be limited to 80 character or less.

**Example:**
```
PERFORMER "The Beatles"
```

**Rules:**
If the PERFORMER command appears before any TRACK commands, then the string will be encoded as the performer of the entire disc. If the command appears after a TRACK command, then the string will be encoded as the performer of the current track. Note: If your recorder does not support CD-TEXT, then this command will be ignored.




POSTGAP
-------

**Description:**
This command is used to specify the length of a track postgap. The postgap data is generated internally by your cdr software. No data is consumed from the current data file.

**Syntax:**
```
POSTGAP [mm:ss:ff]
```

**Parameters:**
  * **mm:ss:ff** – Specifies the postgap length in minutes, seconds, and frames.

**Example:**
```
POSTGAP 00:02:00
```

**Rules:**
The POSTGAP command must appear after all INDEX commands for the current track. Only one POSTGAP command is allowed per track.



PREGAP
------

**Description:**
This command is used to specify the length of a track pregap. The pregap data is generated internally by your cdr software. No data is consumed from the current data file.

**Syntax:**
```
PREGAP [mm:ss:ff]
```

**Parameters:**
  * **mm:ss:ff** – Specifies the pregap length in minutes, seconds, and frames.

**Example:**
```
PREGAP 00:02:00
```

**Rules:**
The PREGAP command must appear after a TRACK command, but before any INDEX commands. Only one PREGAP command is allowed per track.



REM
---

**Description:**
This command is used to put comments in your CUE SHEET file.

**Syntax:**
```
REM (comment)
```

**Example:**
```
REM This is a comment
```



SONGWRITER
----------

**Description:**
This command is used to specify the name of a songwriter for a CD-TEXT enhanced disc.

**Syntax:**
```
SONGWRITER [songwriter-string]
```

**Parameters:**
  * **songwriter-string** – Name of songwriter. If the string contains any spaces, then it must be enclosed in quotation marks. Strings should be limited to 80 character or less.

**Example:**
```
SONGWRITER "Paul McCartney"
```

**Rules:**
If the SONGWRITER command appears before any TRACK commands, then the string will be encoded as the songwriter of the entire disc. If the command appears after a TRACK command, then the string will be encoded as the songwriter of the current track. Note: If your recorder does not support CD-TEXT, then this command will be ignored.



TITLE
-----

**Description:**
This command is used to specify a title for a CD-TEXT enhanced disc.

**Syntax:**
```
TITLE [title-string]
```

**Parameters:**
  * **title-string** – Title of disc or track. If the string contains any spaces, then it must be enclosed in quotation marks. Strings should be limited to 80 character or less.

**Example:**
```
TITLE "The Beatles – Abbey Road"
TITLE "Here Comes the Sun"
```

**Rules:**
If the TITLE command appears before any TRACK commands, then the string will be encoded as the title of the entire disc. If the command appears after a TRACK command, then the string will be encoded as the title of the current track. Note: If your recorder does not support CD-TEXT, then this command will be ignored.



TRACK
-----

**Description:**
This command is used to start a new TRACK.

**Syntax:**
```
TRACK [number] [datatype]
```

**Parameters:**
  * **number** – Track number (1-99)
  * **datatype** – Track datatype The following datatypes are allowed…
    * *AUDIO* – Audio/Music (2352)
    * *CDG* – Karaoke CD+G (2448)
    * *MODE1/2048* – CDROM Mode1 Data (cooked)
    * *MODE1/2352* – CDROM Mode1 Data (raw)
    * *MODE2/2336* – CDROM-XA Mode2 Data
    * *MODE2/2352* – CDROM-XA Mode2 Data
    * *CDI/2336* – CDI Mode2 Data
    * *CDI/2352* – CDI Mode2 Data

Supported datatypes and blocksizes by recorder model…

**Rules:**
All track numbers must be between 1 and 99 inclusive. The first track number can be greater than one, but all track numbers after the first must be sequential. You must specify at least one track per file.


==== 

<sup>*From http://digitalx.org/cue-sheet/syntax/index.html*</sup>