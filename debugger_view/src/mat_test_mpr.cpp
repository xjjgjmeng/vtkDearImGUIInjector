#include <ImGuiCommon.h>

int main()
{
    SETUP_WINDOW
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    vtkNew<vtkMatrix4x4> matA, matB;

    for (auto i : { matA.GetPointer(), matB.GetPointer() })
    {
        auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
        {
            vtkns::mat::genAxes(reinterpret_cast<vtkRenderer*>(clientdata), vtkMatrix4x4::SafeDownCast(caller));
        };
        vtkNew<vtkCallbackCommand> pCC;
        pCC->SetCallback(f);
        pCC->SetClientData(ren);
        i->AddObserver(vtkCommand::ModifiedEvent, pCC);
    }

    {
        double arr[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        matA->DeepCopy(arr);
    }
    {
        double arr[] = {
            1, 0, 0, 0,
            0, 0, 1, 0,
            0, -1, 0, 0,
            0, 0, 0, 1
        };
        matB->DeepCopy(arr);
    }

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("A", matA, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("B", matB, ImGuiTreeNodeFlags_DefaultOpen);

        // AX=B
        // A-1AX=A-1B
        // X=A-1B
        {
            auto genMatX = [&]
            {
                vtkNew<vtkMatrix4x4> matX;
                vtkNew<vtkMatrix4x4> matAInvert;
                vtkMatrix4x4::Invert(matA, matAInvert);
                vtkMatrix4x4::Multiply4x4(matAInvert, matB, matX);
                return matX;
            };
            vtkNew<vtkMatrix4x4> matX = genMatX();
            //vtkNew<vtkMatrix4x4> matAInvert;
            //vtkMatrix4x4::Invert(matA, matAInvert);
            //vtkMatrix4x4::Multiply4x4(matAInvert, matB, matX);
            vtkns::vtkObjSetup("X", matX);
            static vtkNew<vtkMatrix4x4> lockedMatX;
            vtkns::vtkObjSetup("lockX", lockedMatX);
            if (ImGui::Button("LockX"))
            {
                lockedMatX->DeepCopy(matX);
            }
            ImGui::SameLine();
            if (ImGui::Button("GenB"))
            {
                vtkNew<vtkMatrix4x4> r;
                vtkMatrix4x4::Multiply4x4(matA, lockedMatX, r);
                matB->DeepCopy(r);
            }
            ImGui::SameLine();
            static bool gs = false;
            static bool mpr = false;
            ImGui::Checkbox(u8"跟随", &gs); ImGui::SameLine();
            ImGui::Checkbox("mpr", &mpr);
            if (gs)
            {
                vtkNew<vtkMatrix4x4> r;
                vtkMatrix4x4::Multiply4x4(matA, lockedMatX, r);
                matB->DeepCopy(r);
            }
            if (mpr)
            {
                if (ImGui::TreeNodeEx(u8"MPR_TRANSLATE", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    auto f = [&](const double x, const double y, const double z)
                    {
                        auto myMat = std::array{
                            1.,0.,0.,x,
                            0.,1.,0.,y,
                            0.,0.,1.,z,
                            0.,0.,0.,1.
                        };
                        vtkNew<vtkMatrix4x4> newCurrWinMat;
                        vtkMatrix4x4::Multiply4x4(matA->GetData(), myMat.data(), newCurrWinMat->GetData());
                        vtkNew<vtkMatrix4x4> r;
                        vtkMatrix4x4::Multiply4x4(newCurrWinMat->GetData(), lockedMatX->GetData(), r->GetData());
                        matB->DeepCopy(r);
                        matA->Element[0][3] = matB->Element[0][3];
                        matA->Element[1][3] = matB->Element[1][3];
                        matA->Element[2][3] = matB->Element[2][3];
                        matA->Modified();
                    };
                    {
                        ImGui::Text("X:");
                        ImGui::SameLine();
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##X-", ImGuiDir_Left)) { f(-1, 0, 0); }
                        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                        if (ImGui::ArrowButton("##X+", ImGuiDir_Right)) { f(1, 0, 0); }
                        ImGui::PopButtonRepeat();
                    }
                    {
                        ImGui::Text("Y:");
                        ImGui::SameLine();
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Y-", ImGuiDir_Left)) { f(0, -1, 0); }
                        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                        if (ImGui::ArrowButton("##Y+", ImGuiDir_Right)) { f(0, 1, 0); }
                        ImGui::PopButtonRepeat();
                    }
                    {
                        ImGui::Text("Z:");
                        ImGui::SameLine();
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Z-", ImGuiDir_Left)) { f(0, 0, -1); }
                        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                        if (ImGui::ArrowButton("##Z+", ImGuiDir_Right)) { f(0, 0, 1); }
                        ImGui::PopButtonRepeat();
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx(u8"MPR_ROTATE", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    auto f = [&](const double v)
                    {
                        auto myMat = std::array{
                            std::cos(v),-std::sin(v),0.,0.,
                            std::sin(v),std::cos(v),0.,0.,
                            0.,0.,1.,0.,
                            0.,0.,0.,1.
                        };
                        vtkNew<vtkMatrix4x4> newCurrWinMat;
                        vtkMatrix4x4::Multiply4x4(matA->GetData(), myMat.data(), newCurrWinMat->GetData());
                        vtkNew<vtkMatrix4x4> r;
                        vtkMatrix4x4::Multiply4x4(newCurrWinMat->GetData(), lockedMatX->GetData(), r->GetData());
                        matB->DeepCopy(r);
                        matA->Element[0][3] = matB->Element[0][3];
                        matA->Element[1][3] = matB->Element[1][3];
                        matA->Element[2][3] = matB->Element[2][3];
                        matA->Modified();
                        lockedMatX = genMatX(); // 需要更新X
                    };
                    const auto step = vtkMath::Pi() / 2 / 2 / 2 / 2;
                    {
                        ImGui::Text("Z:");
                        ImGui::SameLine();
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Z-", ImGuiDir_Left)) { f(-step); }
                        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                        if (ImGui::ArrowButton("##Z+", ImGuiDir_Right)) { f(step); }
                        ImGui::PopButtonRepeat();
                    }
                    ImGui::TreePop();
                }
            }
        }
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
    vtkInteractorStyleSwitch::SafeDownCast(rwi->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    rwi->EnableRenderOff();
    rwi->Start();
}