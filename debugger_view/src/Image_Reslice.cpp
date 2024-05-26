#include <ImGuiCommon.h>

vtkSmartPointer<vtkImageReslice> reslice;
double spacing[3];
vtkNew<vtkActor> centerPtActor;
vtkNew<vtkActor> meshActor;

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    int extent[6];
    double origin[3];
    img->GetExtent(extent);
    img->GetSpacing(::spacing);
    img->GetOrigin(origin);

    ::reslice = vtkSmartPointer<vtkImageReslice>::New();
    ::reslice->SetInputData(img);
    ::reslice->SetOutputDimensionality(2);

    ren->AddViewProp(::centerPtActor);
    ::centerPtActor->GetProperty()->SetColor(0, 1, 0);
    ::centerPtActor->GetProperty()->SetPointSize(18);

    ren->AddActor(::meshActor);
    ::meshActor->GetProperty()->SetPointSize(12);
    ::meshActor->GetProperty()->SetRepresentationToWireframe();
    ::meshActor->GetProperty()->SetColor(0, 1, 0);
    ::meshActor->GetProperty()->SetOpacity(.1);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false)->GetProperty()->SetColor(1., 1., 0.);

    {
        {
            ::reslice->SetResliceAxesDirectionCosines(1,0,0, 0,1,0, 0,0,1);
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
            ::reslice->SetResliceAxesOrigin(img->GetCenter());
        }

        {
            auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
                {
                    if (auto mat = vtkMatrix4x4::SafeDownCast(caller))
                    {
                        double src[4]{ 0,0,0,1 };
                        double dst[4];
                        mat->MultiplyPoint(src, dst);
                        vtkns::makePoints({ {dst[0], dst[1], dst[2]} }, ::centerPtActor);
                    }

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
    reslice->SetInterpolationModeToLinear();
    reslice->SetOutputOrigin(0, 0, 0);
    reslice->SetOutputExtent(0,400,0,400,0,400);
    reslice->SetOutputSpacing(::spacing);
    //::reslice->Update(); // 没有此句的话在一开始不能显示三维线框

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
    actor->GetProperty()->SetColorWindow(6000);
    actor->GetProperty()->SetColorLevel(2000);
    ren->AddActor(actor);

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("OriginalImageData", img);
        vtkns::vtkObjSetupWin("ResliceOutput", reslice->GetOutput());
        vtkns::vtkObjSetup("vtkImageActor", actor);
        vtkns::vtkObjSetup("Reslice", ::reslice, ImGuiTreeNodeFlags_DefaultOpen);
    };

    AFTER_MY_CODE
}