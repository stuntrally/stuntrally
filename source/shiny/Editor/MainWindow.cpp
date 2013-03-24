#include "MainWindow.hpp"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QTimer>

#include "Editor.hpp"

sh::MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, mRequestShowWindow(false)
	, mIgnoreGlobalSettingChange(false)
{
	ui->setupUi(this);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onIdle()));
	timer->start(50);

	mModel = new QStringListModel(this);

	mProxyModel = new QSortFilterProxyModel(this);
	mProxyModel->setSourceModel(mModel);
	mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	mProxyModel->setDynamicSortFilter(true);
	mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

	ui->materialList->setModel(mProxyModel);
	ui->materialList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->materialList->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(ui->materialList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
			this,								SLOT(onSelectionChanged(QModelIndex,QModelIndex)));


	mGlobalSettingsModel = new QStandardItemModel(0, 2, this);
	mGlobalSettingsModel->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
	mGlobalSettingsModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Value")));
	connect(mGlobalSettingsModel,	SIGNAL(itemChanged(QStandardItem*)),
			this,					SLOT(onGlobalSettingChanged(QStandardItem*)));

	ui->globalSettingsView->setModel(mGlobalSettingsModel);
	ui->globalSettingsView->verticalHeader()->setResizeMode(QHeaderView::Stretch);
	ui->globalSettingsView->verticalHeader()->hide();
	ui->globalSettingsView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

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

	QString selected;

	QModelIndex selectedIndex = ui->materialList->selectionModel()->currentIndex();
	if (selectedIndex.isValid())
		selected = mModel->data(selectedIndex, Qt::DisplayRole).toString();

	QStringList list;

	for (std::vector<std::string>::const_iterator it = mMaterialList.begin(); it != mMaterialList.end(); ++it)
	{
		list.push_back(QString::fromStdString(*it));
	}


	if (mModel->stringList() != list)
	{
		mModel->setStringList(list);

		// quick hack to keep our selection when the model has changed
		if (!selected.isEmpty())
			for (int i=0; i<mModel->rowCount(); ++i)
			{
				const QModelIndex& index = mModel->index(i,0);
				if (mModel->data(index, Qt::DisplayRole).toString() == selected)
				{
					ui->materialList->setCurrentIndex(index);
					break;
				}
			}
	}

	//if (mGlobalSettingsModel->rowCount() == 0)
	{
		mIgnoreGlobalSettingChange = true;
		for (std::map<std::string, std::string>::const_iterator it = mGlobalSettingsMap.begin();
			 it != mGlobalSettingsMap.end(); ++it)
		{
			QList<QStandardItem *> list = mGlobalSettingsModel->findItems(QString::fromStdString(it->first));
			if (!list.empty()) // item was already there
			{
				// if it changed, set the value column
				if (mGlobalSettingsModel->data(mGlobalSettingsModel->index(list.front()->row(), 1)).toString()
						!= QString::fromStdString(it->second))
				{
					std::cout << "changing " << it->first << " to " << it->second << std::endl;

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
	}

}

void sh::MainWindow::onSelectionChanged (const QModelIndex & current, const QModelIndex & previous)
{
}

void sh::MainWindow::onGlobalSettingChanged(QStandardItem *item)
{
	if (mIgnoreGlobalSettingChange)
		return; // we are only interested in changes by the user, not by the backend.

	std::string name = mGlobalSettingsModel->data(mGlobalSettingsModel->index(item->row(), 0)).toString().toStdString();
	std::string value = mGlobalSettingsModel->data(mGlobalSettingsModel->index(item->row(), 1)).toString().toStdString();

	queueAction(new sh::ActionChangeGlobalSetting(name, value));
}

void sh::MainWindow::on_lineEdit_textEdited(const QString &arg1)
{
	mProxyModel->setFilterFixedString(arg1);
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
	QString name = mProxyModel->data(selectedIndex, Qt::DisplayRole).toString();

	queueAction (new sh::ActionDeleteMaterial(name.toStdString()));
}

void sh::MainWindow::queueAction(Action* action)
{
	boost::mutex::scoped_lock lock(mSync->mActionMutex);
	mActionQueue.push(action);
}


void sh::MainWindow::on_actionQuit_triggered()
{
	hide();
}
