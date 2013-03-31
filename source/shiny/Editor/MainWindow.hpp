#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>

#include <queue>

#include "Actions.hpp"
#include "Query.hpp"

namespace Ui {
class MainWindow;
}

namespace sh
{

struct SynchronizationState;


/**
 * @brief A snapshot of the material system's state. Lock the mUpdateMutex before accessing.
 */
struct MaterialSystemState
{
	std::vector<std::string> mMaterialList;

	std::map<std::string, std::string> mGlobalSettingsMap;

	std::vector<std::string> mConfigurationList;

	std::vector<std::string> mMaterialFiles;
	std::vector<std::string> mConfigurationFiles;

	std::vector<std::string> mShaderSets;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	// really should be an std::atomic
	volatile bool mRequestShowWindow;
	// dito
	volatile bool mRequestExit;

	SynchronizationState* mSync;
	bool mIgnoreGlobalSettingChange;
	bool mIgnoreConfigurationChange;
	bool mIgnoreMaterialChange;

	std::queue<Action*> mActionQueue;
	std::vector<Query*> mQueries;

	MaterialSystemState mState;

private:
	Ui::MainWindow *ui;

	// material tab
	QStringListModel* mMaterialModel;
	QSortFilterProxyModel* mMaterialProxyModel;

	QStandardItemModel* mMaterialPropertyModel;

	// global settings tab
	QStandardItemModel* mGlobalSettingsModel;

	// configuration tab
	QStandardItemModel* mConfigurationModel;

	void queueAction(Action* action);
	void requestQuery(Query* query);

	void buildMaterialModel (MaterialQuery* data);
	void buildConfigurationModel (ConfigurationQuery* data);

	QString getSelectedMaterial();

	/// get the context of an index in the material property model
	void getContext(QModelIndex index, int* passIndex, int* textureIndex, bool useParent=true);

	std::string getPropertyKey(QModelIndex index);
	std::string getPropertyValue(QModelIndex index);

protected:
	void closeEvent(QCloseEvent *event);

public slots:
	void onIdle();

	void onMaterialSelectionChanged (const QModelIndex & current, const QModelIndex & previous);
	void onConfigurationSelectionChanged (const QString& current);

	void onGlobalSettingChanged (QStandardItem* item);
	void onConfigurationChanged (QStandardItem* item);
	void onMaterialPropertyChanged (QStandardItem* item);

	void onContextMenuRequested(const QPoint& point);

private slots:
	void on_lineEdit_textEdited(const QString &arg1);
	void on_actionSave_triggered();
	void on_actionNewMaterial_triggered();
	void on_actionDeleteMaterial_triggered();
	void on_actionQuit_triggered();
	void on_actionNewConfiguration_triggered();
	void on_actionDeleteConfiguration_triggered();
	void on_actionDeleteConfigurationProperty_triggered();
	void on_actionCloneMaterial_triggered();
	void on_actionCreatePass_triggered();
	void on_actionDeleteProperty_triggered();
	void on_actionNewProperty_triggered();
};

}

#endif // MAINWINDOW_HPP
