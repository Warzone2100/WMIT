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

#include "ImportDialog.h"
#include "ui_ImportDialog.h"


#include "wmit.h"

ImportDialog::ImportDialog(QWidget *parent):
	QDialog(parent),
	m_ui(new Ui::ImportDialog)
{
	m_ui->setupUi(this);

	m_ui->cb_EnableWelder->setChecked(m_settings.value(WMIT_SETTINGS_IMPORT_WELDER, true).toBool());

	// disable wip parts
	m_ui->gb_CoordinateSystem->setDisabled(true);
}

ImportDialog::~ImportDialog()
{
	if (result() == QDialog::Accepted)
	{
		m_settings.setValue(WMIT_SETTINGS_IMPORT_WELDER, m_ui->cb_EnableWelder->isChecked());
	}
	delete m_ui;
}

void ImportDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		m_ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
