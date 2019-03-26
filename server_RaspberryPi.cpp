#include "matplotlibcpp.h"
#include <arpa/inet.h>
#include <cmath>
#include <iostream>
#include <json.hpp>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include "RTIMULib.h"

using json = nlohmann::json;

struct SensorData {
  double pitch;
  double yaw;
  double roll;
  SensorData()
    :pitch(0), yaw(0), roll(0){};
};
   
void sensorDataGather(SensorData &data, bool &ready, std::mutex &mtx, RTIMU *imu);
void dataTransmission(SensorData &data, bool &ready, std::mutex &mtx);

int main() {
  
  std::mutex mtx; 
  bool ready{false};
  SensorData data{};

  // IMU initialization
  RTIMUSettings *	settings = new RTIMUSettings("calib", "RTIMULib");
  RTIMU *imu = RTIMU::createIMU(settings);
  if ((imu == nullptr) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
		  std::cerr << "IMU not found!\n";
		  return 0;
  }

  imu->IMUInit();
  imu->setSlerpPower(0.02);
  imu->setAccelEnable(true);
  imu->setGyroEnable(true);
  std::cout<<std::endl;
  //imu->setCompassEnable(true);

  std::thread t1(sensorDataGather, std::ref(data), std::ref(ready),
                 std::ref(mtx), imu);  
  std::thread t2(dataTransmission, std::ref(data), std::ref(ready), std::ref(mtx));  
  
  t1.join();
  t2.join();
}


void dataTransmission(SensorData &data, bool &ready, std::mutex &mtx) {

  int clientLen, socketConnection, sendStatus;
  sockaddr_in server, client;
  json jFile;
  int count{1};
  double x{0.1};

  // Parameters
  char message[100];
  int iRetval = -1;
  int clientPort = 90190;
  sockaddr_in remote{};
  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = htonl(INADDR_ANY);
  remote.sin_port = htons(clientPort);

  // Socket creation
  int socketServer = socket(AF_INET, SOCK_STREAM, 0);
  if (socketServer == -1) {
    std::cerr << "Socket could not be created\n";
    return;
  }
  // Socket binding
  int socketBind = bind(socketServer, (sockaddr*)&remote, sizeof(remote));
  if (socketBind < 0) {
    std::cerr << "Socket bind failed\n";
    return;
  }
  // Listening
  listen(socketServer, 3);
  std::cout << "Waiting for connection...\n";

  
  clientLen = sizeof(sockaddr_in);
  socketConnection =
      accept(socketServer, (sockaddr*)&client, (socklen_t*)&clientLen);
  if (socketConnection < 0) {
    std::cerr << "Connection not accepted\n";
    return;
  } else {
    std::cout << "Connection accepted\n";
  }

  ready = true;

  while (ready) {
    
    mtx.lock();
   
    jFile["roll"] = data.roll;
    jFile["pitch"] = data.pitch;
    jFile["yaw"] = data.yaw;

    mtx.unlock();

    std::string stringJSON{jFile.dump()};
    sendStatus =
        send(socketConnection, stringJSON.c_str(), stringJSON.size(), 0);

    if (sendStatus < 0) {
      std::cerr << "Failed to send\n";
      ready = false;
      close(socketConnection);
      break;
    }

    usleep(250000);
    count++;
  }

}

void sensorDataGather(SensorData &data, bool &ready, std::mutex &mtx, RTIMU *imu) {
  while(!ready) {}
    while(ready) {
      usleep(static_cast<__useconds_t>(imu->IMUGetPollInterval() * 1000));
      imu->IMURead();
      RTIMU_DATA imuData = imu->getIMUData();

      mtx.lock();
      data.roll = imuData.fusionPose.x() * RTMATH_RAD_TO_DEGREE;
      data.pitch = imuData.fusionPose.y() * RTMATH_RAD_TO_DEGREE;
      data.yaw = imuData.fusionPose.z() * RTMATH_RAD_TO_DEGREE;
      mtx.unlock();
      usleep(100000);
  }
}
