#!/bin/bash

make clean && make
cleanscratch.sh
rm data/logs/*

ls /mnt/778/778-5000ng/778-5000ng_albacore-2.1.3/fast5/* -lhS | awk '{print $9}' > data/dev.cfg;

ansible all -m copy -a "src=scripts/fast5_pipeline.sh dest=/nanopore/bin/fast5_pipeline.sh mode=0755"
/usr/bin/time -v ./f5pl data/ip_list.cfg data/dev.cfg 2>&1 | tee log.txt
ansible all -m shell -a "cd /nanopore/scratch && tar zcvf logs.tgz *.log"
gather.sh /nanopore/scratch/logs.tgz data/logs/log tgz
mv log.txt data/logs/
mv *.cfg data/logs/
