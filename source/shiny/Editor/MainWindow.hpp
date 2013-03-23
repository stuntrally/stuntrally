#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include <QSortFilterProxyModel>
#include <QStringListModel>

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

	std::queue<Action*> mActionQueue;

private:
	Ui::MainWindow *ui;

	QStringListModel* mModel;
	QSortFilterProxyModel* mProxyModel;


protected:
	void closeEvent(QCloseEvent *event);

public slots:
	void onIdle();

	void onSelectionChanged (const QModelIndex & current, const QModelIndex & previous);

private slots:
	void on_actionNew_triggered();
	void on_lineEdit_textEdited(const QString &arg1);
	void on_actionDelete_triggered();
	void on_actionSave_triggered();
};

}

#endif // MAINWINDOW_HPP
