#include <MainWindow.h>	

#include <HelpClasses.h>
#include <include/Reader.h>			//for reading different types of files
#include <Configurator.h>			//for creating a window of type Configurator

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
/*int MainWindow::pri_planeCount = 0;
int MainWindow::pri_cubeCount = 0;
int MainWindow::pri_sphereCount = 0;
int MainWindow::new_actorCount = 0;

*/
bool MainWindow::auto_camReposition = false;

//#Constructor of our main window.
MainWindow::MainWindow()
{
	//shutdown the VTK Debug window.
	vtkObject::GlobalWarningDisplayOff();

	//just that the checkBox for the automatic camera reposition is safely unchecked when the bool is false
	if (MainWindow::auto_camReposition == true) {
		checkBox->setChecked(false);
	}

	//sets up all Qt objects (see ui_MainWindow.h)
	this->setupUi(this);

	//create a vtkRenderWindow, that we want to assign to the QVTKViewer
	vtkRenderWindow* renwin = vtkRenderWindow::New();			
	VTKViewer->SetRenderWindow(renwin);
	renwin->Delete();
	
	//add a renderer
	Ren1 = vtkRenderer::New();
	VTKViewer->GetRenderWindow()->AddRenderer(Ren1);

	mainWindow_ActorCounter = new ActorCounter;
	
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
	connect(actionOpen_File, SIGNAL(triggered()), this, SLOT(openFile_MainWindow()));

	//same connection as above, but we process the press on the menu and the following press on the QAction* in the SLOT-function
	connect(menuGeometric_Primitives, SIGNAL(triggered(QAction*)), this, SLOT(spawnPrimitive(QAction*)));

	//connection for context menu in our actors-list
	connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(prepareMenu(const QPoint&)));

	//updating the transform-data in the inspector, when item in actors-list is clicked
	connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(displayTransformData(QTreeWidgetItem*, int)));

	//open the Configurator-window
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(openConfigurator()));
	
	
	connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(camReposition(bool)));

	//this class is needed to manage Qt and VTK connections
	Connections = vtkEventQtSlotConnect::New();

	// update coords as we move through the window
	Connections->Connect(VTKViewer->GetRenderWindow()->GetInteractor(),
		vtkCommand::MouseMoveEvent,
		this,
		SLOT(updateCoords(vtkObject*)));

}

MainWindow::~MainWindow()
{
	//make sure to delete everything to avoid leaks!
	Ren1->Delete();
	Connections->Delete();
	polymapper->Delete();
	style->Delete();

	// TODO: test if we have data leaks.
	delete actorlist_contextmenu_item;
}

//------------------- SLOT FUNCTIONS -----------------------------

//Slot for transform-data.
void MainWindow::displayTransformData(QTreeWidgetItem* item, int) {

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
void MainWindow::updateCoords(vtkObject* obj)
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
void MainWindow::openFile_MainWindow() {

	openFile(Ren1, this, treeWidget);

	if (auto_camReposition == true) {
		Ren1->ResetCamera();
	}
	
	VTKViewer->update();
	cout << ">> The file was loaded successfully!" << endl;
	
}

//Slot for spawning geometrical primitives.
void MainWindow::spawnPrimitive(QAction* primitive) {				
	
	//we use a outsourced function in HelpClasses.cxx to spawn the primitives
	spawnGeoPrimitives(primitive, Ren1, treeWidget, mainWindow_ActorCounter);

	if (auto_camReposition == true) {
		Ren1->ResetCamera();
	}

	VTKViewer->update();
}

//Slot for renaming an actor in the actor-list.
void MainWindow::renameActor() {

	//we need to make the item editable (setFlags) to edit it in the next step
	actorlist_contextmenu_item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);

	treeWidget->editItem(actorlist_contextmenu_item, 0);
}

//Slot for deleting an actor out of the scene and the actor-list.
void MainWindow::deleteActor() {
	
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
void MainWindow::deactivateActor() {

	if (actorlist_contextmenu_item->getActorReference() == NULL) {
		actorlist_contextmenu_item->getAssemblyReference()->VisibilityOff();
	}
	else {
		actorlist_contextmenu_item->getActorReference()->VisibilityOff();
	}

	VTKViewer->update();
}

//Slot for reactivating an actor/make him visibile again
void MainWindow::reactivateActor() {

	if (actorlist_contextmenu_item->getActorReference() == NULL) {
		actorlist_contextmenu_item->getAssemblyReference()->VisibilityOn();
	}
	else {
		actorlist_contextmenu_item->getActorReference()->VisibilityOn();
	}

	VTKViewer->update();
}

//Slot for context menu in the actors-list.
void MainWindow::prepareMenu(const QPoint & pos)				
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
void MainWindow::openConfigurator() {

	//TODO: gucken das kein Speicherleck entsteht
	//we want to make sure, that we only open one instance of a Configurator
	if (Configurator::open_instance == false) {

		Configurator* actor_config = new Configurator();
		actor_config->show();

		//give the configurator-window a reference to this mainwindow, so we can 
		//access MainWindow-members from the Configurator
		actor_config->mainwindow = this;
	
	}
}

//Slot for safety warning when trying to close this window
void MainWindow::closeEvent(QCloseEvent *event)
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

void MainWindow::camReposition(bool ticked) {
	
	if (ticked == true) {
		MainWindow::auto_camReposition = true;
	}
	else if (ticked == false) {
		MainWindow::auto_camReposition = false;
	}
}