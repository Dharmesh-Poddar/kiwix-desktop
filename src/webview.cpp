#include "webview.h"

#include <QAction>
#include <QWebEngineProfile>
#include <iostream>
#include "kiwixapp.h"
#include "webpage.h"

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
{
    setPage(new WebPage(this));
    auto profile = page()->profile();
    auto app = KiwixApp::instance();
    profile->installUrlSchemeHandler("zim", app->getSchemeHandler());
    m_keyAltHome = new QShortcut(this);
    m_keyAltHome->setKey(Qt::ALT + Qt::Key_Home);
    connect(m_keyAltHome, SIGNAL(activated()), this, SLOT(openHomePage()));
    QObject::connect(this, &QWebEngineView::urlChanged, this, &WebView::onUrlChanged);
}

WebView::~WebView()
{}

void WebView::openHomePage() {
    setUrl("zim://" + m_currentZimId + ".zim/");
}

bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}

QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    if ( type==QWebEnginePage::WebBrowserBackgroundTab
      || type==QWebEnginePage::WebBrowserTab )
    {
        auto tabWidget = KiwixApp::instance()->getTabWidget();
        return tabWidget->createNewTab(type==QWebEnginePage::WebBrowserTab);
    }
    return nullptr;
}

void WebView::onUrlChanged(const QUrl& url) {
    auto zimId = url.host();
    zimId.resize(zimId.length()-4);
    if (m_currentZimId != zimId ) {
        m_currentZimId = zimId;
        emit zimIdChanged(m_currentZimId);
        auto app = KiwixApp::instance();
        auto reader = app->getLibrary()->getReader(m_currentZimId);
        if (!reader)
            return;
        std::string favicon, _mimetype;
        reader->getFavicon(favicon, _mimetype);
        QPixmap pixmap;
        pixmap.loadFromData((const uchar*)favicon.data(), favicon.size());
        m_icon = QIcon(pixmap);
        emit iconChanged(m_icon);
    }
}
