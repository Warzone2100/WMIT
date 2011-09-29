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
	ui(new Ui::TransformDock), m_selected_mesh(-1)
{
	 ui->setupUi(this);

	 ui->cbMeshIdx->setEditable(false);
	 setMeshCount(0, QStringList());

	 reset(true);
}

TransformDock::~TransformDock()
{
	delete ui;
}

void TransformDock::reset(bool reset_prev_values)
{
	// reset preview values
	scale_all = scale_xyz[0] = scale_xyz[1] = scale_xyz[2] = 1.;
	if (reset_prev_values)
		scale_all_prev = scale_xyz_prev[0] = scale_xyz_prev[1] = scale_xyz_prev[2] = 1.;

	// UI
	setScaleValueOnUI(scale_all);
}

void TransformDock::setMeshCount(int value, QStringList names)
{
	int selected = ui->cbMeshIdx->currentIndex();

	ui->cbMeshIdx->blockSignals(true);

	ui->cbMeshIdx->clear();
	ui->cbMeshIdx->addItem("All meshes");
	for (int i = 1; i <= value; ++i)
	{
		ui->cbMeshIdx->addItem(QString::number(i) + " [" + names.value(i - 1) + "]");
	}

	if (selected <= value)
	{
		if (selected < 0)
		{
			selected = 0;
		}
		ui->cbMeshIdx->setCurrentIndex(selected);
	}

	ui->cbMeshIdx->blockSignals(false);
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

	// reset now
	reset();
}

void TransformDock::setScaleValueOnUI(double value)
{
	ui->doubleSpinBox->blockSignals(true);
	ui->doubleSpinBox->setValue(value);
	ui->doubleSpinBox->blockSignals(false);
	ui->horizontalSlider->blockSignals(true);
	ui->horizontalSlider->setValue(value);
	ui->horizontalSlider->blockSignals(false);
}

void TransformDock::closeEvent(QCloseEvent *event)
{
	acceptTransformations();
	event->accept();
}

void TransformDock::on_pb_revWindings_clicked()
{
	emit reverseWindings(m_selected_mesh);
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
		setScaleValueOnUI(scale_all);
		break;
	case 1: // X
		setScaleValueOnUI(scale_xyz[0]);
		break;
	case 2: // Y
		setScaleValueOnUI(scale_xyz[1]);
		break;
	case 3: // Z
		setScaleValueOnUI(scale_xyz[2]);
		break;
	}
}

void TransformDock::on_cbMeshIdx_currentIndexChanged(int index)
{
	if (index < 0)
		return;

	acceptTransformations();

	m_selected_mesh = index - 1; // all is 0, 1st is 1 and so on... -1 to corresponding mesh

	emit setActiveMeshIdx(m_selected_mesh);
}

void TransformDock::on_pbMirrorX_clicked()
{
	emit mirrorAxis(ui->cbGlobalMirror->isChecked() ? 3 : 0);
}

void TransformDock::on_pbMirrorY_clicked()
{
	emit mirrorAxis(ui->cbGlobalMirror->isChecked() ? 4 : 1);
}

void TransformDock::on_pbMirrorZ_clicked()
{
	emit mirrorAxis(ui->cbGlobalMirror->isChecked() ? 5 : 2);
}
