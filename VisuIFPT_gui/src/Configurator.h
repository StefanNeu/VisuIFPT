#ifndef _Configurator_h_

#include <QMainWindow>				
#include "ui_configurator.h"		//Ui::Configurator is defined here

#include "HelpClasses.h"			//for vtk_InteractorMode
#include <vtkRenderer.h>

class Configurator : public QMainWindow, public Ui::Configurator {

	Q_OBJECT

public:
	Configurator();
	~Configurator();

	//makes sure we only have one open instance of the configurator (could also be solved with the singleton concept)
	static bool open_instance;

public slots:

	//Display the transform-data of the referenced vtkActor of this item
	void displayTransformData(QTreeWidgetItem*, int);
	void spawnPrimitive(QAction*);
	void closeEvent(QCloseEvent*);

private:

	//the interaction style of our window
	vtk_InteractorMode * style;

	//Renderer of the scene
	vtkRenderer* Actor_Renderer;
};

#endif // _Configurator_h