#include <ImGuiCommon.h>

vtkSmartPointer<vtkRenderWindowInteractor> iren;
vtkSmartPointer<vtkRenderWindow> renderWindow;
vtkSmartPointer<vtkRenderer> renderer;
vtkSmartPointer<vtkDICOMImageReader> reader;
ImageSharpenFilter* pMyFilter = nullptr;

int main(int argc, char* argv[])
{
    // Create a renderer, render window, and interactor
    renderer = vtkSmartPointer<vtkRenderer>::New();
    iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetMultiSamples(8);
    renderWindow->AddRenderer(renderer);
    iren->SetRenderWindow(renderWindow);

    reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetFileName(ImguiVtkNs::getDicomFile());
    reader->Update();

    vtkNew<ImageSharpenFilter> myFilter;
    ::pMyFilter = myFilter;
    myFilter->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(myFilter->GetOutputPort());
    renderer->AddActor(actor);

    ::pWindow = renderWindow;
    ::imgui_render_callback = [&]
    {
        ImGuiNs::vtkObjSetup("vtkImageData", ::reader->GetOutput());
        ImGuiNs::vtkObjSetup("vtkImageActor", actor);
        if (auto v = ::pMyFilter->GetSharpenCount(); ImGui::SliderInt("Sharpen", &v, 0, 100))
        {
            ::pMyFilter->SetSharpenCount(v);
        }
    };

    // Start rendering app
    renderer->SetBackground(0.2, 0.3, 0.4);
    renderWindow->Render();

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    // 💉 the overlay.
    dearImGuiOverlay->Inject(iren);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    ImguiVtkNs::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    ImguiVtkNs::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    vtkNew<vtkCameraOrientationWidget> camManipulator;
    camManipulator->SetParentRenderer(renderer);
    camManipulator->On();
    auto rep = vtkCameraOrientationRepresentation::SafeDownCast(camManipulator->GetRepresentation());
    rep->AnchorToLowerRight();

    // Start event loop
#if 0
    renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
// 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, renderWindow->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}