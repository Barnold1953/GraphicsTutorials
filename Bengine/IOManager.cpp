#include "IOManager.h"

#include <fstream>

namespace Bengine {

    bool IOManager::readFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer) {
        std::ifstream file(filePath, std::ios::binary);
        if (file.fail()) {
            perror(filePath.c_str());
            return false;
        }

        //seek to the end
        file.seekg(0, std::ios::end);

        //Get the file size
        unsigned int fileSize = (unsigned int)file.tellg();
        file.seekg(0, std::ios::beg);

        //Reduce the file size by any header bytes that might be present
        fileSize -= (unsigned int)file.tellg();

        buffer.resize(fileSize);
        file.read((char *)&(buffer[0]), fileSize);
        file.close();

        return true;
    }

    bool IOManager::readFileToBuffer(std::string filePath, std::string& buffer) {
        std::ifstream file(filePath, std::ios::binary);
        if (file.fail()) {
            perror(filePath.c_str());
            return false;
        }

        //seek to the end
        file.seekg(0, std::ios::end);

        //Get the file size
        unsigned int fileSize = (unsigned int)file.tellg();
        file.seekg(0, std::ios::beg);

        //Reduce the file size by any header bytes that might be present
        fileSize -= (unsigned int)file.tellg();

        buffer.resize(fileSize);
        file.read((char *)&(buffer[0]), fileSize);
        file.close();

        return true;
    }

}