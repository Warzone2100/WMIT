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

public slots:
	void setMaterialColor(const int type, const QColor color);

private:
	Ui::MaterialDock *m_ui;
};

#endif // MATERIALDOCK_H
