#include "../IncludeAllInOne.h"
#include <ImGuiCommon.h>

#ifdef ADOBE_IMGUI_SPECTRUM
#include "imgui_spectrum.h"
#endif
#include "vtkOpenGLRenderWindow.h" // needed to check if opengl is supported.

int main(int argc, char* argv[])
{
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren);
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0, 0, 0);
  points->InsertNextPoint(1, 0, 0);
  points->InsertNextPoint(1, 1, 0);
  points->InsertNextPoint(0, 1, 0);
  points->InsertNextPoint(0, 0, 1);

  vtkNew<vtkPolygon> polygon;
  polygon->GetPointIds()->SetNumberOfIds(4);
  polygon->GetPointIds()->SetId(0, 0);
  polygon->GetPointIds()->SetId(1, 1);
  polygon->GetPointIds()->SetId(2, 2);
  polygon->GetPointIds()->SetId(3, 3);

  vtkNew<vtkTriangle> triangle;
  triangle->GetPointIds()->SetId(0, 0);
  triangle->GetPointIds()->SetId(1, 2);
  triangle->GetPointIds()->SetId(2, 4);

  vtkNew<vtkCellArray> cells;
  cells->InsertNextCell(polygon);
  cells->InsertNextCell(triangle);

  unsigned char red[3] = { 255,0,0 };
  unsigned char green[3] = { 0,255,0 };
  unsigned char blue[3] = { 0,0,255 };

  vtkNew<vtkUnsignedCharArray> pointColors;
  pointColors->SetNumberOfComponents(3);
  pointColors->InsertNextTuple3(red[0], red[1], red[2]);
  pointColors->InsertNextTuple3(green[0], green[1], green[2]);
  pointColors->InsertNextTuple3(blue[0], blue[1], blue[2]);
  pointColors->InsertNextTuple3(green[0], green[1], green[2]);
  pointColors->InsertNextTuple3(red[0], red[1], red[2]);

  vtkNew<vtkUnsignedCharArray> cellColors;
  cellColors->SetNumberOfComponents(3);
  cellColors->InsertNextTuple3(red[0], red[1], red[2]);
  cellColors->InsertNextTuple3(green[0], green[1], green[2]);

  vtkNew<vtkPolyData> polydata;
  polydata->SetPoints(points);
  polydata->SetPolys(cells);
  polydata->GetPointData()->SetScalars(pointColors);
  polydata->GetCellData()->SetScalars(cellColors);

  vtkNew<vtkPolyDataMapper> polymapper;
  polymapper->SetInputData(polydata);

  vtkNew<vtkActor> polyactor;
  polyactor->SetMapper(polymapper);
  ren->AddActor(polyactor);
#if 1
  vtkNew<vtkLineSource> lineSource;
  lineSource->SetPoint1(50.0, 50.0, 0.0);  // 起点坐标 
  lineSource->SetPoint2(90.0, 50.0, 0.0);  // 终点坐标
  lineSource->Update();
  // 2d
#if 0
  vtkNew<vtkPolyDataMapper2D> lineMapper2D;
  vtkNew<vtkCoordinate> curveCoordinate;
  curveCoordinate->SetCoordinateSystemToWorld();
  lineMapper2D->SetTransformCoordinate(curveCoordinate);
  lineMapper2D->SetInputConnection(lineSource->GetOutputPort());
  vtkNew<vtkActor2D> lineActor2D;
  lineActor2D->GetProperty()->SetColor(0, 0, 1);
  lineActor2D->SetMapper(lineMapper2D);
  ren->AddActor2D(lineActor2D);
#endif
  // 3d
  vtkNew<vtkPolyDataMapper> lineMapper;
  lineMapper->SetInputConnection(lineSource->GetOutputPort());
  vtkNew<vtkActor> lineActor;
  lineActor->SetMapper(lineMapper);
  lineActor->GetProperty()->SetLineStipplePattern(0xAAAA);
  lineActor->GetProperty()->SetLineStippleRepeatFactor(1);
  lineActor->GetProperty()->SetColor(0, 1, 0);
  ren->AddActor(lineActor);
#endif
#if 0
  vtkNew<vtkAnnotatedCubeActor> cubeActor;
#if 1
  cubeActor->SetXPlusFaceText("L");
  cubeActor->SetXMinusFaceText("R");
  cubeActor->SetYMinusFaceText("I");
  cubeActor->SetYPlusFaceText("S");
  cubeActor->SetZMinusFaceText("P");
  cubeActor->SetZPlusFaceText("A");
  cubeActor->SetXFaceTextRotation(-90);
  cubeActor->SetZFaceTextRotation(90);
#endif
  ren->AddActor(cubeActor);

  vtkNew<vtkAnnotatedCubeActor> axesActor;
