#include <ImGuiCommon.h>

vtkSmartPointer<vtkImageReslice> reslice;
vtkNew<vtkActor> centerPtActor;
vtkNew<vtkActor> meshActor;

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);
    auto volume = vtkns::genVR(ren, img, false, false);

    ::reslice = vtkSmartPointer<vtkImageReslice>::New();
    ::reslice->SetInputData(img);
    ::reslice->SetOutputDimensionality(2);
    ::reslice->SetInterpolationModeToLinear();
    ::reslice->SetOutputSpacing(img->GetSpacing());

    ren->AddViewProp(::centerPtActor);
    ::centerPtActor->GetProperty()->SetColor(0, 1, 1);
    ::centerPtActor->GetProperty()->SetPointSize(21);
    ::centerPtActor->GetProperty()->SetRenderPointsAsSpheres(true);

    ren->AddActor(::meshActor);
    ::meshActor->GetProperty()->SetPointSize(12);
    ::meshActor->GetProperty()->SetRepresentationToWireframe();
    ::meshActor->GetProperty()->SetColor(0, 1, 0);
    ::meshActor->GetProperty()->SetOpacity(.1);
    ::meshActor->SetVisibility(0);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false)->GetProperty()->SetColor(1., 1., 0.);

    {
        {
            ::reslice->SetResliceAxesDirectionCosines(1,0,0, 0,1,0, 0,0,1);
#if 1 // 显示切割平面
            {
                auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
                    {
                        vtkns::mat::genAxes(reinterpret_cast<vtkRenderer*>(clientdata), vtkMatrix4x4::SafeDownCast(caller));
                    };
                vtkNew<vtkCallbackCommand> pCC;
                pCC->SetCallback(f);
                pCC->SetClientData(ren);
                reslice->GetResliceAxes()->AddObserver(vtkCommand::ModifiedEvent, pCC);
            }
#endif
            ::reslice->SetResliceAxesOrigin(img->GetCenter());
        }

        {
            auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
                {
                    // 将当前矩阵的origin显示出来
                    if (auto mat = vtkMatrix4x4::SafeDownCast(caller))
                    {
                        double src[4]{ 0,0,0,1 };
                        double dst[4];
                        mat->MultiplyPoint(src, dst);
                        vtkns::makePoints({ {dst[0], dst[1], dst[2]} }, ::centerPtActor);
                        {
                            vtkns::log("{:.2f} {:.2f} {:.2f}", dst[0], dst[1], dst[2]);
                            vtkns::log("{:.2f} {:.2f} {:.2f}", mat->GetElement(0,3),mat->GetElement(1, 3),mat->GetElement(2, 3));
                        }
                    }
                    // 将输出的数据用网格的形式显示出来
                    if (auto img = vtkImageData::SafeDownCast(caller))
                    {
                        vtkNew<vtkDataSetMapper> mapper;
                        vtkNew<vtkImageData> data;
                        data->SetOrigin(img->GetOrigin());
                        data->SetSpacing(img->GetSpacing());
                        data->SetExtent(img->GetExtent());
                        mapper->SetInputData(data);
                        ::meshActor->SetMapper(mapper);
                        ::meshActor->SetPosition(0, 0, 1); // 设置偏移方便看到后面的图像
                    }
                };
            vtkNew<vtkCallbackCommand> pCC;
            pCC->SetCallback(f);
            ::reslice->GetResliceAxes()->AddObserver(vtkCommand::ModifiedEvent, pCC);
            ::reslice->GetOutput()->AddObserver(vtkCommand::ModifiedEvent, pCC);
        }
    }
#if 1
    reslice->SetOutputOrigin(0, 0, 0);
    reslice->SetOutputExtent(0, 400, 0, 400, 0, 400);
    // 如果没有指定Origin和Extend，输出的图像的Extend会从（0，0，0）开始，扩展到包围整个裁切图像，输出图像的Origin指的是extent (0,0,0)的世界坐标
#endif
    //::reslice->Update(); // 没有此句的话在一开始不能显示三维线框

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
    actor->GetProperty()->SetColorWindow(6000);
    actor->GetProperty()->SetColorLevel(2000);
    ren->AddActor(actor);

    ::imgui_render_callback = [&]
        {
            if (bool v = ::meshActor->GetVisibility(); ImGui::Checkbox("ShowMesh", &v))
            {
                ::meshActor->SetVisibility(v);
            }
            ImGui::SameLine();
            if (bool v = volume->GetVisibility(); ImGui::Checkbox("ShowVolume", &v))
            {
                volume->SetVisibility(v);
            }
            vtkns::vtkObjSetup("OriginalImageData", img);
            vtkns::vtkObjSetupWin("ResliceOutput", reslice->GetOutput());
            vtkns::vtkObjSetup("vtkImageActor", actor);
            vtkns::vtkObjSetup("Reslice", ::reslice, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}