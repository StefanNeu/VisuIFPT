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



//Initializing the automatic camera repositioning with false
bool MainWindow::auto_camReposition = false;

//Constructor of our main window.
MainWindow::MainWindow()
{
	//shutdown the VTK Debug window.
	vtkObject::GlobalWarningDisplayOff();

	//just that the checkBox for the automatic camera reposition is safely unchecked when the bool is false
	if (MainWindow::auto_camReposition == true) {
		checkBox_autoCamRepos->setChecked(false);
	}

	//sets up all Qt objects (see ui_MainWindow.h)
	this->setupUi(this);

	property_list->setSortingEnabled(false);

	//create a vtkRenderWindow, that we want to assign to the QVTKViewer
	vtkRenderWindow* renwin = vtkRenderWindow::New();			
	VTKViewer->SetRenderWindow(renwin);
	renwin->Delete();
	
	//add a renderer
	Ren1 = vtkRenderer::New();
	VTKViewer->GetRenderWindow()->AddRenderer(Ren1);

	//allocate memory for the actorcounter of the mainwindow
	mainWindow_ActorCounter = new ActorCounter;
	

	// Example for making an explicit light source that DOESN'T move with the camera/observer (like in reality)
	
	Ren1->SetAutomaticLightCreation(0);
	
	vtkSmartPointer<vtkLightKit> light =
		vtkSmartPointer<vtkLightKit>::New();
	light->AddLightsToRenderer(Ren1);
	light->Update();


	

	//add an InteractionMode (derivitive from InteractionStyleSwitch) and set default to trackball_camera
	/*style = style->New();
	style->SetCurrentStyleToTrackballCamera();
	style->SetDefaultRenderer(Ren1);
	VTKViewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
	*/

	// Set the custom type to use for interaction.
	style = style->New();
	style->SetDefaultRenderer(Ren1);
	VTKViewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
	
	

	//initialize interactor and add callback-object to update the viewer periodically
	VTKViewer->GetRenderWindow()->Render();
	VTKViewer->GetRenderWindow()->GetInteractor()->Initialize();
	VTKViewer->GetRenderWindow()->GetInteractor()->Start();

	//repeating timer makes the interactor send a signal peridodically, so we can update properly
	//VTKViewer->GetRenderWindow()->GetInteractor()->CreateRepeatingTimer(100);
	


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
	connect(mainWindow_actorList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(prepareMenu(const QPoint&)));

	//updating the transform-data in the inspector, when item in actors-list is clicked
	connect(mainWindow_actorList, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(displayActorData(QTreeWidgetItem*, int)));

	//open the Configurator-window
	connect(actionOpen_Config, SIGNAL(triggered()), this, SLOT(openConfigurator()));
	
	//connection for the automatic camera reposition
	connect(checkBox_autoCamRepos, SIGNAL(clicked(bool)), this, SLOT(camReposition(bool)));

	connect(actionOpen_YAML_Configuration, SIGNAL(triggered()), this, SLOT(openYAML()));


	//this class is needed to manage Qt and VTK connections
	Connections = vtkEventQtSlotConnect::New();

	//update transform data periodically
	Connections->Connect(VTKViewer->GetRenderWindow()->GetInteractor(),
		vtkCommand::TimerEvent,
		this,
		SLOT(updateMainWindow()));
}

MainWindow::~MainWindow()
{
	//make sure to delete everything to avoid leaks!
	Ren1->Delete();
	Connections->Delete();
	//style->Delete();

	// TODO: test if we have data leaks.
	delete actorlist_contextmenu_item;
}


//------------------- SLOT FUNCTIONS -----------------------------