#if 1
  axesActor->SetXPlusFaceText("L");
  axesActor->SetXMinusFaceText("R");
  axesActor->SetYMinusFaceText("I");
  axesActor->SetYPlusFaceText("S");
  axesActor->SetZMinusFaceText("P");
  axesActor->SetZPlusFaceText("A");
  axesActor->SetXFaceTextRotation(-90);
  axesActor->SetZFaceTextRotation(90);
#endif
  axesActor->GetTextEdgesProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
  axesActor->GetTextEdgesProperty()->SetLineWidth(2);
  axesActor->GetCubeProperty()->SetColor(colors->GetColor3d("Blue").GetData());
  vtkNew<vtkOrientationMarkerWidget> axes;
  axes->SetViewport(0, 0, 0.1, 0.1);
  axes->SetOrientationMarker(axesActor);
  axes->SetInteractor(iren);
  axes->EnabledOn();
  axes->InteractiveOn();
  ren->ResetCamera();
#endif

  ::imgui_render_callback = [&]
      {
          //if (ImGui::TreeNodeEx("LineActor3D", ImGuiTreeNodeFlags_DefaultOpen))
          //{
          //    ImGuiNs::vtkObjSetup(lineActor);
          //    ImGui::TreePop();
          //}
          if (ImGui::TreeNodeEx("vtkPolyDataMapper", ImGuiTreeNodeFlags_DefaultOpen))
          {
              ImGui::InputScalarN("Bounds", ImGuiDataType_Double, polymapper->GetBounds(), 6, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
              ImGui::TreePop();
          }
          if (ImGui::TreeNodeEx("vtkLineSource", ImGuiTreeNodeFlags_DefaultOpen))
          {
              if (bool v = lineSource->GetUseRegularRefinement(); ImGui::Checkbox("UseRegularRefinement", &v))
              {
                  lineSource->SetUseRegularRefinement(v);
              }
              if (int v = lineSource->GetResolution(); ImGui::DragInt("Resolution", &v))
              {
                  lineSource->SetResolution(v);
              }
              if (int v = lineSource->GetOutputPointsPrecision(); ImGui::DragInt("OutputPointsPrecision", &v))
              {
                  lineSource->SetOutputPointsPrecision(v);
              }
              if (lineSource->GetPoints())
              {
                  ImGui::Text("PointsNumber: %d", lineSource->GetPoints()->GetNumberOfPoints());
              }
              {
                  static double newPoint[3]{};
                  ImGui::DragScalarN("NewPoint", ImGuiDataType_Double, newPoint, std::size(newPoint), 0.01f);
                  ImGui::SameLine();
                  if (ImGui::Button("add"))
                  {
                      vtkSmartPointer<vtkPoints> points = lineSource->GetPoints();
                      if (!points.Get())
                      {
                          points = vtkSmartPointer<vtkPoints>::New();
                          points->InsertNextPoint(0, 0, 0);
                      }
                      points->InsertNextPoint(newPoint);
                      lineSource->SetPoints(points);
                      lineSource->Modified();
                      ren->ResetCamera();
                  }
                  ImGui::SameLine();
                  if (ImGui::Button("Spline"))
                  {
                      vtkNew<vtkParametricSpline> parametricSpline;
                      parametricSpline->SetDerivativesAvailable(true);
                      parametricSpline->SetParameterizeByLength(true);
                      parametricSpline->SetPoints(lineSource->GetPoints());
                      parametricSpline->ClosedOff();
                      vtkNew<vtkParametricFunctionSource> parametricFunctionSource;
                      parametricFunctionSource->SetParametricFunction(parametricSpline);
                      parametricFunctionSource->SetUResolution(200);
                      parametricFunctionSource->Update();
                      vtkPoints* newPoints = parametricFunctionSource->GetOutput()->GetPoints();
                      lineSource->SetPoints(newPoints);
                  }
              }
              ImGui::TreePop();
          }
#if 0
          if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
          {
              ImGuiNs::vtkObjSetup(ren->GetActiveCamera());
              ImGui::TreePop();
          }

          if (ImGui::TreeNodeEx("vtkAnnotatedCubeActor"))
          {
              ImGuiNs::vtkObjSetup(axesActor);
              ImGui::TreePop();
          }

          if (ImGui::TreeNodeEx("vtkAnnotatedCubeActor", ImGuiTreeNodeFlags_DefaultOpen))
          {
              if (float v = vtkMath::RadiansFromDegrees(axesActor->GetXFaceTextRotation()); ImGui::SliderAngle("XFaceTextRotation", &v))
              {
                  axesActor->SetXFaceTextRotation(vtkMath::DegreesFromRadians(v));
              }
              if (float v = vtkMath::RadiansFromDegrees(axesActor->GetYFaceTextRotation()); ImGui::SliderAngle("YFaceTextRotation", &v))
              {
                  axesActor->SetYFaceTextRotation(vtkMath::DegreesFromRadians(v));
              }
              if (float v = vtkMath::RadiansFromDegrees(axesActor->GetZFaceTextRotation()); ImGui::SliderAngle("ZFaceTextRotation", &v))
              {
                  axesActor->SetZFaceTextRotation(vtkMath::DegreesFromRadians(v));
              }

              ImGui::TreePop();
          }

          if (ImGui::TreeNodeEx("TextEdgesProperty", ImGuiTreeNodeFlags_DefaultOpen))
          {
              const auto pProperty = axesActor->GetTextEdgesProperty();
              if (float v[3] = { pProperty->GetColor()[0],pProperty->GetColor()[1],pProperty->GetColor()[2] }; ImGui::ColorEdit3("Color", v))
              {
                  pProperty->SetColor(v[0], v[1], v[2]);
              }
              if (float v = pProperty->GetLineWidth(); ImGui::DragFloat("LineWidth", &v, 0.1, 0.01, 10))
              {
                  pProperty->SetLineWidth(v);
              }

              ImGui::TreePop();
          }
          if (ImGui::TreeNodeEx("CubeProperty", ImGuiTreeNodeFlags_DefaultOpen))
          {
              const auto pProperty = axesActor->GetCubeProperty();
              if (float v[3] = { pProperty->GetColor()[0],pProperty->GetColor()[1],pProperty->GetColor()[2] }; ImGui::ColorEdit3("Color", v))
              {
                  pProperty->SetColor(v[0], v[1], v[2]);
              }
              if (float v = pProperty->GetLineWidth(); ImGui::DragFloat("LineWidth", &v, 0.1, 0.01, 10))
              {
                  pProperty->SetLineWidth(v);
              }

              ImGui::TreePop();
          }

          if (ImGui::TreeNodeEx("vtkOrientationMarkerWidget", ImGuiTreeNodeFlags_DefaultOpen))
          {
              if (float v[3] = { axes->GetOutlineColor()[0],axes->GetOutlineColor()[1],axes->GetOutlineColor()[2] }; ImGui::ColorEdit3("OutlineColor", v))
              {
                  axes->SetOutlineColor(v[0], v[1], v[2]);
              }

              ImGui::TreePop();
          }

          {
              auto camera = ren->GetActiveCamera();
              const auto center = cubeActor->GetCenter();
              const auto bounds = cubeActor->GetBounds();
              constexpr auto ratio = 4;

              if (ImGui::Button("Superior"))
              {
                  camera->SetViewUp(0, 0, -1);
                  camera->SetPosition(center[0], center[1] + (bounds[3] - bounds[2]) * ratio, center[2]);
                  camera->SetFocalPoint(center);
                  ren->ResetCameraClippingRange();
                  cubeActor->SetOrigin(center);

              }
              ImGui::SameLine();
              if (ImGui::Button("Inferior"))
              {
                  camera->SetViewUp(0, 0, 1);
                  camera->SetPosition(center[0], center[1] - (bounds[3] - bounds[2]) * ratio, center[2]);
                  camera->SetFocalPoint(center);
                  ren->ResetCameraClippingRange();
                  cubeActor->SetOrigin(center);
              }
              ImGui::SameLine();
              if (ImGui::Button("Left"))
              {
                  camera->SetViewUp(0, 1, 0);
                  camera->SetPosition(center[0] + (bounds[1] - bounds[0]) * ratio, center[1], center[2]);
                  camera->SetFocalPoint(center);
                  ren->ResetCameraClippingRange();
                  cubeActor->SetOrigin(center);
              }
              ImGui::SameLine();
              if (ImGui::Button("Right"))
              {
                  camera->SetViewUp(0, 1, 0);
                  camera->SetPosition(center[0] - (bounds[1] - bounds[0]) * ratio, center[1], center[2]);
                  camera->SetFocalPoint(center);
                  ren->ResetCameraClippingRange();
                  cubeActor->SetOrigin(center);
              }
              ImGui::SameLine();
              if (ImGui::Button("Anterior"))
              {
                  camera->SetViewUp(0, 1, 0);
                  camera->SetPosition(center[0], center[1], center[2] + (bounds[5] - bounds[4]) * ratio);
                  camera->SetFocalPoint(center);
                  ren->ResetCameraClippingRange();
                  cubeActor->SetOrigin(center);
              }
              ImGui::SameLine();
              if (ImGui::Button("Posterior"))
              {
                  camera->SetViewUp(0, 1, 0);
                  camera->SetPosition(center[0], center[1], center[2] - (bounds[5] - bounds[4]) * ratio);
                  camera->SetFocalPoint(center);
                  ren->ResetCameraClippingRange();
                  cubeActor->SetOrigin(center);
              }
          }
#endif
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
    ImguiVtkNs::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    ImguiVtkNs::DrawUI(dearImGuiOverlay);
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