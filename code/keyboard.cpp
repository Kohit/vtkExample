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

// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera
{
private:
	double pitch;
	double yaw;
	vtkPolyData * centerline;
	vtkIdType pos;
	KeyPressInteractorStyle() {
		std::cout << "init" << std::endl;
		pitch = 0;
		yaw = 0;
		centerline = nullptr;
		pos = 0;
	}
public:
	static KeyPressInteractorStyle* New();
	vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);
	void setCenterline(vtkPolyData * centerline) {
		this->centerline = centerline;
		pos = centerline->GetNumberOfPoints() - 1;

	}

	virtual void OnKeyPress()
	{
		// Get the keypress
		vtkRenderWindowInteractor *rwi = this->Interactor;
		std::string key = rwi->GetKeySym();

		// Output the key that was pressed
		std::cout << "Pressed " << key << std::endl;
		vtkRenderer * renderer = this->GetCurrentRenderer();

		// Handle an arrow key
		if (key == "Up")
		{
			pos = vtkMath::Max(pos - 1, (vtkIdType)0);

			//pitch = vtkMath::Max(pitch, (double)0);
			vtkCamera * cam = renderer->GetActiveCamera();
			//cam->Pitch(++pitch);
			cam->SetPosition(centerline->GetPoint(pos));
			cam->SetFocalPoint(centerline->GetPoint(vtkMath::Max(pos - 1, (vtkIdType)0)));
			std::cout << "The up arrow was pressed:" << pitch << std::endl;
		}

		if (key == "Down")
		{
			pos = vtkMath::Min(pos + 1, centerline->GetNumberOfPoints() - 1);

			//pitch = vtkMath::Max(pitch, (double)0);
			vtkCamera * cam = renderer->GetActiveCamera();
			cam->SetPosition(centerline->GetPoint(pos));
			cam->SetFocalPoint(centerline->GetPoint(vtkMath::Max(pos - 1, (vtkIdType)0)));
			//cam->Pitch(++pitch);
			std::cout << "The up arrow was pressed:" << pitch << std::endl;
		}

		if (key == "Left")
		{
			yaw = vtkMath::Max(yaw, (double)0);
			vtkCamera * cam = renderer->GetActiveCamera();
			cam->Yaw(++yaw);
			std::cout << "The Left arrow was pressed:" << yaw << std::endl;
		}

		if (key == "Right")
		{
			yaw = vtkMath::Min(yaw, (double)0);
			vtkCamera * cam = renderer->GetActiveCamera();
			cam->Yaw(--yaw);

			std::cout << "The Right arrow was pressed:" << yaw << std::endl;
		}

		// Handle a "normal" key
		if (key == "a")
		{
			std::cout << "The a key was pressed." << std::endl;
		}

		// Forward events
		//vtkInteractorStyleTrackballCamera::OnKeyPress();
		rwi->Render();
	}

};
vtkStandardNewMacro(KeyPressInteractorStyle);

/*
	vtkSmartPointer<KeyPressInteractorStyle> style =
		vtkSmartPointer<KeyPressInteractorStyle>::New();
	style->setCenterline(centerline);
	iren->SetInteractorStyle(style);
	style->SetCurrentRenderer(render);

	or 

		//vtkSmartPointer<vtkCallbackCommand> keypressCallback =
	//	vtkSmartPointer<vtkCallbackCommand>::New();
	//keypressCallback->SetCallback(KeypressCallbackFunction);
	//iren->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);

	void KeypressCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

*/