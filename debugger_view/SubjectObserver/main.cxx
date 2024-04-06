#include <ImGuiCommon.h>

vtkns::LogView logView;

class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
    static MouseInteractorStyle* New() {
        return new MouseInteractorStyle;
    }
    void OnLeftButtonDown() override
    {
        logView.AddLog(fmt::format("{} Interactor Style\n", std::chrono::system_clock::now()).c_str());
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }

    void OnChar() override
    {
        logView.Add(fmt::format("{} {}", __func__, this->Interactor->GetKeyCode()));
    }

    void OnKeyPress() override
    {
        logView.Add(fmt::format("{} {}", __func__, this->GetInteractor()->GetKeySym()));
    }
};

class MyCallback : public vtkCommand
{
public:
    MyCallback() = default;
    ~MyCallback() = default;

    static MyCallback* New()
    {
        return new MyCallback;
    }
    void Execute(vtkObject* caller, unsigned long eventId,
        void* vtkNotUsed(callData)) override
    {
        if (vtkCommand::LeftButtonPressEvent == eventId)
        {
            logView.Add("MyCallback");
        }
    }
};

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    auto l = vtkSmartPointer<vtkCylinderSource>::New();
    auto lmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    lmapper->SetInputConnection(l->GetOutputPort());

    auto lactor = vtkSmartPointer<vtkActor>::New();
    lactor->SetMapper(lmapper);
    ren->AddActor(lactor);

    //renderer->SetBackground(0, 0, 0);
    ren->ResetCamera();

    auto style = vtkSmartPointer<MouseInteractorStyle>::New();
    renWin->GetInteractor()->SetInteractorStyle(style);

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
			static bool myOnceFlag = false;
			static bool useMyCallback = false;
			static float priority = 0;
			static std::list<unsigned long> interactorTags;
			static std::list<unsigned long> styleTags;
			ImGui::DragFloat("Priority", &priority);
			if (ImGui::Button("AddInteractorObserver"))
			{
				vtkCommand* pCmd = nullptr;
				if (useMyCallback)
				{
					pCmd = MyCallback::New();
				}
				else
				{
					//vtkNew<vtkCallbackCommand> cb;
					auto cb = vtkCallbackCommand::New();
					pCmd = cb;
					cb->SetCallback([](vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
						{
							//auto cylinderSource = static_cast<MouseInteractorStyle*>(caller);
							//int resolution = cylinderSource->GetResolution();
							//cylinderSource->RemoveObserver(*static_cast<unsigned long*>(clientData));
							logView.Add(fmt::format("Interactor Observer {}", *static_cast<unsigned long*>(clientData)));
							if (myOnceFlag)
							{
							}
						});
					cb->SetClientData(new unsigned long);
					cb->SetAbortFlagOnExecute(myOnceFlag);
				}
				const auto myTag = renWin->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, pCmd, priority);
				//*static_cast<unsigned long*>(cb->GetClientData()) = myTag;
				if (auto p = vtkCallbackCommand::SafeDownCast(pCmd))
				{
					*static_cast<unsigned long*>(p->GetClientData()) = myTag;
				}
				interactorTags.push_back(myTag);
			}

			{
				decltype(interactorTags) removedTag;
				for (auto i : interactorTags)
				{
					ImGui::SameLine();
					if (ImGui::Button(("RemoveInteractorObserver" + std::to_string(i)).c_str()))
					{
						renWin->GetInteractor()->RemoveObserver(i);
						removedTag.push_back(i);
					}
				}
				for (auto i : removedTag)
				{
					interactorTags.remove(i);
				}
			}
			//if (ImGui::Button("RemoveInteractorObserver"))
			//{
			//	vtkViewer.getInteractor()->RemoveObserver(cb);
			//}
			ImGui::Checkbox("Once", &myOnceFlag);
			ImGui::Checkbox("UseMyCallback", &useMyCallback);
			if (ImGui::Button("AddStyleObserver"))
			{
				vtkCommand* pCmd = nullptr;
				if (useMyCallback)
				{
					pCmd = MyCallback::New();
				}
				else
				{
					auto cb_style = vtkCallbackCommand::New();
					pCmd = cb_style;
					cb_style->SetCallback([](vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
						{
							auto cylinderSource = static_cast<MouseInteractorStyle*>(caller);;
							logView.Add(fmt::format("Style Observer {}", *static_cast<unsigned long*>(clientData)));
							if (myOnceFlag) {}
						});
					cb_style->SetClientData(new unsigned long);
					cb_style->SetAbortFlagOnExecute(myOnceFlag);
				}
				const auto myTag = renWin->GetInteractor()->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, pCmd, priority);
				if (auto p = vtkCallbackCommand::SafeDownCast(pCmd))
				{
					*static_cast<unsigned long*>(p->GetClientData()) = myTag;
				}
				styleTags.push_back(myTag);
			}
			//if (ImGui::Button("RemoveStyleObserver"))
			//{
			//	vtkViewer.getInteractor()->GetInteractorStyle()->RemoveObserver(cb_style);
			//}
			{
				decltype(styleTags) removedTag;
				for (auto i : styleTags)
				{
					ImGui::SameLine();
					if (ImGui::Button(("RemoveStyleObserver" + std::to_string(i)).c_str()))
					{
						renWin->GetInteractor()->GetInteractorStyle()->RemoveObserver(i);
						removedTag.push_back(i);
					}
				}
				for (auto i : removedTag)
				{
					styleTags.remove(i);
				}
			}
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    //iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}