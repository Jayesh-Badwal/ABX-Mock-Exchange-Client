#include <iostream>
#include <string>
#include "dataHandler.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: exe server_ip server_port\n";
        return 0;
    }

    std::string serverIp = argv[1];
    uint16_t serverPort = atoi(argv[2]);

    DataHandler dataHandler(serverIp, serverPort);
    dataHandler.getStreamData();
    dataHandler.writeDataToFile();

    return 0;
}
