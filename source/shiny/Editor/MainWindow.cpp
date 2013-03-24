#include "MainWindow.hpp"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QTimer>

#include <QInputDialog>

#include "Editor.hpp"

sh::MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, mRequestShowWindow(false)
	, mIgnoreGlobalSettingChange(false)
	, mIgnoreConfigurationChange(false)
	, mIgnoreMaterialChange(false)
{
	ui->setupUi(this);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onIdle()));
	timer->start(50);

	mMaterialModel = new QStringListModel(this);

	mMaterialProxyModel = new QSortFilterProxyModel(this);
	mMaterialProxyModel->setSourceModel(mMaterialModel);
	mMaterialProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	mMaterialProxyModel->setDynamicSortFilter(true);
	mMaterialProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

	ui->materialList->setModel(mMaterialProxyModel);
	ui->materialList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->materialList->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(ui->materialList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
			this,								SLOT(onMaterialSelectionChanged(QModelIndex,QModelIndex)));

	mMaterialPropertyModel = new QStandardItemModel(0, 1, this);

	ui->materialView->setModel(mMaterialPropertyModel);


	mGlobalSettingsModel = new QStandardItemModel(0, 2, this);
	mGlobalSettingsModel->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
	mGlobalSettingsModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Value")));
	connect(mGlobalSettingsModel,	SIGNAL(itemChanged(QStandardItem*)),
			this,					SLOT(onGlobalSettingChanged(QStandardItem*)));

	ui->globalSettingsView->setModel(mGlobalSettingsModel);
	ui->globalSettingsView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	ui->globalSettingsView->verticalHeader()->hide();
	ui->globalSettingsView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	ui->globalSettingsView->setSelectionMode(QAbstractItemView::SingleSelection);

	ui->configurationList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->configurationList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(ui->configurationList,	SIGNAL(currentTextChanged(QString)),
							this,	SLOT(onConfigurationSelectionChanged(QString)));

	mConfigurationModel = new QStandardItemModel(0, 2, this);
	mConfigurationModel->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
	mConfigurationModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Value")));
	connect(mConfigurationModel,	SIGNAL(itemChanged(QStandardItem*)),
			this,					SLOT(onConfigurationChanged(QStandardItem*)));

	ui->configurationView->setModel(mConfigurationModel);
	ui->configurationView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	ui->configurationView->verticalHeader()->hide();
	ui->configurationView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	ui->configurationView->setSelectionMode(QAbstractItemView::SingleSelection);
}

sh::MainWindow::~MainWindow()
{
	delete ui;
}

void sh::MainWindow::closeEvent(QCloseEvent *event)
{
	this->hide();
	event->ignore();
}

