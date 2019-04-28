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

#ifndef EXPORTDIALOG_HPP
#define EXPORTDIALOG_HPP

#include <QDialog>
#include <Pie.h>

namespace Ui {
    class ExportDialog;
}

class ExportDialog : public QDialog {
    Q_OBJECT
public:
    ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

protected:
    void changeEvent(QEvent *e);

protected:
    Ui::ExportDialog *ui;
};

#include <QAbstractTableModel>
#include <QString>


class PieContentModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	PieContentModel(const PieCaps& caps, QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
	PieCaps m_caps;
};

class PieExportDialog : public ExportDialog
{
	Q_OBJECT
public:
	PieExportDialog(QWidget* parent = nullptr);
private:
};

#endif // EXPORTDIALOG_HPP
