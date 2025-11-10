# CAN-communication

Repository to manage the CAN communication of the vehicle

To use the CAN debugger on Linux :

1. Identify which device it the debugger : ls /dev/ttyACM*
2. sudo modprobe slcan
sudo slcand -o -c -s6 /dev/ttyACM1 can0
sudo ip link set up can0
3. candump can0 //to sniff the can BUS
4. cansend can0 321#00000000 // where 321 is the ID and 00000000 the data, in hex
If you want to send 29-bit IDs : cansend can0 00ABCDEF#776F726C64

To disconnect/reset:

1. sudo ip link set can0 down
2. sudo pkill slcand
