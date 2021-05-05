/***************************************************************************
 *   Copyright (C) 2008 by MacJariel                                       *
 *   echo "badmailet@gbalt.dob" | tr "edibmlt" "ecrmjil"                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "common.h"
#include "chatwidget.h"
#include "config.h"
#include "game.h"

#include <QPainter>
#include <QPaintEvent>
#include <QClipboard>
#include <QtNetwork>
#include <QHttp>
#include <QSound>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

using namespace client;

ChatWidget::ChatWidget(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);
    setContentsMargins(5, 5, 5, 5);

    #ifdef Q_WS_MAC
        mp_tabWidget->setStyleSheet("margin-top: 5px;");
    #endif
    #ifndef Q_WS_MAC
        mp_tabWidget->setStyleSheet("margin-top: 0px;");
    #endif

    /*
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Base, QColor(0, 0, 0, 16));
    setPalette(palette);
    */
    mp_chatView->setOpenExternalLinks(true);

    #ifdef Q_WS_MAC
        // Set the background color because Qt on the Mac doesn't do the transparency correctly
        mp_chatView->setStyleSheet(QString::fromUtf8("background-color: rgb(158, 130, 98);"));
        mp_messageBox->setStyleSheet(QString::fromUtf8("background-color: rgb(158, 130, 98);"));
        mp_chatTranslatedView->setStyleSheet(QString::fromUtf8("background-color: rgb(158, 130, 98);"));
    #endif

    mp_chatView->setFocusPolicy(Qt::TabFocus);
    mp_messageBox->setFocusPolicy(Qt::ClickFocus);

    connect(mp_messageBox,
            SIGNAL(returnPressed()),
            this, SLOT(sendMessage()));

    // Set up flag/language information for use later
    languageMap["sq"]="Albanian";
    languageMap["ar"]="Arabic";
    languageMap["bg"]="Bulgarian";
    languageMap["ca"]="Catalan";
    languageMap["zh-CN"]="Chinese (Simplified)";
    languageMap["zh-TW"]="Chinese (Traditional)";
    languageMap["hr"]="Croatian";
    languageMap["cs"]="Czech";
    languageMap["da"]="Danish";
    languageMap["nl"]="Dutch";
    languageMap["en"]="English";
    languageMap["et"]="Estonian";
    languageMap["tl"]="Filipino";
    languageMap["fi"]="Finnish";
    languageMap["fr"]="French";
    languageMap["gl"]="Galician";
    languageMap["de"]="German";
    languageMap["el"]="Greek";
    languageMap["iw"]="Hebrew";
    languageMap["hi"]="Hindi";
    languageMap["hu"]="Hungarian";
    languageMap["id"]="Indonesian";
    languageMap["it"]="Italian";
    languageMap["ja"]="Japanese";
    languageMap["ko"]="Korean";
    languageMap["lv"]="Latvian";
    languageMap["lt"]="Lithuanian";
    languageMap["mt"]="Maltese";
    languageMap["no"]="Norwegian";
    languageMap["pl"]="Polish";
    languageMap["pt"]="Portuguese";
    languageMap["ro"]="Romanian";
    languageMap["ru"]="Russian";
    languageMap["sr"]="Serbian";
    languageMap["sk"]="Slovak";
    languageMap["sl"]="Slovenian";
    languageMap["es"]="Spanish";
    languageMap["sv"]="Swedish";
    languageMap["th"]="Thai";
    languageMap["tr"]="Turkish";
    languageMap["uk"]="Ukrainian";
    languageMap["vi"]="Vietnamese";
}

ChatWidget::~ChatWidget()
{
}

void ChatWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(event->rect().intersect(contentsRect()), QColor(0, 0, 0, 32));
}

void ChatWidget::clear()
{
    mp_chatView->clear();
    mp_chatTranslatedView->clear();
}

void ChatWidget::sendMessage()
{
    const QString& message = mp_messageBox->text();
    mp_messageBox->clear();
    emit outgoingMessage(Qt::escape(message));
}

// Take incoming messages, create a link to Google Translate, and also send the translation request
void ChatWidget::incomingMessage(int, const QString& senderName, const QString& message)
{
    QString chatHTML = "<a style='color: #000000; text-decoration: none; background: #9E8262' href='%1'>%2</a>";
    QString urlTranslateFriendly = "http://translate.google.com/translate_t?sl=auto&tl=%1&text=%2#";

    // Make all the text a clickable link to translate.google.com, from = auto
    mp_chatView->append(QString("<b>%1:</b> %2")
                        .arg(Qt::escape(removeHTMLtags(senderName)))
                        .arg(QString(chatHTML)
                             .arg(QString(urlTranslateFriendly)
                                  .arg(mp_destinationLanguage)
                                  .arg(QString(Qt::escape(message)).replace(" ", "%20")))
                             .arg(message)));

    // Send the message off to Google Translate and see if we can convert it
    this->getTranslation(senderName, message);
}

