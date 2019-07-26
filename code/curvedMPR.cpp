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

#include <vtkPlaneSource.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataWriter.h>

vtkPolyDataMapper * surfaceMapper = vtkPolyDataMapper::New();
vtkActor * surfaceActor = vtkActor::New();
auto viewer = vtkImageViewer2::New();

// using vtkvmtkCurvedMPRImageFilter
void generate() {
	// show surface
	//vtkXMLPolyDataReader * sreader = vtkXMLPolyDataReader::New();
	//sreader->SetFileName("D:/surface.vtp");
	//sreader->Update();
	//surfaceMapper->SetInputData(sreader->GetOutput());
	//surfaceActor->SetMapper(surfaceMapper);

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

	vtkDICOMImageReader *dicomReader = vtkDICOMImageReader::New();
	dicomReader->SetDirectoryName("D:/demo/demo2/dicom");
	dicomReader->Update();

	auto cpr = vtkvmtkCurvedMPRImageFilter::New();
	cpr->SetInputData(dicomReader->GetOutput());
	cpr->SetCenterline(centerline);
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

	viewer->SetInputData(cpr->GetOutput());
	viewer->Render();
}

/*
// using vtkSplineDrivenImageSlicer
#include "vtkSplineDrivenImageSlicer.h"
#include "vtkFrenetSerretFrame.h"
vtkMetaImageReader * reader = vtkMetaImageReader::New();

void generate(vtkPolyData * polyData) {

	auto reslicer = vtkSmartPointer<vtkSplineDrivenImageSlicer>::New();

	reslicer->SetInputConnection(reader->GetOutputPort());
	reslicer->SetinputPath(polyData);	// change the source code
	//reslicer->SetOffsetPoint(30);
	//reslicer->SetSliceSpacing(4, 4);
	//reslicer->SetSliceExtent(47, 61);

	// by default, vtkSplineDrivenImageSlicer creates one slice, determined by OffsetPoint
	viewer->SetInputConnection(reslicer->GetOutputPort(0));

	//reslicer->Update();
	//viewer->SetInputData(reslicer->GetOutput(0));
	//vtkPolyData * plane = vtkPolyData::SafeDownCast(reslicer->GetOutputDataObject(1));
	//auto color = vtkUnsignedCharArray::New();
	//color->SetNumberOfComponents(3);
	//int len = plane->GetNumberOfPoints();
	//for (int i = 0; i < len; i++)
	//	color->InsertNextTuple3(0, 255, 255);
	//plane->GetPointData()->SetScalars(color);
	//pmapper2->SetInputData(plane);
	//pactor2->SetMapper(pmapper2);

	viewer->Render();
}
*/