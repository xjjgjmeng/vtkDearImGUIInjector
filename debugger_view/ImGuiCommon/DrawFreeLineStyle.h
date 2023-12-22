#pragma once

#include <vtkRenderer.h>
#include <vtkLineSource.h>
#include <vtkInteractorStyleImage.h>

class DrawFreeLineStyle : public vtkInteractorStyleImage
{
public:
    static DrawFreeLineStyle* New();
    vtkTypeMacro(DrawFreeLineStyle, vtkInteractorStyleImage);

    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMouseMove() override;


private:
    bool m_pressed = false;
    vtkLineSource* m_currLineSource = nullptr;
};