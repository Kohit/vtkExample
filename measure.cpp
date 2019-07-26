#include <vtkRegularPolygonSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
// Generic VTK pipeline elements
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
// Auxiliary class
#include <vtkSmartPointer.h>
#include <vtkMassProperties.h>
#include <vtkPoints.h>
#include <vtkTriangleFilter.h>
#include <vtkPolygon.h>
#include <vtkCallbackCommand.h>
#include <vtkInteractorStyleImage.h>
#include <vtkWorldPointPicker.h>
#include <vtkEllipseArcSource.h>
void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
auto source = vtkRegularPolygonSource::New();
auto ellipse = vtkEllipseArcSource::New();
vtkSmartPointer<vtkPolyDataMapper> mapper =
vtkSmartPointer<vtkPolyDataMapper>::New();

vtkSmartPointer<vtkActor> actor =
vtkSmartPointer<vtkActor>::New();

vtkSmartPointer<vtkRenderer> renderer =
vtkSmartPointer<vtkRenderer>::New();
vtkPolyData * drawEllipse();

int main()
{





	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	vtkInteractorStyleImage * style = vtkInteractorStyleImage::New();
	renderWindowInteractor->SetInteractorStyle(style);

	vtkSmartPointer<vtkCallbackCommand> callBack =
		vtkSmartPointer<vtkCallbackCommand>::New();
	callBack->SetCallback(ClickCallbackFunction);
	style->AddObserver(vtkCommand::LeftButtonPressEvent, callBack);
	style->AddObserver(vtkCommand::LeftButtonReleaseEvent, callBack);
	style->AddObserver(vtkCommand::MouseMoveEvent, callBack);


	//mapper->SetInputConnection(ellipse->GetOutputPort());


	auto poly = drawEllipse();
	poly->Print(std::cout);
	mapper->SetInputData(poly);

	actor->SetMapper(mapper);
	renderer->AddActor(actor);

	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyLine.h>
vtkPolyData * drawEllipse() {
	double angle = 0;
	double r1, r2;
	int id = 0;
	int CenterX, CenterY;
	CenterX = 0;
	CenterY = 0;
	r1 = 40;
	r2 = 20;

	auto ellipsePoints = vtkPoints::New();

	while (angle <= 2 * vtkMath::Pi())
	{
		ellipsePoints->InsertNextPoint(r1 * cos(angle) + CenterX, r2 * sin(angle) + CenterY, 0);
		angle = angle + (vtkMath::Pi() / 30);
		id++;
	}
	auto line = vtkPolyLine::New();
	line->GetPointIds()->SetNumberOfIds(id + 1);
	for (int i = 0; i < id; i++)
		line->GetPointIds()->SetId(i, i);
	std::cout << 1;
	auto c_lines = vtkCellArray::New();
	c_lines->InsertNextCell(line);

	auto c_polyData = vtkPolyData::New();
	c_polyData->Allocate(1, 1);
	c_polyData->SetPoints(ellipsePoints);
	c_polyData->SetLines(c_lines);

	return c_polyData;
}

bool isMousePressed = false;
double p1[3];
void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData) {
	vtkInteractorStyleImage *style =
		static_cast<vtkInteractorStyleImage*>(caller);
	vtkRenderWindowInteractor* iren = style->GetInteractor();

	double windowPosition[3] = { iren->GetEventPosition()[0], iren->GetEventPosition()[1], 0 };
	vtkWorldPointPicker * wp = vtkWorldPointPicker::New();
	//wp->Pick(windowPosition, viewXY->GetRenderer());
	wp->Pick(windowPosition, renderer);
	//
	double * pp = wp->GetPickPosition();

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
		p1[0] = pp[0];
		p1[1] = pp[1];
		p1[2] = 0;
		return;
	}
	if (vtkCommand::MouseMoveEvent != eventId) {
		return;
	}
	if (!isMousePressed) return;
	// 拖动鼠标
	// iren->GetLastEventPosition();

	double v1[3] = { pp[0], pp[1], 1 };

	double dist = sqrt(pow(v1[0] - p1[0], 2) + pow(v1[1] - p1[1], 2));
	double center[3];
	center[0] = p1[0] + (v1[0] - p1[0]) / 2;
	center[1] = p1[1] + (v1[1] - p1[1]) / 2;
	center[2] = 0;

	std::cout << "--------" << std::endl;
	std::cout << p1[0] << "," << p1[1] << std::endl;
	std::cout << v1[0] << "," << v1[1] << std::endl;
	std::cout << center[0] << "," << center[1] << std::endl;


	source->GeneratePolygonOff(); // Uncomment this line to generate only the outline of the circle
	source->SetNumberOfSides(50);
	source->SetRadius(dist / 2);
	source->SetCenter(center);

	std::cout << "area:" << vtkMath::Pi() * pow(dist / 2, 2) << std::endl;

	iren->GetRenderWindow()->Render();
	return;
}