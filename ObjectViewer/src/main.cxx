/*=========================================================================
Copyright 2004 Sandia Corporation.
Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
license for use of this work by or on behalf of the
U.S. Government. Redistribution and use in source and binary forms, with
or without modification, are permitted provided that this Notice and any
statement of authorship are reproduced on all copies.
=========================================================================*/

/*========================================================================
For general information about using VTK and Qt, see:
http://www.trolltech.com/products/3rdparty/vtksupport.html
=========================================================================*/

/*========================================================================
!!! WARNING for those who want to contribute code to this file.
!!! If you use a commercial edition of Qt, you can modify this code.
!!! If you use an open source version of Qt, you are free to modify
!!! and use this code within the guidelines of the GPL license.
!!! Unfortunately, you cannot contribute the changes back into this
!!! file.  Doing so creates a conflict between the GPL and BSD-like VTK
!!! license.
=========================================================================*/

#include <QVTKApplication.h>

#include <ObjectViewer.h>				//for creating our main window

//MAIN entry point for the program!
int main(int argc, char** argv)
{

	//we need to create a qvtkapplication is the main instance of our Qt-program

	//we can give it the argc and argv parameter, from the main function
	QVTKApplication app(argc, argv);

	//the mainwindow of type GUI (see GUI.h and GUI.cxx)
	ObjectViewer mainwin;

	//the window isn't shown by default, we need to show it explicitly
	mainwin.show();

	//and finally .exec() needs to be called to start the Qt-program
	return app.exec();
}