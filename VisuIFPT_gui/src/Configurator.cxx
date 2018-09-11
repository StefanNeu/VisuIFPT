#include "Configurator.h"

#include <QMenu>
#include <QVTKInteractor.h>

#include "vtkActor.h"
#include "vtkCommand.h"

#include "vtkEventQtSlotConnect.h"
#include "vtkPolyDataMapper.h"
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include <istream>
#include <qfiledialog.h>
#include "Reader.h"
#include <vtkPLYReader.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPlaneSource.h>
#include <vtkCubeSource.h>
#include <vtkSphereSource.h>
#include <vtkShrinkFilter.h>
#include <vtkNamedColors.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleSwitch.h>
#include <qtreewidget.h>
#include <vtkTransform.h>
#include <vtkCommand.h>
#include <sstream>
#include <iomanip> 
#include "HelpClasses.h"
#include <qtreewidget.h>
#include <QCloseEvent>
#include <QMessageBox>


bool Configurator::open_instance = false;

//#Constructor of our main window.
Configurator::Configurator()
{

	
	open_instance = true;
	

	//sets up all qt objects (see ui_GUI.h)
	this->setupUi(this);


	// create a window to make it stereo capable and give it to QVTKWidget
	vtkRenderWindow* renwin = vtkRenderWindow::New();				//very bad anti-aliasing with opengl instead of "default" render window!

	Actor_Viewer->SetRenderWindow(renwin);
	renwin->Delete();

	//add a renderer
	Actor_Renderer = vtkRenderer::New();
	Actor_Viewer->GetRenderWindow()->AddRenderer(Actor_Renderer);

	//add an InteractionMode (derivitive from InteractionStyleSwitch) and set default to trackball_camera
	style = style->New();
	style->SetCurrentStyleToTrackballCamera();
	Actor_Viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);


	//initialize interactor and add callback-object to update the viewer periodically
	Actor_Viewer->GetRenderWindow()->GetInteractor()->Initialize();

	vtkSmartPointer<vtkTimerCallback> cb =
		vtkSmartPointer<vtkTimerCallback>::New();
	Actor_Viewer->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::TimerEvent, cb);

	Actor_Viewer->GetRenderWindow()->GetInteractor()->CreateRepeatingTimer(100);
	


	//creating a OrientationMarkerWidget
	vtkAxesActor* axes = vtkAxesActor::New();
	vtkOrientationMarkerWidget* widget = vtkOrientationMarkerWidget::New();
	widget->SetOutlineColor(0.9300, 0.5700, 0.1300);
	widget->SetOrientationMarker(axes);
	widget->SetInteractor(Actor_Viewer->GetRenderWindow()->GetInteractor());
	widget->SetViewport(0.0, 0.0, 0.12, 0.12);
	widget->SetEnabled(1);
	widget->InteractiveOn();

	//---------------------- CONNECTIONS -------------------------------------

	connect(menuGeometric_Primitives, SIGNAL(triggered(QAction*)), this, SLOT(spawnPrimitive(QAction*)));


}

Configurator::~Configurator()
{
	Actor_Renderer->Delete();
	
}

// TODO: muss wirklich der header QTreeWidget.h benutzt werden, damit QTreewidgetitem bekannt wird?
//#Slot for transform-data.
void Configurator::displayTransformData(QTreeWidgetItem* item, int) {

	Q_actorTreeWidgetItem* actor_item = dynamic_cast<Q_actorTreeWidgetItem*>(item);
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor = actor_item->getActorReference();


	//get position of actor and store it in the corresponding QLineEdit of the inspector
	double* position = new double[3];
	position = actor->GetPosition();
	
	std::string x_string = std::to_string(position[0]);
	x_string = x_string.substr(0, x_string.size() - 4);

	std::string y_string = std::to_string(position[1]);
	y_string = y_string.substr(0, y_string.size() - 4);

	std::string z_string = std::to_string(position[2]);
	z_string = z_string.substr(0, z_string.size() - 4);


	x_loc->setText(QString(x_string.c_str()));
	y_loc->setText(QString(y_string.c_str()));
	z_loc->setText(QString(z_string.c_str()));
}


// TODO: Actorliste einfügen?
void Configurator::spawnPrimitive(QAction* primitive) {

	//the vtkPolyDataMapper that we fill with 
	vtkSmartPointer<vtkPolyDataMapper> polymapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	//std::string item_name;


	//check what primitive we want to create
	if (primitive->text() == "Plane") {

		//create vtkPlaneSource, set a few parameter, update it and 
		//then connect the vtkPolyDataMapper with it
		vtkSmartPointer<vtkPlaneSource> planeSource =
			vtkSmartPointer<vtkPlaneSource>::New();
		planeSource->SetCenter(0.0, 0.0, 0.0);
		planeSource->SetNormal(1.0, 0.0, 1.0);
		planeSource->Update();

		vtkPolyData* plane = planeSource->GetOutput();
		polymapper->SetInputData(plane);

		//we want to give it a default name und numbering
		//GUI::pri_planeCount++;
		//item_name = "Plane" + std::to_string(GUI::pri_planeCount);

	}
	else if (primitive->text() == "Cube") {

		//same procedure as above
		vtkSmartPointer<vtkCubeSource> cubeSource =
			vtkSmartPointer<vtkCubeSource>::New();
		cubeSource->SetCenter(0.0, 0.0, 0.0);
		cubeSource->Update();

		vtkPolyData* cube = cubeSource->GetOutput();
		polymapper->SetInputData(cube);

		//GUI::pri_cubeCount++;
		//item_name = "Cube" + std::to_string(GUI::pri_cubeCount);

	}
	else if (primitive->text() == "Sphere") {

		vtkSmartPointer<vtkSphereSource> sphereSource =
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetCenter(0.0, 0.0, 0.0);
		sphereSource->Update();

		vtkPolyData* sphere = sphereSource->GetOutput();
		polymapper->SetInputData(sphere);

		//GUI::pri_sphereCount++;
		//item_name = "Sphere" + std::to_string(GUI::pri_sphereCount);
	}

	//create a vtkActor, connect with the vtkPolyDataMapper and add to Ren1
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();

	actor->SetMapper(polymapper);
	
	Actor_Renderer->AddActor(actor);

	//create a new item for our actors-list
	//Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(treeWidget, actor, 1);
	//new_actor->setText(0, QString::fromStdString(item_name));

	Actor_Viewer->update();

}

