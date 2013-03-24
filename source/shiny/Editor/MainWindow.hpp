#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QStandardItemModel>

#include <queue>

#include "Actions.hpp"

namespace Ui {
class MainWindow;
}

namespace sh
{

class SynchronizationState;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	// really should be an std::atomic
	volatile bool mRequestShowWindow;

	SynchronizationState* mSync;
	std::vector<std::string> mMaterialList;

	std::map<std::string, std::string> mGlobalSettingsMap;
	bool mIgnoreGlobalSettingChange;

	std::queue<Action*> mActionQueue;

private:
	Ui::MainWindow *ui;

	// material tab
	QStringListModel* mModel;
	QSortFilterProxyModel* mProxyModel;

	// global settings tab
	QStandardItemModel* mGlobalSettingsModel;

	void queueAction(Action* action);


protected:
	void closeEvent(QCloseEvent *event);

public slots:
	void onIdle();

	void onSelectionChanged (const QModelIndex & current, const QModelIndex & previous);

	void onGlobalSettingChanged (QStandardItem* item);

private slots:
	void on_lineEdit_textEdited(const QString &arg1);
	void on_actionSave_triggered();
	void on_actionNewMaterial_triggered();
	void on_actionDeleteMaterial_triggered();
	void on_actionQuit_triggered();
};

}

#endif // MAINWINDOW_HPP
