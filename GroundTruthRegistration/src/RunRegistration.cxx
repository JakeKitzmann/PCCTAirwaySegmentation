#include <RunRegistrationCLP.h>
#include <iostream>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageIOFactory.h>
#include <itkImageIOBase.h>

constexpr int Dimension = 3;

template <typename TPixel>
typename itk::Image<TPixel, Dimension>::Pointer Preprocess(const std::string& imgLow)
    {
    using ImageType = itk::Image<TPixel, 3>;
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(imgLow);
    reader->Update();
    typename ImageType::Pointer output = reader->GetOutput();
    return output;
}

template <typename TPixel>
auto Process(const std::string& imgLow, std::string& imgHigh, std::string& airwaySegLow){
    auto imgLow_im = Preprocess<TPixel>(imgLow);
    auto imgHigh_im = Preprocess<TPixel>(imgHigh);
    auto airwaySegLow_im = Preprocess<TPixel>(airwaySegLow);

    
}

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
    std::cout << "Component type: " << imageIO->GetComponentTypeAsString(componentType) << std::endl;

    if (componentType == itk::ImageIOBase::IOComponentEnum::SHORT)
    {
        Process<short>(imgLow, imgHigh, airwaySegLow);
    }
    else{
        std::cout << "Invalid image type" << std::endl;
        return EXIT_FAILURE;
    }


    std::cout << "ran" << std::endl;
    return EXIT_SUCCESS;
}