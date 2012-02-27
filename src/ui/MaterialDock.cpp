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

#include "WZM.h"

MaterialDock::MaterialDock(QWidget *parent) :
	QDockWidget(parent),
	m_ui(new Ui::MaterialDock)
{
	m_ui->setupUi(this);

	setMaterialColor(WZM_MAT_AMBIENT, QColor::fromRgbF(0.5, 0.5, 0.5));
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

void MaterialDock::setMaterialColor(const int type, const QColor color)
{
	QPixmap pix(5, 5);
	pix.fill(color);

	switch (static_cast<wzm_material_t>(type))
	{
	case WZM_MAT_AMBIENT:
		m_ui->ambientColor->setPixmap(pix);
		break;
	}
}
