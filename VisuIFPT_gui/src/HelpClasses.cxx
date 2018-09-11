#include "HelpClasses.h"
#include <string>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkAssembly.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropCollection.h>

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

vtkActor* convertAssemblyToActor(vtkAssembly* new_actorAssembly) {

	vtkSmartPointer<vtkActor> new_actorCopy =
		vtkSmartPointer<vtkActor>::New();

	vtkSmartPointer<vtkPolyDataMapper> new_actorMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();

	vtkSmartPointer<vtkPolyData> new_actorData =
		vtkSmartPointer<vtkPolyData>::New();

	vtkSmartPointer<vtkPropCollection> actor_collection =
		vtkSmartPointer<vtkPropCollection>::New();

	new_actorAssembly->GetActors(actor_collection);

	vtkSmartPointer<vtkActor> itemIterator =
		vtkSmartPointer<vtkActor>::New();

	// TODO: check if assembly is empty when exporting

	for (int i = 0; i < actor_collection->GetNumberOfItems; i++) {
		itemIterator = (vtkActor*) actor_collection->GetNextProp();
		
	}



}