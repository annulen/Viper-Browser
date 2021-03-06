#ifndef ADBLOCKWIDGET_H
#define ADBLOCKWIDGET_H

#include <QWidget>

namespace Ui {
    class AdBlockWidget;
}

/**
 * @class AdBlockWidget
 * @brief Acts as a management window for the user to view, add, modify and/or delete
 *        their advertisement blocking subscriptions
 */
class AdBlockWidget : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the ad block widget with the given parent
    explicit AdBlockWidget(QWidget *parent = nullptr);

    /// Destructor
    ~AdBlockWidget();

protected:
    /// Called when the widget is resized - resizes the columns of the table view
    void resizeEvent(QResizeEvent *event) override;

private slots:
    /// Called when a subscription item in the table has been clicked
    void onItemClicked(const QModelIndex &index);

    /// Called when the user chooses to install a new subscription
    void onAddSubscriptionButtonClicked();

private:
    /// Pointer to the user interface items
    Ui::AdBlockWidget *ui;
};

#endif // ADBLOCKWIDGET_H
