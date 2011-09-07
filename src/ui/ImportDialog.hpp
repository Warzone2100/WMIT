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
#ifndef IMPORTDIALOG_HPP
#define IMPORTDIALOG_HPP

#include <QDialog>
#include <QStringList>

namespace Ui {
	class ImportDialog;
}

class QListWidgetItem;

class ImportDialog : public QDialog {
    Q_OBJECT
public:
    ImportDialog(QWidget *parent = 0);
	~ImportDialog();

	QString modelFilePath() const;
	QString textureFilePath() const;
	bool tcmaskChecked() const;
	QString tcmaskFilePath() const;

public slots:
	void scanForTextures(const QStringList&);

protected:
    void changeEvent(QEvent *e);

private:
	Ui::ImportDialog* ui;
	QStringList m_textures;
	QListWidgetItem* lw_autoFoundTextures_previous;

	void setModelFileName(QString fileName);
	void lw_autoFoundTextures_clearSelection();

private slots:
	void on_pb_autoTCM_clicked();
 void on_tb_seekTcmFName_clicked();
	void on_pb_autoTex_clicked();
	void on_lw_autoFoundTextures_itemClicked(QListWidgetItem* item);
	void on_lw_autoFoundTextures_itemSelectionChanged();
	void on_tb_seekTextureFName_clicked();
	void on_tb_seekFileName_clicked();
};

#endif // IMPORTDIALOG_HPP
