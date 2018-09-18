#include "openPLY_dialog.h"
#include "include/Reader.h"

openPLY_dialog::openPLY_dialog(int* openingMode) {

	openModeOfPLY = openingMode;
	this->setupUi(this);
	standardButton = buttonBox->addButton(QString("Open as 3D file"), QDialogButtonBox::AcceptRole);
	pointcloudButton = buttonBox->addButton(QString("Open as pointcloud"), QDialogButtonBox::AcceptRole);


	this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void openPLY_dialog::setStandardMode(bool unused) {
	*openModeOfPLY = 1;
}

void openPLY_dialog::setPointcloudMode(bool unused) {
	*openModeOfPLY = 2;
}

openPLY_dialog::~openPLY_dialog() {

}