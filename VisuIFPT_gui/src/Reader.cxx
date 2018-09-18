#include <vtkVertexGlyphFilter.h>	//Qt/VTK usage
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPLYReader.h>
#include <vtkSTLReader.h>

#include <istream>				//for processing strings and opening files
#include <string>
#include <sstream>
			
//----------------------------FUNCTIONS FOR PROCESSING THE KEYWORDS IN PLY AND PCD------------------
//function that processes keywords in a PLY-header
int processKeywords_PLY(std::string line, long* point_count) {								

	//skip empty lines
	if (line == "")					
		return 0;
	
	//if we read "element vertex"
	if (line.find("element vertex") != std::string::npos) {				

		std::string word;
		std::stringstream line_stream(line);
	
		//we want to store the third string of the whole line
		line_stream >> word >> word >> word;				

		//as double in *point_count
		*point_count = std::stol(word);								

		return 0;
	}

	//if these key words are read, then skip
	if ((line.find("ply") != std::string::npos) || (line.find("format") != std::string::npos) || (line.find("comment") != std::string::npos) ||
		(line.find("property") != std::string::npos) || (line.find("element") != std::string::npos)) {

		return 0;			
	}

	//if we find the key word "end_header", we want to start reading vertices
	if (line.find("end_header") != std::string::npos) {
		return -1;						
	}

	return 0;					// TODO: What to do if we read anything else than the expected keywords?
}		

//function that processes the keywords in a pcd_header
int processKeywords_PCD(std::string line) {								

	if (line.find("DATA") != std::string::npos) {					
		return -1;
	}
	else {
		return 0;
	}
}


//------------------------ FUNCTIONS FOR READING THE SPECIFIC FILES ---------------------------

//Function for reading the vertices from a PLY-file and store them as point cloud
void readPLY_p(vtkPolyDataMapper* mapper, const std::string filename) {


	std::string line;

	//the array in which we will store x,y and z coordinates as string..  when inserting we cast them to doubles
	std::string value[3];					

	//our filestream
	std::ifstream file(filename);			

	//the vertex_count, that we will read in the header of the ply
	long int vertex_count = 0;				


	//--------------------------------READING THE HEADER---------------------------------------------

	//we read one line from the file at a time and process it
	while (getline(file, line)) {						

		switch (processKeywords_PLY(line, &vertex_count)) {

			//if we read an unimportant keyword, an empty line or anything else we want to continue
			case 0:						
				continue;

			//if we read "end_header" we want to break out of this switch and loop
			case -1:					
				break;
		}
		break;
	}


	//------------------------------READING THE POINTS-------------------------------------------------

	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();

	//we read the points in a for-loop that is limited by the vertex_count
	for (int i = 0; i < vertex_count; i++) {

		//we read each line and open a stringstream ssin with it
		getline(file, line);							
		std::stringstream ssin(line);

		for (int j = 0; j < 3; j++) {
			//we read the first three values and store them in value[j]
			ssin >> value[j];							
		}
		points->InsertNextPoint(std::stod(value[0]), std::stod(value[1]), std::stod(value[2]));
	}

	file.close();


	//----------------------------CREATING THE POLYDATA,FILTER AND MAPPER--------------------------------

	vtkSmartPointer<vtkPolyData> polydata =
		vtkSmartPointer<vtkPolyData>::New();

	//give polydata our points
	polydata->SetPoints(points);									

	//use an vertexglyphfilter 
	vtkSmartPointer<vtkVertexGlyphFilter> vertexGlyphFilter =
		vtkSmartPointer<vtkVertexGlyphFilter>::New();

	vertexGlyphFilter->AddInputData(polydata);
	vertexGlyphFilter->Update();
	
	mapper->SetInputConnection(vertexGlyphFilter->GetOutputPort());
}

//Function for reading a PLY-file in "standard mode", so with edges and faces
void readPLY_s(vtkPolyDataMapper* mapper, const std::string filename) {

	vtkSmartPointer<vtkPLYReader> reader =
		vtkSmartPointer<vtkPLYReader>::New();

	//we just use the internal vtkPLYReader and connect the mapper to the outputport of the reader
	reader->SetFileName(filename.c_str());						

	mapper->SetInputConnection(reader->GetOutputPort());
}


//Function for reading STL-files, with the internal vtkSTLReader
void readSTL(vtkPolyDataMapper* mapper, const std::string filename) {

	vtkSmartPointer<vtkSTLReader> reader =
		vtkSmartPointer<vtkSTLReader>::New();

	//we just use the internal vtkSTLReader and connect the mapper to the outputport of the reader
	reader->SetFileName(filename.c_str());						
	reader->Update();

	mapper->SetInputConnection(reader->GetOutputPort());
}

//Function for reading the points out of an ASCII-PCD-file 
void readPCD(vtkPolyDataMapper* mapper, const std::string filename) {

	std::string line;

	//the array in which we will store x,y and z coordinates as string..  when inserting we cast them to doubles
	std::string value[3];					

	//our filestream
	std::ifstream file(filename);			


	//--------------------------------READING THE HEADER---------------------------------------------

	while (getline(file, line)) {

		switch (processKeywords_PCD(line)) {

			//if we read an unimportant keyword, an empty line or anything else we want to continue
			case 0:						
				continue;

			//if we read "DATA" we want to break out of this switch and loop
			case -1:					
				break;
		}
		break;
	}


	//------------------------------READING THE POINTS-------------------------------------------------

	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();
	
	while (getline(file, line)) {							

		//we read each line and open a stringstream ssin with it
		std::stringstream ssin(line);

		for (int j = 0; j < 3; j++) {

			//we read the first three values and store them in value[j]
			ssin >> value[j];						
		}

		points->InsertNextPoint(std::stod(value[0]), std::stod(value[1]), std::stod(value[2]));
	}

	file.close();


	//----------------------------CREATING THE POLYDATA,FILTER AND MAPPER--------------------------------

	vtkSmartPointer<vtkPolyData> polydata =
		vtkSmartPointer<vtkPolyData>::New();

	//give polydata our points
	polydata->SetPoints(points);									

	vtkSmartPointer<vtkVertexGlyphFilter> vertexGlyphFilter =
		vtkSmartPointer<vtkVertexGlyphFilter>::New();

	vertexGlyphFilter->AddInputData(polydata);
	vertexGlyphFilter->Update();

	mapper->SetInputConnection(vertexGlyphFilter->GetOutputPort());
}
