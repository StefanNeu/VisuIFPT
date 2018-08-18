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
class vtkInteractorMode : public vtkInteractorStyleSwitch {
public:
	vtkInteractorMode* New();
	void getMode(std::string&, std::string&);
};

class GUI : public QMainWindow, public Ui::GUI
{
	Q_OBJECT
public:
	GUI();
	~GUI();

public slots:
	
	void updateCoords(vtkObject*);
	void popup(vtkObject * obj, unsigned long,
		void * client_data, void *,
		vtkCommand * command);
	void color1(QAction*);
	void openFile();
	void spawnPrimitive(QAction*);
	void prepareMenu(const QPoint&);
	void renameActor();

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
	vtkInteractorMode* style;

	//public treewidgetitem that we assign when we open a context menu on a item of the actorlist
	QTreeWidgetItem* actorlist_contextmenu_item;
	
};

#endif // _GUI_h