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

vtkPolyData * Centerline = vtkPolyData::New();
int SliceExtent[2] = { 100, 100 };
double SliceSpacing[2] = { 1, 1 };
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
	//print("center", centerSlice, 3);

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

	//vtkPolyData * point = vtkPolyData::New();
	//vtkPoints * ps = vtkPoints::New();
	//vtkCellArray * cell = vtkCellArray::New();
	//vtkIdType pid[1];
	//pid[0] = ps->InsertNextPoint(centerSlice[0], centerSlice[1], centerSlice[2]);
	//cell->InsertNextCell(1, pid);
	//point->SetPoints(ps);
	//point->SetVerts(cell);
	//pmapper2->SetInputData(point);
	//pactor2->SetMapper(pmapper2);
	//pactor2->GetProperty()->SetPointSize(5);
	//pactor2->GetProperty()->SetColor(1, 0, 0);

	//render->ResetCamera();
	//render->GetActiveCamera()->SetFocalPoint(centerSlice[0], centerSlice[1], centerSlice[2]);


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
	//print("t:", t, 3);
	//print("n:", p, 3);
	//print("c:", tp, 3);
	//print("p:", planeOrigin, 3);
	//print("p1:", planePoint1, 3);
	//print("p2:", planePoint2, 3);
	std::cout << "--------------------------" << std::endl;

	plane->SetResolution(SliceExtent[0], SliceExtent[1]);
	plane->Update();

	return plane->GetOutput();
}
