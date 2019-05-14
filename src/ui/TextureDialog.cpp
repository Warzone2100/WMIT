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

#include "TextureDialog.h"
#include "ui_TextureDialog.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <QDir>

#include "wmit.h"

#include "TexConfigDialog.h"

TextureDialog::TextureDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TextureDialog),
	m_texConfigDialog(new TexConfigDialog(this))
{
	ui->setupUi(this);
	connect(this, SIGNAL(finished(int)), this, SLOT(onFinished(int)));

	// shape texture icons
	ui->lwTextures->setViewMode(QListView::IconMode);
	ui->lwTextures->setIconSize(QSize(128, 128));
	ui->lwTextures->setMovement(QListView::Static);
	ui->lwTextures->setFlow(QListView::LeftToRight);
	ui->lwTextures->setFixedWidth(170);

	// UI is ready and now we can load window previous state
	resize(m_settings.value("TextureDialog/size", QSize(1000, 875)).toSize());
	move(m_settings.value("TextureDialog/position", pos()).toPoint());

	connect(ui->lwTextures, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
		this, SLOT(iconDoubleClicked(QListWidgetItem*)));

	// connect then kick loader for chain reaction
	connect(m_texConfigDialog, SIGNAL(updateTextureSearchDirs(QStringList)),
		this, SLOT(setSearchDirs(QStringList)));

	m_texConfigDialog->loadSearchDirs();

	for (int i = WZM_TEX__FIRST; i < WZM_TEX__LAST; ++i)
	{
		QString textypename = QString::fromStdString(WZM::texTypeToString(static_cast<wzm_texture_type_t>(i)));
		types[textypename] = static_cast<wzm_texture_type_t>(i);
	}

	QStringList items;
	QMapIterator<QString, wzm_texture_type_t> it(types);
	while (it.hasNext()) {
		it.next();
		items << it.key();
	}
	ui->cbType->insertItems(0, items);
}

TextureDialog::~TextureDialog()
{
	delete ui;
}

QString TextureDialog::selectTextureFile()
{
	static QString lastDir = m_work_dir;
	QString texture;
	QFileDialog* fileDialog = new QFileDialog(0,
						  tr("Select texture file to use"),
						  lastDir,
						  tr("WZ Compatible (*.png);;WMIT Compatible (*.bmp *.jpg *.jpeg *.png)"));
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->exec();

	if (fileDialog->result() == QDialog::Accepted)
	{
		lastDir = fileDialog->directory().absolutePath();
		texture = fileDialog->selectedFiles().first();
	}

	delete fileDialog;
	return texture;
}

void TextureDialog::addTextureIcon(wzm_texture_type_t type, const QString& filepath)
{
	QString texpath = filepath.isEmpty() ? findTexture(type) : filepath;
	if (texpath.isEmpty())
	{
		 texpath = WMIT_IMAGES_NOTEXTURE;
	}

	if (m_icons.find(type) == m_icons.end())
	{
		m_icons[type] = new QListWidgetItem();
	}
	QListWidgetItem *newicn = m_icons[type];

	newicn->setData(Qt::UserRole, QVariant(static_cast<int>(type)));
	newicn->setData(Qt::UserRole + 1, texpath);

	QPixmap img(texpath);
	QString ttp = "Path: " + texpath + "\nWidth: " + QString::number(img.width())+ ", height: " + QString::number(img.height());
	newicn->setToolTip(ttp);
	newicn->setIcon(QIcon(img));
	newicn->setText(QString::fromStdString(WZM::texTypeToString(type)));
	newicn->setTextAlignment(Qt::AlignHCenter);
	newicn->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	ui->lwTextures->addItem(newicn);
	ui->lwTextures->setCurrentItem(newicn);
}

void TextureDialog::createTextureIcons(const QString& workdir, const QString& modelname)
{
	ui->lwTextures->clear();
	m_icons.clear();

	m_work_dir = workdir;
	m_model_filepath = modelname;

	if (workdir.isEmpty() || modelname.isEmpty() || !m_texnames.size())
		return;

	QMap<wzm_texture_type_t, QString>::const_iterator it;
	for (it = m_texnames.begin(); it != m_texnames.end(); ++it)
	{
		addTextureIcon(it.key());
	}

	ui->lwTextures->setCurrentRow(0);
}

void TextureDialog::setSearchDirs(const QStringList &list)
{
	scanForTexturesInDirs(list);
}

