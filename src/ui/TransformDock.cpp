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
	m_selected_mesh(-1),
	m_ui(new Ui::TransformDock) 
{
	m_ui->setupUi(this);
	m_ui->removeMeshButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

	setMeshCount(0, QStringList());
	reset(true);

	connect(m_ui->scaleSlider, SIGNAL(valueChanged(int)), this, SLOT(setScale(int)));
	connect(m_ui->scaleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setScale(double)));
	connect(m_ui->scaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectScale(int)));
	connect(m_ui->meshComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectMesh(int)));
	connect(m_ui->removeMeshButton, SIGNAL(clicked()), this, SLOT(removeMesh()));
	connect(m_ui->reverseWindingsButton, SIGNAL(clicked()), this, SLOT(reverseWindings()));
	connect(m_ui->flipNormalsButton, SIGNAL(clicked()), this, SLOT(flipNormals()));
	connect(m_ui->mirrorXButton, SIGNAL(clicked()), this, SLOT(mirrorX()));
	connect(m_ui->mirrorYButton, SIGNAL(clicked()), this, SLOT(mirrorY()));
	connect(m_ui->mirrorZButton, SIGNAL(clicked()), this, SLOT(mirrorZ()));
	connect(m_ui->centerMeshButton, SIGNAL(clicked()), this, SLOT(centerMesh()));
}

TransformDock::~TransformDock()
{
	delete m_ui;
}

void TransformDock::changeEvent(QEvent *event)
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

void TransformDock::closeEvent(QCloseEvent *event)
{
	acceptTransformations();

	event->accept();
}

void TransformDock::reset(bool reset_prev_values)
{
	// reset preview values
	m_scale_all = m_scale_xyz[0] = m_scale_xyz[1] = m_scale_xyz[2] = 1.;

	if (reset_prev_values)
	{
		m_scale_all_prev = m_scale_xyz_prev[0] = m_scale_xyz_prev[1] = m_scale_xyz_prev[2] = 1.;
	}

	// UI
	setScaleValueOnUI(m_scale_all);
}

void TransformDock::setMeshCount(int value, QStringList names)
{
	int selected = m_ui->meshComboBox->currentIndex();

	m_ui->meshComboBox->blockSignals(true);
	m_ui->meshComboBox->clear();
	m_ui->meshComboBox->addItem("All meshes");

	for (int i = 1; i <= value; ++i)
	{
		m_ui->meshComboBox->addItem(QString::number(i) + " [" + names.value(i - 1) + "]");
	}

	if (selected > value || selected < 0)
	{
		selected = 0;
	}

	m_ui->meshComboBox->setCurrentIndex(selected);
	m_ui->meshComboBox->blockSignals(false);

	selectMesh(selected); // force this because of possible mesh stack pop
}

void TransformDock::setMirrorState(bool enabled)
{
	m_ui->gbMirror->setEnabled(enabled);
}

void TransformDock::acceptTransformations()
{
	// save and apply
	m_scale_all_prev = m_scale_all;
	m_scale_xyz_prev[0] = m_scale_xyz[0];
	m_scale_xyz_prev[1] = m_scale_xyz[1];
	m_scale_xyz_prev[2] = m_scale_xyz[2];

	emit applyTransformations();

	// reset now
	reset();
}

void TransformDock::setScaleValueOnUI(double value)
{
	m_ui->scaleSpinBox->blockSignals(true);
	m_ui->scaleSpinBox->setValue(value);
	m_ui->scaleSpinBox->blockSignals(false);
	m_ui->scaleSlider->blockSignals(true);
	m_ui->scaleSlider->setValue(value);
	m_ui->scaleSlider->blockSignals(false);
}

void TransformDock::reverseWindings()
{
	emit reverseWindings(m_selected_mesh);
}

void TransformDock::flipNormals()
{
	emit flipNormals(m_selected_mesh);
}

void TransformDock::setScale(int value)
{
	if (m_ui->scaleSpinBox->value() != value)
	{
		m_ui->scaleSpinBox->setValue(value);
	}
}

void TransformDock::setScale(double value)
{
	if (m_ui->scaleSlider->value() != value)
	{
		m_ui->scaleSlider->setValue(value);
	}

	switch(m_ui->scaleComboBox->currentIndex())
	{
	case 0: //XYZ
		m_scale_all = value;

		emit scaleXYZChanged(value);

		break;
	case 1: // X
		m_scale_xyz[0] = value;

		emit scaleXChanged(value);

		break;
	case 2: // Y
		m_scale_xyz[1] = value;
	
		emit scaleYChanged(value);
	
		break;
	case 3: // Z
		m_scale_xyz[2] = value;

		emit scaleZChanged(value);

		break;
	}
}

void TransformDock::selectScale(int index)
{
	switch(index)
	{
	case 0: //XYZ
		setScaleValueOnUI(m_scale_all);

		break;
	case 1: // X
		setScaleValueOnUI(m_scale_xyz[0]);

		break;
	case 2: // Y
		setScaleValueOnUI(m_scale_xyz[1]);

		break;
	case 3: // Z
		setScaleValueOnUI(m_scale_xyz[2]);

		break;
	}
}

void TransformDock::selectMesh(int index)
{
	m_ui->removeMeshButton->setDisabled(!index || m_ui->meshComboBox->count() <= 2); // FIXME: can't remove all and shouldn't remove last remaining mesh

	if (index < 0)
	{
		return;
	}

	acceptTransformations();

	m_selected_mesh = index - 1; // all is 0, 1st is 1 and so on... -1 to corresponding mesh

	emit changeActiveMesh(m_selected_mesh);
}

void TransformDock::removeMesh()
{
	emit removeMesh(m_selected_mesh);
}

void TransformDock::mirrorX()
{
	emit mirrorAxis(m_ui->globalMirrorCheckBox->isChecked() ? 3 : 0);
}

void TransformDock::mirrorY()
{
	emit mirrorAxis(m_ui->globalMirrorCheckBox->isChecked() ? 4 : 1);
}

void TransformDock::mirrorZ()
{
	emit mirrorAxis(m_ui->globalMirrorCheckBox->isChecked() ? 5 : 2);
}

void TransformDock::centerMesh()
{
	emit centerMesh(m_selected_mesh, m_ui->scaleComboBox->currentIndex() - 1);
}
