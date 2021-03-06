#include "WebDialog.h"
#include "WebView.h"

#include <QGridLayout>

WebDialog::WebDialog(QWidget *parent) :
    QWidget(parent),
    m_view(new WebView(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    setLayout(grid);

    grid->addWidget(m_view);
    m_view->show();
    m_view->setFocus();

    connect(m_view, &WebView::titleChanged, this, &QWidget::setWindowTitle);
}

QWebView *WebDialog::getView() const
{
    return static_cast<QWebView*>(m_view);
}
