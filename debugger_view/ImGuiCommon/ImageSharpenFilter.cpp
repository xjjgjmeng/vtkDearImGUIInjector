#include "ImageSharpenFilter.h"

#include <vtkImageGaussianSmooth.h>
#include <vtkSmartPointer.h>
#include <vtkImageCast.h>
#include <vtkImageMathematics.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(ImageSharpenFilter);

int ImageSharpenFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    auto input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    auto output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    this->sharpen(input, output);
    return 1;
}

void ImageSharpenFilter::sharpen(vtkImageData* pIn, vtkImageData* pOut)
{
    auto gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussianSmoothFilter->SetInputData(pIn);
    gaussianSmoothFilter->SetDimensionality(2);
    gaussianSmoothFilter->SetRadiusFactor(1.5); //设置模板范围
    gaussianSmoothFilter->SetStandardDeviation(3);//正态分布/高斯分布标准差
    gaussianSmoothFilter->Update();

    auto castGaussian = vtkSmartPointer<vtkImageCast>::New();
    castGaussian->SetInputData(gaussianSmoothFilter->GetOutput());
    castGaussian->SetOutputScalarTypeToFloat();
    castGaussian->Update();

    auto castOriginal = vtkSmartPointer<vtkImageCast>::New();
    castOriginal->SetInputData(pIn);
    castOriginal->SetOutputScalarTypeToFloat();
    castOriginal->Update();

    auto imgMathSubtract = vtkSmartPointer<vtkImageMathematics>::New();
    imgMathSubtract->SetInput1Data(castOriginal->GetOutput());
    imgMathSubtract->SetInput2Data(castGaussian->GetOutput());
    imgMathSubtract->SetOperationToSubtract(); // mask = src - blur
    imgMathSubtract->Update();

    auto imgMathMulti = vtkSmartPointer<vtkImageMathematics>::New();
    imgMathMulti->SetInput1Data(imgMathSubtract->GetOutput());
    imgMathMulti->SetOperationToMultiplyByK();
    imgMathMulti->SetConstantK(SharpenCount * 0.02);
    imgMathMulti->Update();

    auto imgMathAdd = vtkSmartPointer<vtkImageMathematics>::New();
    imgMathAdd->SetInput1Data(castOriginal->GetOutput());
    imgMathAdd->SetInput2Data(imgMathMulti->GetOutput());
    imgMathAdd->SetOperationToAdd();
    imgMathAdd->Update();

    pOut->DeepCopy(imgMathAdd->GetOutput());
}
