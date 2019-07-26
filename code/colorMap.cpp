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

int main() {
	vtkPolyDataReader *reader = vtkPolyDataReader::New();
	reader->SetFileName("D:/demo/demo2/case/result/vessel.vtk");
	reader->Update();

	vtkPolyData *polydata = reader->GetOutput();
	int len = polydata->GetPointData()->GetNumberOfArrays();
	std::cout << "total array:" << len << endl;
	for (int i = 0; i < len; i++) {
		std::cout << polydata->GetPointData()->GetArrayName(i) << " ";
	}

	vtkDataArray * stress = polydata->GetPointData()->GetArray("wallShearStress");
	//polydata->GetPointData()->SetScalars(stress);
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 3; j++)
			std::cout << stress->GetComponent(i, j) << " ";
		std::cout << endl;
	}
	vtkLookupTable * lookupTable = vtkLookupTable::New();
	//lookupTable->SetNumberOfColors(64);
	lookupTable->SetHueRange(0.667, 0);
	lookupTable->SetTableRange(0, 1);
	lookupTable->Build();

	vtkUnsignedCharArray * colors = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");

	double rgb[3];
	for (int i = 0; i < stress->GetNumberOfTuples(); i++) {
		double r = stress->GetComponent(i, 0);
		double g = stress->GetComponent(i, 1);
		double b = stress->GetComponent(i, 2);
		lookupTable->GetColor(sqrt(r * r + g * g + b * b)*1066, rgb);
		colors->InsertNextTuple3(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255);
		//std::cout << sqrt(r * r + g * g + b * b)*1066 << " "<< rgb[0] * 255 << " " << rgb[1] * 255 << " " << rgb[2] * 255 << endl;
	}
	polydata->GetPointData()->SetScalars(colors);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polydata);
	mapper->SetScalarVisibility(1);
	//mapper->SetInputConnection(reader->GetOutputPort());
	//mapper->SetScalarModeToUsePointFieldData();
	//mapper->SelectColorArray("wallShearStress");
	mapper->SetColorModeToDefault();
	mapper->SetLookupTable(lookupTable);	// important!


	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetOpacity(1);

	vtkSmartPointer<vtkRenderer> render = vtkSmartPointer<vtkRenderer>::New();
	render->AddActor(actor);

	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(render);
	renWin->SetSize(1280, 720);

	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	iren->Initialize();
	iren->Start();
}