#include "GUI.h"

#include <QMenu>

#include "vtkActor.h"
#include "vtkCommand.h"
#include "vtkConeSource.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"

#include <istream>
#include <qfiledialog.h>
#include "Reader.h"
#include <vtkPLYReader.h>

#include "QVTKInteractor.h"

GUI::GUI()
{
	this->setupUi(this);

	// create a window to make it stereo capable and give it to QVTKWidget
	vtkRenderWindow* renwin = vtkRenderWindow::New();
	//renwin->StereoCapableWindowOn();

	VTKViewer->SetRenderWindow(renwin);
	renwin->Delete();

	// add a renderer
	Ren1 = vtkRenderer::New();
	VTKViewer->GetRenderWindow()->AddRenderer(Ren1);


	// add a popup menu for the window and connect it to our slot
	QMenu* popup1 = new QMenu(VTKViewer);
	popup1->addAction("Background White");
	popup1->addAction("Background Black");
	popup1->addAction("Stereo Rendering");
	connect(popup1, SIGNAL(triggered(QAction*)), this, SLOT(color1(QAction*)));

	connect(actionOpen_File, SIGNAL(triggered()), this, SLOT(openFile()));


	//--------------------------- CONNECTIONS -----------------------------------------
	Connections = vtkEventQtSlotConnect::New();

	// get right mouse pressed with high priority
	Connections->Connect(VTKViewer->GetRenderWindow()->GetInteractor(),
		vtkCommand::RightButtonPressEvent,
		this,
		SLOT(popup(vtkObject*, unsigned long, void*, void*, vtkCommand*)),
		popup1, 1.0);

	// update coords as we move through the window
	Connections->Connect(VTKViewer->GetRenderWindow()->GetInteractor(),
		vtkCommand::MouseMoveEvent,
		this,
		SLOT(updateCoords(vtkObject*)));

	Connections->PrintSelf(cout, vtkIndent());
}

GUI::~GUI()
{
	Ren1->Delete();

	Connections->Delete();
}


void GUI::updateCoords(vtkObject* obj)
{
	// get interactor
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	// get event position
	int event_pos[2];
	iren->GetEventPosition(event_pos);
	// update label
	QString str;
	str.sprintf("x=%d : y=%d", event_pos[0], event_pos[1]);
	coord->setText(str);
}

void GUI::popup(vtkObject * obj, unsigned long,
	void * client_data, void *,
	vtkCommand * command)
{
	// A note about context menus in Qt and the QVTKWidget
	// You may find it easy to just do context menus on right button up,
	// due to the event proxy mechanism in place.

	// That usually works, except in some cases.
	// One case is where you capture context menu events that
	// child windows don't process.  You could end up with a second
	// context menu after the first one.

	// See QVTKWidget::ContextMenuEvent enum which was added after the
	// writing of this example.

	// get interactor
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	// consume event so the interactor style doesn't get it
	command->AbortFlagOn();
	// get popup menu
	QMenu* popupMenu = static_cast<QMenu*>(client_data);
	// get event location
	int* sz = iren->GetSize();
	int* position = iren->GetEventPosition();
	// remember to flip y
	QPoint pt = QPoint(position[0], sz[1] - position[1]);
	// map to global
	QPoint global_pt = popupMenu->parentWidget()->mapToGlobal(pt);
	// show popup menu at global point
	popupMenu->popup(global_pt);
}

void GUI::color1(QAction* color)
{
	if (color->text() == "Background White")
		Ren1->SetBackground(1, 1, 1);
	else if (color->text() == "Background Black")
		Ren1->SetBackground(0, 0, 0);
	else if (color->text() == "Stereo Rendering")
	{
		Ren1->GetRenderWindow()->SetStereoRender(!Ren1->GetRenderWindow()->GetStereoRender());
	}
	VTKViewer->update();
}

void GUI::openFile() {

	QString q_filename = QFileDialog::getOpenFileName(this, tr("Open file"), "C:/", tr("3D Files(*.ply *.stl *.pcd)"));
	std::string filename = q_filename.toStdString();

	polymapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	
	if (filename.find(".ply") != std::string::npos) {						//we test if we find the file-extension ".ply" or ".stl"
		
		/*std::cout << "Do you want to read this file as standard .ply or as pointcloud?" << std::endl;
		std::cout << "Type \"s\" for standard mode and \"p\" for pointcloud mode: ";
		
		if (mode == 's') {								//standard mode-> open PLY with internal reader
			readPLY_s(data_mapper, filename);
		}
		else if (mode == 'p') {							//pointcloud mode-> open PLY with our reader
			readPLY_p(data_mapper, filename);
		}*/

		readPLY_p(polymapper, filename);

	}
	else if (filename.find(".stl") != std::string::npos) {
		readSTL(polymapper, filename);
	}
	else if (filename.find(".pcd") != std::string::npos) {
		readPCD(polymapper, filename);
	}

	vtkSmartPointer<vtkActor> actor1 =
		vtkSmartPointer<vtkActor>::New();
	actor1->SetMapper(polymapper);
	Ren1->AddViewProp(actor1);
	VTKViewer->update();
}
