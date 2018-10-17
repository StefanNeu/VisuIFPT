#ifndef _MainWindow_h
#define _MainWindow_h

#include <QMainWindow>		//for inheritance
#include "ui_MainWindow.h"			//for inheritance

#include "HelpClasses.h"		//for some outsourced utility function 

#include <vtkPolyDataMapper.h>	//for VTK/Qt usage
#include <vtkSmartPointer.h>
#include <vtkEventQtSlotConnect.h>


//The MainWindow-Class, that derives from QMainWindow and
//Ui::MainWindow (for members/object that we configured in Qt-Designer)
class MainWindow : public QMainWindow, public Ui::MainWindow
{
	//internal makro by Qt
	Q_OBJECT

public:
	
	MainWindow();
	~MainWindow();

	//currently our only renderer of the scene
	vtkRenderer * Ren1;

	ActorCounter* mainWindow_ActorCounter;

//Here we declare the public slot functions.
public slots:

	//Open 3D-files
	void openFile_MainWindow();

	//Spawn geometrical primitives
	void spawnPrimitive(QAction*);

	//Spawn context-menu when you rightclick in the actors-list
	void prepareMenu(const QPoint&);

	//Rename an item in the actors-list
	void renameActor();

	//Delete item from actors-list and the corresponding actor from the renderer
	void deleteActor();

	//Deactivate an actor to make him invisible and non-interactable
	void deactivateActor();

	//Reactivate an actor to make him visible and interactable again
	void reactivateActor();

	//Display the transform-data of the referenced vtkActor of this item
	void displayTransformData(QTreeWidgetItem*, int);

	//Opens the Configurator-Window
	void openConfigurator();

	//Process the closeEvent (clicking on the "x")
	void closeEvent(QCloseEvent*);

	//Checking the camera reposition option
	void camReposition(bool);

	//Updates components of the mainwindow periodically
	void updateMainWindow();

	void openYAML();

protected:

	//This member manages connections between VTK and Qt objects
	vtkEventQtSlotConnect* Connections;

private:

	//Default false and if true, camera always repositions when a new actor is added, so that every actor is visible
	static bool auto_camReposition;

	//The interaction style of our window
	vtk_InteractorMode* style;

	//Public TreeWidgetItem that we assign when we click on an item of the item list
	Q_actorTreeWidgetItem* actorlist_contextmenu_item = NULL;

};

#endif // _MainWindow_h