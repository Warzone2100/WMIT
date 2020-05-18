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

#include <QColorDialog>

MaterialDock::MaterialDock(QWidget *parent) :
	QDockWidget(parent),
	m_ui(new Ui::MaterialDock)
{
	m_ui->setupUi(this);

	connect(m_ui->colorWidget, SIGNAL(colorsChanged(light_cols_t)), this, SLOT(colorsChanged(light_cols_t)));

	connect(m_ui->shininessDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeShininess(double)));
	connect(m_ui->shininessSlider, SIGNAL(valueChanged(int)), this, SLOT(changeShininess(int)));
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

void copyMaterialColors(const WZMVertex &mat_col, light_col_t& cols)
{
	for (size_t idx = 0; idx < 4; ++idx) {
		cols[idx] = mat_col[idx];
	}
}

void MaterialDock::setMaterial(const WZMaterial &mat)
{
	m_material = mat;

	light_cols_t cols;
	copyMaterialColors(mat.vals[WZM_MAT_AMBIENT], cols[LIGHT_AMBIENT]);
	copyMaterialColors(mat.vals[WZM_MAT_EMISSIVE], cols[LIGHT_EMISSIVE]);
	copyMaterialColors(mat.vals[WZM_MAT_DIFFUSE], cols[LIGHT_DIFFUSE]);
	copyMaterialColors(mat.vals[WZM_MAT_SPECULAR], cols[LIGHT_SPECULAR]);

	m_ui->colorWidget->setLightColors(cols);

	setShininessOnUI(m_material.shininess);
}

void MaterialDock::setShininessOnUI(const float shininess)
{
	m_ui->shininessDoubleSpinBox->blockSignals(true);
	m_ui->shininessDoubleSpinBox->setValue(shininess);
	m_ui->shininessDoubleSpinBox->blockSignals(false);

	m_ui->shininessSlider->blockSignals(true);
	m_ui->shininessSlider->setValue(shininess * 10);
	m_ui->shininessSlider->blockSignals(false);
}

void MaterialDock::changeShininess(int value)
{
	double dval = value / 10.0;
	if (m_ui->shininessDoubleSpinBox->value() != dval)
	{
		m_ui->shininessDoubleSpinBox->setValue(dval);
	}
}

void MaterialDock::changeShininess(double value)
{
	if (m_ui->shininessSlider->value() != value * 10)
	{
		m_ui->shininessSlider->setValue(value * 10);
	}

	m_material.shininess = value;
	emit materialChanged(m_material);
}

void setMaterialColors(WZMVertex &mat_col, const light_col_t& cols)
{
	for (size_t idx = 0; idx < 4; ++idx) {
		mat_col[idx] = cols[idx];
	}
}

void MaterialDock::colorsChanged(const light_cols_t &light_cols)
{
	setMaterialColors(m_material.vals[WZM_MAT_AMBIENT], light_cols[LIGHT_AMBIENT]);
	setMaterialColors(m_material.vals[WZM_MAT_EMISSIVE], light_cols[LIGHT_EMISSIVE]);
	setMaterialColors(m_material.vals[WZM_MAT_DIFFUSE], light_cols[LIGHT_DIFFUSE]);
	setMaterialColors(m_material.vals[WZM_MAT_SPECULAR], light_cols[LIGHT_SPECULAR]);

	emit materialChanged(m_material);
}
