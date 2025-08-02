// ExampleDataPlugin.h - Example Data Provider Plugin demonstrating data processing
#pragma once

#include "../core/PluginInterface.h"
#include "../core/AdvancedInterfaces.h"
#include <QObject>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <QVersionNumber>
#include <QTimer>
#include <QDateTime>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextStream>
#include <QRandomGenerator>
#include <memory>

class DataModel;
class DataProcessor;
class DataImporter;
class DataExporter;
class DataValidator;
class DataGenerator;

class ExampleDataPlugin : public QObject, public IDataProviderPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.IPlugin/2.0" FILE "ExampleDataPlugin.json")
    Q_INTERFACES(IPlugin IDataProviderPlugin)

public:
    ExampleDataPlugin(QObject* parent = nullptr);
    ~ExampleDataPlugin() override;

    // IPlugin interface
    QString name() const override { return "Example Data Provider Plugin"; }
    QString description() const override { return "Demonstrates data processing, import/export, validation, and model creation capabilities"; }
    QVersionNumber version() const override { return QVersionNumber(1, 0, 0); }
    QString author() const override { return "Plugin Framework Team"; }
    QUuid uuid() const override { return QUuid("{aabbccdd-eeff-1122-3344-556677889900}"); }
    QString category() const override { return "Data"; }
    QString homepage() const override { return "https://example.com/data-plugin"; }
    QString license() const override { return "MIT"; }

    PluginCapabilities capabilities() const override {
        return PluginCapability::Database | PluginCapability::FileSystem | PluginCapability::Configuration;
    }

    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override { return m_initialized; }
    PluginStatus status() const override { return m_status; }

    // Configuration
    QJsonObject defaultConfiguration() const override;
    bool configure(const QJsonObject& config) override;
    QJsonObject currentConfiguration() const override;

    // IDataProviderPlugin interface
    QAbstractItemModel* createModel() override;
    QStringList supportedDataTypes() const override;
    bool canHandleData(const QMimeData* data) const override;
    QVariant processData(const QVariant& input) override;
    bool exportData(const QVariant& data, const QString& format, const QString& destination) override;

    // Commands
    QVariant executeCommand(const QString& command, const QVariantMap& params = {}) override;
    QStringList availableCommands() const override;

signals:
    void dataProcessed(const QString& operationId, const QVariant& result);
    void dataImported(const QString& source, int recordCount);
    void dataExported(const QString& destination, int recordCount);
    void validationCompleted(const QString& dataId, bool isValid, const QStringList& errors);
    void modelUpdated(const QString& modelId);

public slots:
    // Data operations
    void importData(const QString& source, const QString& format);
    void exportData(const QString& destination, const QString& format, const QVariant& data);
    void validateData(const QString& dataId, const QVariant& data);
    void processDataAsync(const QString& operationId, const QVariant& input, const QString& operation);
    
    // Model operations
    void addRecord(const QJsonObject& record);
    void updateRecord(int index, const QJsonObject& record);
    void removeRecord(int index);
    void clearAllRecords();
    void sortData(const QString& column, Qt::SortOrder order = Qt::AscendingOrder);
    void filterData(const QString& column, const QVariant& value);
    
    // Data generation
    void generateSampleData(int count, const QString& type = "mixed");
    void generateTimeSeriesData(int count, const QDateTime& startTime = QDateTime::currentDateTime());
    void generateRandomData(int count, const QJsonObject& schema);

private slots:
    void onDataProcessingFinished();
    void onImportFinished();
    void onExportFinished();
    void onValidationFinished();
    void onModelDataChanged();

private:
    void setupDataComponents();
    void cleanupDataComponents();
    void initializeDatabase();
    void createSampleData();
    QString generateOperationId();

    bool m_initialized = false;
    PluginStatus m_status = PluginStatus::Unknown;
    QJsonObject m_configuration;

    // Data components
    std::unique_ptr<DataModel> m_dataModel;
    std::unique_ptr<DataProcessor> m_dataProcessor;
    std::unique_ptr<DataImporter> m_dataImporter;
    std::unique_ptr<DataExporter> m_dataExporter;
    std::unique_ptr<DataValidator> m_dataValidator;
    std::unique_ptr<DataGenerator> m_dataGenerator;

    // Database
    QSqlDatabase m_database;
    QString m_databasePath;

    // Operation tracking
    QHash<QString, QVariant> m_pendingOperations;
    int m_operationCounter = 0;
};

