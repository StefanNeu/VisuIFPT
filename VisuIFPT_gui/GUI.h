#ifndef _GUI_h
#define _GUI_h

#include <QMainWindow>
#include "ui_GUI.h"

#include <istream>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>


class vtkRenderer;
class vtkEventQtSlotConnect;
class vtkObject;
class vtkCommand;

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

protected:
	vtkRenderer * Ren1;
	vtkEventQtSlotConnect* Connections;
private:
	vtkSmartPointer<vtkPolyDataMapper> polymapper;
};

#endif // _GUI_h