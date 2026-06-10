#include <RunRegistrationCLP.h>
#include <iostream>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageIOFactory.h>
#include <itkImageIOBase.h>
#include <itkImageRegistrationMethodv4.h>
#include <itkAffineTransform.h>
#include <itkResampleImageFilter.h>
#include "itkRegularStepGradientDescentOptimizerv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkCastImageFilter.h"
#include "itkCenteredTransformInitializer.h"
#include "itkRegistrationParameterScalesFromPhysicalShift.h"
#include "itkCommand.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"


constexpr int Dimension = 3;

template <typename TPixel>
typename itk::Image<TPixel, Dimension>::Pointer Preprocess(const std::string& imgLow)
    {
    using ImageType = itk::Image<TPixel, Dimension>;
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(imgLow);
    reader->Update();
    typename ImageType::Pointer output = reader->GetOutput();
    return output;
}

// show iterations of registration
class CommandIterationUpdate : public itk::Command
{
    public:
        using Self = CommandIterationUpdate;
        using Superclass = itk::Command;
        using Pointer = itk::SmartPointer<Self>;
        itkNewMacro(Self);

        using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
        using OptimizerPointer = const OptimizerType*;

        void Execute(itk::Object* caller, const itk::EventObject& event) override
        {
            Execute((const itk::Object*)caller, event);
        }

        void Execute(const itk::Object* object, const itk::EventObject& event) override
        {
            auto optimizer = dynamic_cast<OptimizerPointer>(object);
            if (!itk::IterationEvent().CheckEvent(&event))
            {
                return;
            }
            std::cout << optimizer->GetCurrentIteration() << " = "
                    << optimizer->GetValue() << " : "
                    << optimizer->GetCurrentPosition()
                    << "  step: " << optimizer->GetCurrentStepLength()
                    << std::endl;
        }

    protected:
        CommandIterationUpdate() = default;
};

template <typename TPixel>
auto Registration(typename itk::Image<TPixel, Dimension>::Pointer imgLow, typename itk::Image<TPixel, Dimension>::Pointer imgHigh){
    
    std::cout << "Registering high and low resolution images..." << std::endl;

    using PrevImageType = itk::Image<TPixel, Dimension>;
    using RegistrationImageType = itk::Image<double, Dimension>;

    using CastFilterType = itk::CastImageFilter<PrevImageType, RegistrationImageType>;

    auto movingCaster = CastFilterType::New();
    movingCaster->SetInput(imgLow);
    movingCaster->Update();
    const auto movingImage = movingCaster->GetOutput();

    auto fixedCaster = CastFilterType::New();
    fixedCaster->SetInput(imgHigh);
    fixedCaster->Update();
    const auto fixedImage = fixedCaster->GetOutput();

    using TransformType = itk::AffineTransform<double, Dimension>;
    auto initialTransform = TransformType::New();

    using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
    auto optimizer = OptimizerType::New();
    optimizer->SetMinimumStepLength(0.001);
    optimizer->SetRelaxationFactor(0.5);
    optimizer->SetNumberOfIterations(200);

    auto observer = CommandIterationUpdate::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    // scale w/ physical shift
    using MetricType = itk::MeanSquaresImageToImageMetricv4<RegistrationImageType, RegistrationImageType>;
    auto metric = MetricType::New();

    using ScalesEstimatorType = itk::RegistrationParameterScalesFromPhysicalShift<MetricType>;
    auto scalesEstimator = ScalesEstimatorType::New();
    scalesEstimator->SetMetric(metric);
    optimizer->SetScalesEstimator(scalesEstimator);
    optimizer->SetLearningRate(1.0);

    using RegistrationType = itk::ImageRegistrationMethodv4<RegistrationImageType, RegistrationImageType, TransformType>;    
    auto registration = RegistrationType::New();
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    registration->SetFixedImage(fixedImage);
    registration->SetMovingImage(movingImage);

    using InitializerType =
        itk::CenteredTransformInitializer<TransformType,
                                        RegistrationImageType,
                                        RegistrationImageType>;
    auto initializer = InitializerType::New();
    initializer->SetTransform(initialTransform);
    initializer->SetFixedImage(fixedImage);
    initializer->SetMovingImage(movingImage);
    initializer->GeometryOn();          // align geometric centers
    initializer->InitializeTransform();

    registration->SetInitialTransform(initialTransform);

    // multi-resolution pyramid
    // NOTE THIS IS DOWNSAMPLED SO THAT I CAN RUN QUICKLY
    RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
    shrinkFactorsPerLevel.SetSize(3);
    shrinkFactorsPerLevel[0] = 4;   // coarsest
    shrinkFactorsPerLevel[1] = 2;
    shrinkFactorsPerLevel[2] = 1;   // full resolution
    registration->SetShrinkFactorsPerLevel(shrinkFactorsPerLevel);

    RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
    smoothingSigmasPerLevel.SetSize(3);
    smoothingSigmasPerLevel[0] = 2;   // most smoothing
    smoothingSigmasPerLevel[1] = 1;
    smoothingSigmasPerLevel[2] = 0;   // none
    registration->SetSmoothingSigmasPerLevel(smoothingSigmasPerLevel);

    // run
    try
    {
        registration->Update();
        std::cout << "Optimizer stop condition: " << registration->GetOptimizer()->GetStopConditionDescription()
                << std::endl;
    }
    catch (const itk::ExceptionObject & err)
    {
        std::cerr << "Registration failed!" << std::endl;
        std::cerr << err << std::endl;
    }

    typename TransformType::ConstPointer transform = registration->GetTransform();
    auto finalParameters = transform->GetParameters();
    auto numberOfIterations = optimizer->GetCurrentIteration();
    auto bestValue = optimizer->GetValue();

    std::cout << "Result = " << std::endl;
    std::cout << " Final Parameters = " << finalParameters << std::endl;
    std::cout << " Iterations    = " << numberOfIterations << std::endl;
    std::cout << " Metric value  = " << bestValue << std::endl;

    return transform;
}

