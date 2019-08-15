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
#include <vtkTransform2D.h>
#include <vtkProperty.h>

void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
void load();
void test();

vtkSmartPointer<vtkPolyDataMapper> mapper =
vtkSmartPointer<vtkPolyDataMapper>::New();

#include <vtkImageActor.h>
vtkSmartPointer<vtkImageActor> iactor = vtkSmartPointer<vtkImageActor>::New();

vtkSmartPointer<vtkActor> actor =
vtkSmartPointer<vtkActor>::New();

vtkSmartPointer<vtkRenderer> renderer =
vtkSmartPointer<vtkRenderer>::New();
vtkPolyData * drawEllipse(double, double, double, double *);

//void print(const char * title, double * a, int len) {
//	std::cout << title << ": ";
//	for (int i = 0; i < len; i++)
//		std::cout << a[i] << " ";
//	std::cout << std::endl;
//}
//void print(const char * title, int * a, int len) {
//	std::cout << title << ": ";
//	for (int i = 0; i < len; i++)
//		std::cout << a[i] << " ";
//	std::cout << std::endl;
//}
template <typename T>
void print(const char * title, T * a, int len) {
	std::cout << title << ": ";
	for (int i = 0; i < len; i++)
		std::cout << a[i] << " ";
	std::cout << std::endl;
}
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

	load();	// 先显示图片，防止后面绘制被遮挡

	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(1, 0, 0);
	renderer->AddActor(actor);
	//test();


	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

#include <vtkImageProperty.h>
#include <vtkMetaImageReader.h>
#include <vtkImageData.h>

vtkImageData * image;
void load() {
	vtkMetaImageReader * reader = vtkMetaImageReader::New();
	reader->SetFileName("E:/cpr/HeadMRVolume.mhd");
	reader->Update();

	iactor->SetInputData(reader->GetOutput());
	iactor->GetProperty()->SetOpacity(1);
	reader->GetOutput()->Print(std::cout);
	image = reader->GetOutput();
	renderer->AddActor(iactor);
}

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyLine.h>
vtkPolyData * drawEllipse(double rotate, double r1, double r2, double * center) {
	double angle = 0;
	//double r1, r2;
	int id = 0;
	int CenterX, CenterY;
	CenterX = 0;
	CenterY = 0;
	//r1 = 40;
	//r2 = 20;

	auto ellipsePoints = vtkPoints::New();
	auto trans = vtkTransform2D::New();
	trans->Rotate(rotate);

	//trans->Print(std::cout);
	while (angle <= 2 * vtkMath::Pi())
	{
		double p[3] = { r1 * cos(angle) + 0, r2 * sin(angle) + 0, 0 };
		double o[3];
		trans->MultiplyPoint(p, o);	// 先从原点旋转，再将中心位移
		ellipsePoints->InsertNextPoint(o[0] + center[0], o[1] + center[1], 0);
		angle = angle + (vtkMath::Pi() / 30);
		id++;
	}
	auto line = vtkPolyLine::New();
	line->GetPointIds()->SetNumberOfIds(id);	// 多分配id会导致访问出错
	for (int i = 0; i < id; i++)
		line->GetPointIds()->SetId(i, i);

	auto c_lines = vtkCellArray::New();
	c_lines->InsertNextCell(line);

	auto c_polyData = vtkPolyData::New();
	c_polyData->SetPoints(ellipsePoints);
	c_polyData->SetLines(c_lines);

	return c_polyData;
}

int dyeEllipse(double rotate, double r1, double r2, double * center, double* avg, double* var) {
	double spacing[3];
	image->GetSpacing(spacing);

	int dim[3];
	image->GetDimensions(dim);

	auto img = vtkImageData::New();
	img->DeepCopy(image);

	std::vector<int> v;

	int count = 0;
	for (int i = 0; i < dim[0]; i++) {
		for (int j = 0; j < dim[1]; j++) {
			int k = 0;
			unsigned char * p = (unsigned char *)(img->GetScalarPointer(i, j, k));

			double x = i * spacing[0];
			double y = j * spacing[1];

			// 平移
			x = x - center[0];
			y = y - center[1];
			double p3[3] = { x, y, 0 };

			// 旋转(反向）
			auto trans1 = vtkTransform2D::New();
			trans1->Rotate(rotate);
			trans1->MultiplyPoint(p3, p3);

			if ((p3[0] * p3[0]) / (r1 * r1) + (p3[1] * p3[1]) / (r2 * r2) < 1) {
				count++;
				*avg = *avg + p[0];
				v.push_back(p[0]);
				p[0] = vtkMath::Min((int)(p[0] * 2), 255);
			}
		}
	}
	*avg = *avg / count;
	for (int i = 0; i < v.size(); i++) {
		*var = *var + pow(v[i] - *avg, 2);
	}
	*var = *var / count;

	iactor->SetInputData(img);	// 使用原来的image不会触发画面刷新

	return count;
}

