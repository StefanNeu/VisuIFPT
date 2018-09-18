#ifndef _GUI_h
#define _GUI_h

#include <QMainWindow>		//for inheritance
#include "ui_GUI.h"			//for inheritance

#include "HelpClasses.h"		//for some outsourced utility function 

#include <vtkPolyDataMapper.h>	//for VTK/Qt usage
#include <vtkSmartPointer.h>
#include <vtkEventQtSlotConnect.h>


//The MainWindow-Class, that derives from QMainWindow and
//Ui::GUI (for members/object that we configured in Qt-Designer)
class GUI : public QMainWindow, public Ui::GUI
{
	//internal makro by Qt
	Q_OBJECT

public:
	
	GUI();
	~GUI();

	//currently our only renderer of the scene
	vtkRenderer * Ren1;

	//static counters for naming and numbering our actos in the itemlist
	static int pri_cubeCount;
	static int pri_planeCount;
	static int pri_sphereCount;
	static int new_actorCount;

//Here we declare the public slot functions.
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


protected:

	//This member manages connections between VTK and Qt objects
	vtkEventQtSlotConnect* Connections;

private:

	//Polydatamapper for our files or geo. primitives
	vtkSmartPointer<vtkPolyDataMapper> polymapper;

	//The interaction style of our window
	vtk_InteractorMode* style;

	//Public TreeWidgetItem that we assign when we open a context menu on a item of the actorlist
	Q_actorTreeWidgetItem* actorlist_contextmenu_item;

};

#endif // _GUI_h