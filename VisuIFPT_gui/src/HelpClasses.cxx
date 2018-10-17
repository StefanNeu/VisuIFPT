#include <HelpClasses.h>

#include <include\Reader.h>

#include <vtkRenderWindowInteractor.h>		//VTK usage
#include <vtkRenderWindow.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkPlaneSource.h>
#include <vtkCubeSource.h>
#include <vtkSphereSource.h>
#include <vtkPropPicker.h>
#include <qfiledialog.h>
#include <qaction.h>

#include <string>

//Put in two strings and function puts out current interactorstyles
/*void vtk_InteractorMode::getMode(std::string &cam_or_ac, std::string &joy_or_track) {

	//we access the protected variables of the base class
	if (JoystickOrTrackball == 0) {						
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
*/
vtk_InteractorMode::vtk_InteractorMode()
{
	LastPickedActor = NULL;
	LastPickedProperty = vtkProperty::New();
}

vtk_InteractorMode::~vtk_InteractorMode()
{
	LastPickedProperty->Delete();
}


void vtk_InteractorMode::OnLeftButtonDown() {
	
	int* clickPos = this->GetInteractor()->GetEventPosition();

	// Pick from this location.
	vtkSmartPointer<vtkPropPicker>  picker =
		vtkSmartPointer<vtkPropPicker>::New();

	picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());

	// If we picked something before, reset its property
	if (this->LastPickedActor)
	{
		this->LastPickedActor->GetProperty()->DeepCopy(this->LastPickedProperty);
	}
	this->LastPickedActor = picker->GetActor();
	if (this->LastPickedActor)
	{
		// Save the property of the picked actor so that we can
		// restore it next time
		this->LastPickedProperty->DeepCopy(this->LastPickedActor->GetProperty());
		// Highlight the picked actor by changing its properties
		this->LastPickedActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
		this->LastPickedActor->GetProperty()->SetDiffuse(1.0);
		this->LastPickedActor->GetProperty()->SetSpecular(0.0);
	}

	// Forward events
	vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}


//--------------------- SOME USEFULL FUNCTIONS ---------------------------

//Function for opening 3D-files.
void openFile(vtkRenderer* renderer, QWidget* parent_widget, QTreeWidget* item_list) {

	
	//open QFileDialog to open file in the windows-explorer
	QString q_filename = QFileDialog::getOpenFileName(parent_widget, "Open file", "C:/", "3D Files(*.ply *.stl *.pcd)");
	std::string filename = q_filename.toStdString();

	vtkSmartPointer<vtkPolyDataMapper>polymapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	
	//we test if we find the file-extension ply/stl or pcd
	if (filename.find(".ply") != std::string::npos) {

		//openPLY_dialog* dialog = new openPLY_dialog();
		//dialog->exec();

		//open file and read content into the polymapper
		readPLY_s(polymapper, filename);

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
	renderer->AddViewProp(actor);
	
	//new TreeWidgetItem for the actors_list
	Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(item_list, actor, 1);

	new_actor->setText(0, QString::fromStdString(s_file + s_ext)); //OstTODO: numbering when you open same file more than one time

}


//Function for spawning geometrical primitives.
void spawnGeoPrimitives(QAction* primitive, vtkRenderer* renderer, QTreeWidget* item_list, ActorCounter* primitive_counter) {

	//the vtkPolyDataMapper that we fill with 
	vtkSmartPointer<vtkPolyDataMapper>polymapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();

	//the name of the new item for the item-list
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
		primitive_counter->pri_planeCount++;
		item_name = "Plane" + std::to_string(primitive_counter->pri_planeCount);
		

	}
	else if (primitive->text() == "Cube") {

		//same procedure as above
		vtkSmartPointer<vtkCubeSource> cubeSource =
			vtkSmartPointer<vtkCubeSource>::New();
		cubeSource->SetCenter(0.0, 0.0, 0.0);
		cubeSource->Update();

		vtkPolyData* cube = cubeSource->GetOutput();
		polymapper->SetInputData(cube);

		primitive_counter->pri_cubeCount++;
		item_name = "Cube" + std::to_string(primitive_counter->pri_cubeCount);

	}
	else if (primitive->text() == "Sphere") {

		vtkSmartPointer<vtkSphereSource> sphereSource =
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetThetaResolution(30);
		sphereSource->SetPhiResolution(30);
		sphereSource->SetCenter(0.0, 0.0, 0.0);
		sphereSource->Update();

		vtkPolyData* sphere = sphereSource->GetOutput();
		polymapper->SetInputData(sphere);

		primitive_counter->pri_sphereCount++;
		item_name = "Sphere" + std::to_string(primitive_counter->pri_sphereCount);
	}

	//create a vtkActor, connect with the vtkPolyDataMapper and add to Ren1
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();

	actor->SetMapper(polymapper);
	renderer->AddViewProp(actor);

	//create a new item for our actors-list
	Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(item_list, actor, 1);
	new_actor->setText(0, QString::fromStdString(item_name));

}