ansible all -m shell -a "service f5pd stop" -K -b
ansible all -m shell -a "systemctl disable f5pd" -K -b
ansible all -m shell -a "rm /etc/systemd/system/f5pd.service" -K -b
ansible all -m shell -a "systemctl daemon-reload" -K -b


