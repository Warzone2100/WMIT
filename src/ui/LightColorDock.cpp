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

#include "LightColorDock.h"
#include "ui_LightColorDock.h"

#include <QColorDialog>

LightColorDock::LightColorDock(light_cols_t &light_cols, QWidget *parent) :
	QDockWidget(parent),
	m_ui(new Ui::LightColorDock),
	m_light_cols(light_cols)
{
	m_ui->setupUi(this);

	useCustomColors(false);
	connect(m_ui->chkCustomColors, SIGNAL(stateChanged(int)),
		this, SLOT(useCustomColorsChangedOnWidget(int)));

	refreshColorUI();
	connect(m_ui->colorWidget, SIGNAL(colorsChanged(light_cols_t)),
		this, SLOT(colorsChangedOnWidget(light_cols_t)));
}

LightColorDock::~LightColorDock()
{
	delete m_ui;
}

void LightColorDock::changeEvent(QEvent *event)
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

void LightColorDock::colorsChangedOnWidget(const light_cols_t &light_cols)
{
	m_light_cols = light_cols;
	emit colorsChanged();
}

void LightColorDock::useCustomColorsChangedOnWidget(int val)
{
	emit useCustomColorsChanged(val == static_cast<int>(Qt::CheckState::Checked));
}

void LightColorDock::refreshColorUI()
{
	m_ui->colorWidget->setLightColors(m_light_cols);
}

void LightColorDock::useCustomColors(const bool useState)
{
	m_ui->chkCustomColors->setCheckState(useState ?
		Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}
