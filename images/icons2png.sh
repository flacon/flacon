#!/bin/bash

SRC_COLOR="#000000"

ENABLE_LIGHT="#656565"
DISABLE_LIGHT="#a1a1a1"
ENABLE_DARK="#a1a1a1"
DISABLE_DARK="#656565"



RC_FILE="icons.qrc"

GRAY_FILES="add-disk.svg
			remove-disk.svg
			scan.svg
			download-info.svg
			abort-convert.svg
			start-convert.svg
			folder.svg
			configure.svg
			pattern-button.svg
			track-ok.svg
			audio-button.svg
			cue-button.svg
			preferences-audio.svg
			preferences-general.svg
			preferences-programs.svg
			preferences-update.svg
			wait-0.svg
			wait-1.svg
			wait-2.svg
			wait-3.svg
			wait-4.svg
			wait-5.svg
			wait-6.svg
			wait-7.svg"

COLOR_FILES="track-cancel.svg
			warning.svg
			error.svg"


SIZES="16 22 24 32 48 64 128 256 512"


function conv()
{
	local svgFile=$1
	local size=$2
	local color=$3
	local pngFile=$4

	echo "$svgFile => $pngFile"

	local dir=$(dirname $pngFile)

	mkdir -p "${dir}"
	sed -e "s/${SRC_COLOR}/${color}/g" icons/${svgFile} > "${dir}/_tmp.svg"

	rsvg-convert -w ${size} -h ${size} "${dir}/_tmp.svg" > "${pngFile}"
	#inkscape --without-gui -z -e "${pngFile}" -w ${size} -h ${size} "${dir}/_tmp.svg"
	rm "${dir}/_tmp.svg"

	echo "        <file>${pngFile}</file>" >> ${RC_FILE}
}

echo "<!-- Generated from icons2png.sh do not edit by hand. -->" > ${RC_FILE}
echo "<RCC>" >> ${RC_FILE}

echo "    <qresource prefix=\"/\">" >> ${RC_FILE}
for size in ${SIZES}; do

	sizeStr=$(printf "%03d" $size)
	for f in $GRAY_FILES; do
		out=${f/.svg/}

		conv "$f" $size ${ENABLE_LIGHT} 	"icons/light/${sizeStr}/${out}.png"
		conv "$f" $size ${DISABLE_LIGHT} 	"icons/light/${sizeStr}/${out}-disabled.png"
		
		conv "$f" $size ${ENABLE_DARK} 		"icons/dark/${sizeStr}/${out}.png"
		conv "$f" $size ${DISABLE_DARK} 	"icons/dark/${sizeStr}/${out}-disabled.png"
	done

	for f in $COLOR_FILES; do
		out=${f/.svg/}
		conv "$f" $size ""  "icons/light/${sizeStr}/${out}.png"
		conv "$f" $size ""	"icons/dark/${sizeStr}/${out}.png"
	done

	echo "" >> ${RC_FILE}
done

echo "    </qresource>" >> ${RC_FILE}
echo "</RCC>" >> ${RC_FILE}

