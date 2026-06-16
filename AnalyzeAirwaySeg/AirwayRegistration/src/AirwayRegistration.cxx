#include "GroundTruthRegistration.h"
#include <RunRegistrationCLP.h>

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

    imageIO->SetFileName(imgLow);
    imageIO->ReadImageInformation();

    auto componentType = imageIO->GetComponentType();

    if (componentType == itk::ImageIOBase::IOComponentEnum::SHORT)
    {
        Process<short>(movingImg, fixedImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::FLOAT){
        Process<float>(movingImg, fixedImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::DOUBLE){
        Process<double>(movingImg, fixedImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::CHAR){
        Process<char>(movingImg, fixedImg);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::INT){
        Process<int>(movingImg, fixedImg);
    }
    else{
        std::cout << "Unsupported voxel type: " << componentType << std::endl;
        std::cout << "Note: this program does not support unsigned images due to negative HU values" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

