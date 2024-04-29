#pragma once
#include <cstdlib>

struct RendererData {
public:
    //Byte array of raw imported data
    unsigned char* rawData;
    //Void pointer used by the renderer to store its specific data container to reference when rendering
    void* rendererData;

    RendererData(){}
    ~RendererData(){
        releaseRawData();
    }    

    void releaseRawData(){
        if(rawData != nullptr)
            free(rawData);
        rawData = nullptr;
    };
};