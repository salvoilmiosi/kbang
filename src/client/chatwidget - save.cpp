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

#include <QPainter>
#include <QPaintEvent>
#include <QClipboard>
#include <QtNetwork>
#include <QHttp>

using namespace client;

ChatWidget::ChatWidget(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);
    setContentsMargins(5, 5, 5, 5);
    /*
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Base, QColor(0, 0, 0, 16));
    setPalette(palette);
    */
    mp_chatView->setOpenExternalLinks(true);

    // Set the background color because Qt on the Mac doesn't do the transparency correctly
    mp_chatView->setStyleSheet(QString::fromUtf8("background-color: rgb(158, 130, 98);"));
    mp_messageBox->setStyleSheet(QString::fromUtf8("background-color: rgb(158, 130, 98);"));
    mp_chatTranslatedView->setStyleSheet(QString::fromUtf8("background-color: rgb(158, 130, 98);"));

    mp_chatView->setFocusPolicy(Qt::TabFocus);
    mp_messageBox->setFocusPolicy(Qt::ClickFocus);

    // Creating HTTP
    httpRequest = new QHttp(this);
    // Connecting HTTP event done to slot done
    connect(httpRequest,
            SIGNAL(done(bool)),
            this, SLOT(httpDone()));

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

    // This will be used to track translations
    translateTrack.clear();
    translateCounter = 0;
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

void ChatWidget::incomingMessage(int, const QString& senderName, const QString& message)
{
    qDebug(qPrintable(QString("Sender=%1, lang=%2, msg=%3").arg(senderName).arg(mp_destinationLanguage).arg(message)));
    QString chatHTML = "<a style='color: #000000; text-decoration: none; background: #9E8262' href='%1'>%2</a>";
    QString urlTranslateFriendly = "http://translate.google.com/translate_t?sl=auto&tl=%1&text=%2#";
    QString urlTranslateRaw = "http://translate.google.com/translate_a/t?client=t&sl=auto&tl=%1&text=%2#";

    // Make all the text a clickable link to translate.google.com, from = auto
    mp_chatView->append(QString("<b>%1:</b> %2")
                        .arg(Qt::escape(removeHTMLtags(senderName)))
                        .arg(QString(chatHTML)
                             .arg(QString(urlTranslateFriendly)
                                  .arg(mp_destinationLanguage)
                                  .arg(QString(Qt::escape(message)).replace(" ", "%20")))
                             .arg(message)));

    // Prepare the request
    QHttpRequestHeader requestHeader =
            QHttpRequestHeader("POST", QString(urlTranslateRaw)
                                        .arg(mp_destinationLanguage)
                                        .arg(QString(Qt::escape(message)).replace(" ", "%20"))
                               , 1, 1);
    requestHeader.setValue("Host", "www.google.com");
    requestHeader.setValue("User-Agent", "Mozilla/5.0");
    requestHeader.setValue("Accept-Encoding", "deflate");
    requestHeader.setContentLength(message.length());
    requestHeader.setValue("Connection", "Close");

    // Code up the sender name and message
    QByteArray sendByteArray("text=");
    sendByteArray.append( message.toUtf8() + QString("||%1").arg(translateCounter));

    qDebug(qPrintable(QString("BA=%1").arg(QString(sendByteArray))));

    // Send off for the translation
    httpRequest->setHost("www.google.com");
    httpRequest->request(requestHeader, sendByteArray);

    translateTrack[translateCounter] = senderName;
    translateCounter += 1;

}

void ChatWidget::sendMessage()
{
    const QString& message = mp_messageBox->text();
    mp_messageBox->clear();
    emit outgoingMessage(Qt::escape(message));
}

void ChatWidget::httpDone()
{
    bool ok;
    qDebug("HTTP Request done.");

    QString returnedText = returnedText.fromUtf8( httpRequest->readAll() );
    // For use in the regex test app:  \"(.*).*\|.*\|.*([0-9]*)\",\"(.*)\"
    QRegExp regex("\\\"(.*).*\\|.*\\|.*([0-9]+)\\\",\\\"(.*)\\\"");
    regex.setPatternSyntax(QRegExp::RegExp2);
    regex.setMinimal(true);
    QString senderName = "";

qDebug(qPrintable(QString("%1").arg(returnedText)));

    if (regex.indexIn(returnedText) > -1) {

        //qDebug(qPrintable(QString("%1,%2,%3").arg(regex.cap(1)).arg(regex.cap(2)).arg(regex.cap(3))));

        QString newMessage = regex.cap(1);
        QString langFrom = regex.cap(3);
        int findEntry = regex.cap(2).toInt(&ok, 10);
        QString senderName = translateTrack[findEntry];
        translateTrack.remove(findEntry);
        QString imgHTML = "<img src=\":/flags/flags/%1.png\" width=15 align=left><b>%2</b>: <i>%3</i>";
        // Look up sender from our array
        mp_chatTranslatedView->append(QString(imgHTML).arg(langFrom).arg(senderName).arg(newMessage));
    } else
        mp_chatTranslatedView->append("Translation unavailable");
}

void ChatWidget::setLanguage(QString passedLanguage)
{
    //qDebug(qPrintable(QString("passedLanguage=%1").arg(passedLanguage)));
    mp_destinationLanguage = "en";
    if (languageMap.contains(passedLanguage))
            mp_destinationLanguage = passedLanguage;
    //qDebug(qPrintable(QString("Translating to " + mp_destinationLanguage)));

    if (languageMap.contains(mp_destinationLanguage))  {
        mp_destinationLanguageFull = languageMap[mp_destinationLanguage];
        QString path = QString(":/flags/flags/%1.png").arg(mp_destinationLanguage);
        mp_tabWidget->setTabIcon(1, QIcon(path));
        mp_tabWidget->setTabToolTip(1, "Chat translated into " + mp_destinationLanguageFull + " via Google Translate");
    }
}
