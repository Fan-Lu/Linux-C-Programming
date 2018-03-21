# Char Device Driver

Commads:
1. write to the driver and close the driver  
   echo "testing" > /dev/tuxdrv  
2. watch systerm log  (live watch, open another terminal to input command)  
   sudo tail -f /var/log/syslog
3. add to dev directory, 666 means all users can use this device;  
   700: major number; 0: minor number  
   sudo mknod -m 666 /dev/tuxdrv c 700 0
