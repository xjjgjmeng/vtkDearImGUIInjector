#include <ImGuiCommon.h>

vtkns::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(vtkns::getDicomDir());
    reader->Update();
    auto pImageData = reader->GetOutput();

    vtkNew<vtkVolume> pVolume;
    vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
    pMapper->SetInputData(pImageData);
    pMapper->SetBlendModeToComposite();
    pVolume->SetMapper(pMapper);
    ::setupDefaultVolumeProperty(pVolume);
    ren->AddVolume(pVolume);
    ren->ResetCamera();

    {
        vtkNew<vtkImageData> imageData;
        imageData->AllocateScalars(VTK_SHORT, 1);
        imageData->SetDimensions(pImageData->GetDimensions());
        imageData->SetSpacing(pImageData->GetSpacing());
        //imageData->SetExtent(mprData->getImageData()->GetExtent());
        imageData->SetOrigin(pImageData->GetOrigin());

        vtkNew<vtkShortArray> data;
        short* dPtr = (short*)calloc(imageData->GetNumberOfPoints(), sizeof(short));
        data->SetArray(dPtr, imageData->GetNumberOfPoints(), 1);

        // add the field to the data set
        imageData->GetPointData()->SetScalars(data);

        vtkNew<vtkGPUVolumeRayCastMapper> volumeMapper;
        volumeMapper->SetInputData(imageData);
        volumeMapper->SetBlendModeToComposite();

        vtkNew<vtkVolumeProperty> volumeProperty;
        vtkNew<vtkPiecewiseFunction> scalarTransparency;
        vtkNew<vtkColorTransferFunction> colortTransfer;
        volumeProperty->ShadeOn();
        volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
        volumeProperty->SetScalarOpacity(scalarTransparency);
        volumeProperty->SetColor(colortTransfer);
        scalarTransparency->AddPoint(0.0, 0.0);
        scalarTransparency->AddPoint(3000.0, 1.0);
        colortTransfer->AddRGBPoint(-1000.0, 1.0, 0.0, 0.0);
        colortTransfer->AddRGBPoint(3095.0, 1.0, 0.0, 0.0);
        for (int i = imageData->GetDimensions()[0] * 0.3; i < imageData->GetDimensions()[0] * 0.7; ++i) {
            for (int j = imageData->GetDimensions()[1] * 0.2; j < imageData->GetDimensions()[1] * 0.8; ++j) {
                for (int k = imageData->GetDimensions()[2] * 0.2; k < imageData->GetDimensions()[2] * 0.8; ++k) {
                    //imageData->SetScalarComponentFromFloat(i, j, k, 0, -1000.0);
                    if (pImageData->GetScalarComponentAsFloat(i, j, k, 0) <= -500 && pImageData->GetScalarComponentAsFloat(i, j, k, 0) > -1000) {
                        imageData->SetScalarComponentFromFloat(i, j, k, 0, 3000.0);
                    }
                }
            }
        }

        auto mpVolumeActor = vtkSmartPointer<vtkVolume>::New();
        mpVolumeActor->SetMapper(volumeMapper);
        mpVolumeActor->SetProperty(volumeProperty);
        ren->AddVolume(mpVolumeActor);
    }

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                logView.Draw();
                ImGui::TreePop();
            }
            vtkns::vtkObjSetup("Volume", pVolume, ImGuiTreeNodeFlags_DefaultOpen);
        };

    // Start rendering app
    ren->SetBackground(0., 0., 0.);
    renWin->Render(); // 非常重要！！

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    // 💉 the overlay.
    dearImGuiOverlay->Inject(iren);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    vtkNew<vtkCameraOrientationWidget> camManipulator;
    camManipulator->SetParentRenderer(ren);
    camManipulator->On();
    auto rep = vtkCameraOrientationRepresentation::SafeDownCast(camManipulator->GetRepresentation());
    rep->AnchorToLowerRight();

    // Start event loop
#if 0
    renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
    // 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, renWin->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();
    return 0;
}