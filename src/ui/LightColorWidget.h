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

#ifndef LightColorWidget_H
#define LightColorWidget_H

#include <QWidget>

#include "WZLight.h"

namespace Ui
{
	class LightColorWidget;
}

class LightColorWidget : public QWidget
{
	Q_OBJECT

public:
	LightColorWidget(QWidget *parent = nullptr);
	~LightColorWidget();

protected:
	void changeEvent(QEvent *event);

signals:
	void colorsChanged(const light_cols_t& light_cols);

public slots:
	void setLightColors(const light_cols_t& light_cols);

private slots:
	void on_colorTypeComboBox_currentIndexChanged(int index);
	void on_colorDialogButton_clicked();

	void changeRedComponent(int value);
	void changeRedComponent(double value);
	void changeGreenComponent(int value);
	void changeGreenComponent(double value);
	void changeBlueComponent(int value);
	void changeBlueComponent(double value);

private:
	Ui::LightColorWidget *m_ui;
	light_cols_t m_colors;

	void setColorOnUI(const QColor &color);
	void applyColor(const QColor &color);
};

#endif // LightColorWidget_H
