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
#include "vtkSplineDrivenImageSlicer.h"
#include "vtkAlgorithmOutput.h"
#include <vtkvmtkCenterlineAttributesFilter.h>
#include <vtkvmtkCenterlineGeometry.h>
#include <vtkvmtkCurvedMPRImageFilter.h>
#include <vtkMetaImageWriter.h>
#include "vtkFrenetSerretFrame.h"
#include <future>
#include <chrono>

void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

int slice = 0;
vtkImageViewer2 * viewXY = vtkImageViewer2::New();
vtkImageViewer2 * viewYZ = vtkImageViewer2::New();
vtkImageViewer2 * viewXZ = vtkImageViewer2::New();
auto viewer = vtkImageViewer2::New();
auto viewer2 = vtkImageViewer2::New();

double center[3];

vtkMetaImageReader * reader = vtkMetaImageReader::New();

vtkSmartPointer<vtkRenderer> render = vtkSmartPointer<vtkRenderer>::New();

vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();


vtkSmartPointer<vtkRenderWindowInteractor> _iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
vtkPolyData * polyData = vtkPolyData::New();
vtkPolyDataMapper * pmapper = vtkPolyDataMapper::New();
vtkPolyDataMapper * pmapper2 = vtkPolyDataMapper::New();
vtkPolyDataMapper * surfaceMapper = vtkPolyDataMapper::New();
vtkPolyDataMapper * planeMapper = vtkPolyDataMapper::New();
vtkActor * pactor = vtkActor::New();
vtkActor * pactor2 = vtkActor::New();
vtkActor * surfaceActor = vtkActor::New();
vtkActor * planeActor = vtkActor::New();
vtkPoints * points = vtkPoints::New();
vtkCellArray * lines = vtkCellArray::New();
int extent[6];
double spacing[3];
void print(const char * title, double * a, int len) {
	std::cout << title << ": ";
	for (int i = 0; i < len; i++)
		std::cout << a[i] << " ";
	std::cout << std::endl;
}
void print(const char * title, int * a, int len) {
	std::cout << title << ": ";
	for (int i = 0; i < len; i++)
		std::cout << a[i] << " ";
	std::cout << std::endl;
}
void print(vtkDataArray * data) {
	int m = data->GetNumberOfComponents();
	int n = data->GetNumberOfTuples();
	for (int i = 0; i < n; i++) {
		std::cout << "[";
		for (int j = 0; j < m; j++)
			std::cout << data->GetComponent(i, j) << ",";
		std::cout << "],";
	}
	std::cout << std::endl;
}

