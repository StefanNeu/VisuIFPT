#include "GUI.h"

#include <QMenu>

#include "vtkActor.h"
#include "vtkCommand.h"
#include "vtkConeSource.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"

#include <istream>
#include <qfiledialog.h>
#include "Reader.h"
#include <vtkPLYReader.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPlaneSource.h>
#include <vtkCubeSource.h>
#include <vtkShrinkFilter.h>
#include <vtkNamedColors.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleSwitch.h>
#include <qtreewidget.h>


#include "QVTKInteractor.h"


//initialize static counters for geometric primitives (just for naming purposes)
int GUI::pri_planeCount = 0;
int GUI::pri_cubeCount = 0;


//---------------------------- SOME DERIVED CLASSES WE NEED-----------------------------------------------

//Simple constructor for derived class we need
vtk_InteractorMode* vtk_InteractorMode::New() {		
	return new vtk_InteractorMode();			
}

//Put in two strings and function puts out current interactorstyles
void vtk_InteractorMode::getMode(std::string &cam_or_ac, std::string &joy_or_track) {			
	if (JoystickOrTrackball == 0) {						//we access the protected variables of the base class
		joy_or_track = "Joystick";					
	}
	else {
		joy_or_track = "Trackball";
	}

	if (CameraOrActor == 0) {
		cam_or_ac = "Camera";
	}
	else {
		cam_or_ac = "Actor";
	}
}







GUI::GUI()
{
	this->setupUi(this);


	// create a window to make it stereo capable and give it to QVTKWidget
	vtkRenderWindow* renwin = vtkRenderWindow::New();

	VTKViewer->SetRenderWindow(renwin);
	renwin->Delete();

	//add a renderer
	Ren1 = vtkRenderer::New();
	VTKViewer->GetRenderWindow()->AddRenderer(Ren1);

	//add an InteractionMode (derivitive from InteractionStyleSwitch) and set default to trackball_camera
	style = style->New();
	style->SetCurrentStyleToTrackballCamera();
	VTKViewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
	
	//creating a OrientationMarkerWidget
	vtkAxesActor* axes = vtkAxesActor::New();
	vtkOrientationMarkerWidget* widget = vtkOrientationMarkerWidget::New();
	widget->SetOutlineColor(0.9300, 0.5700, 0.1300);
	widget->SetOrientationMarker(axes);
	widget->SetInteractor(VTKViewer->GetRenderWindow()->GetInteractor());
	widget->SetViewport(0.0, 0.0, 0.12, 0.12);
	widget->SetEnabled(1);
	widget->InteractiveOn();

	
	//--------------------------- CONNECTIONS -----------------------------------------

	//connection from (button) QAction* Open_File to the SLOT with function openFile(), when triggered
	connect(actionOpen_File, SIGNAL(triggered()), this, SLOT(openFile()));

	//same connection as above, but we process the press on the menu and the following press on the QAction* in the SLOT-function
	connect(menuGeometric_Primitives, SIGNAL(triggered(QAction*)), this, SLOT(spawnPrimitive(QAction*)));

	//connection for context menu in our actors-list
	connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(prepareMenu(const QPoint&)));

	connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *)), this, SLOT(displayTransformData(QTreeWidgetItem*)));

	Connections = vtkEventQtSlotConnect::New();

	// update coords as we move through the window
	Connections->Connect(VTKViewer->GetRenderWindow()->GetInteractor(),
		vtkCommand::MouseMoveEvent,
		this,
		SLOT(updateCoords(vtkObject*)));

	Connections->PrintSelf(cout, vtkIndent());
}

GUI::~GUI()
{
	Ren1->Delete();
	Connections->Delete();
}

void GUI::displayTransformData(QTreeWidgetItem* item) {


	Q_actorTreeWidgetItem* actor_item = dynamic_cast<Q_actorTreeWidgetItem*>(item);
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor = actor_item->getActorReference();
	double* position = new double[3];
	position = actor->GetOrigin();

	x_loc->setText(QString(std::to_string( position[0]).c_str() ));
}

//test comment
void GUI::updateCoords(vtkObject* obj)
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

void GUI::popup(vtkObject * obj, unsigned long,
	void * client_data, void *,
	vtkCommand * command)
{
	// A note about context menus in Qt and the QVTKWidget
	// You may find it easy to just do context menus on right button up,
	// due to the event proxy mechanism in place.

	// That usually works, except in some cases.
	// One case is where you capture context menu events that
	// child windows don't process.  You could end up with a second
	// context menu after the first one.

	// See QVTKWidget::ContextMenuEvent enum which was added after the
	// writing of this example.

	// get interactor
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	// consume event so the interactor style doesn't get it
	command->AbortFlagOn();
	// get popup menu
	QMenu* popupMenu = static_cast<QMenu*>(client_data);
	// get event location
	int* sz = iren->GetSize();
	int* position = iren->GetEventPosition();
	// remember to flip y
	QPoint pt = QPoint(position[0], sz[1] - position[1]);
	// map to global
	QPoint global_pt = popupMenu->parentWidget()->mapToGlobal(pt);
	// show popup menu at global point
	popupMenu->popup(global_pt);

}

