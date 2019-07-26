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

double center[3];
vtkPoints * points = vtkPoints::New();
vtkCellArray * lines = vtkCellArray::New();
vtkPolyData * polyData = vtkPolyData::New();
vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();

/*

	//int extent[6];
	//double spacing[3];
	//double origin[3];

	//reader->GetOutput()->GetExtent(extent);
	//reader->GetOutput()->GetSpacing(spacing);
	//reader->GetOutput()->GetOrigin(origin);

	//center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
	//center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
	//center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);

	vtkSmartPointer<vtkCallbackCommand> callBack =
		vtkSmartPointer<vtkCallbackCommand>::New();
	callBack->SetCallback(ClickCallbackFunction);
	//iren->AddObserver(vtkCommand::LeftButtonPressEvent, callBack);
*/
void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData) {
	//std::cout << "Event: " << vtkCommand::GetStringFromEventId(eventId) << std::endl;

	// Get the interactor like this(depends on caller type):
	vtkRenderWindowInteractor *iren =
		static_cast<vtkRenderWindowInteractor*>(caller);

	if (eventId == vtkCommand::LeftButtonPressEvent) {

		double windowPosition[3] = { iren->GetEventPosition()[0], iren->GetEventPosition()[1], 0 };
		vtkWorldPointPicker * wp = vtkWorldPointPicker::New();
		wp->Pick(windowPosition, iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		//
		double * pp = wp->GetPickPosition();
		//pp[2] = center[2];
		pp[2] = 0.01;

		std::cout << "window pos:" << windowPosition[0] << ", " << windowPosition[1] << ", " << windowPosition[2] << std::endl;
		std::cout << "world pos:" << pp[0] << ", " << pp[1] << ", " << pp[2] << std::endl;
		std::cout << "center pos:" << center[0] << ", " << center[1] << ", " << center[2] << std::endl;


		vtkIdType pt_id = points->InsertNextPoint(pp);
		std::cout << "points:" << pt_id << std::endl;
		if (pt_id > 0)
		{
			auto line = vtkSmartPointer<vtkLine>::New();
			line->GetPointIds()->SetId(0, pt_id - 1);
			line->GetPointIds()->SetId(1, pt_id);
			lines->InsertNextCell(line);
		}
		else {
			polyData->SetPoints(points);
			polyData->SetLines(lines);
		}

		renWin->Render();

		return;
	}

}