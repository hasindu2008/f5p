#/bin/bash

test -e data/logs/failed_other.cfg || exit 1

mkdir data/logs/failed_other/

grep -v "^#" data/logs/failed_other.cfg | while read filepath
do

	file=$(basename $filepath)
    prefix=${file%%.*} 
	
	folderf5=${filepath%/*}
	folder=${folderf5%/*}
	LOG="$folder/log2/$prefix.log"
	
	echo $LOG
	cp $LOG data/logs/failed_other/
done