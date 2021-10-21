#include "QueryLatestVersionNumber.hpp"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>

using namespace Bloom;

void QueryLatestVersionNumber::run(TargetControllerConsole& targetControllerConsole) {
    auto networkAccessManager = new QNetworkAccessManager(this);
    auto queryVersionEndpointUrl = QUrl("http://bloom.local/latest-version");
    queryVersionEndpointUrl.setQuery(QUrlQuery({
        {"currentVersionNumber", QString::fromStdString(this->currentVersionNumber.toString())}
    }));

    auto response = networkAccessManager->get(QNetworkRequest(queryVersionEndpointUrl));
    QObject::connect(response, &QNetworkReply::finished, this, [this, response] {
        const auto jsonResponseObject = QJsonDocument::fromJson(response->readAll()).object();

        if (jsonResponseObject.contains("latestVersionNumber")) {
            emit this->latestVersionNumberRetrieved(
                VersionNumber(
                    jsonResponseObject.value("latestVersionNumber").toString().toStdString()
                )
            );
        }
    });
}
