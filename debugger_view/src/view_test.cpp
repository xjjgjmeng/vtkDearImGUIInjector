#include <ImGuiCommon.h>

int main()
{
    //BEFORE_MY_CODE
        auto img = vtkns::getVRData();
        auto contextView = vtkSmartPointer<vtkContextView>::New();
        auto ren = contextView->GetRenderer();
        auto rw = contextView->GetRenderWindow();
        auto rwi = contextView->GetInteractor();
        //rwi->SetInteractorStyle(vtkNew<vtkInteractorStyleSwitch>{});
        //vtkInteractorStyleSwitch::SafeDownCast(rwi->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    vtkns::labelWorldZero(ren);
    ::pWindow = rw;
    ::pRen = ren;

    // 将原始的image用线框显示出来
    //vtkns::genImgOutline(ren, img, false)->GetProperty()->SetColor(1., 1., 0.);
    auto volume = vtkns::genVR(ren, img, false);

    //vtkColorTransferControlPointsItem* pvtkColorTransferControlPointsItem = nullptr;
#if 0
    // 创建控制点项目
    auto controlPointsItem = vtkSmartPointer<vtkColorTransferControlPointsItem>::New();
    controlPointsItem->SetShowLabels(true);
    //pvtkColorTransferControlPointsItem = controlPointsItem;
    // 设置与颜色转移函数关联
    controlPointsItem->SetColorTransferFunction(volume->GetProperty()->GetRGBTransferFunction(0));
    // 创建上下文视图

    auto opacityCtrlPts = vtkSmartPointer<vtkPiecewiseControlPointsItem>::New();
    opacityCtrlPts->SetShowLabels(true);
    opacityCtrlPts->SetPiecewiseFunction(volume->GetProperty()->GetScalarOpacity());

    //contextView->GetScene()->AddItem(controlPointsItem);
    contextView->GetScene()->AddItem(opacityCtrlPts);
#else
    vtkNew<vtkCompositeControlPointsItem> ctrlPts;
    ctrlPts->SetOpacityFunction(volume->GetProperty()->GetScalarOpacity());
    ctrlPts->SetColorTransferFunction(volume->GetProperty()->GetRGBTransferFunction(0));
    contextView->GetScene()->AddItem(ctrlPts);
#endif

    // 启动渲染
    //contextView->GetRenderWindow()->SetSize(800, 600);
   contextView->GetRenderer()->SetBackground(0.0, 0.0, 0.0); // 白色背景
    //contextView->SetRenderWindow(rw);
    //contextView->GetRenderer()->SetViewport(0.0, 0.5, 1, 1);
    //contextView->GetRenderer()->SetBackgroundAlpha(0.9);
    //ren->SetViewport(0.5, 0.5, 1, 1);
    //contextView->Render();

#if 0
    std::thread{ [cts = volume->GetProperty()->GetRGBTransferFunction(0),
    &pvtkColorTransferControlPointsItem]
        {
            // 创建控制点项目
            auto controlPointsItem = vtkSmartPointer<vtkColorTransferControlPointsItem>::New();
            pvtkColorTransferControlPointsItem = controlPointsItem;
            // 设置与颜色转移函数关联
            controlPointsItem->SetColorTransferFunction(cts);
            // 创建上下文视图
            auto contextView = vtkSmartPointer<vtkContextView>::New();
            contextView->GetScene()->AddItem(controlPointsItem);

            // 启动渲染
            //contextView->GetRenderWindow()->SetSize(800, 600);
            //contextView->GetRenderer()->SetBackground(1.0, 1.0, 1.0); // 白色背景
            contextView->Render();

            // 创建并设置交互器
            auto renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
            renderWindowInteractor->SetRenderWindow(contextView->GetRenderWindow());
            renderWindowInteractor->Start();
        } }.detach();
#endif
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("OriginalImageData", img);
            vtkns::vtkObjSetup("Volume", volume);
            //vtkns::vtkObjSetup("ColorTransferControlPointsItem", controlPointsItem, ImGuiTreeNodeFlags_DefaultOpen);
            //vtkns::vtkObjSetup("Opacity", opacityCtrlPts, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("666", ctrlPts, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}