// Data Model Class
class DataModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit DataModel(QObject* parent = nullptr);

    // Custom data operations
    void setData(const QJsonArray& data);
    QJsonArray getData() const;
    void addRecord(const QJsonObject& record);
    void updateRecord(int row, const QJsonObject& record);
    bool removeRecord(int row);
    void clearAllData();

    // Filtering and sorting
    void applyFilter(const QString& column, const QVariant& value);
    void clearFilter();
    void sortByColumn(const QString& column, Qt::SortOrder order);

    // Statistics
    int recordCount() const;
    QStringList columnNames() const;
    QVariantMap getColumnStatistics(const QString& column) const;
    QJsonObject getModelStatistics() const;

signals:
    void dataChanged();
    void recordAdded(int row);
    void recordRemoved(int row);
    void recordUpdated(int row);

private:
    void setupHeaders();
    QStandardItem* createItem(const QVariant& value);
    QVariant getItemValue(QStandardItem* item) const;

    QSortFilterProxyModel* m_proxyModel = nullptr;
    QStringList m_headers;
};

// Data Processor Class
class DataProcessor : public QObject
{
    Q_OBJECT

public:
    explicit DataProcessor(QObject* parent = nullptr);

    // Synchronous processing
    QVariant processData(const QVariant& input, const QString& operation);
    QJsonArray processJsonArray(const QJsonArray& input, const QString& operation);
    QJsonObject processJsonObject(const QJsonObject& input, const QString& operation);

    // Asynchronous processing
    void processDataAsync(const QString& operationId, const QVariant& input, const QString& operation);

    // Supported operations
    QStringList supportedOperations() const;
    bool supportsOperation(const QString& operation) const;

signals:
    void processingFinished(const QString& operationId, const QVariant& result);
    void processingError(const QString& operationId, const QString& error);

private slots:
    void performAsyncProcessing();

private:
    // Data transformation operations
    QVariant transformData(const QVariant& input, const QString& transformation);
    QVariant aggregateData(const QJsonArray& input, const QString& aggregation);
    QVariant filterData(const QJsonArray& input, const QJsonObject& criteria);
    QVariant sortData(const QJsonArray& input, const QString& column, Qt::SortOrder order);
    QVariant groupData(const QJsonArray& input, const QString& column);
    QVariant joinData(const QJsonArray& left, const QJsonArray& right, const QString& leftKey, const QString& rightKey);

    // Statistical operations
    QVariant calculateStatistics(const QJsonArray& input, const QString& column);
    double calculateMean(const QJsonArray& values);
    double calculateMedian(const QJsonArray& values);
    double calculateStandardDeviation(const QJsonArray& values);

    struct AsyncOperation {
        QString id;
        QVariant input;
        QString operation;
        QDateTime startTime;
    };

    QQueue<AsyncOperation> m_operationQueue;
    QTimer* m_processingTimer = nullptr;
    bool m_processing = false;
};

// Data Importer Class
class DataImporter : public QObject
{
    Q_OBJECT

public:
    explicit DataImporter(QObject* parent = nullptr);

    // Supported formats
    QStringList supportedFormats() const;
    bool supportsFormat(const QString& format) const;

    // Import operations
    QJsonArray importFromFile(const QString& filePath, const QString& format = QString());
    QJsonArray importFromString(const QString& data, const QString& format);
    QJsonArray importFromUrl(const QUrl& url, const QString& format = QString());

    // Format-specific imports
    QJsonArray importCsv(const QString& data, const QString& delimiter = ",");
    QJsonArray importJson(const QString& data);
    QJsonArray importXml(const QString& data, const QString& rootElement = "data");
    QJsonArray importSql(const QString& connectionString, const QString& query);

signals:
    void importStarted(const QString& source);
    void importProgress(int percentage);
    void importFinished(const QString& source, int recordCount);
    void importError(const QString& source, const QString& error);

private:
    QJsonObject parseCsvLine(const QString& line, const QString& delimiter, const QStringList& headers);
    QJsonObject parseXmlElement(QXmlStreamReader& reader);
    QString detectFormat(const QString& filePath);
};