vtkPolyData * Centerline = vtkPolyData::New();
int SliceExtent[2] = { 100, 100 };
double SliceSpacing[2] = { 1, 1 };
#include <vtkPlaneSource.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataWriter.h>
/*
centerline有1个cell，cell里包含所有的点
centerline的点集为逆序，计算的tangent也是逆序，即tangent[0]表示终点（centerline->getpoints()->getpoint(0))的切线
centerline->getlines()->getnextcell(n, points), 其中points[0]为起点的id，points[n]为终点的id（即0），若采用points记录的id遍历，则可以匹配
centerline->getcell(0)->getpoints() 返回的是顺序点集，和centerline->getpoints()相反，此时点集顺序跟tangent不匹配

*/
vtkPolyData * getPlane(int slice, int * SliceExtent, double * SliceSpacing) {
	
	vtkCell* line = NULL;
	//get the first line in the vktPolyData
	for (int firstLineInCells = 0; firstLineInCells < Centerline->GetNumberOfCells(); firstLineInCells++)
	{
		line = Centerline->GetCell(firstLineInCells);
		if (line->GetCellType() == VTK_LINE || line->GetCellType() == VTK_POLY_LINE)
		{
			break;
		}
	}
	vtkIdList* pointIds = line->GetPointIds();

	vtkDoubleArray* frenetTangentArray = vtkDoubleArray::SafeDownCast(Centerline->GetPointData()->GetArray("FSTangents"));
	vtkDoubleArray* parallelTransportNormalsArray = vtkDoubleArray::SafeDownCast(Centerline->GetPointData()->GetArray("ParallelTransportNormals"));

	int numberOfLinePoints = pointIds->GetNumberOfIds();
	slice = pointIds->GetId(slice);

	double centerSlice[3];
	Centerline->GetPoint(slice, centerSlice);
	print("center", centerSlice, 3);

	// To calculate the outputorigin & the necessarry transform 
	// the vectors are retreived from the array's
	//t is the vector in the direction of the Centerline, so along the z-axis in the MPR volume
	double t[3];
	frenetTangentArray->GetTuple(slice, t);

	//p is a normal of the Centerline, directed to the 'North' direction of the inputvolume,in the MPR volume this will be along the x-axis 
	double p[3];
	parallelTransportNormalsArray->GetTuple(slice, p);

	double tp[3];
	//tp is the crossproduct of  t and p, and will be directed to the 'West' direction of the inputvolume,in the MPR volume this will be along the y-axis 
	tp[0] = (t[1] * p[2] - t[2] * p[1]);
	tp[1] = (t[2] * p[0] - t[0] * p[2]);
	tp[2] = (t[0] * p[1] - t[1] * p[0]);

	vtkPolyData * point = vtkPolyData::New();
	vtkPoints * ps = vtkPoints::New();
	vtkCellArray * cell = vtkCellArray::New();
	vtkIdType pid[1];
	pid[0] = ps->InsertNextPoint(centerSlice[0], centerSlice[1], centerSlice[2]);
	cell->InsertNextCell(1, pid);
	point->SetPoints(ps);
	point->SetVerts(cell);
	pmapper2->SetInputData(point);
	pactor2->SetMapper(pmapper2);
	pactor2->GetProperty()->SetPointSize(5);
	pactor2->GetProperty()->SetColor(1, 0, 0);

	render->ResetCamera();
	render->GetActiveCamera()->SetFocalPoint(centerSlice[0], centerSlice[1], centerSlice[2]);


	// Build the plane output that will represent the slice location in 3D view
	vtkPlaneSource * plane = vtkPlaneSource::New();
	double planeOrigin[3];
	double planePoint1[3];
	double planePoint2[3];
	for (int idx = 0; idx < 3; idx++)
	{
		//planeOrigin[idx] = centerSlice[idx];
		planeOrigin[idx] = centerSlice[idx] - p[idx] * SliceExtent[1] * SliceSpacing[1] / 2.0
			- tp[idx] * SliceExtent[0] * SliceSpacing[0] / 2.0;
		planePoint1[idx] = planeOrigin[idx] + tp[idx] * SliceExtent[0] * SliceSpacing[0];
		planePoint2[idx] = planeOrigin[idx] + p[idx] * SliceExtent[1] * SliceSpacing[1];
		std::cout << "dot:" << vtkMath::Dot(t, p) << std::endl;
	}
	plane->SetOrigin(planeOrigin);
	plane->SetPoint1(planePoint1);
	plane->SetPoint2(planePoint2);
	std::cout << "------------" << slice << "--------------" << std::endl;
	print("t:", t, 3);
	print("n:", p, 3);
	print("c:", tp, 3);
	print("p:", planeOrigin, 3);
	print("p1:", planePoint1, 3);
	print("p2:", planePoint2, 3);
	std::cout << "--------------------------" << std::endl;

	plane->SetResolution(SliceExtent[0], SliceExtent[1]);
	plane->Update();

	return plane->GetOutput();
}


