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

#include "ConfigDialog.hpp"
#include "ui_ConfigDialog.h"

#include <QFileDialog>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
	ui->lw_dirs->setSelectionMode(QAbstractItemView::MultiSelection);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ConfigDialog::setTextureSearchDirs(QStringList dirs)
{
	ui->lw_dirs->clear();
	ui->lw_dirs->addItems(dirs);
}

void ConfigDialog::on_pb_add_clicked()
{
	QFileDialog* fileDialog = new QFileDialog(this);
	fileDialog->setWindowTitle(tr("Select texture search directory."));
	fileDialog->setFileMode(QFileDialog::Directory);
	fileDialog->setFilter("*.png");
	fileDialog->exec();
	if (fileDialog->result() == QDialog::Accepted)
	{
		foreach (QString dir, fileDialog->selectedFiles())
		{
			searchDirChanges.append(qMakePair(true, dir));
			ui->lw_dirs->addItem(dir);
		}
	}
	delete fileDialog;
}

void ConfigDialog::on_pb_remove_clicked()
{
	foreach(QListWidgetItem* item, ui->lw_dirs->selectedItems())
	{
		searchDirChanges.append(qMakePair(false, item->text()));
		delete ui->lw_dirs->takeItem(ui->lw_dirs->row(item));
	}
}

void ConfigDialog::on_buttonBox_accepted()
{
	emit updateTextureSearchDirs(searchDirChanges);
	searchDirChanges.clear();
}

void ConfigDialog::on_buttonBox_rejected()
{
	searchDirChanges.clear();
}
