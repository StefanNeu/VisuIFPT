#ifndef _Configurator_h_

#include <QMainWindow>				
#include "ui_configurator.h"		//Ui::Configurator is defined here

#include "HelpClasses.h"			//for vtk_InteractorMode
#include <vtkRenderer.h>
#include <vtkAssembly.h>
#include <GUI.h>

class Configurator : public QMainWindow, public Ui::Configurator {

	Q_OBJECT

public:
	Configurator();
	~Configurator();

	//makes sure we only have one open instance of the configurator (could also be solved with the singleton concept)
	static bool open_instance;

	GUI* mainwindow;

public slots:

	//Display the transform-data of the referenced vtkActor of this item
	void displayTransformData(QTreeWidgetItem*, int);
	void spawnPrimitive(QAction*);
	void closeEvent(QCloseEvent*);
	void exportActor();

private:

	//the interaction style of our window
	vtk_InteractorMode * style;

	//Renderer of the scene
	vtkRenderer* Actor_Renderer;

	//the actor we are assembling in the configurator and that we want to add to the mainwindow
	vtkAssembly* new_actor;

};

#endif // _Configurator_h