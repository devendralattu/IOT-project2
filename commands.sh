gcc -o gateway  Gateway.c -lpthread
gcc -o mo motion.c -lpthread
gcc -o ko keychain.c -lpthread
gcc -o do door.c -lpthread
gcc -o bo backend.c
gnome-terminal -e ./bo
sleep 1;
gnome-terminal -e ./gateway
sleep 2;
gnome-terminal -e ./do #DoorConfigurationFile.txt DoorStateFile.txt
sleep 2;
gnome-terminal -e ./ko #KeychainConfigurationFile.txt KeychainStateFile.txt
sleep 2;
gnome-terminal -e ./mo #MotionSensorConfigurationFile.txt MotionSensorStateFile.txt
