#ifndef ENGINEUTILS_H
#define ENGINEUTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <iostream>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

class EngineUtils
{
public:
    static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document)
    {
        FILE* file_pointer = nullptr;

    #ifdef _WIN32
        fopen_s(&file_pointer, path.c_str(), "rb");
    #else
        file_pointer = fopen(path.c_str(), "rb");
    #endif

        char buffer[65536];
        rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
        out_document.ParseStream(stream);

        std::fclose(file_pointer);

        if (out_document.HasParseError())
        {
            std::cout << "error parsing json at [" << path << "]";
            std::exit(0);
        }
    }
};

#endif // ENGINEUTILS_H
