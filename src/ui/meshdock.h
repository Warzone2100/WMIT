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

private slots:
	void selectMesh(int index);
	void rmSelConnector();
	void addConnector();

private:
	WZM* m_model;
	int m_selected_mesh;
	Ui::MeshDock *m_ui;
	void resetConnectorViewModel();
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
private:
	Mesh& m_mesh;
};

#endif // MESHDOCK_H
