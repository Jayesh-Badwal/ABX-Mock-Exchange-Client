#include <fstream>
#include <map>
#include "TCPCLient.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class DataHandler
{
public:
    DataHandler(const std::string &_serverIp, const uint16_t &_serverPort)
        : serverIp(_serverIp), serverPort(_serverPort) {}

    void getStreamData()
    {
        TCPClient client(serverIp.c_str(), serverPort, [this](char *buffer, ssize_t receivedBytes)
                         { this->processData(buffer, receivedBytes); });
        client.connectToServer();

        // request payload to receive stream data
        RequestPayload req;
        req.callType = (char)1;

        client.sendRequest(req);
        client.receiveResponse();
    }

    void getMissedData(int seqNo)
    {
        TCPClient client(serverIp.c_str(), serverPort, [this](char *buffer, ssize_t receivedBytes)
                         { this->processData(buffer, receivedBytes); });
        client.connectToServer();

        // request payload to receive missed sequence data
        RequestPayload req;
        req.callType = (char)1;
        req.resendSeq = (char)seqNo;

        client.sendRequest(req);
        client.receiveResponse();
        client.closeConnection();
    }

    void writeDataToFile()
    {
        // recover missed sequence packets
        int lastSeqNo = 0;
        for (auto &it : packets)
        {
            while (lastSeqNo + 1 != it.first)
            {
                getMissedData(lastSeqNo + 1);
                ++lastSeqNo;
            }
        }

        // convert to a json array
        convertJSON();

        // write json array to file
        exportJSON();
    }

private:
    void processData(char *data, ssize_t receivedBytes)
    {
        // calculating number of packets received (17 is the size of each response payload packet received from server/exchange)
        int noOfPackets = receivedBytes / 17;

        // processing each response packet
        while (noOfPackets)
        {
            ResponsePayload *packet = new ResponsePayload();
            memcpy(packet->symbol, data, 4);
            data += 4;
            packet->side = *data;
            data += 1;
            packet->quantity = ntohl(*(int32_t *)data);
            data += 4;
            packet->price = ntohl(*(int32_t *)data);
            data += 4;
            packet->sequence = ntohl(*(int32_t *)data);
            data += 4;

            packets.insert({packet->sequence, packet});

            --noOfPackets;
        }
    }

    void convertJSON()
    {
        for (auto &it : packets)
        {
            // create json object
            json packetJson;
            packetJson["symbol"] = it.second->symbol;
            packetJson["side"] = std::string(1, it.second->side);
            packetJson["quantity"] = it.second->quantity;
            packetJson["price"] = it.second->price;
            packetJson["sequence"] = it.second->sequence;

            // add json object to json array
            jsonArray.push_back(packetJson);
        }
    }

    void exportJSON()
    {
        std::cout << "Writing data to file...\n";

        std::ofstream outputFile("output.json");
        if (outputFile.is_open())
        {
            outputFile << std::setw(4) << jsonArray << std::endl;
            outputFile.close();
            std::cout << "Data written successfully.\n";
        }
        else
            std::cerr << "Unable to open file.\n";
    }

    std::map<int, ResponsePayload *> packets;
    json jsonArray = json::array();
    std::string serverIp;
    uint16_t serverPort;
};