void generate() {
	vtkXMLPolyDataReader * sreader = vtkXMLPolyDataReader::New();
	sreader->SetFileName("D:/surface.vtp");
	sreader->Update();
	surfaceMapper->SetInputData(sreader->GetOutput());
	surfaceActor->SetMapper(surfaceMapper);

	vtkPolyDataReader * creader = vtkPolyDataReader::New();
	creader->SetFileName("D:/centerline2.vtk");
	creader->Update();

	auto centerlineAttributes = vtkSmartPointer<vtkvmtkCenterlineAttributesFilter>::New();
	centerlineAttributes->SetInputData(creader->GetOutput());
	centerlineAttributes->SetAbscissasArrayName("Abscissas");
	centerlineAttributes->SetParallelTransportNormalsArrayName("ParallelTransportNormals");
	centerlineAttributes->Update();

	vtkPolyData * centerline = centerlineAttributes->GetOutput();

	auto filter = vtkvmtkCenterlineGeometry::New();
	filter->SetInputData(centerline);
	filter->SetLengthArrayName("Length");
	filter->SetCurvatureArrayName("cur");
	filter->SetTorsionArrayName("tor");
	filter->SetTortuosityArrayName("tort");
	filter->SetFrenetNormalArrayName("FSNormals");
	filter->SetFrenetTangentArrayName("FSTangents");
	filter->SetFrenetBinormalArrayName("FSBinormals");
	filter->SetLineSmoothing(1);
	filter->Update();

	centerline = filter->GetOutput();
	Centerline = filter->GetOutput();

	vtkDICOMImageReader *dicomReader = vtkDICOMImageReader::New();
	dicomReader->SetDirectoryName("D:/demo/demo2/dicom");
	dicomReader->Update();

	auto cpr = vtkvmtkCurvedMPRImageFilter::New();
	cpr->SetInputData(dicomReader->GetOutput());
	cpr->SetCenterline(Centerline);
	cpr->SetParallelTransportNormalsArrayName("ParallelTransportNormals");

	cpr->SetFrenetTangentArrayName("FSTangents");
	cpr->SetInplaneOutputSpacing(1, 1);
	cpr->SetInplaneOutputSize(100, 100);
	cpr->SetReslicingBackgroundLevel(0);
	cpr->Update();

	//auto writer2 = vtkXMLImageDataWriter::New();
	//writer2->SetInputData(cpr->GetOutput());
	//writer2->SetFileName("D:/cut_tp_p_t.vti");
	//writer2->Write();

	print("extent", SliceExtent, 6);
	print("spacing", SliceSpacing, 6);
	viewer->SetInputData(cpr->GetOutput());
	viewer->Render();

	//auto reader2 = vtkXMLImageDataReader::New();
	//reader2->SetFileName("D:/cut_tp_p_t.vti");
	//reader2->Update();
	//viewer2->SetInputData(reader2->GetOutput());
	//viewer2->Render();
}

void generate(vtkPolyData * path, vtkImageData * img) {

	auto centerlineAttributes = vtkSmartPointer<vtkvmtkCenterlineAttributesFilter>::New();
	centerlineAttributes->SetInputData(path);
	centerlineAttributes->SetAbscissasArrayName("Abscissas");
	centerlineAttributes->SetParallelTransportNormalsArrayName("ParallelTransportNormals");
	centerlineAttributes->Update();

	vtkPolyData * centerline = centerlineAttributes->GetOutput();

	auto filter = vtkvmtkCenterlineGeometry::New();
	filter->SetInputData(centerline);
	filter->SetLengthArrayName("Length");
	filter->SetCurvatureArrayName("cur");
	filter->SetTorsionArrayName("tor");
	filter->SetTortuosityArrayName("tort");
	filter->SetFrenetNormalArrayName("FSNormals");
	filter->SetFrenetTangentArrayName("FSTangents");
	filter->SetFrenetBinormalArrayName("FSBinormals");
	filter->SetLineSmoothing(1);
	filter->Update();

	centerline = filter->GetOutput();

	auto cpr = vtkvmtkCurvedMPRImageFilter::New();
	cpr->SetInputData(img);
	cpr->SetCenterline(centerline);
	cpr->SetParallelTransportNormalsArrayName("ParallelTransportNormals");
	cpr->SetFrenetTangentArrayName("FSTangents");
	cpr->SetInplaneOutputSpacing(4, 4);
	cpr->SetInplaneOutputSize(100, 100);
	cpr->SetReslicingBackgroundLevel(0);
	cpr->Update();

	viewer->SetInputData(cpr->GetOutput());
	viewer->Render();
}

