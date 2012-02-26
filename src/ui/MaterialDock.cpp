/*
	Copyright 2012 Warzone 2100 Project

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

#include "MaterialDock.h"
#include "ui_MaterialDock.h"

MaterialDock::MaterialDock(QWidget *parent) :
	QDockWidget(parent),
	m_ui(new Ui::MaterialDock)
{
	m_ui->setupUi(this);
}

MaterialDock::~MaterialDock()
{
	delete m_ui;
}

void MaterialDock::changeEvent(QEvent *event)
{
	QDockWidget::changeEvent(event);

	switch (event->type())
	{
	case QEvent::LanguageChange:
		m_ui->retranslateUi(this);

		break;
	default:
		break;
	}
}
