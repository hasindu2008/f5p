#!/bin/bash


while true; do
    read -p "This will overwrite stats from the previous run. Do you wish to continue? (y/n) " yn
    case $yn in
        [Yy]* ) make install; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done

make clean && make || exit 1
rm -rf /scratch_nas/scratch/*
cleanscratch.sh
test -d data/logs && rm -r data/logs
mkdir data/logs || exit 1

FOLDER=/mnt/kyle01/Projects/2018/December/LXBAB114049/
FAST5FOLDER=$FOLDER/reads
PIPELINE_SCRIPT="scripts/fast5_pipeline_scott.sh"

test -d $FOLDER/sam || mkdir $FOLDER/sam || exit 1
test -d $FOLDER/bam || mkdir $FOLDER/bam || exit 1
test -d $FOLDER/log2 || mkdir $FOLDER/log2 || exit 1
test -d $FOLDER/methylation || mkdir $FOLDER/methylation || exit 1

test -d $FAST5FOLDER || exit 1
ls $FAST5FOLDER/*.tar -lhS | awk '{print $9}' > data/dev.cfg || exit 1;

ansible all -m copy -a "src=$PIPELINE_SCRIPT dest=/nanopore/bin/fast5_pipeline.sh mode=0755" 
/usr/bin/time -v ./f5pl data/ip_list.cfg data/dev.cfg 2>&1 | tee log.txt
ansible all -m shell -a "cd /nanopore/scratch && tar zcvf logs.tgz *.log"
gather.sh /nanopore/scratch/logs.tgz data/logs/log tgz
cp log.txt data/logs/
mv *.cfg data/logs/
cp $0 data/logs/
cp $PIPELINE_SCRIPT data/logs/
scripts/failed_device_logs.sh
cp -r data $FOLDER/f5pmaster

