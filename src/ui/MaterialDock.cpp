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

	m_ui->colorTypeComboBox->addItem("Diffuse", static_cast<int>(WZM_MAT_DIFFUSE));
	m_ui->colorTypeComboBox->addItem("Ambient", static_cast<int>(WZM_MAT_AMBIENT));
	m_ui->colorTypeComboBox->addItem("Specular", static_cast<int>(WZM_MAT_SPECULAR));
	m_ui->colorTypeComboBox->addItem("Emissive", static_cast<int>(WZM_MAT_EMISSIVE));

	connect(m_ui->colorRedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeRedComponent(double)));
	connect(m_ui->colorRedSlider, SIGNAL(valueChanged(int)), this, SLOT(changeRedComponent(int)));
	connect(m_ui->colorGreenSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeGreenComponent(double)));
	connect(m_ui->colorGreenSlider, SIGNAL(valueChanged(int)), this, SLOT(changeGreenComponent(int)));
	connect(m_ui->colorBlueSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeBlueComponent(double)));
	connect(m_ui->colorBlueSlider, SIGNAL(valueChanged(int)), this, SLOT(changeBlueComponent(int)));

	connect(m_ui->shininessDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeShininess(double)));
	connect(m_ui->shininessSlider, SIGNAL(valueChanged(int)), this, SLOT(changeShininess(int)));

	// disable wip parts
	m_ui->colorAllCheckBox->setDisabled(true);
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

void MaterialDock::setMaterial(const WZMaterial &mat)
{
	m_material = mat;

	wzm_material_t type = static_cast<wzm_material_t>(m_ui->colorTypeComboBox->itemData(m_ui->colorTypeComboBox->currentIndex()).toInt());

    setColorOnUI(QColor::fromRgbF(m_material.vals[type].x(), m_material.vals[type].y(), m_material.vals[type].z()));
	setShininessOnUI(m_material.shininess);
}

void MaterialDock::setColorOnUI(const QColor& color)
{
	QPixmap pix(5, 5);
	pix.fill(color);

	m_ui->colorPreview->setPixmap(pix);

	m_ui->colorRedSpinBox->blockSignals(true);
	m_ui->colorRedSpinBox->setValue(color.redF());
	m_ui->colorRedSpinBox->blockSignals(false);
	m_ui->colorRedSlider->blockSignals(true);
	m_ui->colorRedSlider->setValue(color.red());
	m_ui->colorRedSlider->blockSignals(false);

	m_ui->colorGreenSpinBox->blockSignals(true);
	m_ui->colorGreenSpinBox->setValue(color.greenF());
	m_ui->colorGreenSpinBox->blockSignals(false);
	m_ui->colorGreenSlider->blockSignals(true);
	m_ui->colorGreenSlider->setValue(color.green());
	m_ui->colorGreenSlider->blockSignals(false);

	m_ui->colorBlueSpinBox->blockSignals(true);
	m_ui->colorBlueSpinBox->setValue(color.blueF());
	m_ui->colorBlueSpinBox->blockSignals(false);
	m_ui->colorBlueSlider->blockSignals(true);
	m_ui->colorBlueSlider->setValue(color.blue());
	m_ui->colorBlueSlider->blockSignals(false);
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

void MaterialDock::applyColor(const QColor &color)
{
	if (color.isValid())
	{
		setColorOnUI(color);

		wzm_material_t type = static_cast<wzm_material_t>(m_ui->colorTypeComboBox->itemData(m_ui->colorTypeComboBox->currentIndex()).toInt());
        m_material.vals[type].x() = color.redF();
        m_material.vals[type].y() = color.greenF();
        m_material.vals[type].z() = color.blueF();

		emit materialChanged(m_material);
	}
}

void MaterialDock::on_colorTypeComboBox_currentIndexChanged(int index)
{
	wzm_material_t type = static_cast<wzm_material_t>(m_ui->colorTypeComboBox->itemData(index).toInt());
    setColorOnUI(QColor::fromRgbF(m_material.vals[type].x(), m_material.vals[type].y(), m_material.vals[type].z()));
}

void MaterialDock::on_colorDialogButton_clicked()
{
	applyColor(QColorDialog::getColor(QColor::fromRgbF(m_ui->colorRedSpinBox->value(),
				   m_ui->colorGreenSpinBox->value(), m_ui->colorBlueSpinBox->value()), this));
}

void MaterialDock::changeRedComponent(int value)
{
	double dval = value / 255.0;
	if (m_ui->colorRedSpinBox->value() != dval)
	{
		m_ui->colorRedSpinBox->setValue(dval);
	}
}

void MaterialDock::changeRedComponent(double value)
{
	if (m_ui->colorRedSlider->value() != value * 255)
	{
		m_ui->colorRedSlider->setValue(value * 255);
	}

	applyColor(QColor::fromRgbF(value, m_ui->colorGreenSpinBox->value(), m_ui->colorBlueSpinBox->value()));
}

void MaterialDock::changeGreenComponent(int value)
{
	double dval = value / 255.0;
	if (m_ui->colorGreenSpinBox->value() != dval)
	{
		m_ui->colorGreenSpinBox->setValue(dval);
	}
}

void MaterialDock::changeGreenComponent(double value)
{
	if (m_ui->colorGreenSlider->value() != value * 255)
	{
		m_ui->colorGreenSlider->setValue(value * 255);
	}

	applyColor(QColor::fromRgbF(m_ui->colorRedSpinBox->value(), value, m_ui->colorBlueSpinBox->value()));
}

void MaterialDock::changeBlueComponent(int value)
{
	double dval = value / 255.0;
	if (m_ui->colorBlueSpinBox->value() != dval)
	{
		m_ui->colorBlueSpinBox->setValue(dval);
	}
}

void MaterialDock::changeBlueComponent(double value)
{
	if (m_ui->colorBlueSlider->value() != value * 255)
	{
		m_ui->colorBlueSlider->setValue(value * 255);
	}

	applyColor(QColor::fromRgbF(m_ui->colorRedSpinBox->value(), m_ui->colorGreenSpinBox->value(), value));
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
