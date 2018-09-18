#include <vtkVersion.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkPLYReader.h>
#include <vtkSTLReader.h>
#include <istream>
#include <string>
#include <sstream>
			
void readPLY_p(vtkPolyDataMapper*,  std::string);				//prototypes
void readPLY_s(vtkPolyDataMapper*, std::string);
void readSTL(vtkPolyDataMapper*, std::string);
void readPCD(vtkPolyDataMapper*, std::string);


//--------------------------------------CALLBACK CLASS FOR INTERACTING WITH WINDOWINTERACTOR-------------------------------------

																		// TODO: change name of this callback class
class vtkPositionCallback : public vtkCommand {							
public:
	static vtkPositionCallback *New() {									//constructor that we can ignore
		vtkPositionCallback *cb = new vtkPositionCallback;
		return cb;
	}

	virtual void Execute(vtkObject* caller, unsigned long eventId,			//important function that gets executed when observer is called!
		void* vtkNotUsed(callData)) {										//the argument "caller" is a pointer to the object that we are observing	

		vtkRenderWindowInteractor *iren =								
			static_cast<vtkRenderWindowInteractor*>(caller);

		std::string key = iren->GetKeySym();								//get key of the windowinteractor

		if (key == "k") {				

			vtkSmartPointer<vtkActor> new_actor =							// TODO: maybe make deep copy, instead of shallow one? what if we want to modify the datamapper?
				vtkSmartPointer<vtkActor>::New();
			new_actor->SetMapper(datamapper);								//if we press "k", make shallow copy of datamapper and create new actor (positioned slightly right)
			new_actor->AddPosition(100.0, 0.0, 0.0);

			renderer->AddActor(new_actor);									//add new actor

			iren->GetRenderWindow()->Render();								//render with new actor
		}		
	}

public:
	vtkPolyDataMapper* datamapper;														//through this public members we can reach specific actors/datamappers and work with them
	vtkRenderer* renderer;
};



 
int main(int argc, char * argv[])
{

  if (argc != 2) {																		
	  std::cout << "Too few arguments!" << std::endl;
	  std::cout << "Usage: " << argv[0] << "  <Filename(.ply/.stl/.pcd)>" << std::endl;
	  return EXIT_FAILURE;
  }
  
  std::string filename(argv[1]);

  vtkSmartPointer<vtkPolyDataMapper> data_mapper =						//we create the data_mapper which gets "filled" by the appropriate reader-function
	  vtkSmartPointer<vtkPolyDataMapper>::New();

  //----------------------------------DECIDING WHAT FILE-READER WE NEED TO USE-------------------------------------
  
  if (filename.find(".ply") != std::string::npos) {						//we test if we find the file-extension ".ply" or ".stl"

	  char mode;
	  std::cout << "Do you want to read this file as standard .ply or as pointcloud?" << std::endl;
	  std::cout << "Type \"s\" for standard mode and \"p\" for pointcloud mode: ";
	  std::cin >> mode;
	  
	  if (mode == 's') {								//standard mode-> open PLY with internal reader
	      readPLY_s(data_mapper, filename);
	  }
	  else if (mode == 'p') {							//pointcloud mode-> open PLY with our reader
		  readPLY_p(data_mapper, filename);
	  }
  }
  else if (filename.find(".stl") != std::string::npos) {
	  readSTL(data_mapper, filename);
  }
  else if (filename.find(".pcd") != std::string::npos) {
	  readPCD(data_mapper, filename);
  }
  else {
	  std::cout << "Please enter .ply/.stl or .pcd files!" << std::endl;
	  return EXIT_FAILURE;
  }


  //Create the actor
  vtkSmartPointer<vtkActor> actor = 
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(data_mapper);
  actor->GetProperty()->SetPointSize(0.1);

 
  // Create a renderer, render window, and interactor
  vtkSmartPointer<vtkRenderer> renderer = 
    vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindow> renderWindow = 
    vtkSmartPointer<vtkRenderWindow>::New();

  renderWindow->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
    vtkSmartPointer<vtkRenderWindowInteractor>::New();

  renderWindowInteractor->SetRenderWindow(renderWindow);
 
  // Add the actor to the scene
  renderer->AddActor(actor);
  renderer->SetBackground(0.0, 0.0, 0.0); // Background color green
 
  // Render and interact
  renderWindow->Render();
  renderWindowInteractor->Initialize();

  vtkSmartPointer<vtkPositionCallback> cb =								//initialize our observer-object (if needed with some data like a datamapper or renderer)
	  vtkSmartPointer<vtkPositionCallback>::New();
  cb->datamapper = data_mapper;
  cb->renderer = renderer;

  renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, cb);					//we let our new object "observe" the windowinteractor and react to specific events (in Execute())

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}



