#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "usage : $0 <kyle:~/meth_tsv_dir> <kyle:~/freq.tsv>"
        exit 1
fi

meth_tsv_dir=$1
freq=$2


# prefix_output=$(basename $output)
# prefix_freq=$(basename $freq)  

#concat
# cat $(ls $meth_tsv_dir/*.tsv | head -1) | head -1 > $output
# tail -n +2 -q $meth_tsv_dir/*.tsv >> $output

# scp $output dice:/home/hasgam/scratch/tmp/$prefix_output

ssh dice "'" "qsub -cwd -N meth_freq -V -S /bin/bash -pe smp 1 -l mem_requested=64G,tmp_requested=100G,fast_dm=10 ~/scripts/meth_freq.sh $meth_tsv_dir $freq" "'"

# NANOPOLISH_FSCRIPT=/home/hasgam/hasindu2008.git/nanopolish/scripts/calculate_methylation_frequency.py

# CMD="/usr/bin/time -v python $NANOPOLISH_FSCRIPT -i methylation_all.tsv > methylation_frequency.tsv"
# qsub -cwd -N meth_freq -V -S /bin/bash -pe smp 1 -l mem_requested=64G  "\""$CMD"\""
# scp dice:/home/hasgam/scratch/tmp/freq.tsv $freq
