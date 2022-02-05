#include "QueryLatestVersionNumber.hpp"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>

#include "src/Helpers/Paths.hpp"

namespace Bloom
{
    void QueryLatestVersionNumber::run(TargetControllerConsole& targetControllerConsole) {
        auto* networkAccessManager = new QNetworkAccessManager(this);
        auto queryVersionEndpointUrl = QUrl(QString::fromStdString(Paths::homeDomainName() + "/latest-version"));
        queryVersionEndpointUrl.setScheme("http");
        queryVersionEndpointUrl.setQuery(QUrlQuery({
            {"currentVersionNumber", QString::fromStdString(this->currentVersionNumber.toString())}
        }));

        auto* response = networkAccessManager->get(QNetworkRequest(queryVersionEndpointUrl));
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
}
