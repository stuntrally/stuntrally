#include "MainWindow.hpp"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QTimer>

#include "Editor.hpp"

sh::MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, mRequestShowWindow(false)
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

	connect(ui->materialList->selectionModel(),
			SIGNAL(currentChanged(QModelIndex,QModelIndex)),
			this, SLOT(onSelectionChanged(QModelIndex,QModelIndex)));

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
		show();

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
}

void sh::MainWindow::onSelectionChanged (const QModelIndex & current, const QModelIndex & previous)
{
}

void sh::MainWindow::on_actionNew_triggered()
{
}

void sh::MainWindow::on_lineEdit_textEdited(const QString &arg1)
{
	mProxyModel->setFilterFixedString(arg1);
}

void sh::MainWindow::on_actionDelete_triggered()
{
	QModelIndex selectedIndex = ui->materialList->selectionModel()->currentIndex();
	QString name = mProxyModel->data(selectedIndex, Qt::DisplayRole).toString();

	boost::mutex::scoped_lock lock(mSync->mUpdateMutex);

	sh::Action* action = new sh::ActionDeleteMaterial(name.toStdString());
	mActionQueue.push(action);
}

void sh::MainWindow::on_actionSave_triggered()
{
	boost::mutex::scoped_lock lock(mSync->mUpdateMutex);

	sh::Action* action = new sh::ActionSaveAll();
	mActionQueue.push(action);
}
