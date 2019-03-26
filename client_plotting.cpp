#include "matplotlibcpp.h"
#include <arpa/inet.h>
#include <exception>
#include <json.hpp>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using json = nlohmann::json;
namespace plt = matplotlibcpp;

int main(int argc, char *argv[]) {

  sockaddr_in server;
  char serverReply[100] = {0};
  timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  int errorCount{0};
  int count{1};
  double waitTime(0.05);

  //std::vector<double> time{};
  std::vector<double> values1{};
  std::vector<double> values2{};
  values1.reserve(10000);
  values2.reserve(10000);

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


  plt::plot(values1);
  plt::plot(values2);
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
      double x1 = jValues[0]["value1"].get<double>();
      double x2 = jValues[0]["value2"].get<double>();
      values1.emplace_back(x1);
      values2.emplace_back(x2);

      plt::clf();
      plt::plot(values1);
      plt::plot(values2);
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
