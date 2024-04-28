#include <ImGuiCommon.h>

namespace {
    void CreateColorImage(vtkImageData*, unsigned int channel);
    void CreateMask(vtkImageData*);
} // namespace

int main()
{
    SETUP_WINDOW

    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkImageData> imageRed;
    CreateColorImage(imageRed, 0);
    vtkNew<vtkImageData> imageGreen;
    CreateColorImage(imageGreen, 1);
    vtkNew<vtkImageData> mask;
    CreateMask(mask);

    vtkNew<vtkImageToImageStencil> imageToImageStencil;
    imageToImageStencil->SetInputData(mask);
    imageToImageStencil->ThresholdByUpper(122);

    vtkNew<vtkImageStencil> stencil;
    stencil->SetInputConnection(2, imageToImageStencil->GetOutputPort());
    stencil->SetBackgroundInputData(imageGreen);
    stencil->SetInputData(imageRed);
    stencil->ReverseStencilOn();
    stencil->Update();

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(stencil->GetOutputPort());
    ren->AddViewProp(actor);

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("imageToImageStencil", imageToImageStencil, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("stencil", stencil, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("vtkImageActor", actor);
    };

    // Start rendering app
    rw->Render();

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    // 💉 the overlay.
    dearImGuiOverlay->Inject(rwi);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    // Start event loop
#if 0
    renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
// 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, rw->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
    //vtkInteractorStyleSwitch::SafeDownCast(rwi->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    rwi->SetInteractorStyle(vtkNew<vtkInteractorStyleImage>{});
    rwi->EnableRenderOff();
    rwi->Start();
}

namespace {
    void CreateColorImage(vtkImageData* image, const unsigned int channel)
    {
        unsigned int dim = 20;

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
        unsigned int dim = 20;

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