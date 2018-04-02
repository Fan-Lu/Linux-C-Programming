# Char Device Driver

Commads:
1. write to the driver and close the driver  
   echo "testing" > /dev/tuxdrv  
2. watch systerm log  (live watch, open another terminal to input command)  
   sudo tail -f /var/log/syslog
3. add to dev directory, 666 means all users can use this device;  
   700: major number; 0: minor number  
   sudo mknod -m 666 /dev/tuxdrv c 700 0
4. See the hearder file, look the functions defined here.  
   vim /lib/modules/$(uname -r)/build/include/linux/fs.h  
5. uname -r  
   4.13.xxxxxxx(kernel folders)  
   split /lib/modules/4.13.xxxxxxx/build/include/linux/fs.h  
6. list all modules 
   lsmod
7. see what devices installed, major number can be seen from syslog
   ls -l /dev | grep "250,"  
     
   if there are no device file created under /dev with the same major numeber  
   we can create them by hand, using  
   sudo mknod /dev/mycdev0 c 250 0  
   sudo mknod /dev/mycdev1 c 250 1  
   sudo mknod /dev/mycdev2 c 250 2  
