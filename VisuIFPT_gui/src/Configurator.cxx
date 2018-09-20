#include <Configurator.h>

#include <HelpClasses.h>			//own headers
#include <include/Reader.h>

#include <vtkRenderWindow.h>		//VTK usage
#include <vtkRenderer.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkPlaneSource.h>
#include <vtkCubeSource.h>
#include <vtkSphereSource.h>
#include <qfiledialog.h>
#include <QCloseEvent>			
#include <QMessageBox>

//------------- TESTED HEADERS ---------------
#include <vtkCommand.h>


//Initialize this bool with false, since we have no instance.
bool Configurator::open_instance = false;


//Constructor of the Configurator main window.
Configurator::Configurator()
{

	//when we create an instance, we want to trigger the open_instance bool to true!
	open_instance = true;
	
	//sets up all qt objects (see ui_MainWindow.h)
	this->setupUi(this);


	//our new actor/assembly, that we are assembling in this configurator and that we will export
	new_actor = vtkAssembly::New();
	
	//create renderwindow and assign it to the QVTKViewer
	vtkRenderWindow* renwin = vtkRenderWindow::New();				
	Actor_Viewer->SetRenderWindow(renwin);
	renwin->Delete();

	//add a renderer
	configuratorRen = vtkRenderer::New();
	Actor_Viewer->GetRenderWindow()->AddRenderer(configuratorRen);

	configurator_ActorCounter = new ActorCounter;

	//add an InteractionMode (derivitive from InteractionStyleSwitch) and set default to trackball_camera
	style = style->New();
	style->SetCurrentStyleToTrackballCamera();
	Actor_Viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);


	//initialize interactor and add callback-object to update the viewer periodically
	Actor_Viewer->GetRenderWindow()->GetInteractor()->Initialize();

	//commented because of bugs
	/*
	vtkSmartPointer<vtkTimerCallback> cb =
		vtkSmartPointer<vtkTimerCallback>::New();
	Actor_Viewer->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::TimerEvent, cb);

	Actor_Viewer->GetRenderWindow()->GetInteractor()->CreateRepeatingTimer(100);
	*/


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

	//Process click on QAction of Geo. Primitive menu 
	connect(menuGeometric_Primitives, SIGNAL(triggered(QAction*)), this, SLOT(spawnPrimitive(QAction*)));

	//Export actor to main window
	connect(actionExport_to_Main_Window, SIGNAL(triggered()), this, SLOT(exportActor()));
}


Configurator::~Configurator()
{
	//make sure to delete everything!
	configuratorRen->Delete();
}


// TODO: muss wirklich der header QTreeWidget.h benutzt werden, damit QTreewidgetitem bekannt wird?
//Slot for displaying transform-data
void Configurator::displayTransformData(QTreeWidgetItem* item, int) {

	//see equivalent comments in MainWindow.cxx
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

	spawnGeoPrimitives(primitive, configuratorRen, NULL, configurator_ActorCounter);

	Actor_Viewer->update();
}


//Slot function that adds the new_actor from the configurator to the renderer
void Configurator::exportActor() {

	//add the assembly to the main renderer from our main window and increment the new_actorCount
	mainwindow->Ren1->AddActor(new_actor);
	mainwindow->mainWindow_ActorCounter->assemblyCount;

	//create a new item in the actors-list from the main window
	Q_actorTreeWidgetItem* new_item = new Q_actorTreeWidgetItem(mainwindow->treeWidget, new_actor, 1);

	new_item->setText(0, QString::fromStdString("ConfigItem" + std::to_string(mainwindow->mainWindow_ActorCounter->assemblyCount)));

	close();
}


//We need to process the closeEvent of the Configurator, so we can safely trigger the bool open_instance to false
void Configurator::closeEvent(QCloseEvent *event)
{
		open_instance = false;
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
void MainWindow::openFile() {

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
void MainWindow::spawnPrimitive(QAction* primitive) {					// TODO: maybe even outsource the vtkPolyData (which we create in every if-case equally)?

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
		MainWindow::pri_planeCount++;
		item_name = "Plane" + std::to_string(MainWindow::pri_planeCount);

	}
	else if (primitive->text() == "Cube") {

		//same procedure as above
		vtkSmartPointer<vtkCubeSource> cubeSource =
			vtkSmartPointer<vtkCubeSource>::New();
		cubeSource->SetCenter(0.0, 0.0, 0.0);
		cubeSource->Update();

		vtkPolyData* cube = cubeSource->GetOutput();
		polymapper->SetInputData(cube);

		MainWindow::pri_cubeCount++;
		item_name = "Cube" + std::to_string(MainWindow::pri_cubeCount);
	
	}
	else if (primitive->text() == "Sphere") {

		vtkSmartPointer<vtkSphereSource> sphereSource =
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetCenter(0.0, 0.0, 0.0);
		sphereSource->Update();

		vtkPolyData* sphere = sphereSource->GetOutput();
		polymapper->SetInputData(sphere);

		MainWindow::pri_sphereCount++;
		item_name = "Sphere" + std::to_string(MainWindow::pri_sphereCount);
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

