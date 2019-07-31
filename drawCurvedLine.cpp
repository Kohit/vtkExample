#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
// Generic VTK pipeline elements
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
// Auxiliary class
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkCallbackCommand.h>
#include <vtkInteractorStyleImage.h>
#include <vtkWorldPointPicker.h>

void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

vtkSmartPointer<vtkPolyDataMapper> mapper =
	vtkSmartPointer<vtkPolyDataMapper>::New();

vtkSmartPointer<vtkRenderer> renderer =
	vtkSmartPointer<vtkRenderer>::New();

auto points = vtkSmartPointer<vtkPoints>::New();
auto lines = vtkSmartPointer<vtkCellArray>::New();

int main()
{
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();

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

	auto poly = vtkSmartPointer<vtkPolyData>::New();
	poly->SetPoints(points);
	poly->SetLines(lines);
	mapper->SetInputData(poly);
	actor->SetMapper(mapper);
	renderer->AddActor(actor);

	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

#include <vtkCardinalSpline.h>
#include <vtkSplineFilter.h>
vtkSmartPointer<vtkPolyData> getSpline(vtkPolyData * input) {
	auto spline = vtkSmartPointer<vtkCardinalSpline>::New();
	spline->SetLeftConstraint(2);
	spline->SetLeftValue(0.0);
	spline->SetRightConstraint(2);
	spline->SetRightValue(0.0);

	auto splineFilter = vtkSmartPointer<vtkSplineFilter>::New();
	splineFilter->SetInputData(input);
	splineFilter->SetNumberOfSubdivisions(100);	// 精度
	splineFilter->SetSpline(spline);
	splineFilter->Update();

	auto output = vtkSmartPointer<vtkPolyData>::New();
	output->DeepCopy(splineFilter->GetOutput());

	// 返回vtkSmartPointer对象
	return output;
}


void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData) {
	vtkInteractorStyleImage *style =
		static_cast<vtkInteractorStyleImage*>(caller);
	vtkRenderWindowInteractor* iren = style->GetInteractor();

	double windowPosition[3] = { iren->GetEventPosition()[0], iren->GetEventPosition()[1], 0 };
	vtkWorldPointPicker * wp = vtkWorldPointPicker::New();

	wp->Pick(windowPosition, renderer);

	double * pp = wp->GetPickPosition();

	if (vtkCommand::LeftButtonPressEvent == eventId) {   // 点击鼠标

		int * pos = iren->GetEventPosition();	// iren must be a interactor rather than style
		std::cout << pos[0] << " " << pos[1] << std::endl;

		vtkIdType pt_id = points->InsertNextPoint(pp);
		points->Modified();	// 标记脏数据

		std::cout << "points:" << pt_id << std::endl;
		// vtk没有提供往line内动态插入新点的方法，每次先插入一条新的line
		if (pt_id > 0)
		{
			lines->InsertNextCell(2);
			lines->InsertCellPoint(pt_id - 1);
			lines->InsertCellPoint(pt_id);
		}
		else {
			lines->InsertNextCell(1);
			lines->InsertCellPoint(0);
		}
		lines->Modified();

		if (pt_id > 4) {
			// 生成曲线(重新遍历)
			vtkIdType nbPoints = lines->GetNumberOfCells();
			auto cells = vtkSmartPointer<vtkCellArray>::New();
			cells->InsertNextCell(nbPoints);
			for (int i = 0; i < nbPoints; i++) {
				cells->InsertCellPoint(i);
			}
			auto poly = vtkSmartPointer<vtkPolyData>::New();
			poly->SetPoints(points);
			poly->SetLines(cells);

			auto spline = getSpline(poly);

			mapper->SetInputData(spline);
		}
		iren->GetRenderWindow()->Render();
	}
}