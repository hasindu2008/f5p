#!/bin/bash



#run as qsub -cwd -N meth_freq -V -S /bin/bash -pe smp 1 -l mem_requested=64G,tmp_requested=100G,fast_dm=10 ./meth_freq.sh
scl enable devtoolset-2 bash


if [ "$#" -ne 2 ]; then
    echo "usage : $0 <kyle:~/input_bam> <kyle:~/output_sorted.bam>"
        exit 1
fi


bamfolder=$1
bamfile=$2

mkdir $TMPDIR/bam/
scp -r $bamfolder/*.bam $TMPDIR/bam/


SAMTOOLS=/home/hasgam/samtools-1.7-dice/samtools

/usr/bin/time -v $SAMTOOLS merge $TMPDIR/sorted.bam $TMPDIR/bam/*.bam
scp $TMPDIR/sorted.bam $bamfile 


