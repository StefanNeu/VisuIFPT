#ifndef _Configurator_h_
#define _Configurator_h_

#include <QMainWindow>				//for inheritance
#include <ui_configurator.h>		

#include <HelpClasses.h>		
#include <GUI.h>

#include <vtkRenderer.h>			//Qt/VTK usage
#include <vtkAssembly.h>


//The Configurator window also needs to behave and therefore derive from QMainWindow and 
//from the Ui::Configurator class
class Configurator : public QMainWindow, public Ui::Configurator {

	Q_OBJECT

public:

	Configurator();
	~Configurator();

	//makes sure we only have one open instance of the configurator (could also be solved with the singleton concept)
	static bool open_instance;

	//The reference to the GUI-mainwindow, so we can access some members of it
	GUI* mainwindow;

public slots:

	//Display the transform-data of the referenced vtkActor of this item
	void displayTransformData(QTreeWidgetItem*, int);

	//Spawn a primitive in the configurator
	void spawnPrimitive(QAction*);

	//Exporting the assembly to the main renderer
	void exportActor();

	//Process the closeEvent (clicking on the "x")
	void closeEvent(QCloseEvent*);

private:

	//the interaction style of our window
	vtk_InteractorMode * style;

	//Renderer of the scene
	vtkRenderer* Actor_Renderer;

	//the actor we are assembling in the configurator and that we want to add to the mainwindow
	vtkAssembly* new_actor;

};

#endif // _Configurator_h