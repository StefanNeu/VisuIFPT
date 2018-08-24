#ifndef _openPLY_dialog_h
#define _openPLY_dialog_h

#include <QMainWindow>
#include "ui_openPLY_dialog.h"

class openPLY_dialog : public QDialog, public Ui::Dialog
{
	Q_OBJECT

public:
	openPLY_dialog(int*);
	~openPLY_dialog();

	const QPushButton* standardButton;
	const QPushButton* pointcloudButton;

	//1 for standard; 2 for pointcloud
	int* openModeOfPLY;

public slots:

void setStandardMode(bool);
void setPointcloudMode(bool);


};
#endif // _openPLY_dialog_h