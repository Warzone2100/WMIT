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
#ifndef TRANSFORMDOCK_HPP
#define TRANSFORMDOCK_HPP

#include <QDockWidget>
#include <QCloseEvent>

namespace Ui {
    class TransformDock;
}

class TransformDock : public QDockWidget
{
    Q_OBJECT
public:
	TransformDock(QWidget *parent = 0);
	~TransformDock();

	void reset(bool reset_prev_values = false);

signals:
	void scaleXYZChanged(double);
	void scaleXChanged(double);
	void scaleYChanged(double);
	void scaleZChanged(double);
	void reverseWindings(int mesh);
	void mirrorAxis(int);

	void applyTransformations();
	void setActiveMeshIdx(int);
	void removeMeshIdx(int);

public slots:
	void setMeshCount(int value, QStringList names);

protected:
	void changeEvent(QEvent *e);
	virtual void closeEvent(QCloseEvent *event);

private:
	Ui::TransformDock *ui;
	int m_selected_mesh;
	double scale_all, scale_xyz[3];
	double scale_all_prev, scale_xyz_prev[3];

	void acceptTransformations();
	void setScaleValueOnUI(double value);

private slots:
	void on_scaleComboBox_currentIndexChanged(int index);
	void on_scaleSlider_valueChanged(int value);
	void on_scaleSpinBox_valueChanged(double );
	void on_reverseWindingsButton_clicked();
	void on_meshComboBox_currentIndexChanged(int index);
	void on_mirrorXButton_clicked();
	void on_mirrorYButton_clicked();
	void on_mirrorZButton_clicked();
	void on_removeMeshButton_clicked();
};

#endif // TRANSFORMDOCK_HPP
