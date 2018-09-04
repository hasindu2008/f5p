ssh rock1 "cd ~/f5p && make clean && make" || exit 1
scp rock1:~/f5p/f5pd ./f5pd || exit 1
ansible all -m shell -a "service f5pd stop" -K -b
ansible all -m copy -a "src=./f5pd dest=/nanopore/bin/f5pd mode=0755"
ansible all -m shell -a "service f5pd start" -K -b