#include "meshdock.h"
#include "ui_meshdock.h"

MeshDock::MeshDock(QWidget *parent) :
	QDockWidget(parent),
	m_model(nullptr),
	ui(new Ui::MeshDock)
{
	ui->setupUi(this);
}

MeshDock::~MeshDock()
{
	delete ui;
}

void MeshDock::setModel(WZM *model)
{
	m_model = model;
	if (m_model->meshes() == 0)
	{
		ui->meshConnectors->setModel(nullptr);
		return;
	}
	auto connModel = new WzmConnectorsModel(m_model->getMesh(0));
	ui->meshConnectors->setModel(connModel);
}

WzmConnectorsModel::WzmConnectorsModel(Mesh &mesh, QObject *parent):
	QAbstractTableModel(parent), m_mesh(mesh)
{
}

int WzmConnectorsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_mesh.connectors());
}

int WzmConnectorsModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 3;
}

QVariant WzmConnectorsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= static_cast<int>(m_mesh.connectors()) || index.row() < 0)
		return QVariant();

	if (role == Qt::DisplayRole) {
		const auto &connector = m_mesh.getConnector(index.row());

		if (index.column() == 0)
			return connector.getPos().x();
		else if (index.column() == 1)
			return connector.getPos().y();
		else if (index.column() == 2)
			return connector.getPos().z();
	}
	return QVariant();
}

QVariant WzmConnectorsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section) {
		case 0:
			return tr("X");
		case 1:
			return tr("Y");
		case 2:
			return tr("Z");
		default:
			return QVariant();
		}
	}
	return QVariant();
}

bool WzmConnectorsModel::insertRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
		m_mesh.addConnector(WZMConnector());

	endInsertRows();
	return true;
}

bool WzmConnectorsModel::removeRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);
	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
		m_mesh.rmConnector(position);

	endRemoveRows();
	return true;
}

bool WzmConnectorsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole) {
		int row = index.row();

		auto connector = m_mesh.getConnector(row);

		if (index.column() == 0)
			connector.getPos().x() = value.toFloat();
		else if (index.column() == 1)
			connector.getPos().y() = value.toFloat();
		else if (index.column() == 2)
			connector.getPos().z() = value.toFloat();
		else
			return false;

		emit dataChanged(index, index, {role});

		return true;
	}

	return false;
}

Qt::ItemFlags WzmConnectorsModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}
