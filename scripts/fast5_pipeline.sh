#!/bin/bash

# @author: Hasindu Gamaarachchi (hasindu@unsw.edu.au)

###############################################################################

#some changeable definitions

#program paths
MINIMAP=/nanopore/bin/minimap2-arm
METHCALL=/nanopore/bin/f5c #or nanopolish
SAMTOOLS=/nanopore/bin/samtools

#reference fasta
REF=/nanopore/reference/hg38noAlt.fa
#reference index for minimap2
REFIDX=/nanopore/reference/hg38noAlt.idx

#temporary space on the local storage or the network mount
SCRATCH=/nanopore/scratch

###############################################################################

#argument is a path of a tar file
if [ "$#" -ne 1 ]; then
    echo "usage : $0 <filepath>"
        exit 1
fi
filepath=$1
#get the folder which the tar files reside (strip the file name from $filepath)
folderf5=${filepath%/*}
#get the folder which the datasets reside (one level up of $folderf5)
folder=${folderf5%/*}

#initial exit status is assumed to be success
exit_status=0

#the name of the tar file (strip the path and get only the name with extension)
file=$(basename $filepath)
#name of the tar file without .tar extension
prefix=${file%%.*}    

#derive the locations of input and output files
FAST5TAR=$filepath
#FASTQGZ="$folder/fastq/*"$prefix".fastq.gz"
#FASTQGZ=$(ls $FASTQGZ)
FASTQ="$folder/fastq/fastq_$prefix.fastq"
SAM="$folder/sam/$prefix.sam"
BAM="$folder/bam/$prefix.bam"
METH="$folder/methylation/$prefix.tsv"
LOG="$folder/log2/$prefix.log"

# echo $FAST5TAR
# echo $FASTQGZ
# echo $BAM
# echo $BAM
# echo $METH
# echo $LOG
# echo ""

#derive the locations of temporary intermediate files
FAST5TARLOCAL=$SCRATCH/$prefix.fast5.tar
FAST5EXTRACT=$SCRATCH/$prefix
#FASTQGZLOCAL=$SCRATCH/$prefix.fastq.gz
FASTQLOCAL=$SCRATCH/$prefix.fastq
SAMLOCAL=$SCRATCH/$prefix.sam
BAMLOCAL=$SCRATCH/$prefix.bam
METHLOCAL=$SCRATCH/$prefix.tsv
LOGLOCAL=/nanopore/scratch/$prefix.log
TMP=$SCRATCH/$prefix".minimap"

#untar fast5
test -d $FAST5EXTRACT && rm -rf $FAST5EXTRACT
mkdir $FAST5EXTRACT
tar xf $FAST5TAR -C $FAST5EXTRACT 

#copy fastq file
cp $FASTQ $FASTQLOCAL
#copy and uncompress fastq
#cp $FASTQGZ $FASTQGZLOCAL
#test -e $FASTQLOCAL && rm $FASTQLOCAL
#gunzip $FASTQGZLOCAL
	
#index
/usr/bin/time -v $METHCALL index -d $FAST5EXTRACT $FASTQLOCAL 2> $LOGLOCAL || exit_status=1

#minimap
/usr/bin/time -v $MINIMAP -x map-ont -a -t4 -K5M --secondary=no  --multi-prefix=$TMP $REFIDX $FASTQLOCAL > $SAMLOCAL 2>> $LOGLOCAL || exit_status=1

#sorting
/usr/bin/time -v $SAMTOOLS sort -@3 $SAMLOCAL > $BAMLOCAL 2>> $LOGLOCAL || exit_status=1
/usr/bin/time -v $SAMTOOLS index $BAMLOCAL 2>> $LOGLOCAL || exit_status=1

#methylation
/usr/bin/time -v $METHCALL call-methylation -t 4 -r  $FASTQLOCAL -g $REF -b $BAMLOCAL -K 256 > $METHLOCAL  2>> $LOGLOCAL || exit_status=1   

#copy results to the correct place
cp $METHLOCAL $METH
cp $SAMLOCAL $SAM
cp $BAMLOCAL $BAM
cp $LOGLOCAL $LOG
	
#remove the rest    
rm -rf $FAST5EXTRACT 
rm -f $FASTQLOCAL $FASTQLOCAL.index $FASTQLOCAL.index.fai $FASTQLOCAL.index.gzi $FASTQLOCAL.index.readdb 
rm -f $SAMLOCAL $BAMLOCAL $BAMLOCAL.bai $METHLOCAL

#return the exit status
exit $exit_status    




