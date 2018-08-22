#include "searchbar.h"

#include <QCompleter>
#include <QTimer>

#include "kiwixapp.h"

SearchBar::SearchBar(QWidget *parent) :
    QLineEdit(parent),
    m_completer(&m_completionModel, this)
{
    setPlaceholderText("Search");
    m_completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setCompleter(&m_completer);
    connect(this, &QLineEdit::textEdited, this, &SearchBar::updateCompletion);
#if 0 //The `activated` signal seems to not be emitted if user navigate in the page.
      // Something is broken here .. :/
    connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
            this, &SearchBar::openCompletion);
#else
    connect(this, &QLineEdit::returnPressed, this, &SearchBar::openTitle);
#endif
}

void SearchBar::updateCompletion(const QString &text)
{
    QStringList wordList;
    m_urlList.clear();
    auto currentWidget = KiwixApp::instance()->getTabWidget()->currentWidget();
    if (!currentWidget) {
        m_completionModel.setStringList(wordList);
        return;
    }
    auto qurl = currentWidget->url();
    m_currentHost = qurl.host();
    qInfo() << "Get reader for " << m_currentHost;
    auto reader = KiwixApp::instance()->getLibrary()->getReader(m_currentHost);
    if (!reader) {
        qInfo() << "No reader";
        m_completionModel.setStringList(wordList);
        return;
    }
    qInfo() << "Get suggestions for" << text;
    reader->searchSuggestionsSmart(text.toStdString(), 15);
    std::string title, url;
    while (reader->getNextSuggestion(title, url)) {
        qInfo() << "Have suggestion " << QString::fromStdString(title) << "&" << QString::fromStdString(title);
        wordList << QString::fromStdString(title);
        m_urlList.push_back(url);
    }
    m_completionModel.setStringList(wordList);
}

void SearchBar::openTitle()
{
    QString title = text();
    clear();
    auto tabWidget = KiwixApp::instance()->getTabWidget();
    auto zimId = tabWidget->currentZimId();
    auto reader = KiwixApp::instance()->getLibrary()->getReader(zimId);
    if ( reader == nullptr) {
        return;
    }
    std::string path;
    try {
        auto entry = reader->getEntryFromTitle(title.toStdString());
        path = entry.getPath();
    } catch (kiwix::NoEntry& e)
    {
        return;
    }

    QUrl qurl;
    qurl.setScheme("zim");
    qurl.setHost(zimId);
    qurl.setPath("/" + QString::fromStdString(path));
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(qurl, true);});
}

void SearchBar::openCompletion(const QModelIndex &index)
{
    auto url = m_urlList.at(index.row());
    QUrl qurl;
    qurl.setScheme("zim");
    qurl.setHost(m_currentHost);
    qurl.setPath(QString::fromStdString(url));
    QTimer::singleShot(0, [=](){KiwixApp::instance()->openUrl(qurl, true);});
}

