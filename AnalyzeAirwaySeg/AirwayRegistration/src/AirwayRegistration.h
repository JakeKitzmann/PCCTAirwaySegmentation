#pragma once

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
#include "itkEuler3DTransform.h"

constexpr int Dimension = 3;

template <typename TPixel>
typename itk::Image<TPixel, Dimension>::Pointer Preprocess(const std::string& img)
    {
    using ImageType = itk::Image<TPixel, Dimension>;
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(img);
    reader->Update();
    typename ImageType::Pointer output = reader->GetOutput();
    return output;
}

template <typename TPixel>
void WriteImageT(typename itk::Image<TPixel, Dimension>::Pointer img, const std::string& filename)
{
    using WriterType = itk::ImageFileWriter<itk::Image<TPixel, Dimension>>;
    auto writer = WriterType::New();
    writer->SetFileName(filename);
    writer->SetInput(img);
    writer->Update();
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
auto Registration(typename itk::Image<TPixel, Dimension>::Pointer movingImg, typename itk::Image<TPixel, Dimension>::Pointer fixedImg){
    
    using PrevImageType = itk::Image<TPixel, Dimension>;
    using RegistrationImageType = itk::Image<double, Dimension>;

    using CastFilterType = itk::CastImageFilter<PrevImageType, RegistrationImageType>;

    auto movingCaster = CastFilterType::New();
    movingCaster->SetInput(movingImg);
    movingCaster->Update();
    const auto movingImage = movingCaster->GetOutput();

    auto fixedCaster = CastFilterType::New();
    fixedCaster->SetInput(fixedImg);
    fixedCaster->Update();
    const auto fixedImage = fixedCaster->GetOutput();

    using TransformType = itk::Euler3DTransform<double>;
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

template <typename TPixel>
typename itk::Image<TPixel, Dimension>::Pointer
Resample(typename itk::Image<TPixel, Dimension>::Pointer inputImage,
         typename itk::Image<TPixel, Dimension>::Pointer referenceImage,
         const itk::Euler3DTransform<double>* transform)
{
    using ImageType = itk::Image<TPixel, Dimension>;

    using InterpolatorType = itk::NearestNeighborInterpolateImageFunction<ImageType, double>;
    auto interpolator = InterpolatorType::New();

    using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
    auto resampleFilter = ResampleFilterType::New();

    resampleFilter->SetInput(inputImage);
    resampleFilter->SetTransform(transform);
    resampleFilter->SetInterpolator(interpolator);

    resampleFilter->SetSize(referenceImage->GetLargestPossibleRegion().GetSize());
    resampleFilter->SetOutputSpacing(referenceImage->GetSpacing());
    resampleFilter->SetOutputOrigin(referenceImage->GetOrigin());
    resampleFilter->SetOutputDirection(referenceImage->GetDirection());

    resampleFilter->SetDefaultPixelValue(0);
    resampleFilter->Update();

    typename ImageType::Pointer output = resampleFilter->GetOutput();
    output->DisconnectPipeline();
    return output;
}


template <typename TPixel>
int Process(const std::string& movingImg, const std::string& fixedImg, const std::string& registeredImg){
    auto movingImage = Preprocess<TPixel>(movingImg);
    auto fixedImage = Preprocess<TPixel>(fixedImg);

    auto transform = Registration<TPixel>(movingImage, fixedImage);

    std::cout << "Resampling moving image..." << std::endl;
    auto resampled = Resample<TPixel>(movingImage, fixedImage, transform);

    WriteImageT<TPixel>(resampled, registeredImg);

    return 0;
    }
