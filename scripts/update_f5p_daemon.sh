make clean && make
ansible all -m shell -a "service f5p_daemon stop" -K -b
ansible all -m copy -a "src=f5p_daemon dest=/nanopore/bin/f5p_daemon mode=0755"
ansible all -m shell -a "service f5p_daemon start" -K -b