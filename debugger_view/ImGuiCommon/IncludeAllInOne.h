﻿#pragma once

#include <Windows.h>

#include <sstream>
#include <string>
#include <string_view>
#include <functional>
#include <filesystem>
#include <thread>

#include "vtkDearImGuiInjector.h"

#include "vtkActor.h"
#include "vtkCallbackCommand.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkCameraOrientationRepresentation.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkConeSource.h"
#include "vtkCubeSource.h"
#include "vtkInteractorStyle.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkImageReslice.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include <vtkShortArray.h>
#include <vtkGenericOutlineFilter.h>
#include <vtkImageDataOutlineFilter.h>
#include <vtkImageCursor3D.h>
#include <vtkLine.h>
#include <vtkFloatArray.h>
#include <vtkXMLImageDataReader.h>
#include <vtkPlane.h>
#include <vtkPointWidget.h>
#include <vtkLeaderActor2D.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlab.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageProperty.h>
#include <vtkFlyingEdges3D.h>
#include <vtkMarchingCubes.h>
#include <vtkLineWidget2.h>
#include <vtkSplineWidget2.h>
#include <vtkImplicitFunction.h>
#include <vtkProperty2D.h>
#include <vtkHandleRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkLineRepresentation.h>
#include <vtkTransform.h>
#include <vtkLinearTransform.h>
#include <vtkImageSlabReslice.h>
#include <vtkKdTreePointLocator.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkResliceCursorWidget.h>
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget2.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkDistanceRepresentation3D.h>
#include <vtkBorderRepresentation.h>
#include <vtkBorderWidget.h>
#include <vtkWidgetEvent.h>
#include <vtkAxisActor2D.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkVolumeOutlineSource.h>
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
#include <vtkNamedColors.h>
#include <vtkOutlineSource.h>
#include <vtkVolume.h>
#include <vtkOpenGLActor.h>
#include <vtkVolumeProperty.h>
#include <vtkImageFlip.h>
#include <vtkSplineRepresentation.h>
#include <vtkPiecewiseFunction.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkColorTransferFunction.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkDistanceWidget.h>
#include <vtkResliceImageViewerMeasurements.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkImageViewer2.h>
#include <vtkCellPicker.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkPropPicker.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkSphereSource.h>
#include <vtkCamera.h>
#include <vtkImageDataOutlineFilter.h>
#include <vtkWorldPointPicker.h>
#include <vtkInformationVector.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkPlaneSource.h>
#include <vtkPropAssembly.h>
#include <vtkPolyLine.h>
#include <vtkBoundedPlanePointPlacer.h>
#include <vtkInformation.h>
#include <vtkMath.h>
#include <vtkBox.h>
#include <vtkBoundedPointSource.h>
#include <vtkRegularPolygonSource.h>
#include <vtkROIStencilSource.h>
#include <vtkTexture.h>
#include <vtkResliceCursor.h>
#include <vtkImageCast.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageMathematics.h>
#include <vtkPolygon.h>
#include <vtkExtractVOI.h>
#include <vtkRendererCollection.h>
#include <vtkImageThreshold.h>
#include <vtkTriangle.h>
#include <vtkCellArray.h>
#include <vtkProp3DCollection.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkLineSource.h>
#include <vtkSelectionNode.h>
#include <vtkExtractSelection.h>
#include <vtkDataSetMapper.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkParametricSpline.h>
#include <vtkImageChangeInformation.h>
#include <vtkParametricFunctionSource.h>
#include <vtkJPEGReader.h>
#include <vtkCylinderSource.h>
#include <vtkLight.h>
#include <vtkCaptionRepresentation.h>
#include <vtkCaptionActor2D.h>
#include <vtkCaptionWidget.h>
#include <vtkOutlineFilter.h>
#include <vtkWeakPointer.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include "vtkfmt/core.h"
#include "vtkfmt/ranges.h"
#include "vtkfmt/format.h"
#include "vtkfmt/chrono.h"

#include <imgui.h>