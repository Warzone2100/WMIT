#include "meshdock.h"
#include "ui_meshdock.h"

#include "Pie.h"

MeshDock::MeshDock(QWidget *parent) :
	QDockWidget(parent),
	m_model(nullptr),
	m_selected_mesh(-1),
	m_ui(new Ui::MeshDock)
{
	m_ui->setupUi(this);

	setMeshCount(0, QStringList());

	connect(m_ui->meshComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectMesh(int)));
	connect(m_ui->btnAddConnector, SIGNAL(clicked(bool)), this, SLOT(addConnector()));
	connect(m_ui->btnDeleteConnector, SIGNAL(clicked(bool)), this, SLOT(rmSelConnector()));
}

MeshDock::~MeshDock()
{
	delete m_ui;
}

void MeshDock::resetConnectorViewModel()
{
	if ((m_selected_mesh < 0) || !m_model || (m_model->meshes() == 0))
	{
		m_ui->meshConnectors->setModel(nullptr);
		m_ui->gbConnectors->setEnabled(false);
		return;
	}
	auto connModel = new WzmConnectorsModel(m_model->getMesh(m_selected_mesh), m_ui->meshConnectors);
	connect(connModel, SIGNAL(connectorsWereUpdated()), this, SIGNAL(connectorsWereUpdated()));
	m_ui->meshConnectors->setModel(connModel);
	m_ui->gbConnectors->setEnabled(true);
}

void MeshDock::resetLevelSettingsViewModel()
{
	if ((m_selected_mesh < 0) || !m_model || (m_model->meshes() == 0))
	{
		m_ui->meshLevelSettings->setModel(nullptr);
		m_ui->gbLevelSettings->setEnabled(false);
		return;
	}

	auto settingsModel = new WzmLevelSettingsModel(m_model->getMesh(m_selected_mesh), m_ui->meshLevelSettings);
	connect(settingsModel, SIGNAL(levelSettingsWereUpdated()), this, SIGNAL(levelSettingsWereUpdated()));
	m_ui->meshLevelSettings->setModel(settingsModel);
	m_ui->gbLevelSettings->setEnabled(true);
}

void MeshDock::setModel(WZM *model)
{
	m_model = model;
	resetConnectorViewModel();
	resetLevelSettingsViewModel();
}

void MeshDock::setMeshCount(int value, QStringList names)
{
	int selected = m_ui->meshComboBox->currentIndex();

	if (selected >= value)
	{
		selected = -1;
	}

	m_ui->meshComboBox->blockSignals(true);
	m_ui->meshComboBox->clear();

	for (int i = 1; i <= value; ++i)
	{
		m_ui->meshComboBox->addItem(QString::number(i) + " [" + names.value(i - 1) + "]");
	}

	if ((selected < 0) && (value > 0))
	{
		selected = 0;
	}

	if (selected >= 0)
		m_ui->meshComboBox->setCurrentIndex(selected);
	m_ui->meshComboBox->blockSignals(false);

	selectMesh(selected); // force this because of possible mesh stack pop
}

void MeshDock::selectMesh(int index)
{
	m_selected_mesh = index;
	resetConnectorViewModel();
	resetLevelSettingsViewModel();
}

void MeshDock::rmSelConnector()
{
	if (!m_ui->meshConnectors->selectionModel()->hasSelection())
		return;
	// We use single selection, so there should be no duplicated rows
	for (auto& curRow: m_ui->meshConnectors->selectionModel()->selectedIndexes())
	{
		m_ui->meshConnectors->model()->removeRow(curRow.row());
	}
}

void MeshDock::addConnector()
{
	m_ui->meshConnectors->model()->insertRow(m_ui->meshConnectors->model()->rowCount());
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
	return 4;
}

QVariant WzmConnectorsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= static_cast<int>(m_mesh.connectors()) || index.row() < 0)
		return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		const auto &connector = m_mesh.getConnector(index.row());

		if (index.column() == 0)
			return index.row();
		else if (index.column() == 1)
			return -connector.getPos().x();
		else if (index.column() == 2)
			return connector.getPos().y();
		else if (index.column() == 3)
			return connector.getPos().z();
	}
	else if (role == Qt::BackgroundRole)
	{
		if (index.column() == 0)
		{
			auto color = CONNECTOR_COLORS[static_cast<size_t>(index.row()) % MAX_CONNECTOR_COLORS].scale(255.f);
			return QBrush(QColor(color.x(), color.y(), color.z()));
		}
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
			return tr("#");
		case 1:
			return tr("X");
		case 2:
			return tr("Y");
		case 3:
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

	// FIXME: we can only append
	for (int row = 0; row < rows; ++row)
		m_mesh.addConnector(WZMConnector());
	emit connectorsWereUpdated();

	endInsertRows();
	return true;
}