#include <vtkProperty.h>
void ShowPoint(double x, double y) {
	vtkPolyData * point = vtkPolyData::New();
	vtkPoints * ps = vtkPoints::New();
	vtkCellArray * cell = vtkCellArray::New();
	vtkIdType pid[1];
	pid[0] = ps->InsertNextPoint(x, y, 0);
	cell->InsertNextCell(1, pid);
	point->SetPoints(ps);
	point->SetVerts(cell);
	auto mapper = vtkPolyDataMapper::New();
	auto actor = vtkActor::New();
	mapper->SetInputData(point);
	actor->SetMapper(mapper);
	actor->GetProperty()->SetPointSize(5);
	actor->GetProperty()->SetColor(1, 0, 0);

	renderer->AddActor(actor);

}

#include <vtkRegularPolygonSource.h>
vtkPolyData* drawCircle(double * center, double radius) {
	auto source = vtkRegularPolygonSource::New();
	source->GeneratePolygonOff(); // Uncomment this line to generate only the outline of the circle
	source->SetNumberOfSides(50);
	source->SetRadius(radius);
	source->SetCenter(center);
	source->Update();
	return source->GetOutput();
}

#include <vtkImageViewer2.h>
#include <vector>
int dyeCircle(double * center, double radius, double* avg, double* var) {
	double spacing[3];
	image->GetSpacing(spacing);

	int dim[3];
	image->GetDimensions(dim);

	auto img = vtkSmartPointer<vtkImageData>::New();
	img->DeepCopy(image);

	std::vector<int> v;
	//image->Print(std::cout);
	int count = 0;
	for (int i = 0; i < dim[0]; i++) {
		for (int j = 0; j < dim[1]; j++) {
			int k = 0;
			unsigned char * p = (unsigned char *)(img->GetScalarPointer(i, j, k));

			double x = i * spacing[0];
			double y = j * spacing[1];
			double dist = sqrt(pow(center[0] - x, 2) + pow(center[1] - y, 2));
			if (dist - radius <= -1e-3) {
				count++;
				*avg = *avg + p[0];
				v.push_back(p[0]);
				p[0] = vtkMath::Min((int)(p[0] * 2), 255);
			}
		}
	}
	*avg = *avg / count;
	for (int i = 0; i < v.size(); i++) {
		*var = *var + pow(v[i] - *avg, 2);
	}
	*var = *var / count;

	iactor->SetInputData(img);	// 使用原来的image不会触发画面刷新

	return count;
}

vtkPolyData * drawRectangle(double * tl, double * br) {
	double tr[3] = { br[0], tl[1], 0 };
	double bl[3] = { tl[0], br[1], 0 };
	auto points = vtkPoints::New();
	points->InsertNextPoint(bl);
	points->InsertNextPoint(br);
	points->InsertNextPoint(tr);
	points->InsertNextPoint(tl);

	auto line = vtkPolyLine::New();
	line->GetPointIds()->SetNumberOfIds(5);	// 多分配id会导致访问出错
	for (int i = 0; i < 4; i++)
		line->GetPointIds()->SetId(i, i);
	line->GetPointIds()->SetId(4, 0);	// 闭环

	auto lines = vtkCellArray::New();
	lines->InsertNextCell(line);

	auto poly = vtkPolyData::New();
	poly->SetPoints(points);
	poly->SetLines(lines);

	return poly;
}

int dyeRectangle(double * tl, double * br, double* avg, double* var) {
	double spacing[3];
	image->GetSpacing(spacing);

	int dim[3];
	image->GetDimensions(dim);

	auto img = vtkImageData::New();
	img->DeepCopy(image);

	double minx = vtkMath::Min(tl[0], br[0]);
	double maxx = vtkMath::Max(tl[0], br[0]);
	double miny = vtkMath::Min(tl[1], br[1]);
	double maxy = vtkMath::Max(tl[1], br[1]);

	std::vector<int> v;
	int count = 0;
	for (int i = 0; i < dim[0]; i++) {
		for (int j = 0; j < dim[1]; j++) {
			int k = 0;
			unsigned char * p = (unsigned char *)(img->GetScalarPointer(i, j, k));

			double x = i * spacing[0];
			double y = j * spacing[1];
			if (x >= minx && x <= maxx && y >= miny && y <= maxy) {
				count++;
				*avg = *avg + p[0];
				v.push_back(p[0]);
				p[0] = vtkMath::Min((int)(p[0] * 2), 255);
			}
		}
	}
	*avg = *avg / count;
	for (int i = 0; i < v.size(); i++) {
		*var = *var + pow(v[i] - *avg, 2);
	}
	*var = *var / count;
	iactor->SetInputData(img);	// 使用原来的image不会触发画面刷新
	// points.Modified()
	return count;
}