//Slot for transform-data.
void MainWindow::displayActorData(QTreeWidgetItem* item, int) {

	//we want to clear the list everytime we display all (and maybe sometimes different) property of actors
	property_list->clear();

	//allocate memory
	double* location_data = new double[3];
	double* rotation_data = new double[3];
	double* scale = new double[3];

	//we cast the QTreeWidgetItem into the derived class, so we can use a few extra functions
	actorlist_contextmenu_item = dynamic_cast<Q_actorTreeWidgetItem*>(item);


	//we need to access the vtkTransform of the actor/assembly to get the true position and rotation of the actorS
	vtkSmartPointer<vtkTransform> transformation =
		vtkSmartPointer<vtkTransform>::New();

	//we need to find out, if we clicked on an item with an actor or assembly-reference
	//(assemblies come from the configurator)
	if (actorlist_contextmenu_item->getActorReference() != NULL) {
		transformation->SetMatrix(actorlist_contextmenu_item->getActorReference()->GetMatrix());
	}
	else if (actorlist_contextmenu_item->getAssemblyReference() != NULL) {
		transformation->SetMatrix(actorlist_contextmenu_item->getAssemblyReference()->GetMatrix());
	}
	else if (actorlist_contextmenu_item->getYamlActor() != NULL) {
		transformation->SetMatrix(actorlist_contextmenu_item->getYamlActor()->GetMatrix());
	}

	transformation->GetPosition(location_data);
	transformation->GetOrientation(rotation_data);
	


	//------------------------ POSITION ------------------------------
	//we need to cut off a little bit, off the too long double string
	std::string x_stringPOS = std::to_string(location_data[0]);
	x_stringPOS = x_stringPOS.substr(0, x_stringPOS.size() - 4);

	std::string y_stringPOS = std::to_string(location_data[1]);
	y_stringPOS = y_stringPOS.substr(0, y_stringPOS.size() - 4);

	std::string z_stringPOS = std::to_string(location_data[2]);
	z_stringPOS = z_stringPOS.substr(0, z_stringPOS.size() - 4);

	//we create a parent item and give it the children "location" and "rotation" with the corresponding data
	QTreeWidgetItem* transform = new QTreeWidgetItem(property_list, 1);
	transform->setText(0, QString::fromStdString("Transform Data"));
	transform->setExpanded(true);

	QTreeWidgetItem* location = new QTreeWidgetItem(transform, 1);
	location->setText(0, QString::fromStdString("Location"));
	location->setText(1, QString::fromStdString("x=" + x_stringPOS + "     y=" + y_stringPOS + "     z=" + z_stringPOS));


	//---------------------- ROTATION ----------------------------------
	//same procedure as above
	std::string x_stringROT = std::to_string(rotation_data[0]);
	x_stringROT = x_stringROT.substr(0, x_stringROT.size() - 4);

	std::string y_stringROT = std::to_string(rotation_data[1]);
	y_stringROT = y_stringROT.substr(0, y_stringROT.size() - 4);

	std::string z_stringROT = std::to_string(rotation_data[2]);
	z_stringROT = z_stringROT.substr(0, z_stringROT.size() - 4);


	QTreeWidgetItem* rotation = new QTreeWidgetItem(transform, 1);
	rotation->setText(0, QString::fromStdString("Rotation"));
	rotation->setText(1, QString::fromStdString("x=" + x_stringROT + "     y=" + y_stringROT + "     z=" + z_stringROT));

	if (actorlist_contextmenu_item->getYamlActor() != NULL) {
		displayYAMLdata(property_list, actorlist_contextmenu_item->getYamlActor()->dataNode);
	}
}

//Slot for opening 3D-files.
void MainWindow::openFile_MainWindow() {

	openFile(Ren1, this, mainWindow_actorList);

	if (auto_camReposition == true) {
		Ren1->ResetCamera();
	}
	
	VTKViewer->update();
	cout << ">> The file was loaded successfully!" << endl;
	
}

//Slot for spawning geometrical primitives.
void MainWindow::spawnPrimitive(QAction* primitive) {				
	
	//we use a outsourced function in HelpClasses.cxx to spawn the primitives
	spawnGeoPrimitives(primitive, Ren1, mainWindow_actorList, mainWindow_ActorCounter);

	if (auto_camReposition == true) {
		Ren1->ResetCamera();
	}

	VTKViewer->update();
}

//Slot for renaming an actor in the actor-list.
void MainWindow::renameActor() {

	//we need to make the item editable (setFlags) to edit it in the next step
	actorlist_contextmenu_item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);

	mainWindow_actorList->editItem(actorlist_contextmenu_item, 0);
}

//Slot for deleting an actor out of the scene and the actor-list.
void MainWindow::deleteActor() {
	
	//the subclass "actorlist_contextmenu_item" has an private reference on its actor/assembly, 
	//so we can easily remove the actor when removing the item

	//we need to test if the item has an assembly or actor reference
	if (actorlist_contextmenu_item->getActorReference() != NULL) {
		Ren1->RemoveActor(actorlist_contextmenu_item->getActorReference());
	}
	else if (actorlist_contextmenu_item->getAssemblyReference() != NULL) {
		Ren1->RemoveActor(actorlist_contextmenu_item->getAssemblyReference());
	}
	else if (actorlist_contextmenu_item->getYamlActor() != NULL) {
		Ren1->RemoveActor(actorlist_contextmenu_item->getYamlActor());
	}
	
	delete actorlist_contextmenu_item;
	actorlist_contextmenu_item = NULL;

	VTKViewer->update();
}

//Slot for deactivating an actor/make him invisible
void MainWindow::deactivateActor() {

	if (actorlist_contextmenu_item->getActorReference() != NULL) {
		actorlist_contextmenu_item->getActorReference()->VisibilityOff();
	}
	else if(actorlist_contextmenu_item->getAssemblyReference() != NULL){
		actorlist_contextmenu_item->getAssemblyReference()->VisibilityOff();
	}
	else if (actorlist_contextmenu_item->getYamlActor() != NULL) {
		actorlist_contextmenu_item->getYamlActor()->VisibilityOff();
	}

	VTKViewer->update();
}