template <typename TPixel,
          template <typename, typename> class TInterpolator = itk::LinearInterpolateImageFunction>
typename itk::Image<TPixel, Dimension>::Pointer
Resample(typename itk::Image<TPixel, Dimension>::Pointer inputImage,
         typename itk::Image<TPixel, Dimension>::Pointer referenceImage,
         const itk::AffineTransform<double, Dimension>* transform)
{
    using ImageType = itk::Image<TPixel, Dimension>;

    using InterpolatorType = TInterpolator<ImageType, double>;
    auto interpolator = InterpolatorType::New();

    using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
    auto resampleFilter = ResampleFilterType::New();

    resampleFilter->SetInput(inputImage);
    resampleFilter->SetTransform(transform);
    resampleFilter->SetInterpolator(interpolator);

    // Output grid = the fixed (high-res) image's grid
    resampleFilter->SetReferenceImage(referenceImage);
    resampleFilter->UseReferenceImageOn();

    resampleFilter->SetDefaultPixelValue(0);
    resampleFilter->Update();

    typename ImageType::Pointer output = resampleFilter->GetOutput();
    output->DisconnectPipeline();
    return output;
}


template <typename TPixel>
auto Process(const std::string& imgLow, std::string& imgHigh, std::string& airwaySegLow){
    auto imgLow_im = Preprocess<TPixel>(imgLow);
    auto imgHigh_im = Preprocess<TPixel>(imgHigh);
    auto airwaySegLow_im = Preprocess<TPixel>(airwaySegLow);

    auto transform = Registration<TPixel>(imgLow_im, imgHigh_im);

    std::cout << "Resampling low res image..." << std::endl;
    auto resampledLowResImage = Resample<TPixel>(imgLow_im, imgHigh_im, transform);
    std::cout << "Resampling low res mask..." << std::endl;
    auto resampledMask = Resample<TPixel, itk::NearestNeighborInterpolateImageFunction>(
        airwaySegLow_im, imgHigh_im, transform);

    itk::WriteImage(resampledLowResImage, "test.nii.gz");
    itk::WriteImage(resampledMask, "testMask.nii.gz");
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

    if (componentType == itk::ImageIOBase::IOComponentEnum::SHORT)
    {
        Process<short>(imgLow, imgHigh, airwaySegLow);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::FLOAT){
        Process<float>(imgLow, imgHigh, airwaySegLow);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::DOUBLE){
        Process<double>(imgLow, imgHigh, airwaySegLow);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::CHAR){
        Process<char>(imgLow, imgHigh, airwaySegLow);
    }
    else if(componentType == itk::ImageIOBase::IOComponentEnum::INT){
        Process<int>(imgLow, imgHigh, airwaySegLow);
    }
    else{
        std::cout << "Unsupported voxel type: " << componentType << std::endl;
        std::cout << "Note: this program does not support unsigned images due to negative HU values" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}