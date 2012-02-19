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

#include "TransformDock.h"
#include "ui_TransformDock.h"

TransformDock::TransformDock(QWidget *parent) :
	QDockWidget(parent),
	ui(new Ui::TransformDock), m_selected_mesh(-1)
{
	 ui->setupUi(this);

	 ui->meshComboBox->setEditable(false);
	 setMeshCount(0, QStringList());

	 ui->removeMeshButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon, 0, ui->removeMeshButton));

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
	int selected = ui->meshComboBox->currentIndex();

	ui->meshComboBox->blockSignals(true);

	ui->meshComboBox->clear();
	ui->meshComboBox->addItem("All meshes");
	for (int i = 1; i <= value; ++i)
	{
		ui->meshComboBox->addItem(QString::number(i) + " [" + names.value(i - 1) + "]");
	}

	if (selected > value || selected < 0)
	{
		selected = 0;
	}
	ui->meshComboBox->setCurrentIndex(selected);

	ui->meshComboBox->blockSignals(false);

	on_meshComboBox_currentIndexChanged(selected); // force this because of possible mesh stack pop
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
	ui->scaleSpinBox->blockSignals(true);
	ui->scaleSpinBox->setValue(value);
	ui->scaleSpinBox->blockSignals(false);
	ui->scaleSlider->blockSignals(true);
	ui->scaleSlider->setValue(value);
	ui->scaleSlider->blockSignals(false);
}

void TransformDock::closeEvent(QCloseEvent *event)
{
	acceptTransformations();
	event->accept();
}

void TransformDock::on_reverseWindingsButton_clicked()
{
	emit reverseWindings(m_selected_mesh);
}

void TransformDock::on_scaleSpinBox_valueChanged(double value)
{
	if (ui->scaleSpinBox->value() != value)
	{
		ui->scaleSpinBox->setValue(value);
	}
	switch(ui->scaleComboBox->currentIndex())
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

void TransformDock::on_scaleSlider_valueChanged(int value)
{
	if (ui->scaleSpinBox->value() != value)
	{
		ui->scaleSpinBox->setValue(value);
	}
}

void TransformDock::on_scaleComboBox_currentIndexChanged(int index)
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

void TransformDock::on_meshComboBox_currentIndexChanged(int index)
{
	ui->removeMeshButton->setDisabled(!index || ui->meshComboBox->count() <= 2); // FIXME: can't remove all and shouldn't remove last remaining mesh

	if (index < 0)
		return;

	acceptTransformations();

	m_selected_mesh = index - 1; // all is 0, 1st is 1 and so on... -1 to corresponding mesh

	emit setActiveMeshIdx(m_selected_mesh);
}

void TransformDock::on_mirrorXButton_clicked()
{
	emit mirrorAxis(ui->globalMirrorCheckBox->isChecked() ? 3 : 0);
}

void TransformDock::on_mirrorYButton_clicked()
{
	emit mirrorAxis(ui->globalMirrorCheckBox->isChecked() ? 4 : 1);
}

void TransformDock::on_mirrorZButton_clicked()
{
	emit mirrorAxis(ui->globalMirrorCheckBox->isChecked() ? 5 : 2);
}

void TransformDock::on_removeMeshButton_clicked()
{
	emit removeMeshIdx(m_selected_mesh);
}
