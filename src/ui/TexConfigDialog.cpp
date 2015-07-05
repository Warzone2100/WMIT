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

#include "TexConfigDialog.h"
#include "ui_TexConfigDialog.h"

#include <QFileDialog>
#include <QSettings>

#include "wmit.h"

TexConfigDialog::TexConfigDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TexConfigDialog)
{
	ui->setupUi(this);

	ui->lw_dirs->setSelectionMode(QAbstractItemView::MultiSelection);
}

TexConfigDialog::~TexConfigDialog()
{
	delete ui;
}

void TexConfigDialog::loadSearchDirs()
{
	QSettings settings;
	QStringList list = settings.value(WMIT_SETTINGS_TEXSEARCHDIRS, QStringList()).toStringList();
	m_searchdirs = QSet<QString>::fromList(list);

	resetDirsList();

	emit updateTextureSearchDirs(m_searchdirs.toList());
}

void TexConfigDialog::saveSearchDirs()
{
	QSettings settings;
	settings.setValue(WMIT_SETTINGS_TEXSEARCHDIRS, QVariant(m_searchdirs.toList()));

	emit updateTextureSearchDirs(m_searchdirs.toList());
}

void TexConfigDialog::resetDirsList()
{
	ui->lw_dirs->clear();
	ui->lw_dirs->addItems(m_searchdirs.toList());
}

void TexConfigDialog::changeEvent(QEvent *e)
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

void TexConfigDialog::on_pb_add_clicked()
{
	QFileDialog* fileDialog = new QFileDialog(this);

	fileDialog->setWindowTitle(tr("Select texture search directory."));
	fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setNameFilter("*.png");
	fileDialog->exec();

	if (fileDialog->result() == QDialog::Accepted)
	{
		foreach (QString dir, fileDialog->selectedFiles())
		{
			ui->lw_dirs->addItem(dir);
		}
	}

	delete fileDialog;
}

void TexConfigDialog::on_pb_remove_clicked()
{
	foreach(QListWidgetItem* item, ui->lw_dirs->selectedItems())
	{
		delete ui->lw_dirs->takeItem(ui->lw_dirs->row(item));
	}
}

void TexConfigDialog::on_buttonBox_accepted()
{
	m_searchdirs.clear();
	QListWidgetItem* itm;

	for (int i = 0; i < ui->lw_dirs->count(); ++i)
	{
		itm = ui->lw_dirs->item(i);
		if (itm)
			m_searchdirs += itm->text();
	}

	saveSearchDirs();
	resetDirsList();
}

void TexConfigDialog::on_buttonBox_rejected()
{
	resetDirsList();
}