// Take the message and sender, and start the translation process
void ChatWidget::getTranslation(QString senderName, QString message)
{
    //QString urlDetectLanguage = "http://ajax.googleapis.com/ajax/services/language/detect?v=1.0&q=%1|||%2";
    //QString urlTranslateRaw = "http://translate.google.com/translate_a/t?client=t&sl=auto&tl=%1||%2";

    QString translateAPIkey = "ABQIAAAA4UeerbxCfjgY6m-zjAbD8BRSFJHZ7M4uQnszWwrq9-QKTCCYQhRFYRYufHK79Te3smB-l4NSm9ppbg";
    QString urlTranslate = "http://ajax.googleapis.com/ajax/services/language/translate?v=1.0&langpair=|%1&q=%2";  // http://ajax.googleapis.com

    QNetworkAccessManager *netManager = new QNetworkAccessManager(this);
    QNetworkRequest netRequest;

    // First thing we have to do is find out what language in which the message is written
    netRequest.setUrl(QUrl(QString(urlTranslate)
                                 .arg(mp_destinationLanguage)
                                 .arg(message)));
//                                 .arg( QString(QUrl::toPercentEncoding(message)) )));

    qDebug() << netRequest.url();

    netRequest.setRawHeader("User-Agent", "Mozilla/5.0");
    netRequest.setRawHeader("Referrer", "http://spectregadget.net/category/hobbies/bang/");
    netRequest.setRawHeader("Key", "ABQIAAAA4UeerbxCfjgY6m-zjAbD8BRSFJHZ7M4uQnszWwrq9-QKTCCYQhRFYRYufHK79Te3smB-l4NSm9ppbg");

    QNetworkReply *netReply = netManager->get(netRequest);

    // Create a loop and connect the QNetworkRequest signals to break it
    QEventLoop loop;
    connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(netReply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    connect(netReply, SIGNAL(sslErrors(QList<QSslError>)), &loop, SLOT(quit()));
    loop.exec(QEventLoop::AllEvents|QEventLoop::WaitForMoreEvents);

    qDebug("Detect language HTTP Request done.");

    // Get the translated text
    QByteArray returnedText(netReply->readAll());

//    // Debug statements
//    qDebug() << "Sender is " << senderName;
//    if (returnedText.isEmpty()) qDebug() << "returnedText is empty.";
//    if (returnedText.isNull()) qDebug() << "returnedText is null.";
//    qDebug() << QString("returnedText size is %1").arg(returnedText.size());
//    qDebug() << "Returned text = " << returnedText.constData();

    // Variables to get the translated details
    QString transText;
    QString transLang;

    QString returnedTextString;
    returnedTextString = returnedText.constData();
    QScriptValue scriptValue;
    QScriptEngine scriptEngine;
    scriptValue = scriptEngine.evaluate("value = " + returnedTextString);

    while (scriptValue.isObject()) {
        QScriptValueIterator it(scriptValue);
        while (it.hasNext()) {
            it.next();
            if (it.name() == "responseData")
            {
//                qDebug() << it.name();
//                qDebug() << " translatedText         = " << it.value().property("translatedText").toString();
//                qDebug() << " detectedSourceLanguage = " << it.value().property("detectedSourceLanguage").toString();
                if (it.value().property("detectedSourceLanguage").toString() != "")
                    transLang = it.value().property("detectedSourceLanguage").toString();
                if (it.value().property("detectedSourceLanguage").toString() != "")
                    transText.append(it.value().property("translatedText").toString());
            }
        }
        scriptValue = scriptValue.prototype();
    }

    qDebug() << "Translated from " << transLang;
    qDebug() << "Translated text = " << transText;

    if (transText.length() > 0) {
        static QString imgHTML = "<img src=\":/flags/flags/%1.png\" width=15 align=abmiddle title=\"Translated from %2\"> <b>%3</b>: <i>%4</i>";
        mp_chatTranslatedView->append(QString(imgHTML).arg(transLang).arg(languageMap[transLang]).arg(senderName).arg(transText));
    } else
        mp_chatTranslatedView->append("Translation unavailable");

}

void ChatWidget::setLanguage(QString passedLanguage)
{
    mp_destinationLanguage = "en";

    if (languageMap.contains(passedLanguage))
            mp_destinationLanguage = passedLanguage;

    if (languageMap.contains(mp_destinationLanguage))  {
        mp_destinationLanguageFull = languageMap[mp_destinationLanguage];
        QString path = QString(":/flags/flags/%1.png").arg(mp_destinationLanguage);
        mp_tabWidget->setTabIcon(1, QIcon(path));
        mp_tabWidget->setTabToolTip(1, "Chat translated into " + mp_destinationLanguageFull + " via Google Translate");
    }
}
