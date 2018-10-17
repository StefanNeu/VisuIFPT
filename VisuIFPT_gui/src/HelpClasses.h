#ifndef _HelpClasses_h_
#define _HelpClasses_h_

#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkCommand.h>
#include <qtreewidget.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>

#include <string>



//Derived class of QTreeWidgetItem, with private reference to the actor/assembly of the item
class Q_actorTreeWidgetItem : public QTreeWidgetItem {
public:

	//constructor that accepts an actor as reference
	Q_actorTreeWidgetItem(QTreeWidget* view, vtkActor* referenced_actor, int type = 1) : QTreeWidgetItem(view, type) {
		refToActor = referenced_actor;
		refToAssembly = NULL;
	}

	//Constructor that accepts an assembly as reference
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


//Derived class to access protected members "JoystickOrCamera" and "CameraOrActor" of the base class
class vtk_InteractorMode : public vtkInteractorStyleTrackballCamera {

public:
	static vtk_InteractorMode * New();
	vtkTypeMacro(vtk_InteractorMode, vtkInteractorStyleTrackballCamera);

	//Return mode of vtkInteractor
	void getMode(std::string& actor_or_camera, std::string& joystick_or_trackball);

	vtk_InteractorMode();
	~vtk_InteractorMode();

	virtual void OnLeftButtonDown();

private:
	vtkActor * LastPickedActor;
	vtkProperty *LastPickedProperty;
};


//Simple counter that can be used to count how many actors of a specific type
//have been added to the scene. Helpful for default numbering/naming of items!
class ActorCounter {
public:
	int pri_cubeCount = 0;
	int pri_planeCount = 0;
	int pri_sphereCount = 0;
	int assemblyCount = 0;
};


//---------------- USEFUL FUNCTIONS -------------------------

//-----Function for opening 3D-files. 
//- "renderer" is the renderer you want to add the new actor to
//- "parent_widget" is the parent of the (Windows) File Dialog.. you can use the "this" pointer of the window
//- "item_list" is the treewidget, where you want the actor to be listed as an item
void openFile(vtkRenderer* renderer, QWidget* parent_widget, QTreeWidget* item_list);

//-----Function for spawning geometrical primitives.
//- "primitive" the QAction that the user pressed on.. important for the slot-principle of Qt!
//- "renderer" is the renderer you want to add the new primitive to
//- "item_list" is the treewidget, where you want the new primitive to be listet as an item
//- "primitive_counter" is an ActorCounter of your scene
void spawnGeoPrimitives(QAction* primitive, vtkRenderer* renderer, QTreeWidget* item_list, ActorCounter* primitive_counter);


#endif
