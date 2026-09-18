#ifndef MESHDOCK_H
#define MESHDOCK_H

#include <QDockWidget>

#include "Mesh.h"
#include "WZM.h"

namespace Ui {
class MeshDock;
}

class MeshDock : public QDockWidget
{
	Q_OBJECT

public:
	explicit MeshDock(QWidget *parent = nullptr);
	~MeshDock();

	void setModel(WZM* model);

public slots:
	void setMeshCount(int value, QStringList names);
	void resetConnectorViewModel();
	void resetLevelSettingsViewModel();

signals:
	void connectorsWereUpdated();
	void levelSettingsWereUpdated();

private slots:
	void selectMesh(int index);
	void rmSelConnector();
	void addConnector();

private:
	WZM* m_model;
	int m_selected_mesh;
	Ui::MeshDock *m_ui;
};

#include <QAbstractTableModel>
#include <QString>

class WzmConnectorsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	WzmConnectorsModel(Mesh& mesh,  QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	bool insertRows(int position, int rows, const QModelIndex &index) override;
	bool removeRows(int position, int rows, const QModelIndex &index) override;

signals:
	void connectorsWereUpdated();
private:
	Mesh& m_mesh;
};

/**
  * The settings one mesh overrides for itself in a PIE 4 file: its own TYPE and
  * INTERPOLATE, and a texture page per tileset. An empty cell means the mesh
  * takes the model wide setting.
  */
class WzmLevelSettingsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	WzmLevelSettingsModel(Mesh& mesh, QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:
	void levelSettingsWereUpdated();

private:
	Mesh& m_mesh;
};

#endif // MESHDOCK_H
