# Real-time plotting data from Raspberry Pi
Server send pose data from IMU(connected to Raspberry PI) to client via TCP. Then the plot is generated on client's side.

# Used libraries(all are included as header only or a source library) :
 - Nlohmann JSON library(https://github.com/nlohmann/json)
 - matlplotlib wrapped for C++(https://github.com/lava/matplotlib-cpp)
 - RTIMULib2(https://github.com/fjp/RTIMULib2)

# Install
Project should be built on Raspberry Pi and on some other computer with GNU/Linux OS(linux sockets used!).
First server executable must be run on Raspberry Pi and then client on other computer(they must be conected to the same network).

Build order:
```
mkdir build
cd build
cmake ..
cmake --build .
./server
```

And for plotting computer:
```
./clientPlot IP_ADDRESS_OF_RASPBERRY_PI_SERVER
```
