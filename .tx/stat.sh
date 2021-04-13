#!/bin/sh

DIR="../translations"
BARS_LEN=100
BARS_CHAR="⫼"
BARS_CHAR=":"

function graph()
{
	local s=$1
	local t=$2 

	let l=($t*${BARS_LEN})/$s
	local graph=""
	for ((i=0; i<$l; i++)); do 
		printf "${BARS_CHAR}"; 
	done
	for ((i=$l; i<${BARS_LEN}; i++)); do 
		printf " "; 
	done
}


ssum=0
tsum=0
psum=0
cnt=0
IFS=$'
'
for f in `find .. -name "*.ts"`; do
	src=$(grep '<source>' "$f" | wc -l)
	trn=$(grep '<translation>' "$f" | wc -l)
	let p=($trn*100)/$src
	let ssum=$ssum+$src
	let psum=$psum+$p
	let tsum=$tsum+$trn
	let cnt=$cnt+1
	printf "%-20s | %s | %3d%%   %4s of %s\n" $(basename "$f") $(graph $src $trn) "$p" "$trn" "$src"
done

let p=($tsum*100)/$ssum
let t=$tsum/$cnt
echo "-----------------------------------------"
printf "%-20s |%s|%3d%%   %4s of %s\n" "Average" $(graph $ssum $tsum) "$p" "$t" "$src"
