/*
  The MIT License (MIT)

  Copyright (c) 2015 Andrea Scarpino <me@andreascarpino.it>

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

#include "hackernewsapi.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

const static QString API_URL = "https://hacker-news.firebaseio.com/v0/";

HackerNewsAPI::HackerNewsAPI(QObject *parent) :
    QObject(parent)
  , network(new QNetworkAccessManager(this))
{
}

HackerNewsAPI::~HackerNewsAPI()
{
    delete network;
}

void HackerNewsAPI::getItem(const qint32 id)
{
    qDebug() << "Requesting item with id" << id;

    QUrl url(API_URL + QString("item/%1.json").arg(id));
    QNetworkRequest req(url);
    QNetworkReply* reply = network->get(req);

    connect(reply, SIGNAL(finished()), this, SLOT(onGetItemResult()));
}

void HackerNewsAPI::getMaxItemId()
{
    qDebug() << "Requesting max item id";

    QUrl url(API_URL + QString("maxitem.json"));
    QNetworkRequest req(url);
    QNetworkReply* reply = network->get(req);

    connect(reply, SIGNAL(finished()), this, SLOT(onMaxItemIdResult()));
}

void HackerNewsAPI::getNewStories()
{
    qDebug() << "Requesting new stories";

    QUrl url(API_URL + QString("newstories.json"));
    QNetworkRequest req(url);
    QNetworkReply* reply = network->get(req);

    connect(reply, SIGNAL(finished()), this, SLOT(onMultipleStoriesResult()));
}

void HackerNewsAPI::getTopStories()
{
    qDebug() << "Requesting top stories";

    QUrl url(API_URL + QString("topstories.json"));
    QNetworkRequest req(url);
    QNetworkReply* reply = network->get(req);

    connect(reply, SIGNAL(finished()), this, SLOT(onMultipleStoriesResult()));
}

void HackerNewsAPI::onGetItemResult()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (!reply || reply->error() != QNetworkReply::NoError) {
        qCritical() << "Cannot fetch item";
        return;
    }

    QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
    qDebug() << "Got item:\n" << json;
    emit itemFetched(json.toVariant().toMap());

    reply->deleteLater();
}

void HackerNewsAPI::onMaxItemIdResult()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (!reply || reply->error() != QNetworkReply::NoError) {
        qCritical() << "Cannot fetch max id";
        return;
    }

    bool* ok = 0;
    const int maxId = reply->readAll().toInt(ok);

    if (ok) {
        qCritical() << "Cannot parse max id value!";
    }

    emit maxItemIdFetched(maxId);

    reply->deleteLater();
}

void HackerNewsAPI::onMultipleStoriesResult()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (!reply || reply->error() != QNetworkReply::NoError) {
        qCritical() << "Cannot fetch stories";
        return;
    }

    QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
    qDebug() << "Got" << json.array().size() << "items";
    emit multipleStoriesFetched(json.array().toVariantList());

    reply->deleteLater();
}
