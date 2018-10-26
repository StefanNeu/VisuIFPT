#ifndef _MainWindow_h
#define _MainWindow_h

#include <QMainWindow>		//for inheritance
#include "ui_ObjectViewer.h"			//for inheritance


//The MainWindow-Class, that derives from QMainWindow and
//Ui::MainWindow (for members/object that we configured in Qt-Designer)
class ObjectViewer : public QMainWindow, public Ui::ObjectViewer
{
	//internal makro by Qt
	Q_OBJECT

public:
	
	ObjectViewer();
	~ObjectViewer();

	//currently our only renderer of the scene
	vtkRenderer * MainRenderer;

};

#endif // _MainWindow_h