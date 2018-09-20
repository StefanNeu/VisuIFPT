#include <HelpClasses.h>

#include <vtkRenderWindowInteractor.h>		//VTK usage
#include <vtkRenderWindow.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <qfiledialog.h>
#include <include\Reader.h>

#include <string>


//Simple constructor for derived class we need
vtk_InteractorMode* vtk_InteractorMode::New() {
	return new vtk_InteractorMode();
}

//Put in two strings and function puts out current interactorstyles
void vtk_InteractorMode::getMode(std::string &cam_or_ac, std::string &joy_or_track) {

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


//Simple constructor
vtkTimerCallback* vtkTimerCallback::New() {
	return new vtkTimerCallback();
}

//Important function! Execute() gets called, when the observed objects sends a signal.
void vtkTimerCallback::Execute(vtkObject *caller, unsigned long eventId,
	void * vtkNotUsed(callData)) {

	vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::SafeDownCast(caller);
	iren->GetRenderWindow()->Render();

}


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
	renderer->AddViewProp(actor);
	
	//new TreeWidgetItem for the actors_list
	Q_actorTreeWidgetItem* new_actor = new Q_actorTreeWidgetItem(item_list, actor, 1);

	new_actor->setText(0, QString::fromStdString(s_file + s_ext)); //TODO: numbering when you open same file more than one time

}