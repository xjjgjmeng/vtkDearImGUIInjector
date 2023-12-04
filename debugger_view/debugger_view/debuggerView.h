#pragma once

class vtkRenderer;
class vtkDearImGuiInjector;

void init();
void showView(vtkRenderer* renderer);
void showView1(vtkDearImGuiInjector* inj);