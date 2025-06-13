#include "Query.h"

#include "NeuronDescriptor.h"

#include "util/FileUtil.h"

#include "curl/curl.h"

#include <iostream>

#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

size_t curlReplyCallback(void* contents, size_t size, size_t nmemb, std::string* s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char*)contents, newLength);
    }
    catch (std::bad_alloc& e)
    {
        //handle memory problem
        return 0;
    }
    return newLength;
}

QString Query::loadQueryFromFile(std::string fileName)
{
    // Open the input file
    QString contents = mv::util::loadFileContents(QString::fromStdString(fileName));
    contents = contents.replace("\"", "\\\"");
    contents = contents.simplified();
    contents = QString("{\"query\":\"") + contents + "\"}";

    std::cout << contents.toStdString() << std::endl;
    return contents;
}

std::vector<NeuronDescriptor> Query::send()
{
    QString query = loadQueryFromFile(":/query.q");

    // Dynamically allocate memory for the returned string
    char* cQuery = new char[query.toStdString().size() + 1]; // +1 for terminating NUL

    // Copy source string in dynamically allocated string buffer
    strcpy(cQuery, query.toStdString().c_str());

    std::string result;

    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    std::string url = "https://idf-api-dev.aibs-idk-dev.net";

    if (curl) {
        struct curl_slist* hs = NULL;
        hs = curl_slist_append(hs, "Content-Type: application/json");
        hs = curl_slist_append(hs, "Accept: application/json");
        //hs = curl_slist_append(slist1, "Connection: keep-alive");
        hs = curl_slist_append(hs, "DNT: 1");
        //hs = curl_slist_append(slist1, "Origin: https://idf-api-test.aibs-idk-test.net");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlReplyCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
        //curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cQuery);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)-1);
        //curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        //curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        curl_slist_free_all(hs);

        /* always cleanup */
        curl_easy_cleanup(curl);

        free(cQuery);
    }

    qDebug() << "BEEEEP" << result.c_str();

    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(result).toUtf8());

    QJsonObject jsonObj = doc.object();

    QJsonArray sd = jsonObj["data"].toObject()["aio_specimen"].toArray();
    qDebug() << sd.size();

    std::vector<NeuronDescriptor> neuronDescriptors;
    for (int i = 0; i < sd.size(); i++)
    {
        NeuronDescriptor descriptor;

        QJsonObject specimen = sd[i].toObject();
        QJsonObject cRID = specimen["cRID"].toObject();
        QString symbol = cRID["symbol"].toString();

        descriptor.symbol = symbol;
        qDebug() << symbol;

        // Images
        QJsonArray images = specimen["images"].toArray();
        for (int j = 0; j < images.size(); j++)
        {
            QString referenceId = images[j].toObject()["featureType"].toObject()["referenceId"].toString();

            QString url = images[j].toObject()["url"].toString();
            qDebug() << referenceId;
            qDebug() << url;

            if (referenceId == "3JH1B8UL3YEO4UAFC6Y")
            {
                descriptor.morphologyUrl = url;
                qDebug() << url;
            }
            if (referenceId == "YAHK4SNZMIJ7MO6C39X")
            {
                descriptor.evUrl = url;
            }
            if (referenceId == "EPMT6VK9G59620WK8G3")
            {
                descriptor.wheelUrl = url;
            }
        }

        // Annotations
        QJsonArray annotations = specimen["annotations"].toArray();
        for (int j = 0; j < annotations.size(); j++)
        {
            QString referenceId = annotations[j].toObject()["featureType"].toObject()["referenceId"].toString();
            QJsonArray taxons = annotations[j].toObject()["taxons"].toArray();

            for (int k = 0; k < taxons.size(); k++)
            {
                QString symbol = taxons[k].toObject()["symbol"].toString();
                if (referenceId == "Z8BZR1LDP9G4I97O394")
                {
                    descriptor.tTypeClass = symbol;
                }
                if (referenceId == "O267WEUPC2MMK3MI7ZY")
                {
                    descriptor.tTypeSubClass = symbol;
                }
                if (referenceId == "DW0F0S320VR4NX0DBPT")
                {
                    descriptor.tType = symbol;
                }
                if (referenceId == "17Q8WESJFABC8QV7O69")
                {
                    descriptor.corticalLayer = symbol;
                }
            }
        }
        qDebug() << descriptor.tTypeClass;
        qDebug() << descriptor.tType;
        neuronDescriptors.push_back(descriptor);
    }
    return neuronDescriptors;
}
