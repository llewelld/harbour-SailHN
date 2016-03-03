/*
  The MIT License (MIT)

  Copyright (c) 2016 Andrea Scarpino <me@andreascarpino.it>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "hnmanager.h"

#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrlQuery>

const static QString BASE_URL = QStringLiteral("https://news.ycombinator.com");

HNManager::HNManager(QObject *parent) :
    QObject(parent)
  , network(new QNetworkAccessManager(this))
  , user("")
  , logged(false)
{
    network->setCookieJar(new QNetworkCookieJar(this));
}

HNManager::~HNManager()
{
}

void HNManager::authenticate(const QString &username, const QString &password)
{
    qDebug() << "Log in with username" << username;
    user = username;

    QNetworkRequest req(QUrl(BASE_URL + QLatin1String("/login")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrlQuery data;
    data.addQueryItem(QLatin1String("acct"), username);
    data.addQueryItem(QLatin1String("pw"), password);

    QNetworkReply* reply = network->post(req, data.toString(QUrl::FullyEncoded).toUtf8());

    connect(reply, &QNetworkReply::finished, this, &HNManager::onAuthenticateResult);
}

void HNManager::onAuthenticateResult()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Cannot perform login";
    } else {
        if (!reply->readAll().contains("Bad login.")) {
            logged = true;
        }
    }

    Q_EMIT authenticated(logged);

    reply->deleteLater();
}

bool HNManager::isAuthenticated() const
{
    //qDebug() << "Is authenticated:" << logged;

    return logged;
}

QString HNManager::loggedUser() const
{
    return user;
}

void HNManager::logout()
{
    logged = false;
    user.clear();

    // FIXME: is there a better way?
    network->cookieJar()->deleteLater();
    network->setCookieJar(new QNetworkCookieJar(this));
}

void HNManager::submit(const QString &title, const QString &url, const QString &text)
{
    qDebug() << "Submit item with title" << title;

    QNetworkRequest req(QUrl(BASE_URL + QLatin1String("/r")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrlQuery data;
    data.addQueryItem(QLatin1String("title"), title);
    data.addQueryItem(QLatin1String("url"), url);
    data.addQueryItem(QLatin1String("text"), text);
    data.addQueryItem(QLatin1String("fnop"), QLatin1String("submit-page"));
    data.addQueryItem(QLatin1String("fnid"), getSubmitPage());

    QNetworkReply* reply = network->post(req, data.toString(QUrl::FullyEncoded).toUtf8());

    connect(reply, &QNetworkReply::finished, this, &HNManager::onSubmitResult);
}

void HNManager::onSubmitResult()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    bool res = false;

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Cannot submit item";
    } else {
        if (!reply->readAll().contains("Unknown or expired link.")) {
            res = true;
        }
    }

    Q_EMIT submitted(res);

    reply->deleteLater();
}

QString HNManager::getSubmitPage() const
{
    QNetworkRequest req(QUrl(BASE_URL + QLatin1String("/submit")));
    QNetworkReply* reply = network->get(req);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QTextStream stream(reply->readAll(), QIODevice::ReadOnly);

    const QRegularExpression regexp("<input type=\"hidden\" name=\"fnid\" value=\"([^\"]+)\"");

    QString line;
    while (!stream.atEnd()) {
        line = stream.readLine();

        QRegularExpressionMatch match = regexp.match(line);
        if (match.hasMatch()) {
            return match.captured(1);
        }
    }

    return QString();
}
