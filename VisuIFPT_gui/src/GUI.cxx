#include <GUI.h>	

#include <HelpClasses.h>
#include <include/Reader.h>				//for reading different types of files
#include <Configurator.h>		//for creating a window of type Configurator

#include <vtkActor.h>			//VTK/Qt usage
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPlaneSource.h>
#include <vtkCubeSource.h>
#include <vtkSphereSource.h>
#include <vtkCommand.h>
#include <vtkAssembly.h>
#include <qfiledialog.h>
#include <QCloseEvent>			
#include <QMessageBox>

//----------- TESTED HEADERS ----------
#include <vtkLightKit.h>
#include <vtkLight.h>
#include <vtkLightCollection.h>


//Initialize static counters for items from the actors-list (just for naming purposes)
int GUI::pri_planeCount = 0;
int GUI::pri_cubeCount = 0;
int GUI::pri_sphereCount = 0;
int GUI::new_actorCount = 0;


//#Constructor of our main window.
GUI::GUI()
{
	//shutdown the VTK Debug window.
	vtkObject::GlobalWarningDisplayOff();

	//sets up all Qt objects (see ui_GUI.h)
	this->setupUi(this);

	//create a vtkRenderWindow, that we want to assign to the QVTKViewer
	vtkRenderWindow* renwin = vtkRenderWindow::New();			
	VTKViewer->SetRenderWindow(renwin);
	renwin->Delete();
	
	//add a renderer
	Ren1 = vtkRenderer::New();
	VTKViewer->GetRenderWindow()->AddRenderer(Ren1);


	// Example for making an explicit light source that DOESN'T move with the camera/observer (like in reality)
	/* 
	Ren1->SetAutomaticLightCreation(0);
	
	vtkSmartPointer<vtkLight> lightKit =
		vtkSmartPointer<vtkLight>::New();
	lightKit->SetPosition(0.0, 2.0, 0.0);
	lightKit->SetLightTypeToSceneLight();
	lightKit->SetFocalPoint(0.0, 0.0, 0.0);
	lightKit->SetAmbientColor(0.0, 1.0, 0.0);
	Ren1->AddLight(lightKit);
	*/
	

	//add an InteractionMode (derivitive from InteractionStyleSwitch) and set default to trackball_camera
	style = style->New();
	style->SetCurrentStyleToTrackballCamera();
	VTKViewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);


	//initialize interactor and add callback-object to update the viewer periodically
	VTKViewer->GetRenderWindow()->GetInteractor()->Initialize();


	// Periodically updating the Interactor, but commented because of bugs
	/*
	vtkSmartPointer<vtkTimerCallback> cb =
		vtkSmartPointer<vtkTimerCallback>::New();
	VTKViewer->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::TimerEvent, cb);

	VTKViewer->GetRenderWindow()->GetInteractor()->CreateRepeatingTimer(100);
	*/


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

	//updating the transform-data in the inspector, when item in actors-list is clicked
	connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(displayTransformData(QTreeWidgetItem*, int)));

	//open the Configurator-window
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(openConfigurator()));
	

	//this class is needed to manage Qt and VTK connections
	Connections = vtkEventQtSlotConnect::New();

	// update coords as we move through the window
	Connections->Connect(VTKViewer->GetRenderWindow()->GetInteractor(),
		vtkCommand::MouseMoveEvent,
		this,
		SLOT(updateCoords(vtkObject*)));

}

GUI::~GUI()
{
	//make sure to delete everything to avoid leaks!
	Ren1->Delete();
	Connections->Delete();
	polymapper->Delete();
	style->Delete();

	// TODO: test if we have data leaks.
	delete actorlist_contextmenu_item;
}

//Slot for transform-data.
void GUI::displayTransformData(QTreeWidgetItem* item, int) {

	double* position = new double[3];
	double* rotation = new double[3];
	double* scale = new double[3];

	//we cast the QTreeWidgetItem into the derived class, so we can use a few extra functions
	Q_actorTreeWidgetItem* actor_item = dynamic_cast<Q_actorTreeWidgetItem*>(item);

	//we need to find out, if we clicked on an item with an actor or assembly-reference
	//(assemblies come from the configurator)
	if (actor_item->getActorReference() == NULL) {

		position = actor_item->getAssemblyReference()->GetPosition();
		rotation = actor_item->getAssemblyReference()->GetOrientation();

	}
	else if (actor_item->getAssemblyReference() == NULL) {

		position = actor_item->getActorReference()->GetPosition();
		rotation = actor_item->getActorReference()->GetOrientation();
	}
	

	//------------------------ POSITION ------------------------------
	//we need to cut off a little bit, off the too long double string
	std::string x_stringPOS = std::to_string(position[0]);
	x_stringPOS = x_stringPOS.substr(0, x_stringPOS.size() - 4);

	std::string y_stringPOS = std::to_string(position[1]);
	y_stringPOS = y_stringPOS.substr(0, y_stringPOS.size() - 4);

	std::string z_stringPOS = std::to_string(position[2]);
	z_stringPOS = z_stringPOS.substr(0, z_stringPOS.size() - 4);

	//and set the text of the QLineEdit-objects
	x_loc->setText(QString(x_stringPOS.c_str()));
	y_loc->setText(QString(y_stringPOS.c_str()));
	z_loc->setText(QString(z_stringPOS.c_str()));


	//---------------------- ROTATION ----------------------------------
	//same procedure as above
	std::string x_stringROT = std::to_string(rotation[0]);
	x_stringROT = x_stringROT.substr(0, x_stringROT.size() - 4);

	std::string y_stringROT = std::to_string(rotation[1]);
	y_stringROT = y_stringROT.substr(0, y_stringROT.size() - 4);

	std::string z_stringROT = std::to_string(rotation[2]);
	z_stringROT = z_stringROT.substr(0, z_stringROT.size() - 4);

	x_rot->setText(QString(x_stringROT.c_str()));
	y_rot->setText(QString(y_stringROT.c_str()));
	z_rot->setText(QString(z_stringROT.c_str()));
}

