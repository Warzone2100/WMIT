/*
	Copyright 2010 Warzone 2100 Project

	This file is part of WMIT.

	WMIT is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	WMIT is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with WMIT.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef IMPORTDIALOG_HPP
#define IMPORTDIALOG_HPP

#include <QDialog>
#include <QSettings>

namespace Ui {
	class ImportDialog;
}

class QListWidgetItem;

class ImportDialog : public QDialog {
	Q_OBJECT
public:
	ImportDialog(QWidget *parent = 0);
	~ImportDialog();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::ImportDialog* m_ui;

	QSettings m_settings;
};

#endif // IMPORTDIALOG_HPP
