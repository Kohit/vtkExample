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
#include <future>
#include <chrono>
#include <vtkPlaneSource.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataWriter.h>

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

vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
vtkPolyDataMapper * pmapper2 = vtkPolyDataMapper::New();
vtkActor * pactor2 = vtkActor::New();

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

	// time step
	double lo = 0, hi = range[1], step = 0.001, currentTime = 0.05;
	double streakLineTimeLength = 0.01;
	int currentCycle = 0;
	pactor2->SetMapper(pmapper2);

	while (currentTime < hi) {
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

		std::cout << currentTime << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::cout << "done";
}
