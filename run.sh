gcc -g -o bo backend.c
gcc -g -o go gateway.c -lpthread
gcc -g -o mo motion.c -lpthread
gcc -g -o ko keychain.c -lpthread
gcc -g -o do door.c -lpthread
gcc -g -o so security.c

>GatewayOutputFile.txt
>DoorOutputFile.txt
>KeychainOutputFile.txt
>MotionSensorOutputFile.txt
>PersistentStorage.txt

gnome-terminal -e ./bo
sleep 1;
gnome-terminal -e ./go #GatewayOutputFile.txt
sleep 1;
gnome-terminal -e ./so
sleep 2;

gnome-terminal -e ./do #DoorConfigurationFile.txt DoorStateFile.txt
sleep 2;
gnome-terminal -e ./ko #KeychainConfigurationFile.txt KeychainStateFile.txt
sleep 2;
gnome-terminal -e ./mo #MotionSensorConfigurationFile.txt MotionSensorStateFile.txt
