
// Auxiliary class
#include <vtkSmartPointer.h>
#include "vtkCommand.h"

#include <vtkDICOMReader.h>
#include <vtkDICOMDirectory.h>
#include <vtkDICOMItem.h>
#include <vtkStringArray.h>
#include <vtkDICOMMetaData.h>
#include <vtkDICOMParser.h>

template<typename T>
void p(T str) {
	std::cout << str << std::endl;
}

class ReaderProgress : public vtkCommand
{
public:
	vtkTypeMacro(ReaderProgress, vtkCommand);

	static ReaderProgress* New() { return new ReaderProgress; }

	void Execute(vtkObject* object, unsigned long event, void* data)
		VTK_DICOM_OVERRIDE;
};

void ReaderProgress::Execute(
	vtkObject* object, unsigned long event, void* data)
{
	if (event == vtkCommand::ProgressEvent)
	{
		if (data)
		{
			double progress = *static_cast<double*>(data);
			const char* text = "";
			vtkAlgorithm* algorithm = vtkAlgorithm::SafeDownCast(object);
			if (algorithm)
			{
				text = algorithm->GetProgressText();
			}
			if (text)
			{
				std::cout << text << ": ";
			}
			std::cout << static_cast<int>(100.0 * progress + 0.5) << std::endl;
		}
	}
}

int main()
{

	auto r = vtkDICOMReader::New();
	r->SetFileName("D:/demo/demo2/dicom/IMG-0002-00001.dcm");
	r->Update();

	auto d = vtkDICOMDirectory::New();
	d->SetDirectoryName("D:/demo/demo2/dicom");
	d->Update();
	//auto a = d->GetNumberOfSeries();
	auto n = d->GetFileNamesForSeries(0);
	//for (vtkIdType i = 0; i < n->GetNumberOfValues(); i++) {
	//	std::cout << n->GetValue(i) << std::endl;
	//}

	r->SetFileNames(n);
	r->UpdateInformation();
	
	auto m = r->GetMetaData();
	std::cout << m->Get(DC::PatientAge).AsString() << std::endl;
	p(m->Get(DC::StudyDate));		// y
	p(m->Get(DC::InstitutionName));	// y
	p(m->Get(DC::PatientName));
	p(m->Get(DC::PatientSex));		// y

	p(d->GetNumberOfStudies());
	auto s = d->GetPatientRecordForStudy(0);
	p(s.Get(DC::PatientName));		// y
	p(s.Get(DC::PatientAge));
	p(s.Get(DC::PatientSex));
	p(s.Get(DC::InstitutionName));	// n
	auto stu = d->GetStudyRecord(0);
	p(stu.Get(DC::PatientName));	// n
	p(stu.Get(DC::StudyDate));		// y
	p(stu.Get(DC::PatientSex));		// n
	p(stu.Get(DC::InstitutionName));// n

	vtkStringArray* a = d->GetFileNamesForSeries(0);
	vtkSmartPointer<ReaderProgress> progressCommand =
		vtkSmartPointer<ReaderProgress>::New();
	vtkSmartPointer<vtkDICOMReader> reader =
		vtkSmartPointer<vtkDICOMReader>::New();
	reader->SetMemoryRowOrderToFileNative();	// Ë³Ðò¶ÁÈ¡

	reader->AddObserver(vtkCommand::ProgressEvent, progressCommand);
	reader->SetFileNames(a);
	reader->Update();

	return EXIT_SUCCESS;
}
