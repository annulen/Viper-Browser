#include "HistoryMenu.h"
#include "BrowserApplication.h"
#include "FaviconStorage.h"
#include "HistoryManager.h"

#include <QAction>
#include <QList>
#include <QKeySequence>
#include <QDebug>

HistoryMenu::HistoryMenu(QWidget *parent) :
    QMenu(parent),
    m_actionShowHistory(nullptr),
    m_actionClearHistory(nullptr)
{
    setup();
}

HistoryMenu::HistoryMenu(const QString &title, QWidget *parent) :
    QMenu(title, parent)
{
    setup();
}

HistoryMenu::~HistoryMenu()
{
}

void HistoryMenu::addHistoryItem(const QUrl &url, const QString &title, const QIcon &favicon)
{
    QList<QAction*> menuActions = actions();

    QAction *beforeItem = nullptr;
    if (menuActions.size() > 3)
        beforeItem = menuActions[3];

    QAction *historyItem = new QAction(title, parentWidget());
    historyItem->setIcon(favicon);
    connect(historyItem, &QAction::triggered, [=](){
        emit loadUrl(url);
    });

    if (beforeItem)
        insertAction(beforeItem, historyItem);
    else
        addAction(historyItem);

    int menuSize = menuActions.size();
    while (menuSize > 17)
    {
        QAction *actionToRemove = menuActions[menuSize - 1];
        removeAction(actionToRemove);
        --menuSize;
    }
}

void HistoryMenu::clearItems()
{
    QList<QAction*> menuActions = actions();
    if (menuActions.size() < 3)
        return;

    for (int i = 3; i < menuActions.size(); ++i)
    {
        QAction *currItem = menuActions.at(i);
        removeAction(currItem);
    }
}

void HistoryMenu::resetItems()
{
    clearItems();

    FaviconStorage *faviconStorage = sBrowserApplication->getFaviconStorage();
    const std::deque<WebHistoryItem> &historyItems = sBrowserApplication->getHistoryManager()->getRecentItems();
    for (auto &it : historyItems)
    {
        if (!it.Title.isEmpty())
            addHistoryItem(it.URL, it.Title, faviconStorage->getFavicon(it.URL));
    }
}

void HistoryMenu::onPageVisited(const QString &url, const QString &title)
{
    QUrl itemUrl(url);
    QIcon favicon = sBrowserApplication->getFaviconStorage()->getFavicon(itemUrl);
    addHistoryItem(itemUrl, title, favicon);
}

void HistoryMenu::setup()
{
    BrowserApplication *app = sBrowserApplication;
    connect(app->getHistoryManager(), &HistoryManager::pageVisited, this, &HistoryMenu::onPageVisited);
    connect(app, &BrowserApplication::resetHistoryMenu, this, &HistoryMenu::resetItems);

    m_actionShowHistory = addAction(QStringLiteral("&Show all History"));
    m_actionShowHistory->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_H));

    m_actionClearHistory = addAction(QStringLiteral("&Clear Recent History..."));
    m_actionClearHistory->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete));

    addSeparator();

    resetItems();
}
