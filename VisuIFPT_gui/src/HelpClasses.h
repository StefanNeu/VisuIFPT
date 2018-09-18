#ifndef _HelpClasses_h_
#define _HelpClasses_h_

#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkCommand.h>
#include <qtreewidget.h>

#include <string>

//Derived class of QTreeWidgetItem, with private reference to the actor/assembly of the item
class Q_actorTreeWidgetItem : public QTreeWidgetItem {
public:

	//constructor that accepts a actor as reference
	Q_actorTreeWidgetItem(QTreeWidget* view, vtkActor* referenced_actor, int type = 1) : QTreeWidgetItem(view, type) {
		refToActor = referenced_actor;
		refToAssembly = NULL;
	}

	Q_actorTreeWidgetItem(QTreeWidget* view, vtkAssembly* referenced_assembly, int type = 1) : QTreeWidgetItem(view, type) {
		refToAssembly = referenced_assembly;
		refToActor = NULL;
	}

	//Return the vtkActor of this item
	vtkActor* getActorReference() {
		return refToActor;
	}

	vtkAssembly* getAssemblyReference() {
		return refToAssembly;
	}

private:

	//the pointers to the actor/assembly
	vtkActor* refToActor;
	vtkAssembly* refToAssembly;
};



//Derived class to access protected members "JoystickOrCamera" and "CameraOrActor" of base class
class vtk_InteractorMode : public vtkInteractorStyleSwitch {

public:
	vtk_InteractorMode * New();

	//Return mode of vtkInteractor
	void getMode(std::string& actor_or_camera, std::string& joystick_or_trackball);
};


//Callback class used for updating the QVTKWidget periodically
class vtkTimerCallback : public vtkCommand {
public:
	static vtkTimerCallback* New();
	virtual void Execute(vtkObject *caller, unsigned long eventId,
		void * vtkNotUsed(callData));
};

#endif
