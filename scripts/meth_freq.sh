#!/bin/bash



#run as qsub -cwd -N meth_freq -V -S /bin/bash -pe smp 1 -l mem_requested=64G ./meth_freq.sh
scl enable devtoolset-2 bash


if [ "$#" -ne 2 ]; then
    echo "usage : $0 <input_tsv> <output_freq.tsv>"
        exit 1
fi


tsvfile=$1
freqfile=$2

test -e tsvfile || exit 1

NANOPOLISH_FSCRIPT=/home/hasgam/hasindu2008.git/nanopolish/scripts/calculate_methylation_frequency.py

#frequencies
/usr/bin/time -v python $NANOPOLISH_FSCRIPT -i $tsvfile > $freqfile


