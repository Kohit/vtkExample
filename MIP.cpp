
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPlane.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkImageProperty.h>
#include <vtkImageReslice.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkImageSlabReslice.h>
int main() {



	//renWin->SetInteractor(iren);

	vtkDICOMImageReader *dicomReader = vtkDICOMImageReader::New();
	dicomReader->SetDirectoryName("D:/demo/demo2/dicom");
	dicomReader->Update();

	double center[3];
	dicomReader->GetOutput()->GetCenter(center);

	vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
	plane->SetOrigin(center);

	vtkSmartPointer<vtkImageResliceMapper> imageResliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	imageResliceMapper->SetInputConnection(dicomReader->GetOutputPort());
	imageResliceMapper->SetSlicePlane(plane);
	imageResliceMapper->SetSlabThickness(5);
	//imageResliceMapper->SetSlabTypeToMax();
	imageResliceMapper->SetSlabTypeToMin();
	double thickness = imageResliceMapper->GetSlabThickness();

	//imageResliceMapper->SetInputData(colorImage);
	//auto actor = vtkImageActor::New();
	//actor->SetMapper(imageResliceMapper);
	//actor->GetProperty()->SetInterpolationTypeToNearest();
	vtkSmartPointer<vtkImageSlice> imageSlice = vtkSmartPointer<vtkImageSlice>::New();

	imageSlice->SetMapper(imageResliceMapper);
	imageSlice->GetProperty()->SetInterpolationTypeToNearest();

	// Setup renderers
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	//renderer->AddActor(actor);
	renderer->AddViewProp(imageSlice);
	//renderer->ResetCamera();

	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	iren->SetRenderWindow(renWin);
	renWin->AddRenderer(renderer);

	renWin->SetSize(600, 600);
	renWin->Render();

	iren->Initialize();
	iren->Start();



	return 0;
}