#include <vtkCellData.h>
#include <vtkTuple.h>
void cal_time() {
	auto reader = vtkPolyDataReader::New();
	reader->SetFileName("D:/demo/demo1/case/postProcessing/sets/streamLines/280/track0_U.vtk");
	reader->Update();

	auto data = reader->GetOutput();
	auto speed = data->GetPointData()->GetArray("U");
	//print("bound", data->GetBounds(), 6);
	vtkIdType maxp = 0;
	for (int i = 0; i < data->GetNumberOfCells(); i++) {
		vtkCell * cell = data->GetCell(i);
		maxp = vtkMath::Max(maxp, cell->GetNumberOfPoints());
	}
	vtkDoubleArray* time = vtkDoubleArray::New();
	time->SetNumberOfComponents(1);
	time->SetNumberOfTuples(data->GetNumberOfPoints());
	time->SetName("IntegrationTime");

	double max = 0;
	int stop = 0;
	for (int i = 0; i < data->GetNumberOfCells(); i++) {
		vtkCell * cell = data->GetCell(i);
		vtkIdList * ids = cell->GetPointIds();
		double sum = 0;
		time->SetComponent(ids->GetId(0), 0, sum);

		for (int j = 1; j < cell->GetNumberOfPoints(); j++) {
			double p1[3] = { 0, 0, 0 };
			data->GetPoint(ids->GetId(j - 1), p1);
			double p2[3] = { 0, 0, 0 };
			data->GetPoint(ids->GetId(j), p2);
			double dist = sqrt(pow(p2[0] - p1[0], 2) + pow(p2[1] - p1[1], 2) + pow(p2[2] - p1[2], 2));
			double * u = speed->GetTuple3(ids->GetId(j - 1));	// 起点的速度
			double s = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
			sum = sum + dist / s;
			max = vtkMath::Max(max, sum);
			time->SetComponent(ids->GetId(j), 0, sum);
		}
	}
	data->GetPointData()->AddArray(time);

	auto writer = vtkXMLPolyDataWriter::New();
	writer->SetInputData(data);
	writer->SetDataMode(vtkXMLWriter::Ascii);
	writer->SetFileName("D:/demo/demo1/case/postProcessing/sets/streamLines/280/track0_U_IntegrationTime.vtp");
	writer->Write();

	///////////////////////////////
	//vtkCell * cell = data->GetCell(0);
	//vtkIdList * ids = cell->GetPointIds();
	//double p1[3] = { 0, 0, 0 };
	//double p2[3] = { 0, 0, 0 };
	//data->GetPoint(ids->GetId(0), p1);
	//data->GetPoint(ids->GetId(1), p2);
	//double dist = sqrt(pow(p2[0] - p1[0], 2) + pow(p2[1] - p1[1], 2) + pow(p2[2] - p1[2], 2));
	//double * u = speed->GetTuple3(ids->GetId(0));
	//double s = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
	//std::cout << "p1:" << ids->GetId(0) << " p2:" << ids->GetId(1) << std::endl;
	//print("p1", p1, 3);
	//print("p2", p2, 3);
	//print("speed", u, 3);
	//std::cout << "dist:" << dist << " s:" << s << " time:" << dist / s << std::endl;
	//std::cout << data->GetCellData()->GetArray("IntegrationTime")->GetComponent(0,1) << std::endl;
}

