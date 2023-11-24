#pragma once

#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkCaptionRepresentation.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCaptionWidget.h>
#include <vtkCellPicker.h>
#include <vtkPropPicker.h>
#include <vtkProp3D.h>
#include <vtkActor2D.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkfmt/core.h>
#include <vtkfmt/format.h>

namespace Inj
{
	struct GData
	{
		vtkSmartPointer<vtkActor> actor;
		vtkSmartPointer<vtkCaptionRepresentation> captionRepresentation;
		vtkSmartPointer<vtkCaptionWidget> captionWidget;
		vtkSmartPointer<vtkCellPicker> InteractionPicker;
		vtkSmartPointer<vtkPropPicker> PropPicker;
		vtkSmartPointer<vtkProp3D> InteractionProp;
		vtkSmartPointer<vtkActor2D> lineActor;
	};

	static void Pan(vtkRenderer* CurrentRenderer, vtkProp* InteractionProp)
	{
		fmt::print("---------------{}\n", __LINE__);
		if (CurrentRenderer == nullptr || InteractionProp == nullptr)
		{
			return;
		}

		vtkRenderWindowInteractor* rwi = CurrentRenderer->GetRenderWindow()->GetInteractor();

		if (auto prop = vtkActor2D::SafeDownCast(InteractionProp))
		{
			fmt::print("---------------{}\n", __LINE__);
		}
		return;
		// Use initial center as the origin from which to pan

		double* obj_center = nullptr;
		if (vtkProp3D::SafeDownCast(InteractionProp))
		{
			obj_center = vtkProp3D::SafeDownCast(InteractionProp)->GetCenter();
		}
		else if (auto prop = vtkActor2D::SafeDownCast(InteractionProp))
		{
			auto pos = prop->GetPosition();
			obj_center = new double[3] {pos[0], pos[1], 0 };
		}

		double disp_obj_center[3], new_pick_point[4];
		double old_pick_point[4], motion_vector[3];

		auto ComputeWorldToDisplay = [](vtkRenderer * ren, double x, double y, double z, double displayPt[3])
		{
			ren->SetWorldPoint(x, y, z, 1.0);
			ren->WorldToDisplay();
			ren->GetDisplayPoint(displayPt);
		};
		ComputeWorldToDisplay(CurrentRenderer, obj_center[0], obj_center[1], obj_center[2], disp_obj_center);

		auto ComputeDisplayToWorld = [](vtkRenderer* ren, double x, double y, double z, double worldPt[4])
			{
				ren->SetDisplayPoint(x, y, z);
				ren->DisplayToWorld();
				ren->GetWorldPoint(worldPt);
				if (worldPt[3])
				{
					worldPt[0] /= worldPt[3];
					worldPt[1] /= worldPt[3];
					worldPt[2] /= worldPt[3];
					worldPt[3] = 1.0;
				}
			};
		ComputeDisplayToWorld(
			CurrentRenderer, rwi->GetEventPosition()[0], rwi->GetEventPosition()[1], disp_obj_center[2], new_pick_point);

		ComputeDisplayToWorld(CurrentRenderer, rwi->GetLastEventPosition()[0], rwi->GetLastEventPosition()[1],
			disp_obj_center[2], old_pick_point);

		motion_vector[0] = new_pick_point[0] - old_pick_point[0];
		motion_vector[1] = new_pick_point[1] - old_pick_point[1];
		motion_vector[2] = new_pick_point[2] - old_pick_point[2];
#if 0
		if (InteractionProp->GetUserMatrix() != nullptr)
		{
			vtkTransform* t = vtkTransform::New();
			t->PostMultiply();
			t->SetMatrix(this->InteractionProp->GetUserMatrix());
			t->Translate(motion_vector[0], motion_vector[1], motion_vector[2]);
			this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
			t->Delete();
		}
		else
		{
			InteractionProp->AddPosition(motion_vector[0], motion_vector[1], motion_vector[2]);
		}
#endif
		if (auto prop = vtkProp3D::SafeDownCast(InteractionProp))
		{
			prop->AddPosition(motion_vector[0], motion_vector[1], motion_vector[2]);
		}
		else if (auto prop = vtkActor2D::SafeDownCast(InteractionProp))
		{
			prop->SetPosition(prop->GetPosition()[0] + motion_vector[0], prop->GetPosition()[1] + motion_vector[1]);
		}
		//if (this->AutoAdjustCameraClippingRange)
		//{
		//	this->CurrentRenderer->ResetCameraClippingRange();
		//}

		rwi->Render();
	}
}

extern Inj::GData* gData;