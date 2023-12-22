#pragma once

#include <vtkThreadedImageAlgorithm.h>
#include <vtkInformationVector.h>
#include <vtkImageData.h>
#include <vtkInformation.h>

class ImageSharpenFilter : public vtkThreadedImageAlgorithm
{
public:
    static ImageSharpenFilter* New();
    vtkTypeMacro(ImageSharpenFilter, vtkThreadedImageAlgorithm);

    int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;
    vtkSetMacro(SharpenCount, int);
    vtkGetMacro(SharpenCount, int);

private:
    void sharpen(vtkImageData* pInput, vtkImageData* pOut);

private:
    int SharpenCount = 0;
};