void sh::MainWindow::onIdle()
{
	if (mRequestShowWindow)
	{
		mRequestShowWindow = false;
		show();
	}

	boost::mutex::scoped_lock lock(mSync->mUpdateMutex);


	mIgnoreMaterialChange = true;
	QString selected;

	QModelIndex selectedIndex = ui->materialList->selectionModel()->currentIndex();
	if (selectedIndex.isValid())
		selected = mMaterialModel->data(selectedIndex, Qt::DisplayRole).toString();

	QStringList list;

	for (std::vector<std::string>::const_iterator it = mState.mMaterialList.begin(); it != mState.mMaterialList.end(); ++it)
	{
		list.push_back(QString::fromStdString(*it));
	}

	if (mMaterialModel->stringList() != list)
	{
		mMaterialModel->setStringList(list);

		// quick hack to keep our selection when the model has changed
		if (!selected.isEmpty())
			for (int i=0; i<mMaterialModel->rowCount(); ++i)
			{
				const QModelIndex& index = mMaterialModel->index(i,0);
				if (mMaterialModel->data(index, Qt::DisplayRole).toString() == selected)
				{
					ui->materialList->setCurrentIndex(index);
					break;
				}
			}
	}
	mIgnoreMaterialChange = false;

	mIgnoreGlobalSettingChange = true;
	for (std::map<std::string, std::string>::const_iterator it = mState.mGlobalSettingsMap.begin();
		 it != mState.mGlobalSettingsMap.end(); ++it)
	{
		QList<QStandardItem *> list = mGlobalSettingsModel->findItems(QString::fromStdString(it->first));
		if (!list.empty()) // item was already there
		{
			// if it changed, set the value column
			if (mGlobalSettingsModel->data(mGlobalSettingsModel->index(list.front()->row(), 1)).toString()
					!= QString::fromStdString(it->second))
			{
				mGlobalSettingsModel->setItem(list.front()->row(), 1, new QStandardItem(QString::fromStdString(it->second)));
			}
		}
		else // item wasn't there; insert new row
		{
			QList<QStandardItem*> toAdd;
			QStandardItem* name = new QStandardItem(QString::fromStdString(it->first));
			name->setFlags(name->flags() &= ~Qt::ItemIsEditable);
			QStandardItem* value = new QStandardItem(QString::fromStdString(it->second));
			toAdd.push_back(name);
			toAdd.push_back(value);
			mGlobalSettingsModel->appendRow(toAdd);
		}
	}
	mIgnoreGlobalSettingChange = false;


	mIgnoreConfigurationChange = true;
	QList<QListWidgetItem*> selected_ = ui->configurationList->selectedItems();
	QString selectedStr;
	if (selected_.size())
		selectedStr = selected_.front()->text();

	ui->configurationList->clear();

	for (std::vector<std::string>::const_iterator it = mState.mConfigurationList.begin(); it != mState.mConfigurationList.end(); ++it)
		ui->configurationList->addItem(QString::fromStdString(*it));

	if (!selectedStr.isEmpty())
		for (int i=0; i<ui->configurationList->count(); ++i)
		{
			if (ui->configurationList->item(i)->text() == selectedStr)
			{
				ui->configurationList->setCurrentItem(ui->configurationList->item(i), QItemSelectionModel::ClearAndSelect);
			}
		}

	mIgnoreConfigurationChange = false;


	// process query results
	boost::mutex::scoped_lock lock2(mSync->mQueryMutex);
	for (std::vector<Query*>::iterator it = mQueries.begin(); it != mQueries.end();)
	{
		if ((*it)->mDone)
		{
			if (typeid(**it) == typeid(ConfigurationQuery))
			{
				ConfigurationQuery* q = static_cast<ConfigurationQuery*>(*it);
				while (mConfigurationModel->rowCount())
					mConfigurationModel->removeRow(0);
				for (std::map<std::string, std::string>::iterator it = q->mProperties.begin();
					 it != q->mProperties.end(); ++it)
				{
					QList<QStandardItem*> toAdd;
					QStandardItem* name = new QStandardItem(QString::fromStdString(it->first));
					name->setFlags(name->flags() &= ~Qt::ItemIsEditable);
					QStandardItem* value = new QStandardItem(QString::fromStdString(it->second));
					toAdd.push_back(name);
					toAdd.push_back(value);
					mConfigurationModel->appendRow(toAdd);
				}

				// add items that are in global settings, but not in this configuration (with a "inactive" color)
				for (std::map<std::string, std::string>::const_iterator it = mState.mGlobalSettingsMap.begin();
					 it != mState.mGlobalSettingsMap.end(); ++it)
				{
					if (q->mProperties.find(it->first) == q->mProperties.end())
					{
						QColor color = ui->configurationView->palette().color(QPalette::Disabled, QPalette::WindowText);
						QList<QStandardItem*> toAdd;
						QStandardItem* name = new QStandardItem(QString::fromStdString(it->first));
						name->setFlags(name->flags() &= ~Qt::ItemIsEditable);
						name->setData(color, Qt::ForegroundRole);
						QStandardItem* value = new QStandardItem(QString::fromStdString(it->second));
						value->setData(color, Qt::ForegroundRole);
						toAdd.push_back(name);
						toAdd.push_back(value);
						mConfigurationModel->appendRow(toAdd);
					}
				}
			}
			delete *it;
			it = mQueries.erase(it);
		}
		else
			++it;
	}
}

void sh::MainWindow::onMaterialSelectionChanged (const QModelIndex & current, const QModelIndex & previous)
{
	if (mIgnoreMaterialChange)
		return;

	QModelIndex selectedIndex = ui->materialList->selectionModel()->currentIndex();
	QString name = mMaterialProxyModel->data(selectedIndex, Qt::DisplayRole).toString();

	requestQuery(new sh::MaterialQuery(name.toStdString()));
}

