#include "AirwayRegistration.h"
#include <AirwayRegistrationCLP.h>

int main(int argc, char* argv[])
{  
    PARSE_ARGS;

    auto imageIO = itk::ImageIOFactory::CreateImageIO(
        movingImg.c_str(), itk::ImageIOFactory::FileModeEnum::ReadMode);

    if (!imageIO)
    {
        std::cerr << "Could not read image: " << movingImg << std::endl;
        return EXIT_FAILURE;
    }

    imageIO->SetFileName(movingImg);
    imageIO->ReadImageInformation();

    auto componentType = imageIO->GetComponentType();

    if (componentType == itk::ImageIOBase::IOComponentEnum::SHORT)
    {
        Process<short>(movingImg, fixedImg, registeredImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::FLOAT){
        Process<float>(movingImg, fixedImg, registeredImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::DOUBLE){
        Process<double>(movingImg, fixedImg, registeredImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::CHAR){
        Process<char>(movingImg, fixedImg, registeredImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::INT){
        Process<int>(movingImg, fixedImg, registeredImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::UCHAR){
        Process<char>(movingImg, fixedImg, registeredImg);
    }
    else{
        std::cout << "Unsupported voxel type: " << componentType << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

