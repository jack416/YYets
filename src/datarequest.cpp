#include "datarequest.h"
#include "datarequest_p.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

const QString DataRequestPrivate::RequestURL("http://api1.ousns.net/index.php?accesskey=519fc5ab85c8015b175449477561a827&client=2&%1&g=api/v1");

DataRequest::DataRequest(QNetworkAccessManager *network, QObject *parent)
    : QObject(*(new DataRequestPrivate(network)), parent)
{

}

void DataRequest::requestIndex(int mediaSize, int articleSize)
{
    Q_D(DataRequest);

    QString url(DataRequestPrivate::RequestURL.arg("a=index&m=index&tlimit=%1&alimit=%2"));
    setIsUpdatingIndex(d->startRequest(INDEX, QUrl(url.arg(mediaSize).arg(articleSize))));
}

void DataRequest::requestResource(int id)
{
    Q_D(DataRequest);
    QString url(DataRequestPrivate::RequestURL.arg("a=resource&m=index&id=%1"));
    d->startRequest(RESOURCE, QUrl(url.arg(id)));
}

void DataRequest::requestResourceItem(int id, int season, int episode)
{
    Q_D(DataRequest);
    QString url(DataRequestPrivate::RequestURL.arg("a=resource_item&m=index%1&id=%2"));
    d->startRequest(ITEM, QUrl(url.arg(season >= 0 ? QString("&season=%1&episode=%2").arg(season).arg(episode) : "").arg(id)));
}

bool DataRequest::getIsUpdatingIndex() const
{
    Q_D(const DataRequest);
    return d->m_isUpdatingIndex;
}

void DataRequest::setIsUpdatingIndex(bool value)
{
    Q_D(DataRequest);
    if (d->m_isUpdatingIndex == value)
        return ;

    d->m_isUpdatingIndex = value;
    emit isUpdatingIndexChanged(d->m_isUpdatingIndex);
}

DataRequestPrivate::DataRequestPrivate(QNetworkAccessManager *network)
    : m_isUpdatingIndex(false),
      m_network(network)
{

}

DataRequestPrivate::~DataRequestPrivate()
{

}

bool DataRequestPrivate::startRequest(int type, const QUrl &url)
{
    QNetworkRequest request(url);

    QNetworkReply *reply = m_network->get(request);
    if (reply == NULL)
        return false;

    reply->setReadBufferSize(0);
    reply->setTextModeEnabled(true);
    qDebug()<<"REQUEST:" << url.toString() << "\n\t> Header:" << reply->rawHeaderList();
    Q_Q(DataRequest);


    QObject::connect(reply, &QNetworkReply::finished, [=](){ emit q->dataReady(type, reply->readAll()); });
    QObject::connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                     [=](QNetworkReply::NetworkError code) {

        qDebug()<<"REQUEST:" << url.toString() << "\n\t ERROR: #" << code << ">" << reply->errorString();
    });
    QObject::connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal == 0)
        {
            emit q->downloadProgress(type, 0.0);
            return ;
        }

        if (bytesReceived >= bytesTotal)
        {
            emit q->downloadProgress(type, 1.0);
            return ;
        }

        emit q->downloadProgress(type, bytesReceived / bytesTotal);
    });

    return true;
}