// Data Exporter Class
class DataExporter : public QObject
{
    Q_OBJECT

public:
    explicit DataExporter(QObject* parent = nullptr);

    // Supported formats
    QStringList supportedFormats() const;
    bool supportsFormat(const QString& format) const;

    // Export operations
    bool exportToFile(const QJsonArray& data, const QString& filePath, const QString& format = QString());
    QString exportToString(const QJsonArray& data, const QString& format);
    bool exportToDatabase(const QJsonArray& data, const QString& connectionString, const QString& tableName);

    // Format-specific exports
    QString exportToCsv(const QJsonArray& data, const QString& delimiter = ",");
    QString exportToJson(const QJsonArray& data, bool formatted = true);
    QString exportToXml(const QJsonArray& data, const QString& rootElement = "data");
    QString exportToHtml(const QJsonArray& data, const QString& title = "Data Export");

signals:
    void exportStarted(const QString& destination);
    void exportProgress(int percentage);
    void exportFinished(const QString& destination, int recordCount);
    void exportError(const QString& destination, const QString& error);

private:
    QString formatJsonValue(const QJsonValue& value) const;
    void writeXmlElement(QXmlStreamWriter& writer, const QString& name, const QJsonValue& value);
    QString detectFormat(const QString& filePath);
};

// Data Validator Class
class DataValidator : public QObject
{
    Q_OBJECT

public:
    explicit DataValidator(QObject* parent = nullptr);

    // Validation operations
    bool validateData(const QVariant& data, const QJsonObject& schema);
    bool validateRecord(const QJsonObject& record, const QJsonObject& schema);
    QStringList validateDataWithErrors(const QVariant& data, const QJsonObject& schema);

    // Schema operations
    QJsonObject createSchema(const QJsonArray& sampleData);
    QJsonObject mergeSchemas(const QJsonObject& schema1, const QJsonObject& schema2);
    bool isValidSchema(const QJsonObject& schema);

    // Built-in validation rules
    bool validateRequired(const QJsonValue& value);
    bool validateType(const QJsonValue& value, const QString& expectedType);
    bool validateRange(const QJsonValue& value, double min, double max);
    bool validateLength(const QJsonValue& value, int minLength, int maxLength);
    bool validatePattern(const QJsonValue& value, const QString& pattern);
    bool validateEnum(const QJsonValue& value, const QJsonArray& allowedValues);

signals:
    void validationStarted(const QString& dataId);
    void validationFinished(const QString& dataId, bool isValid, const QStringList& errors);

private:
    QStringList validateField(const QString& fieldName, const QJsonValue& value, const QJsonObject& fieldSchema);
    QString getTypeString(const QJsonValue& value);
};

// Data Generator Class
class DataGenerator : public QObject
{
    Q_OBJECT

public:
    explicit DataGenerator(QObject* parent = nullptr);

    // Generation operations
    QJsonArray generateSampleData(int count, const QString& type = "mixed");
    QJsonArray generateTimeSeriesData(int count, const QDateTime& startTime, int intervalSeconds = 60);
    QJsonArray generateRandomData(int count, const QJsonObject& schema);
    QJsonArray generateSequentialData(int count, const QJsonObject& template_);

    // Data type generators
    QJsonValue generateRandomValue(const QString& type, const QJsonObject& constraints = {});
    QString generateRandomString(int length = 10, const QString& charset = "");
    int generateRandomInt(int min = 0, int max = 100);
    double generateRandomDouble(double min = 0.0, double max = 100.0);
    QDateTime generateRandomDateTime(const QDateTime& start = QDateTime::currentDateTime().addDays(-30), 
                                   const QDateTime& end = QDateTime::currentDateTime());
    bool generateRandomBool();

signals:
    void generationStarted(int count);
    void generationProgress(int percentage);
    void generationFinished(int count);

private:
    QJsonObject generatePersonData();
    QJsonObject generateCompanyData();
    QJsonObject generateProductData();
    QJsonObject generateTransactionData();

    QStringList m_firstNames;
    QStringList m_lastNames;
    QStringList m_companyNames;
    QStringList m_productNames;
    QStringList m_cities;
    QStringList m_countries;
};
