#pragma once
#include <string>
#include <vtkPolyDataMapper.h>

int processKeywords_PLY(std::string, long*);
int processKeywords_PCD(std::string);

void readPLY_p(vtkPolyDataMapper*, const std::string);
void readPLY_s(vtkPolyDataMapper*, const std::string);
void readSTL(vtkPolyDataMapper*, const std::string);
void readPCD(vtkPolyDataMapper*, const std::string);
