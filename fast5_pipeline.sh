#!/bin/bash

folder=/mnt/778/778-5000ng/778-5000ng_albacore-2.1.3

cat /nanopore/scratch/dev.cfg| while read filepath
do

    #folder=${filepath%%.*}
    file=$(basename $filepath)
    prefix=${file%%.*}    
    
    FAST5TAR=$filepath
    FASTQGZ="$folder/fastq/*"$prefix".fastq.gz"
    FASTQGZ=$(ls $FASTQGZ)
	#SAM="$folder/sam/"$prefix".sam"
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
    
    # FAST5TAR=/mnt/778/778-5000ng/778-5000ng_albacore-2.1.3/fast5/744973-130.fast5.tar
    # FASTQGZ=/mnt/778/778-5000ng/778-5000ng_albacore-2.1.3/fastq/fastq_runid_9b0196d568ad3f05647e4c57c20e863c48d29201_0.744973-130.fastq.gz
    # BAM=/mnt/778/778-5000ng/778-5000ng_albacore-2.1.3/meth/744973-130.bam
    # METH=/mnt/778/778-5000ng/778-5000ng_albacore-2.1.3/meth/744973-130.tsv
    # LOG=/mnt/778/778-5000ng/778-5000ng_albacore-2.1.3/log/744973-130.log    

    # FAST5TAR=/mnt/778/778-5000ng/778-5000ng_albacore-2.1.3/fast5/744973-130.fast5.tar
    # 



    MINIMAP=/nanopore/bin/minimap2
    NANOPOLISH=/nanopore/bin/nanopolish
    SAMTOOLS=/nanopore/bin/samtools

    REF=/nanopore/reference/hg38noAlt.fa
    REFIDX=/nanopore/reference/hg38noAlt.idx

    SCRATCH=/nanopore/scratch


    FAST5TARLOCAL=$SCRATCH/$prefix.fast5.tar
    FAST5EXTRACT=$SCRATCH/$prefix
    FASTQGZLOCAL=$SCRATCH/$prefix.fastq.gz
    FASTQLOCAL=$SCRATCH/$prefix.fastq
    SAMLOCAL=$SCRATCH/$prefix.sam
    BAMLOCAL=$SCRATCH/$prefix.bam
    METHLOCAL=$SCRATCH/$prefix.tsv
    LOGLOCAL=$SCRATCH/$prefix.log
    TMP=$SCRATCH/$prefix".minimap"

    #copy and untar fast5
    cp $FAST5TAR $FAST5TARLOCAL
    mkdir $FAST5EXTRACT
    tar xf $FAST5TARLOCAL -C $FAST5EXTRACT 
    rm $FAST5TARLOCAL
        
    #copy and uncompress fastq
    cp $FASTQGZ $FASTQGZLOCAL
    gunzip $FASTQGZLOCAL
        
    #index
    /usr/bin/time -v $NANOPOLISH index -d $FAST5EXTRACT $FASTQLOCAL 2> $LOGLOCAL

    #minimap
    /usr/bin/time -v $MINIMAP -x map-ont -a -t4 -K20M --secondary=no  --multi-prefix=$TMP $REFIDX $FASTQLOCAL > $SAMLOCAL 2>> $LOGLOCAL


    #sorting
    /usr/bin/time -v $SAMTOOLS sort -@3 $SAMLOCAL > $BAMLOCAL 2>> $LOGLOCAL
    /usr/bin/time -v $SAMTOOLS index $BAMLOCAL 2>> $LOGLOCAL

    #methylation
    /usr/bin/time -v $NANOPOLISH call-methylation -t 4 -r  $FASTQLOCAL -g $REF -b $BAMLOCAL -K 4096 > $METHLOCAL  2>> $LOGLOCAL    


    cp $METHLOCAL $METH
    cp $BAMLOCAL $BAM
    cp $LOGLOCAL $LOG
        
    #remove the rest    
    rm -rf $FAST5EXTRACT
    rm "$FASTQLOCAL" #bad
    rm $SAMLOCAL $BAMLOCAL $METHLOCAL
    rm $SCRATCH/*.tmp ##bad
 
    

done


