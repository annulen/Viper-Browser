#ifndef USERSCRIPTMANAGER_H
#define USERSCRIPTMANAGER_H

#include "Settings.h"
#include "UserScript.h"

#include <memory>
#include <vector>
#include <QObject>
#include <QString>
#include <QUrl>

class UserScriptModel;

/**
 * @class UserScriptManager
 * @brief Manages a collection of GreaseMonkey-style user scripts
 */
class UserScriptManager : public QObject
{
    Q_OBJECT
public:
    /// Constructs the user script manager, given a shared pointer to the application settings object and an optional parent object
    explicit UserScriptManager(std::shared_ptr<Settings>, QObject *parent = nullptr);

    /// Saves user script information (i.e. which scripts are enabled) before destruction
    virtual ~UserScriptManager();

    /// Returns a pointer to the user script model
    UserScriptModel *getModel();

    /**
     * @brief Searches all user scripts for any that match the given URL for injection
     * @param url The URL of the resource being loaded
     * @param injectionTime The time of script injection relative to the initial page load request
     * @param isMainFrame True if the frame associated with the url is the main frame of the page, false if it is a sub-frame
     * @return Concatenated user script data if one or more scripts are found, or an empty string if no scripts match the URL
     */
    QString getScriptsFor(const QUrl &url, ScriptInjectionTime injectionTime, bool isMainFrame);

public slots:
    /**
     * @brief Attempts to download and install the user script from the given URL
     * @param url The location of the user script to be installed
     */
    void installScript(const QUrl &url);

private:
    /// Pointer to the user scripts model
    UserScriptModel *m_model;
};

#endif // USERSCRIPTMANAGER_H