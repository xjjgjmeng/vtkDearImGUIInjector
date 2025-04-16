#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();

    vtkNew<vtkImageReslice> reslice;
    reslice->SetInputData(img);
    reslice->SetOutputDimensionality(2);
    reslice->SetInterpolationModeToLinear();
    reslice->SetResliceAxesDirectionCosines(1, 0, 0, 0, 1, 0, 0, 0, 1);
    reslice->SetResliceAxesOrigin(img->GetCenter());

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
    actor->GetProperty()->SetColorWindow(6000);
    actor->GetProperty()->SetColorLevel(2000);
    ren->AddActor(actor);

    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("Reslice", reslice);
            vtkns::vtkObjSetup("ImgActor", actor);
            if (ImGui::Button("SaveImg2Png"))
            {
                vtkNew<vtkImageMapToWindowLevelColors> windowLevelFilter;
                auto img = actor->GetInput();
                auto rng = img->GetScalarRange();
                windowLevelFilter->SetInputData(img);
                windowLevelFilter->SetWindow(actor->GetProperty()->GetColorWindow());
                windowLevelFilter->SetLevel(actor->GetProperty()->GetColorLevel());
                windowLevelFilter->SetOutputFormatToLuminance();

                vtkNew<vtkPNGWriter> writer;
                writer->SetCompressionLevel(0);
                writer->SetFileName("666.png");
                writer->SetInputConnection(windowLevelFilter->GetOutputPort());
                writer->Write();
            }
            ImGui::SameLine();
            if (ImGui::Button("SaveImg2PngWithText"))
            {
                auto img = actor->GetInput();
                const auto dims = img->GetDimensions();
                const auto imgWidth = dims[0];
                const auto imgHeight = dims[1];

                vtkNew<vtkRenderer> renderer;
                renderer->SetBackground(0,0,0);

                {
                    vtkNew<vtkImageActor> imgActor;
                    imgActor->GetMapper()->SetInputData(img);
                    imgActor->GetProperty()->SetColorWindow(actor->GetProperty()->GetColorWindow());
                    imgActor->GetProperty()->SetColorLevel(actor->GetProperty()->GetColorLevel());
                    imgActor->SetDisplayExtent(0, imgWidth - 1, 0, imgHeight - 1, 0, 0);
                    renderer->AddActor(imgActor);

                    vtkNew<vtkTextActor> textActor;
                    textActor->SetInput("12345hello");
                    auto textProperty = textActor->GetTextProperty();
                    textProperty->SetFontSize(24);
                    textProperty->SetColor(1, 1, 1);
                    int padding = 10;
                    int textPosY = imgHeight - textProperty->GetFontSize() - padding;
                    textActor->SetPosition(padding, textPosY);
                    renderer->AddActor2D(textActor);
                }

                vtkNew<vtkRenderWindow> renWin;
                renWin->AddRenderer(renderer);
                renWin->SetSize(imgWidth, imgHeight);
                renWin->OffScreenRenderingOn();
                renWin->Render();

                vtkNew<vtkWindowToImageFilter> windowToImageFilter;
                windowToImageFilter->SetInput(renWin);
                windowToImageFilter->SetInputBufferTypeToRGB();

                vtkNew<vtkImageLuminance> luminanceFilter;
                luminanceFilter->SetInputConnection(windowToImageFilter->GetOutputPort());

                vtkNew<vtkPNGWriter> writer;
                writer->SetCompressionLevel(0);
                writer->SetFileName("666.png");
                writer->SetInputConnection(luminanceFilter->GetOutputPort());
                writer->Write();
            }
            ImGui::SameLine();
            if (ImGui::Button("SaveWin2Png"))
            {
                vtkNew<vtkWindowToImageFilter> windowToImage;
                windowToImage->SetInput(rw);
                windowToImage->SetScale(1); // 控制图像分辨率（默认1，可调高为2或3）
                windowToImage->SetInputBufferTypeToRGB(); // 捕获 RGB 数据
                windowToImage->ReadFrontBufferOff(); // 避免读取前端缓冲区

                vtkNew<vtkPNGWriter> writer;
                writer->SetCompressionLevel(0);
                writer->SetFileName("666.png");
                writer->SetInputConnection(windowToImage->GetOutputPort());
                writer->Write();
            }
        };

    AFTER_MY_CODE
}