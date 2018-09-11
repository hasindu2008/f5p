#/bin/bash

ansible all -m "shell" -a "service f5pd restart" -K -b

