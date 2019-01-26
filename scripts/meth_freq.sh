#!/bin/bash



#run as qsub -cwd -N meth_freq -V -S /bin/bash -pe smp 1 -l mem_requested=64G,tmp_requested=100G,fast_dm=10 ./meth_freq.sh
scl enable devtoolset-2 bash


if [ "$#" -ne 2 ]; then
    echo "usage : $0 <kyle:~/input_tsv> <kyle:~/output_freq.tsv>"
        exit 1
fi


tsvfolder=$1
freqfile=$2

mkdir $TMPDIR/meth/
scp -r $tsvfolder/*.tsv $TMPDIR/meth/
cat $(ls $TMPDIR/meth/*.tsv | head -1) | head -1 > $TMPDIR/concat.tsv
tail -n +2 -q $TMPDIR/meth/*.tsv >> $TMPDIR/concat.tsv

test -e $TMPDIR/concat.tsv || exit 1

NANOPOLISH_FSCRIPT=/home/hasgam/hasindu2008.git/nanopolish/scripts/calculate_methylation_frequency.py

#frequencies
/usr/bin/time -v python $NANOPOLISH_FSCRIPT -i $TMPDIR/concat.tsv > $TMPDIR/freq.tsv
scp $TMPDIR/freq.tsv $freqfile 

