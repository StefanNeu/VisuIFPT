#pragma once
#include <string>
#include <vtkActor.h>
#include <vtkInteractorStyleSwitch.h>
#include <qtreewidget.h>
#include <vtkCommand.h>



//Derived class of QTreeWidgetItem, with private reference to the actor of the item
class Q_actorTreeWidgetItem : public QTreeWidgetItem {
public:

	Q_actorTreeWidgetItem(QTreeWidget* view, vtkActor* referenced_actor, int type = 1) : QTreeWidgetItem(view, type) {
		refToActor = referenced_actor;
	}

	//Return the vtkActor of this item
	vtkActor* getActorReference() {
		return refToActor;
	}

private:
	vtkActor * refToActor;
};



//Derived class to access protected members "JoystickOrCamera" and "CameraOrActor" of base class
class vtk_InteractorMode : public vtkInteractorStyleSwitch {
public:
	vtk_InteractorMode * New();

	//Return mode of vtkInteractor
	void getMode(std::string&, std::string&);
};


//Callback class used for updating the QVTKWidget periodically
class vtkTimerCallback : public vtkCommand {
public:
	static vtkTimerCallback* New();
	virtual void Execute(vtkObject *caller, unsigned long eventId,
		void * vtkNotUsed(callData));
};