bool WzmConnectorsModel::removeRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);
	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
		m_mesh.rmConnector(position);
	emit connectorsWereUpdated();

	endRemoveRows();
	return true;
}

bool WzmConnectorsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole && value.isValid())
	{
		int row = index.row();

		auto& connector = m_mesh.getConnector(row);

		if (index.column() == 1)
			connector.getPos().x() = -value.toFloat();
		else if (index.column() == 2)
			connector.getPos().y() = value.toFloat();
		else if (index.column() == 3)
			connector.getPos().z() = value.toFloat();
		else
			return false;

		emit connectorsWereUpdated();
		emit dataChanged(index, index, {role});

		return true;
	}

	return false;
}

Qt::ItemFlags WzmConnectorsModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractTableModel::flags(index) | (index.column() > 0 ? Qt::ItemIsEditable : Qt::NoItemFlags);
}

enum level_setting_row_t
{
	LEVEL_ROW_TYPE = 0,
	LEVEL_ROW_INTERPOLATE,
	LEVEL_ROW_TEXTURE,
	LEVEL_ROW__TEXTURE_LAST = LEVEL_ROW_TEXTURE + (PIE_MODEL_TILESETS * (WZM_TEX__LAST - WZM_TEX__FIRST)) - 1,
	LEVEL_ROW__LAST
};

static wzm_texture_type_t levelRowTexType(int row)
{
	const int offset = row - LEVEL_ROW_TEXTURE;
	return static_cast<wzm_texture_type_t>(WZM_TEX__FIRST + offset / PIE_MODEL_TILESETS);
}

static unsigned levelRowTileset(int row)
{
	const int offset = row - LEVEL_ROW_TEXTURE;
	return static_cast<unsigned>(offset % PIE_MODEL_TILESETS);
}

static QString levelRowName(int row)
{
	static const char* const tilesets[PIE_MODEL_TILESETS] = {"arizona", "urban", "rockies"};

	switch (row)
	{
	case LEVEL_ROW_TYPE: return QObject::tr("TYPE (hex)");
	case LEVEL_ROW_INTERPOLATE: return QObject::tr("INTERPOLATE");
	default:
		break;
	}

	return QString::fromStdString(WZM::texTypeToPieDirective(levelRowTexType(row))) +
			" " + tilesets[levelRowTileset(row)];
}

WzmLevelSettingsModel::WzmLevelSettingsModel(Mesh &mesh, QObject *parent): QAbstractTableModel(parent),
	m_mesh(mesh)
{
}

int WzmLevelSettingsModel::rowCount(const QModelIndex &) const
{
	return LEVEL_ROW__LAST;
}

int WzmLevelSettingsModel::columnCount(const QModelIndex &) const
{
	return 2;
}

QVariant WzmLevelSettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	return section == 0 ? tr("Setting") : tr("Override");
}

QVariant WzmLevelSettingsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
		return QVariant();

	const int row = index.row();

	if (index.column() == 0)
		return levelRowName(row);

	switch (row)
	{
	case LEVEL_ROW_TYPE:
	{
		const auto type = m_mesh.getPieType();
		return type ? QString::number(*type, 16) : QString();
	}
	case LEVEL_ROW_INTERPOLATE:
	{
		const auto interpolate = m_mesh.getPieInterpolate();
		return interpolate ? QString::number(*interpolate) : QString();
	}
	default:
		break;
	}

	const std::string directive = WZM::texTypeToPieDirective(levelRowTexType(row));
	return QString::fromStdString(m_mesh.getPieTextureOverride(levelRowTileset(row), directive));
}

bool WzmLevelSettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid() || role != Qt::EditRole || index.column() != 1)
		return false;

	const int row = index.row();
	const QString text = value.toString().trimmed();

	switch (row)
	{
	case LEVEL_ROW_TYPE:
	{
		bool ok = false;
		const unsigned type = text.toUInt(&ok, 16);
		if (text.isEmpty())
			m_mesh.setPieType(std::optional<unsigned>());
		else if (ok)
			m_mesh.setPieType(type);
		else
			return false;
		break;
	}
	case LEVEL_ROW_INTERPOLATE:
	{
		bool ok = false;
		const unsigned interpolate = text.toUInt(&ok);
		if (text.isEmpty())
			m_mesh.setPieInterpolate(std::optional<unsigned>());
		else if (ok)
			m_mesh.setPieInterpolate(interpolate);
		else
			return false;
		break;
	}
	default:
	{
		const std::string directive = WZM::texTypeToPieDirective(levelRowTexType(row));
		m_mesh.setPieTextureOverride(levelRowTileset(row), directive, text.toStdString());
		break;
	}
	}

	emit levelSettingsWereUpdated();
	emit dataChanged(index, index, {role});

	return true;
}

Qt::ItemFlags WzmLevelSettingsModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractTableModel::flags(index) | (index.column() == 1 ? Qt::ItemIsEditable : Qt::NoItemFlags);
}
