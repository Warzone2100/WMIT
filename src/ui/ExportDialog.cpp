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

#include "ExportDialog.h"
#include "ui_ExportDialog.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

PieExportDialog::PieExportDialog(QWidget* parent)
	: ExportDialog(parent)
{
	auto model = new PieContentModel(PIE3_CAPS);
	ui->tvExportCaps->setModel(model);
}

PieContentModel::PieContentModel(const PieCaps &caps, QObject *parent): QAbstractTableModel(parent),
	m_caps(caps)
{

}

int PieContentModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_caps.size();
}

int PieContentModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 3;
}

QVariant PieContentModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || (index.row() < 0))
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		int row = index.row();

		if (index.column() == 0)
			return getPieDirectiveName(static_cast<PIE_OPT_DIRECTIVES>(row));
		else if (index.column() == 1)
			return m_caps.test(static_cast<PIE_OPT_DIRECTIVES>(row));
		else if (index.column() == 2)
			return getPieDirectiveDescription(static_cast<PIE_OPT_DIRECTIVES>(row));
	}

	return QVariant();
}

bool PieContentModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole)
	{
		int row = index.row();

		if (index.column() == 1)
			m_caps.set(static_cast<PIE_OPT_DIRECTIVES>(row), value.toBool());
		else
			return false;

		emit dataChanged(index, index, {role});

		return true;
	}

	return false;
}

Qt::ItemFlags PieContentModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractTableModel::flags(index) | (index.column() == 1 ? Qt::ItemIsEditable : Qt::NoItemFlags);
}

QVariant PieContentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section) {
		case 0:
			return tr("Directive");
		case 1:
			return tr("State");
		case 2:
			return tr("Description");
		default:
			return QVariant();
		}
	}

	return QVariant();
}
