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
rm /scratch_nas/scratch/*
cleanscratch.sh
test -d data/logs && rm -r data/logs
mkdir data/logs || exit 1

ls /scratch_nas/mcf7/1-A1-D1/fast5/* -lhS | awk '{print $9}' > data/dev.cfg;
ls /scratch_nas/mcf7/2-A9-D9/fast5/* -lhS | awk '{print $9}' >> data/dev.cfg;

ansible all -m copy -a "src=scripts/fast5_pipeline_mcf7.sh dest=/nanopore/bin/fast5_pipeline.sh mode=0755"
/usr/bin/time -v ./f5pl data/ip_list.cfg data/dev.cfg 2>&1 | tee log.txt
ansible all -m shell -a "cd /nanopore/scratch && tar zcvf logs.tgz *.log"
gather.sh /nanopore/scratch/logs.tgz data/logs/log tgz
cp log.txt data/logs/
mv *.cfg data/logs/
cp $0 data/logs/
cp scripts/fast5_pipeline_mcf7.sh data/logs/