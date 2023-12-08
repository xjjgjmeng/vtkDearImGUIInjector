#pragma once

#include <Windows.h>

#include <sstream>
#include <string>

#include "vtkDearImGuiInjector.h"

#include "vtkActor.h"
#include "vtkCallbackCommand.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkCameraOrientationRepresentation.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkConeSource.h"
#include "vtkInteractorStyle.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkImageReslice.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkRenderWindow.h"
#include "vtkImageMapToColors.h"
#include "vtkLookupTable.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkDICOMImageReader.h"
#include "vtkImageActor.h"
#include "vtkInteractorStyleImage.h"
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkDistanceWidget.h>
#include <vtkResliceImageViewerMeasurements.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkImageViewer2.h>
#include <vtkCellPicker.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkPlaneSource.h>
#include "vtkfmt/core.h"
#include "vtkfmt/ranges.h"
#include "vtkfmt/format.h"