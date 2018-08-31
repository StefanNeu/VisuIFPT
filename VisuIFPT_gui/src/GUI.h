#ifndef _GUI_h
#define _GUI_h

#include <QMainWindow>
#include "ui_GUI.h"

#include <istream>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkInteractorStyleSwitch.h>


class vtkRenderer;
class vtkEventQtSlotConnect;
class vtkObject;
class vtkCommand;

class vtkTimerCallback : public vtkCommand {
public:
	static vtkTimerCallback* New();
	virtual void Execute(vtkObject *caller, unsigned long eventId,
		void * vtkNotUsed(callData));
};


//Derived class to access protected members "JoystickOrCamera" and "CameraOrActor" of base class
class vtk_InteractorMode : public vtkInteractorStyleSwitch {
public:
	vtk_InteractorMode* New();

	//Return mode of vtkInteractor
	void getMode(std::string&, std::string&);
};

//Derived class of QTreeWidgetItem, with private reference to the actor of the item
class Q_actorTreeWidgetItem : public QTreeWidgetItem {
public:

	Q_actorTreeWidgetItem(QTreeWidget* view, vtkActor* referenced_actor, int type = 1) : QTreeWidgetItem(view, type){
		refToActor = referenced_actor;
	}

	//Return the vtkActor of this item
	vtkActor* getActorReference() {
		return refToActor;
	}

private:
	vtkActor * refToActor;
};

class GUI : public QMainWindow, public Ui::GUI
{
	Q_OBJECT
public:
	GUI();
	~GUI();

public slots:
	
	//Update coordinates of mouse
	void updateCoords(vtkObject*);

	//Open 3D-files
	void openFile();

	//Spawn geometrical primitives
	void spawnPrimitive(QAction*);

	//Spawn context-menu when you rightclick in the actors-list
	void prepareMenu(const QPoint&);

	//Rename an item in the actors-list
	void renameActor();

	//Delete item from actors-list and the corresponding actor from the renderer
	void deleteActor();

	void deactivateActor();

	void reactivateActor();

	//Display the transform-data of the referenced vtkActor of this item
	void displayTransformData(QTreeWidgetItem*, int);

protected:

	//currently our only renderer of the scene
	vtkRenderer * Ren1;

	//objects that contains almost all connections between buttons and other objects
	vtkEventQtSlotConnect* Connections;


private:
	//polydatamapper for our files or geo. primitives
	vtkSmartPointer<vtkPolyDataMapper> polymapper;

	//static counters for geometric primitives
	static int pri_cubeCount;
	static int pri_planeCount;
	static int pri_sphereCount;

	//the interaction style of our window
	vtk_InteractorMode* style;

	//public treewidgetitem that we assign when we open a context menu on a item of the actorlist
	Q_actorTreeWidgetItem* actorlist_contextmenu_item;
	
};

#endif // _GUI_h