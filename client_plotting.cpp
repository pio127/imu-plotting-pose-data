#include "matplotlibcpp.h"
#include <arpa/inet.h>
#include <exception>
#include <json.hpp>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using json = nlohmann::json;
namespace plt = matplotlibcpp;

struct SensorData
{
  double pitch;
  double yaw;
  double roll;
  SensorData()
      : pitch(0), yaw(0), roll(0){};
};

int main(int argc, char *argv[]) {

  SensorData data{};
  sockaddr_in server;
  char serverReply[100] = {0};
  timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  int errorCount{0};
  int count{1};
  double waitTime(0.05);

  std::vector<double> rollValues{};
  std::vector<double> pitchValues{};
  std::vector<double> yawValues{};
  rollValues.reserve(10000);
  pitchValues.reserve(10000);
  yawValues.reserve(10000);

  char *address;
  if (argc >1 ) {
    address = const_cast<char*>(argv[1]);
    std::cout << "IP address of server: " << address << '\n';
  } else {
    address = const_cast<char*>("127.0.0.1");
    std::cout << "Using default local IP address: " << address << '\n';
  }

  // Parameters
  int serverPort = 90190;
  sockaddr_in remote{};
  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = inet_addr(address);
  remote.sin_port = htons(serverPort);

  // Socket creation
  int socketClient = socket(AF_INET, SOCK_STREAM, 0);
  if (socketClient == -1) {
    std::cerr << "Socket could not be created\n";
    return 1;
  }

  plt::plot(pitchValues);
  plt::plot(rollValues);
  plt::plot(yawValues);
  plt::pause(waitTime);

  // Connecting with server
  int isConnected =
      connect(socketClient, (sockaddr*)&remote, sizeof(sockaddr_in));
  if (isConnected < 0) {
    std::cerr << "Could not connect with server\n";
    return 1;
  }

  std::cout << "Connected with server\n";


  while (true) {
    int gettingMessage = setsockopt(socketClient, SOL_SOCKET, SO_RCVTIMEO,
                                    (char*)&tv, sizeof(tv));
    if (gettingMessage < 0) {
      std::cerr << "Waiting to long for data\n";
      return 1;
    }
    // Clearing char table before getting data
    memset(serverReply, 0, sizeof(serverReply));

    int respondVal = recv(socketClient, serverReply, 200, 0);

    try {
      auto jValues{json::parse(serverReply)};

      // Getting data from JSON and adding to vector
      data.roll = jValues[0]["roll"].get<double>();
      data.pitch = jValues[0]["pitch"].get<double>();
      data.yaw = jValues[0]["yaw"].get<double>();

      rollValues.emplace_back(data.roll);
      pitchValues.emplace_back(data.pitch);
      yawValues.emplace_back(data.yaw);

      plt::clf();
      plt::plot(rollValues);
      plt::plot(pitchValues);
      plt::plot(yawValues);
      plt::pause(waitTime);
      count++;

    } catch (const std::exception&) {
      ++errorCount;
      if (errorCount < 6) {
        std::cerr << count << ". data transfer error\n";
        count++;
        continue;
      } else {
        std::cerr << count << ". data transfer error\n";
        std::cerr << "Connection terminated\n";
        close(socketClient);
        break;
      }
    }
  }
  close(socketClient);
}
