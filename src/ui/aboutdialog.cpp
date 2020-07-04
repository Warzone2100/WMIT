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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "wmit.h"

#include <QFile>

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);
	QPixmap logo;
	logo.load(WMIT_IMAGES_BANNER);
	ui->lblLogo->setPixmap(logo);

	setWindowTitle(tr("About WMIT %1").arg(WMIT_VER_STR));

	m_licenseText = ui->textEdit->toHtml();
}

AboutDialog::~AboutDialog()
{
	delete ui;
}

void AboutDialog::on_pbCredits_clicked()
{
	QFile resfile(":/AUTHORS");
	resfile.open(QIODevice::ReadOnly);
	QString text(resfile.readAll());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	ui->textEdit->setMarkdown(text);
#else
	ui->textEdit->setText(text);
#endif
}

void AboutDialog::on_pbLicense_clicked()
{
	ui->textEdit->setHtml(m_licenseText);
}