void GUI::openFile() {

	QString q_filename = QFileDialog::getOpenFileName(this, tr("Open file"), "C:/", tr("3D Files(*.ply *.stl *.pcd)"));
	std::string filename = q_filename.toStdString();

	polymapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	
	if (filename.find(".ply") != std::string::npos) {						//we test if we find the file-extension ".ply" or ".stl"
		
		/*std::cout << "Do you want to read this file as standard .ply or as pointcloud?" << std::endl;
		std::cout << "Type \"s\" for standard mode and \"p\" for pointcloud mode: ";
		
		if (mode == 's') {								//standard mode-> open PLY with internal reader
			readPLY_s(data_mapper, filename);
		}
		else if (mode == 'p') {							//pointcloud mode-> open PLY with our reader
			readPLY_p(data_mapper, filename);
		}*/

		readPLY_p(polymapper, filename);

	}
	else if (filename.find(".stl") != std::string::npos) {
		readSTL(polymapper, filename);
	}
	else if (filename.find(".pcd") != std::string::npos) {
		readPCD(polymapper, filename);
	}

	char* file = new char[50];
	char* ext = new char[10];
	_splitpath(filename.c_str(), NULL, NULL, file, ext);
	std::string s_file = file;
	std::string s_ext = ext;

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(polymapper);
	Ren1->AddViewProp(actor);

	Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(treeWidget, actor, 1);

	new_actor->setText(0, QString::fromStdString(s_file + s_ext));

	VTKViewer->update();
}

void GUI::spawnPrimitive(QAction* primitive) {					// TODO: maybe even outsource the vtkPolyData (which we create in every if-case equally)?

	polymapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	std::string item_name;

	if (primitive->text() == "Plane") {

		vtkSmartPointer<vtkPlaneSource> planeSource =
			vtkSmartPointer<vtkPlaneSource>::New();
		planeSource->SetCenter(0.0, 0.0, 0.0);
		planeSource->SetNormal(1.0, 0.0, 1.0);
		planeSource->Update();

		vtkPolyData* plane = planeSource->GetOutput();
		polymapper->SetInputData(plane);
		GUI::pri_planeCount++;

		item_name = "Plane" + std::to_string(GUI::pri_planeCount);
		//new_actor->setText(0, QString::fromStdString("Plane" + std::to_string(GUI::pri_planeCount)));

	}
	else if (primitive->text() == "Cube") {

		vtkSmartPointer<vtkCubeSource> cubeSource =
			vtkSmartPointer<vtkCubeSource>::New();
		cubeSource->SetCenter(0.0, 0.0, 0.0);
		cubeSource->Update();

		vtkPolyData* cube = cubeSource->GetOutput();
		polymapper->SetInputData(cube);
		GUI::pri_cubeCount++;

		item_name = "Cube" + std::to_string(GUI::pri_planeCount);
		//new_actor->setText(0, QString::fromStdString("Cube" + std::to_string(GUI::pri_cubeCount)));
	}

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(polymapper);
	Ren1->AddViewProp(actor);

	Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(treeWidget, actor, 1);
	new_actor->setText(0, QString::fromStdString(item_name));

	VTKViewer->update();
	
}

void GUI::renameActor() {
	actorlist_contextmenu_item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
	treeWidget->editItem(actorlist_contextmenu_item, 0);
}

void GUI::deleteActor() {
	
	Ren1->RemoveActor(actorlist_contextmenu_item->getActorReference());
	delete actorlist_contextmenu_item;

	VTKViewer->update();
}

void GUI::prepareMenu(const QPoint & pos)				
{
	vtkSmartPointer<vtkActorCollection> actors =
		vtkSmartPointer<vtkActorCollection>::New();
	actors = Ren1->GetActors();
	actors->PrintSelf(cout, vtkIndent());

	if (treeWidget->itemAt(pos) != NULL) {

		actorlist_contextmenu_item = dynamic_cast<Q_actorTreeWidgetItem*>(treeWidget->itemAt(pos));


		QAction *renameAct = new QAction(QString("Rename"), this);
		QAction *deleteAct = new QAction(QString("Communism works and I can prove it"), this);

		connect(renameAct, SIGNAL(triggered()), this, SLOT(renameActor()));
		connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteActor()));

		QMenu menu(this);
		menu.addAction(renameAct);
		menu.addAction(deleteAct);

		menu.exec(treeWidget->mapToGlobal(pos));
	}
}