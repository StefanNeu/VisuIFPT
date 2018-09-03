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

public slots:

	//Display the transform-data of the referenced vtkActor of this item
	void displayTransformData(QTreeWidgetItem*, int);

private:

	//the interaction style of our window
	vtk_InteractorMode * style;

	//Renderer of the scene
	vtkRenderer* Actor_Renderer;
};
#endif // _Configurator_h