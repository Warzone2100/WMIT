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

namespace Ui
{
	class TransformDock;
}

class TransformDock : public QDockWidget
{
	Q_OBJECT

public:
	TransformDock(QWidget *parent = nullptr);
	~TransformDock();

	void reset(bool reset_prev_values = false);

public slots:
	void setMeshCount(int value, QStringList names);
	void setMirrorState(bool enabled);

protected:
	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent *event);

private:
	double m_scale_all;
	double m_scale_xyz[3];
	double m_scale_all_prev;
	double m_scale_xyz_prev[3];
	int m_selected_mesh;
	Ui::TransformDock *m_ui;

	void acceptTransformations();
	void setScaleValueOnUI(double value);

private slots:
	void selectScale(int index);
	void setScale(int value);
	void setScale(double value);
	void reverseWindings();
	void flipNormals();
	void selectMesh(int index);
	void removeMesh();
	void mirrorX();
	void mirrorY();
	void mirrorZ();

signals:
	void scaleXYZChanged(double);
	void scaleXChanged(double);
	void scaleYChanged(double);
	void scaleZChanged(double);
	void reverseWindings(int mesh);
	void flipNormals(int mesh);
	void mirrorAxis(int);

	void applyTransformations();
	void changeActiveMesh(int index);
	void removeMesh(int index);
};

#endif // TRANSFORMDOCK_HPP
