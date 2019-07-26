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

int main() {
	vtkPolyDataReader * reader = vtkPolyDataReader::New();
	reader->SetFileName("C:/Users/Ad/Desktop/centerline.vtk");
	reader->Update();
	vtkPolyData * centerline = vtkPolyData::New();
	centerline->DeepCopy(reader->GetOutput());

	vtkDICOMImageReader *dicomReader = vtkDICOMImageReader::New();
	dicomReader->SetDirectoryName("D:/demo/demo2/dicom");
	dicomReader->Update();


	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper =
		vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapper->SetInputConnection(dicomReader->GetOutputPort());
	volumeMapper->SetBlendModeToComposite();


	vtkSmartPointer<vtkColorTransferFunction> volumeColor =
		vtkSmartPointer<vtkColorTransferFunction>::New();

	volumeColor->AddRGBPoint(0, 0.0, 0.0, 0.0);
	volumeColor->AddRGBPoint(500, 0.3, 0.2, 0.2);
	volumeColor->AddRGBPoint(1000, 0.5, 0.5, 0.3);
	volumeColor->AddRGBPoint(1500, 1.0, 0, 0);

	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity = \
		vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeScalarOpacity->AddPoint(0, 0.00);
	volumeScalarOpacity->AddPoint(500, 0.15);
	volumeScalarOpacity->AddPoint(1000, 0.85);
	volumeScalarOpacity->AddPoint(1500, 0.99);

	vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity =
		vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity->AddPoint(0, 0.0);
	volumeGradientOpacity->AddPoint(90, 0.2);
	volumeGradientOpacity->AddPoint(100, 1.0);

	vtkSmartPointer<vtkVolumeProperty> volumeProperty =
		vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(volumeColor);
	volumeProperty->SetScalarOpacity(volumeScalarOpacity);
	volumeProperty->SetGradientOpacity(volumeGradientOpacity);
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();
	volumeProperty->SetAmbient(0.4);
	volumeProperty->SetDiffuse(0.6);
	volumeProperty->SetSpecular(0.2);

	vtkSmartPointer<vtkRenderer> render = vtkSmartPointer<vtkRenderer>::New();

	vtkCamera * cam = render->GetActiveCamera();
	cam->SetPosition(centerline->GetPoint(40));	// look inside the volume
	cam->SetFocalPoint(centerline->GetPoint(41));
	//cam->SetClippingRange(0.01, 200);
	cam->SetViewAngle(90);

	vtkVolume *volume = vtkVolume::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);
	render->AddVolume(volume);


	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(render);
	renWin->SetSize(1280, 720);

	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	iren->Initialize();
	iren->Start();
}