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

#include "ImportDialog.hpp"
#include "ui_ImportDialog.h"

#include <limits>

#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

#include "Util.hpp"

ImportDialog::ImportDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ImportDialog),
	lw_autoFoundTextures_previous(NULL)
{
	ui->setupUi(this);
}

ImportDialog::~ImportDialog()
{
	delete ui;
}

QString ImportDialog::modelFilePath() const
{
	return ui->le_fileName->text();
}

QString ImportDialog::textureFilePath() const
{
	return ui->le_textureFName->text();
}

bool ImportDialog::tcmaskChecked() const
{
	return ui->gb_tcmask->isEnabled();
}

QString ImportDialog::tcmaskFilePath() const
{
	return ui->le_tcmFName->text();
}

void ImportDialog::scanForTextures(const QStringList& dirs)
{
	ui->lw_autoFoundTextures->clear();
	m_textures.clear();
	foreach (QString path, dirs)
	{
		QDir dir(path, "*.png", QDir::Type, QDir::Files);
		QStringList textures = dir.entryList();
		foreach (QString texture, textures)
		{
			m_textures.append(dir.absoluteFilePath(texture));
			if (!texture.endsWith("tcmask.png", Qt::CaseInsensitive))
			{
				ui->lw_autoFoundTextures->addItem(dir.absoluteFilePath(texture));
			}
		}
	}
}

void ImportDialog::changeEvent(QEvent *e)
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

inline void ImportDialog::lw_autoFoundTextures_clearSelection()
{
	ui->lw_autoFoundTextures->clearSelection();
	ui->lw_autoFoundTextures->clearFocus();
	lw_autoFoundTextures_previous = NULL;
}

void ImportDialog::on_tb_seekFileName_clicked()
{
	static QString lastDir = QDir::currentPath();
	QFileDialog* fileDialog = new QFileDialog(this,
								 tr("Select File to open"),
								 lastDir,
								 tr("All Compatible (*.wzm *.pie *.3ds *.obj);;"
									"WZM models (*.wzm);;"
									"PIE models (*.pie);;"
									"3DS files (*.3ds);;"
									"OBJ files (*.obj)"));
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->exec();
	lastDir = fileDialog->directory().absolutePath();
	if (fileDialog->result() == QDialog::Accepted)
	{
		ui->le_fileName->setText(fileDialog->selectedFiles().first());
		on_pb_autoTex_clicked();
	}
	delete fileDialog;
}

void ImportDialog::on_tb_seekTextureFName_clicked()
{
	static QString lastDir = QDir::currentPath();
	QFileDialog* fileDialog = new QFileDialog(this,
											  tr("Select texture to use"),
											  lastDir,
											  tr("WZ Compatible (*.png);;"
												 "WMIT Compatible (*.bmp *.jpg *.jpeg *.png)"));
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->exec();
	lastDir = fileDialog->directory().absolutePath();
	if (fileDialog->result() == QDialog::Accepted)
	{
		ui->le_textureFName->setText(fileDialog->selectedFiles().first());
		lw_autoFoundTextures_clearSelection();
	}
	delete fileDialog;
}

void ImportDialog::on_lw_autoFoundTextures_itemSelectionChanged()
{
	if (!ui->lw_autoFoundTextures->selectedItems().empty())
	{
		ui->le_textureFName->setText(ui->lw_autoFoundTextures->selectedItems().first()->text());
	}
}

void ImportDialog::on_lw_autoFoundTextures_itemClicked(QListWidgetItem* item)
{
	if (lw_autoFoundTextures_previous == item)
	{
		lw_autoFoundTextures_clearSelection();
	}
	else
	{
		lw_autoFoundTextures_previous = item;
	}
}

void ImportDialog::on_pb_autoTex_clicked()
{
	QString fileTexName = getTextureName(ui->le_fileName->text());
	if (fileTexName.isNull())
	{
		return;
	}
	foreach(QString filePath, m_textures)
	{
		if (fileTexName.compare(QFileInfo(filePath).fileName(), Qt::CaseSensitive) == 0)
		{
			ui->le_textureFName->setText(filePath);
			return;
		}
	}
}


void ImportDialog::on_tb_seekTcmFName_clicked()
{
	static QString lastDir = QDir::currentPath();
	QFileDialog* fileDialog = new QFileDialog(this,
											  tr("Select tcmask to use"),
											  lastDir,
											  tr("WZ Compatible (*.png);;"));
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->exec();
	lastDir = fileDialog->directory().absolutePath();
	if (fileDialog->result() == QDialog::Accepted)
	{
		ui->le_tcmFName->setText(fileDialog->selectedFiles().first());
	}
	delete fileDialog;
}

void ImportDialog::on_pb_autoTCM_clicked()
{
	QRegExp pageNoRegX("page\\-(\\d+)");

	if (pageNoRegX.indexIn(ui->le_textureFName->text()) != -1)
	{
		QFileInfo nfo(ui->le_textureFName->text());

		nfo.setFile(QDir(nfo.absolutePath()), pageNoRegX.cap(0).append("_tcmask.png"));

		if (nfo.exists())
		{
			ui->le_tcmFName->setText(nfo.absoluteFilePath());
			return;
		}
		foreach (QString texture, m_textures)
		{
			if (texture.endsWith(pageNoRegX.cap(0).append("_tcmask.png"), Qt::CaseInsensitive))
			{
				ui->le_tcmFName->setText(texture);
				return;
			}
		}
	}
}
