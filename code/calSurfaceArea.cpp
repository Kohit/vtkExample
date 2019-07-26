#include <vtkPolygon.h>
#include <vtkTriangleFilter.h>
#include <vtkMassProperties.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
static double GetSurfaceArea(vtkPoints * points) {
	vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();

	// 构建多边形
	polygon->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());

	for (int i = 0; i < points->GetNumberOfPoints(); i++) {
		polygon->GetPointIds()->SetId(i, i);
	}

	vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
	polyData->Allocate();
	polyData->InsertNextCell(polygon->GetCellType(), polygon->GetPointIds());
	polyData->SetPoints(points);

	// 划分为多个三角形
	vtkSmartPointer<vtkTriangleFilter> tri1 = vtkSmartPointer<vtkTriangleFilter>::New();
	tri1->SetInputData(polyData);

	vtkSmartPointer<vtkPolyDataMapper> polygonMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	polygonMapper->SetInputConnection(tri1->GetOutputPort());

	// 计算表面积
	vtkSmartPointer<vtkMassProperties> polygonProperties = vtkSmartPointer<vtkMassProperties>::New();
	polygonProperties->SetInputConnection(tri1->GetOutputPort());
	polygonProperties->Update();

	return polygonProperties->GetSurfaceArea();
}
int main() {
	vtkPoints* polygonPoints1 = vtkPoints::New();

	polygonPoints1->SetNumberOfPoints(4);

	polygonPoints1->InsertPoint(0, 0, 0, 0);


	polygonPoints1->InsertPoint(1, 0, 1, 0);


	polygonPoints1->InsertPoint(2, 1, 1, 0);


	polygonPoints1->InsertPoint(3, 1, 0, 0);

	std::cout << GetSurfaceArea(polygonPoints1);
	return 0;
}