#include <vtkAxesActor.h>
void test() {
	double p1[3] = { 1, 1, 0 };
	double p2[3] = { 3, 1, 0 };
	double p3[3] = { 1, 2, 0 };
	p3[0] = 1 - sqrt(2) / 2;
	p3[1] = 1 + sqrt(2) / 2;
	// 长轴
	double a = sqrt(pow(p2[0] - p1[0], 2) + pow(p2[1] - p1[1], 2));

	// 平移
	p3[0] = p3[0] - p1[0];
	p3[1] = p3[1] - p1[1];

	// 旋转(反向）
	auto trans1 = vtkTransform2D::New();
	trans1->Rotate(45);
	trans1->MultiplyPoint(p3, p3);

	// 短轴
	double b = sqrt((1 - (p3[0] * p3[0]) / (a * a)) / (p3[1] * p3[1]));
	std::cout << "a:" << a << ", b:" << b << std::endl;


	double angle = 0;
	int id = 0;

	auto ellipsePoints = vtkPoints::New();
	auto trans = vtkTransform2D::New();
	trans->Rotate(360-45);

	//trans->Print(std::cout);
	while (angle <= 2 * vtkMath::Pi())
	{
		double p[3] = { 2 * cos(angle) + 0, 1 * sin(angle) + 0, 0 };
		double o[3];
		trans->MultiplyPoint(p, o);	// 先从原点旋转，再将中心位移
		ellipsePoints->InsertNextPoint(o[0] + p1[0], o[1] + p1[1], 0);
		angle = angle + (vtkMath::Pi() / 30);
		id++;
	}
	auto line = vtkPolyLine::New();
	line->GetPointIds()->SetNumberOfIds(id);	// 多分配id会导致访问出错
	for (int i = 0; i < id; i++)
		line->GetPointIds()->SetId(i, i);

	auto c_lines = vtkCellArray::New();
	c_lines->InsertNextCell(line);

	auto c_polyData = vtkPolyData::New();
	c_polyData->SetPoints(ellipsePoints);
	c_polyData->SetLines(c_lines);

	mapper->SetInputData(c_polyData);

	vtkSmartPointer<vtkAxesActor> axes =
		vtkSmartPointer<vtkAxesActor>::New();
	axes->AxisLabelsOn();
	renderer->AddActor(axes);
	//renderer->Render();
}
double p1[3];
double semiMajorAxis = 0;
double ellipseAngle = 0;
bool majorAxisDrawed = false;



bool isMousePressed = false;
int count = 0;
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
		majorAxisDrawed = true;

		std::cout << "release" << std::endl;
		return;
	}
	if (vtkCommand::LeftButtonPressEvent == eventId) {   // 点击鼠标
		std::cout << "press" << std::endl;
		int * pos = iren->GetEventPosition();	// iren must be a interactor rather than style
		std::cout << pos[0] << " " << pos[1] << std::endl;

		if (majorAxisDrawed) {

			isMousePressed = true;

			return;
		}
		else {
			p1[0] = pp[0];
			p1[1] = pp[1];
			p1[2] = 0;
			ShowPoint(p1[0], p1[1]);

			isMousePressed = true;
		}
		return;
	}
	if (vtkCommand::MouseMoveEvent != eventId) {
		return;
	}
	if (!isMousePressed) return;
	// 拖动鼠标
	// iren->GetLastEventPosition();
	//return;

	if (majorAxisDrawed) {
		double minorAxis = sqrt(pow(pp[0] - p1[0], 2) + pow(pp[1] - p1[1], 2));
		auto poly = drawEllipse(ellipseAngle, semiMajorAxis, minorAxis, p1);
		mapper->SetInputData(poly);
		double avg = 0, variance = 0;
		int count = dyeEllipse(360 - ellipseAngle, semiMajorAxis, minorAxis, p1, &avg, &variance);
		//int count = dyeCircle(center, dist / 2, &avg, &variance);
		std::cout << count << ", avg:" << avg << ", variance:" << variance << std::endl;
		
		iren->GetRenderWindow()->Render();
		return;
	}

	double v1[3] = { pp[0], pp[1], 1 };

	double dist = sqrt(pow(v1[0] - p1[0], 2) + pow(v1[1] - p1[1], 2));
	double center[3];
	center[0] = p1[0] + (v1[0] - p1[0]) / 2;
	center[1] = p1[1] + (v1[1] - p1[1]) / 2;
	center[2] = 0;
	double degree = vtkMath::AngleBetweenVectors(p1, v1) * 180 / vtkMath::Pi();	// 0-v1, 0-v2
	double angle = atan2(p1[1] - v1[1], p1[0] - v1[0]) * 180 / vtkMath::Pi(); // v1平移至原点, v1-v2
	std::cout << "--------" << std::endl;
	std::cout << p1[0] << "," << p1[1] << std::endl;
	std::cout << v1[0] << "," << v1[1] << std::endl;
	std::cout << center[0] << "," << center[1] << std::endl;
	std::cout << degree << ", " << angle << std::endl;
	std::cout << "area:" << vtkMath::Pi() * pow(dist / 2, 2) << std::endl;

	//auto poly = drawCircle(center, dist / 2);
	semiMajorAxis = dist;
	ellipseAngle = 360 - angle;
	auto poly = drawEllipse(360 - angle, dist, dist / 2, p1);

	//auto poly = drawRectangle(p1, v1);
	mapper->SetInputData(poly);
	//double avg = 0, variance = 0;
	//int count = dyeRectangle(p1, v1, &avg, &variance);
	////int count = dyeCircle(center, dist / 2, &avg, &variance);
	//std::cout << count << ", avg:" << avg << ", variance:" << variance << std::endl;
	iren->GetRenderWindow()->Render();
	return;
}