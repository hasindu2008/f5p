ansible all -m copy -a "src=scripts/f5pd.service dest=/etc/systemd/system/f5pd.service mode=0644" -K -b
ansible all -m shell -a "systemctl daemon-reload" -K -b
ansible all -m shell -a "systemctl enable f5pd" -K -b

