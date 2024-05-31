#include <ImGuiCommon.h>
#include <implot.h>

namespace {
    class vtkBoxCallback : public vtkCommand
    {
    public:
        static vtkBoxCallback* New()
        {
            return new vtkBoxCallback;
        }

        void Execute(vtkObject* caller, unsigned long, void*) override
        {
            auto boxWidget = vtkBoxWidget2::SafeDownCast(caller);
            vtkNew<vtkTransform> t;
            vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation())->GetTransform(t);
            this->m_actor->SetUserTransform(t);
        }

        vtkSmartPointer<vtkProp3D> m_actor;
    };
} // namespace

int main(int argc, char* argv[])
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();

    // 显示体数据用于对比
#if 0
    {
        vtkNew<vtkRenderer> ren;
        ren->SetViewport(0.8, 0.5, 1, 1);
        renWin->AddRenderer(ren);

        vtkNew<vtkVolume> pVolume;
        vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
        pMapper->SetInputData(reader->GetOutput());
        pMapper->SetBlendModeToComposite();
        pVolume->SetMapper(pMapper);
        ::setupDefaultVolumeProperty(pVolume);
        ren->AddVolume(pVolume);
        ren->ResetCamera();
    }
#endif

    // 只能处理一张图？？
    vtkNew<vtkImageToPolyDataFilter> imgPolyFilter;
    imgPolyFilter->SetColorModeToLUT();
    {
        vtkNew<vtkScalarsToColors> lut;
        lut->SetRange(0, 255);
        imgPolyFilter->SetLookupTable(lut);
    }
    imgPolyFilter->SetInputData(img);
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(imgPolyFilter->GetOutputPort());
    vtkNew<vtkActor> polydataActor;
    polydataActor->SetMapper(mapper);
    polydataActor->GetProperty()->SetColor(1, 0, 0);
    ren->AddActor(polydataActor);

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetupWin("FilterOutput", imgPolyFilter->GetOutput());
        vtkns::vtkObjSetup("filter", imgPolyFilter, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("actor", polydataActor);
    };

    AFTER_MY_CODE
}