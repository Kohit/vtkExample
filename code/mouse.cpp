#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkImageMapper.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageMapper.h>
#include <vtkImageData.h>
#include <vtkPolyDataReader.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkCommand.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"
#include "vtkXMLImageDataReader.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkGPUVolumeRayCastMapper.h"
#include <vtkMetaImageReader.h>
#include <vtkImageViewer2.h>
#include <vtkCallbackCommand.h>
#include <vtkWorldPointPicker.h>
#include <vtkMatrix4x4.h>
#include <vtkLine.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRendererCollection.h>
#include <vtkImageReslice.h>
#include <vtkDoubleArray.h>
#include "vtkAlgorithmOutput.h"
#include <vtkvmtkCenterlineAttributesFilter.h>
#include <vtkvmtkCenterlineGeometry.h>
#include <vtkvmtkCurvedMPRImageFilter.h>
#include <vtkMetaImageWriter.h>

bool isMousePressed = false;
double lastPost[3];
int slice = 0;

auto viewer = vtkImageViewer2::New();

/*
	vtkSmartPointer<vtkCallbackCommand> callBack =
		vtkSmartPointer<vtkCallbackCommand>::New();
	callBack->SetCallback(ClickCallbackFunction);
	//style->AddObserver(vtkCommand::MouseWheelForwardEvent, callBack);
	//style->AddObserver(vtkCommand::MouseWheelBackwardEvent, callBack);
	//style->AddObserver(vtkCommand::LeftButtonPressEvent, callBack);
	//style->AddObserver(vtkCommand::LeftButtonReleaseEvent, callBack);
	//style->AddObserver(vtkCommand::MouseMoveEvent, callBack);
*/

void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData) {
	//std::cout << "Event: " << vtkCommand::GetStringFromEventId(eventId) << std::endl;

	// iteractor/style设置observer回调函数获得的caller格式不同
	vtkInteractorStyleImage *style =
		static_cast<vtkInteractorStyleImage*>(caller);
	vtkRenderWindowInteractor* iren = style->GetInteractor();

	// Get the interactor like this:
	//vtkRenderWindowInteractor *iren =
	//	static_cast<vtkRenderWindowInteractor*>(caller);

		// get angle between two position
	if (true) {
		if (vtkCommand::LeftButtonReleaseEvent == eventId) {   // 释放鼠标
			isMousePressed = false;
			std::cout << "release" << std::endl;
			return;
		}
		if (vtkCommand::LeftButtonPressEvent == eventId) {   // 点击鼠标
			isMousePressed = true;
			std::cout << "press" << std::endl;
			int * pos = iren->GetEventPosition();	// iren must be a interactor rather than style
			std::cout << pos[0] << " " << pos[1] << std::endl;
			lastPost[0] = iren->GetEventPosition()[0];
			lastPost[1] = iren->GetEventPosition()[1];
			lastPost[2] = 1;
			return;
		}
		if (vtkCommand::MouseMoveEvent != eventId) {
			return;
		}
		if (!isMousePressed) return;
		// 拖动鼠标
		// iren->GetLastEventPosition();
		int * p1 = iren->GetEventPosition();
		double v1[3] = { p1[0], p1[1], 1 };
		std::cout << "--------" << std::endl;
		std::cout << lastPost[0] << "," << lastPost[1] << std::endl;
		std::cout << v1[0] << "," << v1[1] << std::endl;
		// angle, line (0,0)-(x1, y1) and line (0,0)-(x2,y2)
		double radians = vtkMath::AngleBetweenVectors(lastPost, v1);
		// same as: radians = atan2(norm(cross(v1,v2)), dot(v1,v2))
		std::cout << radians * 180 / vtkMath::Pi() << std::endl;
		std::cout << "--------" << std::endl;

		return;
	}

	// slice
	if (false) {
		// handle wheels

		if (eventId == vtkCommand::MouseWheelForwardEvent) {
			slice++;
		}
		else if (eventId == vtkCommand::MouseWheelBackwardEvent) {
			slice--;
		}
		else {
			return;
		}

		viewer->SetSlice(slice);
		viewer->GetRenderer()->ResetCamera();
		viewer->Render();
	}
}