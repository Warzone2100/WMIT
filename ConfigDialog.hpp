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
#ifndef CONFIGDIALOG_HPP
#define CONFIGDIALOG_HPP

#include <QDialog>

#include <QList>
#include <QPair>
#include <QString>

namespace Ui {
    class ConfigDialog;
}

class ConfigDialog : public QDialog {
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ConfigDialog *ui;
	QList<QPair<bool,QString> > searchDirChanges;
signals:
	void updateTextureSearchDirs(QList<QPair<bool,QString> >);

public slots:
	void setTextureSearchDirs(QStringList);

private slots:
	void on_buttonBox_rejected();
	void on_buttonBox_accepted();
	void on_pb_remove_clicked();
	void on_pb_add_clicked();
};

#endif // CONFIGDIALOG_HPP
