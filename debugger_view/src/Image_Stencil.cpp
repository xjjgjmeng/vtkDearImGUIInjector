#include <ImGuiCommon.h>

namespace {
    void CreateColorImage(vtkImageData*, unsigned int channel);
    void CreateMask(vtkImageData*);
} // namespace

int main()
{
    BEFORE_MY_CODE
    rwi->SetInteractorStyle(vtkNew<vtkInteractorStyleImage>{});

    vtkNew<vtkImageData> imageRed;
    CreateColorImage(imageRed, 0);
    vtkNew<vtkImageData> imageGreen;
    CreateColorImage(imageGreen, 1);
    vtkNew<vtkImageData> mask;
    CreateMask(mask);

    vtkNew<vtkImageToImageStencil> imageToImageStencil;
    imageToImageStencil->SetInputData(mask);
    imageToImageStencil->ThresholdByUpper(122);

    vtkNew<vtkImageStencil> imgStencil;
#if 0
    imgStencil->SetInputConnection(2, imageToImageStencil->GetOutputPort());
#else
    imgStencil->SetStencilConnection(imageToImageStencil->GetOutputPort());
#endif
    //imgStencil->SetBackgroundInputData(imageGreen);
    //imgStencil->SetBackgroundColor(1, 1, 0, 1);
    imgStencil->SetBackgroundValue(123);
    imgStencil->SetInputData(imageRed);
    imgStencil->ReverseStencilOn();
    imgStencil->Update();

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(imgStencil->GetOutputPort());
    ren->AddViewProp(actor);

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("imageToImageStencil", imageToImageStencil, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("imgStencil", imgStencil, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("vtkImageActor", actor);
    };

    AFTER_MY_CODE
}

namespace {
    void CreateColorImage(vtkImageData* image, const unsigned int channel)
    {
        unsigned int dim = 200;

        image->SetDimensions(dim, dim, 1);
        image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

        for (unsigned int x = 0; x < dim; x++)
        {
            for (unsigned int y = 0; y < dim; y++)
            {
                unsigned char* pixel =
                    static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 0;

                pixel[channel] = 255;
            }
        }

        image->Modified();
    }

    void CreateMask(vtkImageData* image)
    {
        unsigned int dim = 200;

        image->SetDimensions(dim, dim, 1);
        image->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

        for (unsigned int x = 0; x < dim; x++)
        {
            for (unsigned int y = 0; y < dim; y++)
            {
                unsigned char* pixel =
                    static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));
                if (x < dim / 4)
                {
                    pixel[0] = 0;
                }
                else
                {
                    pixel[0] = 255;
                }
            }
        }

        image->Modified();
    }
} // namespace