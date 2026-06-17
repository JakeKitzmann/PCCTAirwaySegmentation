#include "GroundTruthRegistration.h"
#include <GroundTruthRegistrationCLP.h>

int main(int argc, char* argv[])
{  
    PARSE_ARGS;

    auto imageIO = itk::ImageIOFactory::CreateImageIO(
        imgLow.c_str(), itk::ImageIOFactory::FileModeEnum::ReadMode);

    if (!imageIO)
    {
        std::cerr << "Could not read image: " << imgLow << std::endl;
        return EXIT_FAILURE;
    }

    imageIO->SetFileName(imgLow);
    imageIO->ReadImageInformation();

    auto componentType = imageIO->GetComponentType();

    if (componentType == itk::ImageIOBase::IOComponentEnum::SHORT)
    {
        Process<short>(imgLow, imgHigh, airwaySegLow, registeredImg, airwaySegHigh);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::FLOAT){
        Process<float>(imgLow, imgHigh, airwaySegLow, registeredImg, airwaySegHigh);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::DOUBLE){
        Process<double>(imgLow, imgHigh, airwaySegLow, registeredImg, airwaySegHigh);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::CHAR){
        Process<char>(imgLow, imgHigh, airwaySegLow, registeredImg, airwaySegHigh);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::INT){
        Process<int>(imgLow, imgHigh, airwaySegLow, registeredImg, airwaySegHigh);
    }
    else{
        std::cout << "Unsupported voxel type: " << componentType << std::endl;
        std::cout << "Note: this program does not support unsigned images due to negative HU values" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}