void sh::MainWindow::onConfigurationSelectionChanged (const QString& current)
{
	if (mIgnoreConfigurationChange)
		return;
	requestQuery(new sh::ConfigurationQuery(current.toStdString()));
}

void sh::MainWindow::onGlobalSettingChanged(QStandardItem *item)
{
	if (mIgnoreGlobalSettingChange)
		return; // we are only interested in changes by the user, not by the backend.

	std::string name = mGlobalSettingsModel->data(mGlobalSettingsModel->index(item->row(), 0)).toString().toStdString();
	std::string value = mGlobalSettingsModel->data(mGlobalSettingsModel->index(item->row(), 1)).toString().toStdString();

	queueAction(new sh::ActionChangeGlobalSetting(name, value));
}

void sh::MainWindow::onConfigurationChanged (QStandardItem* item)
{
	QList<QListWidgetItem*> items = ui->configurationList->selectedItems();
	if (items.size())
	{
		std::string name = items.front()->text().toStdString();
		std::string key = mConfigurationModel->data(mConfigurationModel->index(item->row(), 0)).toString().toStdString();
		std::string value = mConfigurationModel->data(mConfigurationModel->index(item->row(), 1)).toString().toStdString();

		queueAction(new sh::ActionChangeConfiguration(name, key, value));

		requestQuery(new sh::ConfigurationQuery(name));
	}
}

void sh::MainWindow::on_lineEdit_textEdited(const QString &arg1)
{
	mMaterialProxyModel->setFilterFixedString(arg1);
}

void sh::MainWindow::on_actionSave_triggered()
{
	queueAction (new sh::ActionSaveAll());
}

void sh::MainWindow::on_actionNewMaterial_triggered()
{

}

void sh::MainWindow::on_actionDeleteMaterial_triggered()
{
	QModelIndex selectedIndex = ui->materialList->selectionModel()->currentIndex();
	QString name = mMaterialProxyModel->data(selectedIndex, Qt::DisplayRole).toString();

	queueAction (new sh::ActionDeleteMaterial(name.toStdString()));
}

void sh::MainWindow::queueAction(Action* action)
{
	boost::mutex::scoped_lock lock(mSync->mActionMutex);
	mActionQueue.push(action);
}

void sh::MainWindow::requestQuery(Query *query)
{
	boost::mutex::scoped_lock lock(mSync->mActionMutex);
	mQueries.push_back(query);
}

void sh::MainWindow::on_actionQuit_triggered()
{
	hide();
}

void sh::MainWindow::on_actionNewConfiguration_triggered()
{
	QInputDialog dialog(this);

	QString text = QInputDialog::getText(this, tr("New Configuration"),
											  tr("Configuration name:"));

	if (!text.isEmpty())
	{
		queueAction(new ActionCreateConfiguration(text.toStdString()));
	}
}

void sh::MainWindow::on_actionDeleteConfiguration_triggered()
{
	QList<QListWidgetItem*> items = ui->configurationList->selectedItems();
	if (items.size())
		queueAction(new ActionDeleteConfiguration(items.front()->text().toStdString()));
}

void sh::MainWindow::on_actionDeleteConfigurationProperty_triggered()
{
	QList<QListWidgetItem*> items = ui->configurationList->selectedItems();
	if (items.empty())
		return;
	std::string configurationName = items.front()->text().toStdString();

	QModelIndex current = ui->configurationView->currentIndex();
	if (!current.isValid())
		return;

	std::string propertyName = mConfigurationModel->data(mConfigurationModel->index(current.row(), 0)).toString().toStdString();

	queueAction(new sh::ActionDeleteConfigurationProperty(configurationName, propertyName));
	requestQuery(new sh::ConfigurationQuery(configurationName));
}

void sh::MainWindow::on_actionCloneMaterial_triggered()
{
	QModelIndex selectedIndex = ui->materialList->selectionModel()->currentIndex();
	QString name = mMaterialProxyModel->data(selectedIndex, Qt::DisplayRole).toString();
	if (name.isEmpty())
		return;

	QInputDialog dialog(this);

	QString text = QInputDialog::getText(this, tr("Clone material"),
											  tr("Name:"));

	if (!text.isEmpty())
	{
		queueAction(new ActionCloneMaterial(name.toStdString(), text.toStdString()));
	}
}
