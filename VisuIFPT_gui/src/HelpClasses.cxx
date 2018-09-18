#include <HelpClasses.h>

#include <vtkRenderWindowInteractor.h>		//VTK usage
#include <vtkRenderWindow.h>

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
