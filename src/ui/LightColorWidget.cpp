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

#include "LightColorWidget.h"
#include "ui_LightColorWidget.h"

#include <QColorDialog>

LightColorWidget::LightColorWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::LightColorWidget)
{
	m_ui->setupUi(this);

	m_ui->colorTypeComboBox->addItem("Diffuse", static_cast<int>(LIGHT_DIFFUSE));
	m_ui->colorTypeComboBox->addItem("Ambient", static_cast<int>(LIGHT_AMBIENT));
	m_ui->colorTypeComboBox->addItem("Specular", static_cast<int>(LIGHT_SPECULAR));
	m_ui->colorTypeComboBox->addItem("Emissive", static_cast<int>(LIGHT_EMISSIVE));

	connect(m_ui->colorRedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeRedComponent(double)));
	connect(m_ui->colorRedSlider, SIGNAL(valueChanged(int)), this, SLOT(changeRedComponent(int)));
	connect(m_ui->colorGreenSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeGreenComponent(double)));
	connect(m_ui->colorGreenSlider, SIGNAL(valueChanged(int)), this, SLOT(changeGreenComponent(int)));
	connect(m_ui->colorBlueSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeBlueComponent(double)));
	connect(m_ui->colorBlueSlider, SIGNAL(valueChanged(int)), this, SLOT(changeBlueComponent(int)));

	// disable wip parts
	m_ui->colorAllCheckBox->setDisabled(true);
}

LightColorWidget::~LightColorWidget()
{
	delete m_ui;
}

void LightColorWidget::changeEvent(QEvent *event)
{
	QWidget::changeEvent(event);

	switch (event->type())
	{
	case QEvent::LanguageChange:
		m_ui->retranslateUi(this);

		break;
	default:
		break;
	}
}

void LightColorWidget::setLightColors(const light_cols_t &light_cols)
{
	m_colors = light_cols;
	on_colorTypeComboBox_currentIndexChanged(m_ui->colorTypeComboBox->currentIndex());
}

void LightColorWidget::setColorOnUI(const QColor& color)
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

void LightColorWidget::applyColor(const QColor &color)
{
	if (color.isValid())
	{
		setColorOnUI(color);

		LIGHTING_TYPE type = static_cast<LIGHTING_TYPE>(
			m_ui->colorTypeComboBox->itemData(m_ui->colorTypeComboBox->currentIndex()).toInt());
		m_colors[type][0] = color.redF();
		m_colors[type][1] = color.greenF();
		m_colors[type][2] = color.blueF();

		emit colorsChanged(m_colors);
	}
}

void LightColorWidget::on_colorTypeComboBox_currentIndexChanged(int index)
{
	LIGHTING_TYPE type = static_cast<LIGHTING_TYPE>(m_ui->colorTypeComboBox->itemData(index).toInt());
	setColorOnUI(QColor::fromRgbF(m_colors[type][0], m_colors[type][1], m_colors[type][2]));
}

void LightColorWidget::on_colorDialogButton_clicked()
{
	applyColor(QColorDialog::getColor(QColor::fromRgbF(m_ui->colorRedSpinBox->value(),
				   m_ui->colorGreenSpinBox->value(), m_ui->colorBlueSpinBox->value()), this));
}

void LightColorWidget::changeRedComponent(int value)
{
	double dval = value / 255.0;
	if (m_ui->colorRedSpinBox->value() != dval)
	{
		m_ui->colorRedSpinBox->setValue(dval);
	}
}

void LightColorWidget::changeRedComponent(double value)
{
	if (m_ui->colorRedSlider->value() != value * 255)
	{
		m_ui->colorRedSlider->setValue(value * 255);
	}

	applyColor(QColor::fromRgbF(value, m_ui->colorGreenSpinBox->value(), m_ui->colorBlueSpinBox->value()));
}

void LightColorWidget::changeGreenComponent(int value)
{
	double dval = value / 255.0;
	if (m_ui->colorGreenSpinBox->value() != dval)
	{
		m_ui->colorGreenSpinBox->setValue(dval);
	}
}

void LightColorWidget::changeGreenComponent(double value)
{
	if (m_ui->colorGreenSlider->value() != value * 255)
	{
		m_ui->colorGreenSlider->setValue(value * 255);
	}

	applyColor(QColor::fromRgbF(m_ui->colorRedSpinBox->value(), value, m_ui->colorBlueSpinBox->value()));
}

void LightColorWidget::changeBlueComponent(int value)
{
	double dval = value / 255.0;
	if (m_ui->colorBlueSpinBox->value() != dval)
	{
		m_ui->colorBlueSpinBox->setValue(dval);
	}
}

void LightColorWidget::changeBlueComponent(double value)
{
	if (m_ui->colorBlueSlider->value() != value * 255)
	{
		m_ui->colorBlueSlider->setValue(value * 255);
	}

	applyColor(QColor::fromRgbF(m_ui->colorRedSpinBox->value(), m_ui->colorGreenSpinBox->value(), value));
}
