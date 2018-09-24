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
test -d data/logs_failed && rm -r data/logs_failed
mkdir data/logs_failed || exit 1

cp data/logs/failed_other.cfg  data/dev.cfg;


ansible all -m copy -a "src=scripts/fast5_pipeline_nas_prom_failed.sh dest=/nanopore/bin/fast5_pipeline.sh mode=0755"
/usr/bin/time -v ./f5pl data/ip_list.cfg data/dev.cfg 2>&1 | tee log.txt
ansible all -m shell -a "cd /nanopore/scratch && tar zcvf logs.tgz *.log"
gather.sh /nanopore/scratch/logs.tgz data/logs_failed/log tgz
cp log.txt data/logs_failed/
mv *.cfg data/logs_failed/
cp $0 data/logs_failed/
cp scripts/fast5_pipeline_nas_prom_failed.sh data/logs_failed/