//----------------------------FUNCTIONS FOR PROCESSING THE KEYWORDS IN PLY AND PCD------------------
int processKeywords_PLY(std::string line, long* point_count) {								//FUNCTION THAT PROCESSES THE KEYWORDS IN THE STL-HEADER

	if (line == "")					//skip empty lines
		return 0;

	if (line.find("element vertex") != std::string::npos) {				//if we read "element vertex"

		std::string word;
		std::stringstream line_stream(line);
	
		line_stream >> word >> word >> word;							//we want to store the third string of the whole line
		*point_count = std::stol(word);									//as double in *point_count
		return 0;
	}

	if ((line.find("ply") != std::string::npos) || (line.find("format") != std::string::npos) || (line.find("comment") != std::string::npos) ||
		(line.find("property") != std::string::npos) || (line.find("element") != std::string::npos)) {

		return 0;			//if these key words are read, then skip
	}

	if (line.find("end_header") != std::string::npos) {
		return -1;						//if we find the key word "end_header", we want to start reading vertices
	}

	return 0;					// TODO: What to do if we read anything else than the expected keywords?
}		

int processKeywords_PCD(std::string line) {								//FUNCTION THAT PROCESSES THE KEYWORDS IN THE PCD-HEADER

	if (line.find("DATA") != std::string::npos) {					
		return -1;
	}
	else {
		return 0;
	}
}



//------------------------FUNCTION FOR READING THE SPECIFIC FILES------------------

void readPLY_p(vtkPolyDataMapper* mapper, const std::string filename) {


	std::string line;
	std::string value[3];					//the array in which we will store x,y and z coordinates as string..  when inserting we cast them to doubles

	std::ifstream file(filename);			//our filestream

	long int vertex_count = 0;				//the vertex_count, that we will read in the header of the ply

	//--------------------------------READING THE HEADER---------------------------------------------

	while (getline(file, line)) {						

		switch (processKeywords_PLY(line, &vertex_count)) {
		case 0:						//if we read an unimportant keyword, an empty line or anything else we want to continue
			continue;
		case -1:					//if we read "end_header" we want to break out of this switch and loop
			break;
		}
		break;
	}

	//------------------------------READING THE POINTS-------------------------------------------------

	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();

	for (int i = 0; i < vertex_count; i++) {

		getline(file, line);							//we read each line and open a stringstream ssin with it
		std::stringstream ssin(line);

		for (int j = 0; j < 3; j++) {
			ssin >> value[j];							//we read the first three values and store them in value[j]
		}
		points->InsertNextPoint(std::stod(value[0]), std::stod(value[1]), std::stod(value[2]));
	}

	file.close();

	//----------------------------CREATING THE POLYDATA,FILTER AND MAPPER--------------------------------

	vtkSmartPointer<vtkPolyData> polydata =
		vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(points);									//give polydata our points

	vtkSmartPointer<vtkVertexGlyphFilter> vertexGlyphFilter =
		vtkSmartPointer<vtkVertexGlyphFilter>::New();
#if VTK_MAJOR_VERSION <= 5
	vertexGlyphFilter->AddInput(polydata);
#else
	vertexGlyphFilter->AddInputData(polydata);
#endif
	vertexGlyphFilter->Update();
	
	mapper->SetInputConnection(vertexGlyphFilter->GetOutputPort());

}

void readPLY_s(vtkPolyDataMapper* mapper, const std::string filename) {

	vtkSmartPointer<vtkPLYReader> reader =
		vtkSmartPointer<vtkPLYReader>::New();
	reader->SetFileName(filename.c_str());							//we just use the internal vtkPLYReader and connect the mapper to the outputport of the reader

	mapper->SetInputConnection(reader->GetOutputPort());
}

void readSTL(vtkPolyDataMapper* mapper, const std::string filename) {

	vtkSmartPointer<vtkSTLReader> reader =
		vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName(filename.c_str());							//we just use the internal vtkPLYReader and connect the mapper to the outputport of the reader
	reader->Update();

	mapper->SetInputConnection(reader->GetOutputPort());
}

void readPCD(vtkPolyDataMapper* mapper, const std::string filename) {

	std::string line;
	std::string value[3];					//the array in which we will store x,y and z coordinates as string..  when inserting we cast them to doubles

	std::ifstream file(filename);			//our filestream

	//--------------------------------READING THE HEADER---------------------------------------------

	while (getline(file, line)) {

		switch (processKeywords_PCD(line)) {
		case 0:						//if we read an unimportant keyword, an empty line or anything else we want to continue
			continue;
		case -1:					//if we read "DATA" we want to break out of this switch and loop
			break;
		}
		break;
	}

	//------------------------------READING THE POINTS-------------------------------------------------

	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();
	
	while (getline(file, line)) {							//we read each line and open a stringstream ssin with it

		std::stringstream ssin(line);

		for (int j = 0; j < 3; j++) {
			ssin >> value[j];							//we read the first three values and store them in value[j]
		}
		points->InsertNextPoint(std::stod(value[0]), std::stod(value[1]), std::stod(value[2]));
	}
	file.close();

	//----------------------------CREATING THE POLYDATA,FILTER AND MAPPER--------------------------------

	vtkSmartPointer<vtkPolyData> polydata =
		vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(points);									//give polydata our points

	vtkSmartPointer<vtkVertexGlyphFilter> vertexGlyphFilter =
		vtkSmartPointer<vtkVertexGlyphFilter>::New();
#if VTK_MAJOR_VERSION <= 5
	vertexGlyphFilter->AddInput(polydata);
#else
	vertexGlyphFilter->AddInputData(polydata);
#endif
	vertexGlyphFilter->Update();

	mapper->SetInputConnection(vertexGlyphFilter->GetOutputPort());
}
