#include "BookmarkTableModel.h"
#include "BrowserApplication.h"
#include "FaviconStorage.h"
#include <vector>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QMimeData>
#include <QSet>
#include <QUrl>
#include <QDebug>

BookmarkTableModel::BookmarkTableModel(BookmarkManager *bookmarkMgr, QObject *parent) :
    QAbstractTableModel(parent),
    m_bookmarkMgr(bookmarkMgr),
    m_folder(bookmarkMgr->getBookmarksBar())
{
}

QVariant BookmarkTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0: return "Name";
            case 1: return "Location";
        }
    }

    return QVariant();
}

int BookmarkTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return (m_folder) ? m_folder->getNumChildren() : 0;
}

int BookmarkTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    // Fixed number of columns
    return 2;
}

QVariant BookmarkTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_folder)
        return QVariant();

    if (index.row() < m_folder->getNumChildren())
    {
        BookmarkNode *b = m_folder->getNode(index.row());
        switch (index.column())
        {
            case 0:
            {
                if (role == Qt::EditRole || role == Qt::DisplayRole)
                    return b->getName();

                // Try to display icon next to name
                QIcon nodeIcon;
                switch (b->getType())
                {
                    case BookmarkNode::Folder:
                        nodeIcon = b->getIcon();
                        break;
                    case BookmarkNode::Bookmark:
                        nodeIcon = sBrowserApplication->getFaviconStorage()->getFavicon(QUrl(b->getURL()));
                        break;
                }
                if (role == Qt::DecorationRole)
                    return nodeIcon.pixmap(16, 16);
                if (role == Qt::SizeHintRole)
                    return QSize(16, 16);
            }
            case 1: if (role == Qt::DisplayRole || role == Qt::EditRole) return b->getURL();
        }
    }
    return QVariant();
}

bool BookmarkTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole && m_folder->getNumChildren() > index.row())
    {
        BookmarkNode *b = m_folder->getNode(index.row());
        if (!b)
            return false;

        // Copy contents of bookmark before modifying, so the old record can be found in the DB
        BookmarkNode oldValue;
        oldValue.setName(b->getName());
        oldValue.setURL(b->getURL());
        switch (index.column())
        {
            case 0: b->setName(value.toString()); break;
            case 1: b->setURL(value.toString()); break;
        }

        m_bookmarkMgr->updatedBookmark(b, oldValue, m_folder->getFolderId());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags BookmarkTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
    BookmarkNode *b = m_folder->getNode(index.row());
    if (b->getType() == BookmarkNode::Bookmark) //|| (b->getType() == BookmarkNode::Folder && index.column() == 0))
        flags |= Qt::ItemIsEditable;
    return flags;
}

bool BookmarkTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!m_folder)
        return false;

    QString name("New Bookmark");
    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i)
        m_bookmarkMgr->insertBookmark(name, QString("Location %1").arg(i), m_folder, row + i);
    endInsertRows();
    return true;
}

bool BookmarkTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!m_folder)
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i)
    {
        BookmarkNode *n = m_folder->getNode(row + i);
        m_bookmarkMgr->removeBookmark(n);
    }
    endRemoveRows();
    return true;
}

QStringList BookmarkTableModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-bookmark-data";
    return types;
}

Qt::DropActions BookmarkTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData *BookmarkTableModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    for (const QModelIndex &index : indexes)
    {
        if (index.isValid() && m_folder->getNumChildren() > index.row())
        {
            BookmarkNode *n = m_folder->getNode(index.row());
            stream << n;
        }
    }
    mimeData->setData("application/x-bookmark-data", encodedData);
    return mimeData;
}

bool BookmarkTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex &parent)
{
    if (!data || action != Qt::MoveAction)
        return false;

    if (!data->hasFormat("application/x-bookmark-data"))
        return false;

    // retrieve rows from serialized data
    QByteArray encodedData = data->data("application/x-bookmark-data");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    std::vector<BookmarkNode*> nodes;
    while (!stream.atEnd())
    {
        BookmarkNode *node;
        stream >> node;
        nodes.push_back(node);
    }

    // Shift row positions
    int newRow = parent.row();
    bool needUpdateModel = false;

    beginResetModel();
    for (BookmarkNode *n : nodes)
    {
        if (n->getType() == BookmarkNode::Folder)
            needUpdateModel = true;
        m_bookmarkMgr->setNodePosition(n, newRow);
        ++newRow;
    }
    endResetModel();

    if (needUpdateModel)
        emit movedFolder();

    return true;
}

void BookmarkTableModel::setCurrentFolder(BookmarkNode *folder)
{
    beginResetModel();
    m_folder = folder;
    endResetModel();
}

BookmarkNode *BookmarkTableModel::getCurrentFolder() const
{
    return m_folder;
}
