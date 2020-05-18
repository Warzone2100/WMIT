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

#ifndef MATERIALDOCK_H
#define MATERIALDOCK_H

#include "WZM.h"
#include "WZLight.h"

#include <QDockWidget>

namespace Ui
{
	class MaterialDock;
}

class MaterialDock : public QDockWidget
{
	Q_OBJECT

public:
	MaterialDock(QWidget *parent = 0);
	~MaterialDock();

protected:
	void changeEvent(QEvent *event);

signals:
	void materialChanged(const WZMaterial& mat);

public slots:
	void setMaterial(const WZMaterial& mat);

private slots:
	void changeShininess(int value);
	void changeShininess(double value);
	void colorsChanged(const light_cols_t& light_cols);

private:
	Ui::MaterialDock *m_ui;
	WZMaterial m_material;

	void setShininessOnUI(const float shininess);
};

#endif // MATERIALDOCK_H
