ansible all -m copy -a "src=scripts/f5p_daemon.service dest=/etc/systemd/system/f5p_daemon.service mode=0644" -K -b
ansible all -m shell -a "systemctl daemon-reload" -K -b
ansible all -m shell -a "systemctl enable f5p_daemon" -K -b