//#Slot for updating mouse-coordinates.
void GUI::updateCoords(vtkObject* obj)
{
	//get interactor
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	//get event position
	int event_pos[2];
	iren->GetEventPosition(event_pos);

	std::string ac_or_cam, joy_or_tra;
	//we want to get the mode of our WindowInteractor
	style->getMode(ac_or_cam, joy_or_tra);

	//update label
	QString str;
	str.sprintf("Mode:  %s     %s                      x=%d : y=%d", ac_or_cam.c_str(), joy_or_tra.c_str(), event_pos[0], event_pos[1]);
	coord->setText(str);
}

//Slot for opening 3D-files.
void GUI::openFile() {

	//open QFileDialog to open file in the windows-explorer
	QString q_filename = QFileDialog::getOpenFileName(this, tr("Open file"), "C:/", tr("3D Files(*.ply *.stl *.pcd)"));
	std::string filename = q_filename.toStdString();

	polymapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	
	//we test if we find the file-extension ply/stl or pcd
	if (filename.find(".ply") != std::string::npos) {	
		
		//openPLY_dialog* dialog = new openPLY_dialog();
		//dialog->exec();

		//open file and read content into the polymapper
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

	//new TreeWidgetItem for the actors_list
	Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(treeWidget, actor, 1);

	new_actor->setText(0, QString::fromStdString(s_file + s_ext)); //TODO: numbering when you open same file more than one time

	VTKViewer->update();
}

//Slot for spawning geometrical primitives.
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

//Slot for renaming an actor in the actor-list.
void GUI::renameActor() {

	//we need to make the item editable (setFlags) to edit it in the next step
	actorlist_contextmenu_item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);

	treeWidget->editItem(actorlist_contextmenu_item, 0);
}

//Slot for deleting an actor out of the scene and the actor-list.
void GUI::deleteActor() {
	
	//the subclass "actorlist_contextmenu_item" has an private reference on its actor/assembly, 
	//so we can easily remove the actor when removing the item

	//we need to test if the item has an assembly or actor reference
	if (actorlist_contextmenu_item->getActorReference() == NULL) {
		Ren1->RemoveActor(actorlist_contextmenu_item->getAssemblyReference());
	}
	else {
		Ren1->RemoveActor(actorlist_contextmenu_item->getActorReference());
	}
	
	delete actorlist_contextmenu_item;

	VTKViewer->update();
}

//Slot for deactivating an actor/make him invisible
void GUI::deactivateActor() {

	if (actorlist_contextmenu_item->getActorReference() == NULL) {
		actorlist_contextmenu_item->getAssemblyReference()->VisibilityOff();
	}
	else {
		actorlist_contextmenu_item->getActorReference()->VisibilityOff();
	}

	VTKViewer->update();
}

//Slot for reactivating an actor/make him visibile again
void GUI::reactivateActor() {

	if (actorlist_contextmenu_item->getActorReference() == NULL) {
		actorlist_contextmenu_item->getAssemblyReference()->VisibilityOn();
	}
	else {
		actorlist_contextmenu_item->getActorReference()->VisibilityOn();
	}

	VTKViewer->update();
}

//Slot for context menu in the actors-list.
void GUI::prepareMenu(const QPoint & pos)				
{

	//we want to check if we clicked on an item, otherwise we DONT want a context menu!
	if (treeWidget->itemAt(pos) != NULL) {

		//get the item we clicked on
		actorlist_contextmenu_item = dynamic_cast<Q_actorTreeWidgetItem*>(treeWidget->itemAt(pos));

		//create two QActions and connect them to the corresponding slot-functions
		QAction *renameAct = new QAction(QString("Rename"), this);
		QAction *deleteAct = new QAction(QString("Delete"), this);
		QAction *deactivateAct = new QAction(QString("Deactivate actor"), this);
		QAction *reactivateAct = new QAction(QString("Activate actor"), this);

		connect(renameAct, SIGNAL(triggered()), this, SLOT(renameActor()));
		connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteActor()));
		connect(deactivateAct, SIGNAL(triggered()), this, SLOT(deactivateActor()));
		connect(reactivateAct, SIGNAL(triggered()), this, SLOT(reactivateActor()));

		//creating the QMenu that we want to show and add the QActions from above
		QMenu menu(this);
		menu.addAction(renameAct);
		menu.addAction(deleteAct);

		menu.addSeparator();

		menu.addAction(deactivateAct);
		menu.addAction(reactivateAct);

		menu.exec(treeWidget->mapToGlobal(pos));
	}
}

//Slot for opening the Configurator window
void GUI::openConfigurator() {

	//TODO: gucken das kein Speicherleck entsteht
	//we want to make sure, that we only open one instance of a Configurator
	if (Configurator::open_instance == false) {

		Configurator* actor_config = new Configurator();
		actor_config->show();

		//give the configurator-window a reference to this mainwindow, so we can 
		//access GUI-members from the Configurator
		actor_config->mainwindow = this;
	
	}
}

//Safety warning when trying to close this window
void GUI::closeEvent(QCloseEvent *event)
{
	QMessageBox::StandardButton resBtn = QMessageBox::question(this, QString("VisuIFPT"),
		tr("Are you sure?\n"),
		QMessageBox::No | QMessageBox::Yes,
		QMessageBox::Yes);

	if (resBtn != QMessageBox::Yes) {
		event->ignore();

	}
	else {
		event->accept();
	}
}