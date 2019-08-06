#!/bin/bash

# @author: Hasindu Gamaarachchi (hasindu@unsw.edu.au)

###############################################################################

#some changeable definitions

#Folder containing the dataset
FOLDER=/mnt/f5p_example_dataset/

#Folder containing the fast5 tar files
FAST5FOLDER=$FOLDER/fast5

#The script to be copied and run on worker nodes
PIPELINE_SCRIPT="scripts/fast5_pipeline.sh"

###############################################################################

#a test before cleaning logs
while true; do
    read -p "This will overwrite stats from the previous run. Do you wish to continue? (y/n) " yn
    case $yn in
        [Yy]* ) make install; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done

#a freshly compiled f5pl
make clean && make || exit 1

#clean temporary locations on NAS and the worker nodes
rm -rf /scratch_nas/scratch/*
cleanscratch.sh #https://github.com/hasindu2008/nanopore-cluster/blob/master/system/cleanscratch.sh

#remove previous logs
test -d data/logs && rm -r data/logs
mkdir data/logs || exit 1

#create folders to copy the results (SAM files, BAM files, logs and methylation calls)
test -d $FOLDER/sam || mkdir $FOLDER/sam || exit 1
test -d $FOLDER/bam || mkdir $FOLDER/bam || exit 1
test -d $FOLDER/log2 || mkdir $FOLDER/log2 || exit 1
test -d $FOLDER/methylation || mkdir $FOLDER/methylation || exit 1

#check  the existence of the folder containing tar files
test -d $FAST5FOLDER || exit 1
#make a list of tar files
ls $FAST5FOLDER/*.tar -lhS | awk '{print $9}' > data/dev.cfg || exit 1;

#copy the pipeline script accrross the worker nodes
ansible all -m copy -a "src=$PIPELINE_SCRIPT dest=/nanopore/bin/fast5_pipeline.sh mode=0755" 

#run f5pl
/usr/bin/time -v ./f5pl data/ip_list.cfg data/dev.cfg 2>&1 | tee log.txt

#handle the logs
ansible all -m shell -a "cd /nanopore/scratch && tar zcvf logs.tgz *.log"
gather.sh /nanopore/scratch/logs.tgz data/logs/log tgz #https://github.com/hasindu2008/nanopore-cluster/blob/master/system/gather.sh

cp log.txt data/logs/
mv *.cfg data/logs/
cp $0 data/logs/
cp $PIPELINE_SCRIPT data/logs/

#get the logs of the datasets which the pipeline crashed
scripts/failed_device_logs.sh
cp -r data $FOLDER/f5pmaster

