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

//we need this derived class to access protected members "JoystickOrCamera" and "CameraOrActor" of base class
class vtk_InteractorMode : public vtkInteractorStyleSwitch {
public:
	vtk_InteractorMode* New();
	void getMode(std::string&, std::string&);
};

class Q_actorTreeWidgetItem : public QTreeWidgetItem {
public:

	Q_actorTreeWidgetItem(QTreeWidget* view, vtkActor* referenced_actor, int type = 1) : QTreeWidgetItem(view, type){
		refToActor = referenced_actor;
	}

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
	
	void updateCoords(vtkObject*);
	void openFile();
	void spawnPrimitive(QAction*);
	void prepareMenu(const QPoint&);
	void renameActor();
	void deleteActor();
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

	//the interaction style of our window
	vtk_InteractorMode* style;

	//public treewidgetitem that we assign when we open a context menu on a item of the actorlist
	Q_actorTreeWidgetItem* actorlist_contextmenu_item;
	
};

#endif // _GUI_h