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

#include "TransformDock.hpp"
#include "ui_TransformDock.h"

TransformDock::TransformDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::TransformDock)
{
	 scale_all = scale_xyz[0] = scale_xyz[1] = scale_xyz[2] = 1.;
	 scale_all_prev = scale_xyz_prev[0] = scale_xyz_prev[1] = scale_xyz_prev[2] = 1.;

	 ui->setupUi(this);
	 ui->doubleSpinBox->setValue(scale_all);
	 ui->horizontalSlider->setValue(scale_all);
}

TransformDock::~TransformDock()
{
    delete ui;
}

void TransformDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void TransformDock::acceptTransformations()
{
	// save and apply
	scale_all_prev = scale_all;
	scale_xyz_prev[0] = scale_xyz[0];
	scale_xyz_prev[1] = scale_xyz[1];
	scale_xyz_prev[2] = scale_xyz[2];

	emit applyTransformations();

	// reset preview values
	scale_all = scale_xyz[0] = scale_xyz[1] = scale_xyz[2] = 1.;

	emit scaleXYZChanged(scale_all);
	emit scaleXChanged(scale_xyz[0]);
	emit scaleYChanged(scale_xyz[1]);
	emit scaleZChanged(scale_xyz[2]);
}

void TransformDock::closeEvent(QCloseEvent *event)
{
	acceptTransformations();
	event->accept();
}

void TransformDock::on_pb_revWindings_clicked()
{
	emit reverseWindings();
}

void TransformDock::on_doubleSpinBox_valueChanged(double value)
{
	if (ui->doubleSpinBox->value() != value)
	{
		ui->doubleSpinBox->setValue(value);
	}
	switch(ui->comboBox->currentIndex())
	{
	case 0: //XYZ
		scale_all = value;
		emit scaleXYZChanged(value);
		break;
	case 1: // X
		scale_xyz[0] = value;
		emit scaleXChanged(value);
		break;
	case 2: // Y
		scale_xyz[1] = value;
		emit scaleYChanged(value);
		break;
	case 3: // Z
		scale_xyz[2] = value;
		emit scaleZChanged(value);
		break;
	}
}

void TransformDock::on_horizontalSlider_valueChanged(int value)
{
	if (ui->doubleSpinBox->value() != value)
	{
		ui->doubleSpinBox->setValue(value);
	}
}

void TransformDock::on_comboBox_currentIndexChanged(int index)
{
	switch(index)
	{
	case 0: //XYZ
		ui->doubleSpinBox->blockSignals(true);
		ui->doubleSpinBox->setValue(scale_all);
		ui->doubleSpinBox->blockSignals(false);
		ui->horizontalSlider->blockSignals(true);
		ui->horizontalSlider->setValue(scale_all);
		ui->horizontalSlider->blockSignals(false);
		break;
	case 1: // X
		ui->doubleSpinBox->blockSignals(true);
		ui->doubleSpinBox->setValue(scale_xyz[0]);
		ui->doubleSpinBox->blockSignals(false);
		ui->horizontalSlider->blockSignals(true);
		ui->horizontalSlider->setValue(scale_xyz[0]);
		ui->horizontalSlider->blockSignals(false);
		break;
	case 2: // Y
		ui->doubleSpinBox->blockSignals(true);
		ui->doubleSpinBox->setValue(scale_xyz[1]);
		ui->doubleSpinBox->blockSignals(false);
		ui->horizontalSlider->blockSignals(true);
		ui->horizontalSlider->setValue(scale_xyz[1]);
		ui->horizontalSlider->blockSignals(false);
		break;
	case 3: // Z
		ui->doubleSpinBox->blockSignals(true);
		ui->doubleSpinBox->setValue(scale_xyz[2]);
		ui->doubleSpinBox->blockSignals(false);
		ui->horizontalSlider->blockSignals(true);
		ui->horizontalSlider->setValue(scale_xyz[2]);
		ui->horizontalSlider->blockSignals(false);
		break;
	}
}
