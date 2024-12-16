APE Tags
========

An APE tag item key is a key for accessing special meta-information in an audio file. 

Member of [APE Tag Item](https://wiki.hydrogenaud.io/index.php?title=APE_Tag_Item). 

APE tag item keys can have a length of 2 (including) up to 255 (including) characters in the range from 0x20 (Space) until 0x7E (Tilde). 

Typical keys should have a length of 2–16 characters using the following characters:<br>
Space (0x20), Slash (0x2F), Digits (0x30–0x39), Letters (0x41–0x5A, 0x61–0x7A). 


Values can contain binary data, a value or a list of values. See [APE Item Value](https://wiki.hydrogenaud.io/index.php?title=APE_Item_Value).<br>
List of values can be mixed, i.e. contain [UTF-8](https://wiki.hydrogenaud.io/index.php?title=APEtag-UTF8) strings and external references<br>
beginning with _file://..._, _http://www..._, _ftp://ftp...._ 

Not allowed are the following keys: ID3, TAG, OggS and MP+. 

Currently the following keys are defined:

|Key                |Meaning|Recommended format of value
|-------------------|-------|------------
|Title              |Music Piece Title<br>Music Work|UTF-8 string
|Subtitle           |Title when TITLE contains the work or additional sub title|UTF-8 string
|Artist             |Performing artist<br>List of Performing Artists|UTF-8 string<br>List of UTF-8 strings
|Album              |Album name|UTF-8 string
|Debut album        |Debut album name|UTF-8 string
|Publisher          |Record label or publisher|UTF-8 string
|Conductor          |Conductor|UTF-8 string
|Track              |Track Number<br>Track Number/Total Tracks Number|Integer<br>Integer/Integer
|Composer           |Name of the original composer<br>Name of the original arranger|UTF-8 string<br>List of UTF-8 strings
|Comment            |User comment(s)|UTF-8 string<br>List of UTF-8 strings<br>Link<br>List of Links
|Copyright          |Copyright holder|UTF-8 string<br>List of UTF-8 strings
|Publicationright   |Publication right holder|UTF-8 string<br>List of UTF-8 strings
|File               |File location  |Link<br>List of Links
|EAN/UPC            |EAN-13/UPC-A bar code identifier|12- or 13-digit Integer
|ISBN               |ISBN number with check digit| [9-digit number with a check digit (0–9 or X)](https://wiki.hydrogenaud.io/index.php?title=ISBN)
|Catalog            |Catalog number|sometimes in the EAN/UPC, often some letters followed by some numbers
|LC                 |Label Code|[LC + 4- or 5-digit Integer](https://wiki.hydrogenaud.io/index.php?title=APE-LC)
|Year [1]           |Release date|Date
|Record Date        |Record date|Date
|Record Location    |Record location(s)|UTF-8 string<br>List of UTF-8 strings
|Genre [2]          |Genre(s)|UTF-8 string<br>List of UTF-8 strings
|Media              |Source<br>Source Media Number/Total Media Number<br>Source Time|Media<br>Media Integer/Integer<br>Media Time
|Index              |Indexes for quick access|Index Time<br>List of Index Times
|Related            |Location of related information|Link<br>List of Links
|ISRC               |International Standard Recording Number|ASCII String
|Abstract           |Abstract|Link
|Language           |Used Language(s) for music/spoken words|UTF-8 string<br>List of UTF-8 strings
|Bibliography       |Bibliography/Discography|Link
|Introplay          |Characteric part of piece for intro playing|Index Time Range
|Dummy              |Place holder|Zero data


[1] This key name is used to be upward compatible.

[2] Genre keywords should be normally English terms. A native language supporting plugin can translate common expression to the local language and vice versa when storing the genre in the file.

---
Source https://wiki.hydrogenaud.io/index.php?title=APE_key
