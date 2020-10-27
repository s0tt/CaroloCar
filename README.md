**Carolo Car**

The Carolo-Cup is a contest featuring 1:10 scaled RC cars which compete against each other in several disiciplines like autonomous driving, parking, obstacle detection etc.
We developed a complete system including hardware and software with the goal to participate in the Carolo Cup.
This repository contains our software stack incuding build toolchain, image processing, lane extraction and vehicle dynamic control.


**Vehicle setup**
The RC car consist of the following parts:

- 1:10 remote-controlled vehicle
- Eletrical power supply (2x LiPo batteries)
- Nvidia Jetson Tx 2 for perception, detection and communication
- Arduino Uno for actuator control

![CaroloCar Setup](demo/vehicle_parts.jpg "Vehicle setup")