void Configurator::closeEvent(QCloseEvent *event)
{
	QMessageBox::StandardButton resBtn = QMessageBox::question(this, QString("Configurator"),
		tr("Are you sure?\n"),
		QMessageBox::No | QMessageBox::Yes,
		QMessageBox::Yes);
	if (resBtn != QMessageBox::Yes) {
		event->ignore();
		
	}
	else {
		event->accept();
		open_instance = false;
	}
}

// TODO: Kameramodus updaten
//#Slot for updating mouse-coordinates.
/*void Configurator::updateCoords(vtkObject* obj)
{
	// get interactor
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	// get event position
	int event_pos[2];
	iren->GetEventPosition(event_pos);

	std::string ac_or_cam, joy_or_tra;
	style->getMode(ac_or_cam, joy_or_tra);

	// update label
	QString str;
	str.sprintf("Mode:  %s     %s                      x=%d : y=%d", ac_or_cam.c_str(), joy_or_tra.c_str(), event_pos[0], event_pos[1]);
	coord->setText(str);
}
*/

// TODO: Option einbauen, um Primitive oder Files als "Bausteine" für einen Actor einzufügen
/*
//#Slot for opening 3D-files.
void GUI::openFile() {

	//open QFileDialog to open file in the windows-explorer
	QString q_filename = QFileDialog::getOpenFileName(this, tr("Open file"), "C:/", tr("3D Files(*.ply *.stl *.pcd)"));
	std::string filename = q_filename.toStdString();

	polymapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	
	//we test if we find the file-extension ply/stl or pcd
	if (filename.find(".ply") != std::string::npos) {	
		
		//openPLY_dialog* dialog = new openPLY_dialog();
		//dialog->exec();

		readPLY_p(polymapper, filename);

	}
	else if (filename.find(".stl") != std::string::npos) {
		readSTL(polymapper, filename);
	}
	else if (filename.find(".pcd") != std::string::npos) {
		readPCD(polymapper, filename);
	}

	//create c_string and use _splitpath_s() to fill them with the filename-data
	char* file = new char[50];
	char* ext = new char[10];
	_splitpath_s(filename.c_str(), NULL, 0, NULL, 0, file, 30, ext, 30);
	std::string s_file = file;
	std::string s_ext = ext;

	//create actor, connect to polymapper and add to renderer
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(polymapper);
	Ren1->AddViewProp(actor);

	Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(treeWidget, actor, 1);

	new_actor->setText(0, QString::fromStdString(s_file + s_ext)); //TODO: numbering when you open same file more than one time

	VTKViewer->update();
}

//#Slot for spawning geometrical primitives.
void GUI::spawnPrimitive(QAction* primitive) {					// TODO: maybe even outsource the vtkPolyData (which we create in every if-case equally)?

	//the vtkPolyDataMapper that we fill with 
	polymapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	std::string item_name;

	//check what primitive we want to create
	if (primitive->text() == "Plane") {

		//create vtkPlaneSource, set a few parameter, update it and 
		//then connect the vtkPolyDataMapper with it
		vtkSmartPointer<vtkPlaneSource> planeSource =
			vtkSmartPointer<vtkPlaneSource>::New();
		planeSource->SetCenter(0.0, 0.0, 0.0);
		planeSource->SetNormal(1.0, 0.0, 1.0);
		planeSource->Update();

		vtkPolyData* plane = planeSource->GetOutput();
		polymapper->SetInputData(plane);

		//we want to give it a default name und numbering
		GUI::pri_planeCount++;
		item_name = "Plane" + std::to_string(GUI::pri_planeCount);

	}
	else if (primitive->text() == "Cube") {

		//same procedure as above
		vtkSmartPointer<vtkCubeSource> cubeSource =
			vtkSmartPointer<vtkCubeSource>::New();
		cubeSource->SetCenter(0.0, 0.0, 0.0);
		cubeSource->Update();

		vtkPolyData* cube = cubeSource->GetOutput();
		polymapper->SetInputData(cube);

		GUI::pri_cubeCount++;
		item_name = "Cube" + std::to_string(GUI::pri_cubeCount);
	
	}
	else if (primitive->text() == "Sphere") {

		vtkSmartPointer<vtkSphereSource> sphereSource =
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetCenter(0.0, 0.0, 0.0);
		sphereSource->Update();

		vtkPolyData* sphere = sphereSource->GetOutput();
		polymapper->SetInputData(sphere);

		GUI::pri_sphereCount++;
		item_name = "Sphere" + std::to_string(GUI::pri_sphereCount);
	}

	//create a vtkActor, connect with the vtkPolyDataMapper and add to Ren1
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	
	actor->SetMapper(polymapper);
	Ren1->AddViewProp(actor);

	//create a new item for our actors-list
	Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(treeWidget, actor, 1);
	new_actor->setText(0, QString::fromStdString(item_name));

	VTKViewer->update();
	
}
*/

