#include <ObjectViewer.h>	


#include <vtkActor.h>			//VTK/Qt usage
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>

#include <vtkSTLReader.h>
#include <vtkCubeAxesActor.h>


//----------- TESTED HEADERS ----------
#include <vtkLightKit.h>


#include <yaml-cpp\yaml.h>
#include <assert.h>
#include <vector>
#include <vtkOutlineFilter.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkCubeSource.h>
#include <vtkTransformPolyDataFilter.h>
#include <qtreewidget.h>

#include <vtkExtractEdges.h>

//#include <vtkInteractorStyleTrackballCamera.h>
//#include <vtkInteractorStyleTrackball.h>
#include <vtkProperty.h>
//#include <vtkPropPicker.h>



//Constructor of our main window.
ObjectViewer::ObjectViewer()
{
	//shutdown the VTK Debug window.
	vtkObject::GlobalWarningDisplayOff();

	//sets up all Qt objects (see ui_MainWindow.h)
	this->setupUi(this);

	//create a vtkRenderWindow, that we want to assign to the QVTKViewer
	vtkRenderWindow* renwin = vtkRenderWindow::New();			
	VTKViewer->SetRenderWindow(renwin);
	renwin->Delete();
	
	//add a renderer
	MainRenderer = vtkRenderer::New();
	VTKViewer->GetRenderWindow()->AddRenderer(MainRenderer);
	

	// Example for making an explicit light source that DOESN'T move with the camera/observer (like in reality)
	
	MainRenderer->SetAutomaticLightCreation(0);
	vtkSmartPointer<vtkLightKit> light =
		vtkSmartPointer<vtkLightKit>::New();
	light->AddLightsToRenderer(MainRenderer);
	light->Update();
	


	//creating a OrientationMarkerWidget
	vtkAxesActor* axes = vtkAxesActor::New();
	vtkOrientationMarkerWidget* widget = vtkOrientationMarkerWidget::New();
	widget->SetOutlineColor(0.9300, 0.5700, 0.1300);
	widget->SetOrientationMarker(axes);
	widget->SetInteractor(VTKViewer->GetRenderWindow()->GetInteractor());
	widget->SetViewport(0.0, 0.0, 0.12, 0.12);
	widget->SetEnabled(1);
	widget->InteractiveOn();


	QTreeWidgetItem* test_item = new QTreeWidgetItem(treeWidget);
	test_item->setText(0, QString("Object1"));

	QTreeWidgetItem* test_property1 = new QTreeWidgetItem(test_item);
	test_property1->setText(0, QString("Transform Data"));

	QTreeWidgetItem* test_property2 = new QTreeWidgetItem(test_property1);
	test_property2->setText(0, QString("Location"));
	test_property2->setText(1, QString("Float"));
	test_property2->setText(2, QString("x= 14.52  y= 7.89  z= 57.11"));

	QTreeWidgetItem* test_property3 = new QTreeWidgetItem(test_property1);
	test_property3->setText(0, QString("Rotation"));
	test_property3->setText(1, QString("Float"));
	test_property3->setText(2, QString("x= 78.12  y= 53.22  z= 4.32"));


	QTreeWidgetItem* test_property4 = new QTreeWidgetItem(test_item);
	test_property4->setText(0, QString("Geometrical Data"));

	QTreeWidgetItem* test_property5 = new QTreeWidgetItem(test_property4);
	test_property5->setText(0, QString("STL"));
	test_property5->setText(1, QString("File Path"));
	test_property5->setText(2, QString("C:\\User\\..."));
	test_property5->setCheckState(3, Qt::Unchecked);
	

	QTreeWidgetItem* test_property6 = new QTreeWidgetItem(test_property4);
	test_property6->setText(0, QString("PCD"));
	test_property6->setText(1, QString("File Path"));
	test_property6->setText(2, QString("C:\\User\\..."));
	test_property6->setCheckState(3, Qt::Checked);

	QTreeWidgetItem* test_property7 = new QTreeWidgetItem(test_property4);
	test_property7->setText(0, QString("Bounding Box"));
	test_property7->setText(1, QString("-"));
	test_property7->setCheckState(3, Qt::Checked);

	treeWidget->header()->resizeSection(3, 60);

	vtkSmartPointer<vtkSTLReader> reader =
		vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName("C:\\Users\\Stefan\\Desktop\\low_poly_fawn_bin.stl");
	reader->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(reader->GetOutputPort());

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	MainRenderer->AddActor(actor);

	vtkSmartPointer<vtkCubeAxesActor> cubeAxesActor =
		vtkSmartPointer<vtkCubeAxesActor>::New();
	cubeAxesActor->SetBounds(actor->GetBounds());
	cubeAxesActor->SetCamera(MainRenderer->GetActiveCamera());
	

	cubeAxesActor->DrawXGridlinesOn();
	cubeAxesActor->DrawYGridlinesOn();
	cubeAxesActor->DrawZGridlinesOn();

	cubeAxesActor->SetGridLineLocation(
		cubeAxesActor->VTK_GRID_LINES_FURTHEST);

	cubeAxesActor->XAxisMinorTickVisibilityOff();
	cubeAxesActor->YAxisMinorTickVisibilityOff();
	cubeAxesActor->ZAxisMinorTickVisibilityOff();

	MainRenderer->AddActor(cubeAxesActor);




	//--------------------------- CONNECTIONS -----------------------------------------

	//connection from (button) QAction* Open_File to the SLOT with function openFile(), when triggered
	//connect(actionOpen_File, SIGNAL(triggered()), this, SLOT(openFile_MainWindow()));

}
ObjectViewer::~ObjectViewer()
{
	//make sure to delete everything to avoid leaks!
	MainRenderer->Delete();
}