void TextureDialog::setTexturesMap(const QMap<wzm_texture_type_t, QString> &texnames)
{
	m_texnames = texnames;
}

void TextureDialog::getTexturesFilepath(QMap<wzm_texture_type_t, QString> &files) const
{
	files.clear();

	for (int i = 0; i < ui->lwTextures->count(); ++i)
	{
		QListWidgetItem *itm = ui->lwTextures->item(i);
		if (itm)
		{
			wzm_texture_type_t typ = static_cast<wzm_texture_type_t>(itm->data(Qt::UserRole).toInt());
			QString texpath = itm->data(Qt::UserRole + 1).toString();

			if (!texpath.isEmpty() && !texpath.contains(WMIT_IMAGES_NOTEXTURE))
			{
				files.insert(typ, texpath);
			}
		}
	}
}

QString TextureDialog::findTexture(wzm_texture_type_t type) const
{
	QFileInfo modelnfo(m_model_filepath);
	QString fileTexName = m_texnames.value(type);

	if (!fileTexName.isEmpty())
	{
		// Pre-configured search
		foreach(QString filePath, m_predefined_textures)
		{
			if (fileTexName.compare(QFileInfo(filePath).fileName(), Qt::CaseSensitive) == 0)
			{
				return filePath;
			}
		}

		// Local search
		QFileInfo nfo(modelnfo.path() + "/" + fileTexName);
		if (nfo.exists())
		{
			return nfo.filePath();
		}
	}

	// Wild guess for Diffuse type
	if (type == WZM_TEX_DIFFUSE)
	{
		QFileInfo nfo(modelnfo.absolutePath() + "/" + modelnfo.completeBaseName() + ".png");
		if (nfo.exists())
			return nfo.filePath();
	}
	return QString();
}

void TextureDialog::scanForTexturesInDirs(const QStringList& dirs)
{
	m_predefined_textures.clear();

	foreach (QString path, dirs)
	{
		QDir dir(path, "*.png", QDir::Type, QDir::Files);
		QStringList textures = dir.entryList();
		foreach (QString texture, textures)
		{
			m_predefined_textures.append(dir.absoluteFilePath(texture));
		}
		ui->lwPredefined->addItems(m_predefined_textures);
	}

	filePredefinedList(ui->leFilter->text());
}

void TextureDialog::filePredefinedList(const QString& filter)
{
	ui->lwPredefined->clear();

	if (filter.isEmpty())
	{
		ui->lwPredefined->addItems(m_predefined_textures);
	}
	else
	{
		ui->lwPredefined->addItems(m_predefined_textures.filter(filter));
	}
}

void TextureDialog::iconDoubleClicked(QListWidgetItem *icon)
{
	QString newtex = selectTextureFile();
	if (!newtex.isEmpty())
	{
		icon->setData(Qt::UserRole + 1, newtex);

		QPixmap img(newtex);
		QString ttp = "Path: " + newtex + "\nWidth: " + QString::number(img.width())+ ", Height: " + QString::number(img.height());
		icon->setToolTip(ttp);
		icon->setIcon(QIcon(img));
	}
}

// FIXME: probably this is NOT a good way to do it
void TextureDialog::on_pbAddType_clicked()
{
	QString item = ui->cbType->currentText();
	wzm_texture_type_t textype = types[item];
	if (!m_icons.contains(textype))
	{
		addTextureIcon(textype);
	}
}

void TextureDialog::on_pbRemoveType_clicked()
{
	QListWidgetItem *itm = ui->lwTextures->currentItem();
	if (itm)
	{
		m_icons.remove(static_cast<wzm_texture_type_t>(itm->data(Qt::UserRole).toInt()));
		delete itm;
	}
}

void TextureDialog::on_leFilter_textChanged(QString text)
{
	filePredefinedList(text);
}

void TextureDialog::on_lwPredefined_itemClicked(QListWidgetItem* item)
{
	QListWidgetItem* icon = ui->lwTextures->currentItem();
	if (item && icon)
	{
		addTextureIcon(static_cast<wzm_texture_type_t>(icon->data(Qt::UserRole).toInt()),
			       item->text());
	}
}

void TextureDialog::on_pbConfig_clicked()
{
	m_texConfigDialog->show();
}

void TextureDialog::onFinished(int)
{
	m_settings.setValue("TextureDialog/size", size());
	m_settings.setValue("TextureDialog/position", pos());
}
