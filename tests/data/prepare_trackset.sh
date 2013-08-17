echo -ne '\xEF\xBB\xBF' > ru_utf8_BOM.cue
cat ru_utf8.cue >> ru_utf8_BOM.cue

iconv -f UTF-8 -t CP1251 ru_utf8.cue > ru_cp1251.cue