#include <vtkClipPolyData.h>
#include <thread>
void animate() {
	auto reader = vtkXMLPolyDataReader::New();
	reader->SetFileName("D:/demo/demo1/case/postProcessing/sets/streamLines/280/track0_U_IntegrationTime.vtp");
	reader->Update();

	auto track = vtkPolyData::New();
	track->DeepCopy(reader->GetOutput());
	track->GetPointData()->SetActiveScalars("IntegrationTime");
	double * range = track->GetPointData()->GetScalars()->GetRange(0);
	vtkCell * cell = track->GetCell(50);
	vtkIdList * ids = cell->GetPointIds();
	
	double p1[3], p2[3], t[3];
	track->GetPoint(ids->GetNumberOfIds() / 3, p1);
	track->GetPoint(ids->GetNumberOfIds() / 3 + 1, p2);
	p1[0] = p1[0] + 0.015;
	p1[1] = p1[1] + 0.005;
	//p1[2] = p1[2] - 0.005;
	p2[0] = p2[0] - 0.01;
		//p2[2] = p2[2] - 0.01;
	//for (int i = 0; i < 3; i++)
	//	t[i] = (p2[i] - p1[i]) / 2;
	//vtkMath::Normalize(t);

	//double n
	
	render->GetActiveCamera()->SetPosition(p1);
	render->GetActiveCamera()->SetFocalPoint(p2);
	render->GetActiveCamera()->Dolly(0.3);
	// time step
	double lo = 0, hi = range[1], step = 0.001, currentTime = 0.00;
	double streakLineTimeLength = 0.01;
	int currentCycle = 0;
	pactor2->SetMapper(pmapper2);
	
	while (currentTime < hi) {
		//render->GetActiveCamera()->Dolly(0.95);

		currentTime = currentTime + step;
		auto clip1 = vtkClipPolyData::New();
		clip1->SetInputData(track);
		clip1->GenerateClipScalarsOff();
		clip1->InsideOutOn();

		clip1->SetValue(currentTime);
		//clip2->SetValue(time - streakLineTimeLength);
		//clip1->Update();

		pmapper2->SetInputConnection(clip1->GetOutputPort());
		//pmapper2->SetScalarRange(0, currentTime);

		renWin->Render();

		//std::cout << currentTime << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	std::cout << "done";
}

vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();

#include <vtkPlane.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkImageProperty.h>
int main() {

	//cal_time();
	//vtkSmartPointer<vtkRenderer> render = vtkSmartPointer<vtkRenderer>::New();

	//vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	//renWin->AddRenderer(render);
	//renWin->SetSize(1280, 720);

	//iren->SetRenderWindow(renWin);
	//viewer->SetupInteractor(iren);
	//generate();
	vtkInteractorStyleImage * style = vtkInteractorStyleImage::New();
	//vtkInteractorStyleTrackballCamera * style = vtkInteractorStyleTrackballCamera::New();
	//iren->SetInteractorStyle(style);

	reader->SetFileName("C:/Users/Ad/Downloads/cpr/HeadMRVolume.mhd");
	reader->Update();

	pmapper->SetInputData(polyData);
	pactor->SetMapper(pmapper);


	renWin->SetInteractor(iren);





	//viewXY->SetInputConnection(reader->GetOutputPort());
	////viewXY->SetupInteractor(iren);
	//viewXY->SetSlice(1);
	//viewXY->SetSliceOrientationToXY();
	//viewXY->SetSize(300, 300);
	//viewXY->GetRenderer()->AddActor(pactor);

	////viewXY->Render();

	//viewYZ->SetInputConnection(reader->GetOutputPort());
	////viewYZ->SetupInteractor(iren);
	//viewYZ->SetSlice(1);
	//viewYZ->SetSliceOrientationToYZ();
	//viewYZ->SetSize(300, 300);

	////viewYZ->Render();

	//viewXZ->SetInputConnection(reader->GetOutputPort());
	////viewXZ->SetupInteractor(iren);
	//viewXZ->SetSlice(1);
	//viewXZ->SetSliceOrientationToXZ();
	//viewXZ->SetSize(300, 300);

	//viewXZ->Render();

	vtkSmartPointer<vtkCallbackCommand> callBack =
		vtkSmartPointer<vtkCallbackCommand>::New();
	callBack->SetCallback(ClickCallbackFunction);
	//iren->AddObserver(vtkCommand::MouseWheelForwardEvent, callBack);
	//iren->AddObserver(vtkCommand::MouseWheelBackwardEvent, callBack);
	iren->AddObserver(vtkCommand::LeftButtonPressEvent, callBack);
	//style->AddObserver(vtkCommand::LeftButtonReleaseEvent, callBack);
	//iren->AddObserver(vtkCommand::MouseMoveEvent, callBack);


	//int extent[6];
	//double spacing[3];
	//double origin[3];

	//reader->GetOutput()->GetExtent(extent);
	//reader->GetOutput()->GetSpacing(spacing);
	//reader->GetOutput()->GetOrigin(origin);

	//center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
	//center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
	//center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);

	//print("extent", extent, 6);
	//print("spacing", spacing, 3);
	//print("origin", origin, 3);
	//print("center", center, 3);



	vtkImageActor * iactor = vtkImageActor::New();
	iactor->SetInputData(reader->GetOutput());



	//render->AddActor(iactor);
	//render->AddActor(pactor);
	render->AddActor(pactor2);
	//surfaceActor->GetProperty()->SetOpacity(0.7);
	//render->AddActor(surfaceActor);
	//render->AddActor(planeActor);

	renWin->AddRenderer(render);
	renWin->SetSize(600, 600);
	renWin->Render();
	iren->Initialize();
	//iren->SetDesiredUpdateRate(30);
	animate();
	//std::thread th(animate);
	//th.join();
	//std::cout << "h";
	iren->Start();



	return 0;
}



int stop = 0;
bool isMousePressed = false;
double lastPost[3];
#include <vtkCamera.h>
void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData) {
	//std::cout << "Event: " << vtkCommand::GetStringFromEventId(eventId) << std::endl;
	
	// iteractor/style设置observer回调函数获得的caller格式不同
	//vtkInteractorStyleImage *style =
	//	static_cast<vtkInteractorStyleImage*>(caller);
	//vtkRenderWindowInteractor* iren = style->GetInteractor();
	// Get the interactor like this:
	//vtkRenderWindowInteractor *iren =
	//	static_cast<vtkRenderWindowInteractor*>(caller);
	//int * pos = iren->GetEventPosition();
	//std::cout << pos[0] << " " << pos[1] << std::endl;
	if (eventId == vtkCommand::LeftButtonPressEvent && !stop) {

		double windowPosition[3] = { iren->GetEventPosition()[0], iren->GetEventPosition()[1], 0 };
		vtkWorldPointPicker * wp = vtkWorldPointPicker::New();
		//wp->Pick(windowPosition, viewXY->GetRenderer());
		wp->Pick(windowPosition, render);
		//
		double * pp = wp->GetPickPosition();
		//pp[2] = center[2];
		//pp[2] = 0.01;	// viewer2只显示平面数据

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
		//else {
			polyData->SetPoints(points);
			polyData->SetLines(lines);
		//}
			pmapper->SetInputData(polyData);
			pactor->SetMapper(pmapper);
			render->AddActor(pactor);
		viewXY->Render();
		renWin->Render();



		if (pt_id > 3 && false) {
			auto reslicer = vtkSmartPointer<vtkSplineDrivenImageSlicer>::New();

			reslicer->SetInputConnection(reader->GetOutputPort());
			reslicer->SetinputPath(polyData);
			//reslicer->SetOffsetPoint(30);
			//reslicer->SetSliceSpacing(4, 4);
			//reslicer->SetSliceExtent(47, 61);
			
			viewer->SetInputConnection(reslicer->GetOutputPort(0));
			//reslicer->Update();
			//
			//viewer->SetInputData(reslicer->GetOutput(0));
			//reslicer->Update();
			//vtkPolyData * plane = vtkPolyData::SafeDownCast(reslicer->GetOutputDataObject(1));
			//auto color = vtkUnsignedCharArray::New();
			//color->SetNumberOfComponents(3);
			//int len = plane->GetNumberOfPoints();
			//for (int i = 0; i < len; i++)
			//	color->InsertNextTuple3(0, 255, 255);
			//plane->GetPointData()->SetScalars(color);
			//pmapper2->SetInputData(plane);
			//pactor2->SetMapper(pmapper2);

			renWin->Render();
			viewer->Render();
			stop = 1;
			//generate(polyData, reader->GetOutput());

		}

		return;
	}
	
	// get angle between two position
	if (false) {
		if (vtkCommand::LeftButtonReleaseEvent == eventId) {   // 释放鼠标
			isMousePressed = false;
			std::cout << "release" << std::endl;
			return;
		}
		if (vtkCommand::LeftButtonPressEvent == eventId) {   // 点击鼠标
			isMousePressed = true;
			std::cout << "press" << std::endl;
			int * pos = iren->GetEventPosition();
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
		//slice = 50;
		// handle wheels
		std::cout << "SLICE:" << slice << std::endl;

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

		//viewer2->SetSlice(slice);
		//viewer2->GetRenderer()->ResetCamera();
		//viewer2->Render();

		//render->ResetCamera();

		viewXY->SetSlice(slice);
		viewXY->GetRenderer()->ResetCamera();
		viewXY->Render();

		viewYZ->SetSlice(slice);
		viewYZ->GetRenderer()->ResetCamera();
		viewYZ->Render();

		viewXZ->SetSlice(slice);
		viewXZ->GetRenderer()->ResetCamera();
		viewXZ->Render();

		vtkPolyData * plane = getPlane(vtkMath::Max(slice, 0), SliceExtent, SliceSpacing);

		planeMapper->SetInputData(plane);
		planeActor->SetMapper(planeMapper);
		renWin->Render();
	}
}