//Slot for reactivating an actor/make him visibile again
void MainWindow::reactivateActor() {

	if (actorlist_contextmenu_item->getActorReference() != NULL) {
		actorlist_contextmenu_item->getActorReference()->VisibilityOn();
	}
	else if(actorlist_contextmenu_item->getAssemblyReference() != NULL){
		actorlist_contextmenu_item->getAssemblyReference()->VisibilityOn();
	}
	else if (actorlist_contextmenu_item->getYamlActor() != NULL) {
		actorlist_contextmenu_item->getYamlActor()->VisibilityOn();
	}

	VTKViewer->update();
}

//Slot for context menu in the actors-list.
void MainWindow::prepareMenu(const QPoint & pos)				
{

	//we want to check if we clicked on an item, otherwise we DONT want a context menu!
	if (mainWindow_actorList->itemAt(pos) != NULL) {

		//get the item we clicked on
		actorlist_contextmenu_item = dynamic_cast<Q_actorTreeWidgetItem*>(mainWindow_actorList->itemAt(pos));

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

		menu.exec(mainWindow_actorList->mapToGlobal(pos));
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
	/*
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
	*/
}

//Slot for controlling the option of automatic camera reposition
void MainWindow::camReposition(bool ticked) {
	
	if (ticked == true) {
		MainWindow::auto_camReposition = true;
	}
	else if (ticked == false) {
		MainWindow::auto_camReposition = false;
	}
}

//Slot for updating the transformdata periodically
void MainWindow::updateMainWindow() {

	if (actorlist_contextmenu_item != NULL) {

		//we just use the other slot function again
		//displayActorData(actorlist_contextmenu_item, 1);
		
	}
}

void MainWindow::openYAML() {

	QString yaml_filename = QFileDialog::getOpenFileName(this, "Open YAML configuration", "C:/", "YAML files (*.yml *.yaml)");
	std::string filename = yaml_filename.toStdString();

	try {
		std::vector<YAML::Node> node_collection = YAML::LoadAllFromFile(filename);

		if ( !( node_collection[0].IsNull() ) ) {

			for (int i = 0; i < node_collection.size(); i++) {

				YAML::Node node1 = node_collection[i];
				YAML::Emitter yaml_emit;
				yaml_emit << node1;

				//vtkSmartPointer<vtkActor> boundBox_actor =
				//	vtkSmartPointer<vtkActor>::New();
			
				vtk_yamlActor* boundBox_actor = new vtk_yamlActor(node1);

				Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(mainWindow_actorList, boundBox_actor, 1);
				new_actor->setText(0, QString::fromStdString(node1["name"].as<std::string>()));


				double* transform_data = new double[node1["transformation"].size()];
				for (int i = 0; i < node1["transformation"].size(); i++) {
					transform_data[i] = node1["transformation"][i].as<double>();
				}


				double* geometry_origin = new double[node1["geometry"]["origin"].size()];
				for (int i = 0; i < node1["geometry"]["origin"].size(); i++) {
					geometry_origin[i] = node1["geometry"]["origin"][i].as<double>();
				}

				double* geometry_dimensions = new double[node1["geometry"]["extension"].size()];
				for (int i = 0; i < node1["geometry"]["extension"].size(); i++) {
					geometry_dimensions[i] = node1["geometry"]["extension"][i].as<double>();
				}


				vtkSmartPointer<vtkCubeSource> bounding_box =
					vtkSmartPointer<vtkCubeSource>::New();
				bounding_box->SetBounds(0.0, geometry_dimensions[0], 0.0, geometry_dimensions[1], 0.0, geometry_dimensions[2]);
				bounding_box->SetCenter(0.0, 0.0, 0.0);
				bounding_box->Update();		

				vtkSmartPointer<vtkMatrix4x4> transform_matrix =
					vtkSmartPointer<vtkMatrix4x4>::New();
				transform_matrix->DeepCopy(transform_data);
				transform_matrix->Transpose();

				vtkSmartPointer<vtkTransform> bound_transform =
					vtkSmartPointer<vtkTransform>::New();
				bound_transform->SetMatrix(transform_matrix);
			

				vtkSmartPointer<vtkPolyDataMapper> bounding_mapper =
					vtkSmartPointer<vtkPolyDataMapper>::New();
				bounding_mapper->SetInputConnection(bounding_box->GetOutputPort());

				boundBox_actor->SetUserTransform(bound_transform);
				boundBox_actor->SetMapper(bounding_mapper);
				boundBox_actor->GetProperty()->SetRepresentationToWireframe();
				Ren1->AddActor(boundBox_actor);

			}
		}
		

	}
	catch (const YAML::Exception& e) {
		cout << e.what() << endl;
	}
}