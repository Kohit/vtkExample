#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCornerAnnotation.h>

int main(int, char* [])
{

	// Create a sphere
	vtkSmartPointer<vtkSphereSource> sphereSource =
		vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetCenter(0.0, 0.0, 0.0);
	sphereSource->SetRadius(5.0);
	sphereSource->Update();

	// Create a mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
#if VTK_MAJOR_VERSION <= 5
	mapper->SetInput(sphereSource->GetOutput());
#else
	mapper->SetInputData(sphereSource->GetOutput());
#endif

	// Create an actor
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// Create a renderer
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderer->SetBackground(1, 1, 1); // Set background color to white
	renderer->AddActor(actor);

	// Create a render window
	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	// Create an interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// Setup the text and add it to the renderer
	vtkSmartPointer<vtkTextActor> textActor =
		vtkSmartPointer<vtkTextActor>::New();
	textActor->SetInput("Hello world.");
	textActor->SetPosition(50, 0);
	textActor->GetTextProperty()->SetFontSize(24);
	textActor->GetTextProperty()->SetColor(230.0/255, 230.0/255, 130.0/255);
	renderer->AddActor2D(textActor);

	// Annotate the image with window/level and mouse over pixel information
	vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation =
		vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation->SetLinearFontScaleFactor(2);
	cornerAnnotation->SetNonlinearFontScaleFactor(1);
	cornerAnnotation->SetMaximumFontSize(20);
	cornerAnnotation->SetText(0, "lower left1");
	cornerAnnotation->SetText(1, "lower right\nblabla");
	cornerAnnotation->SetText(2, "\nupper left\nblabla");
	cornerAnnotation->SetText(3, "\n\n\nupper right");
	cornerAnnotation->GetTextProperty()->SetColor(1, 0, 0);

	renderer->AddViewProp(cornerAnnotation);

	vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation2 =
		vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation2->SetLinearFontScaleFactor(2);
	cornerAnnotation2->SetNonlinearFontScaleFactor(1);
	cornerAnnotation2->SetMaximumFontSize(20);
	cornerAnnotation2->SetText(0, "\nlower left2\n");
	cornerAnnotation2->SetText(1, "lower right\nblabla");
	cornerAnnotation2->SetText(2, "upper left\nblabla");
	cornerAnnotation2->SetText(3, "upper right");
	cornerAnnotation2->GetTextProperty()->SetColor(1, 0, 0);

	renderer->AddViewProp(cornerAnnotation2);

	// Render and interact
	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}