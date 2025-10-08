#include <QCoreApplication>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QFileInfo>

class BuildInfo
{
public:
    QString path;
    QString name;
    QString usageMsg;
    QString installDir;
    QString description;
    QStringList libs;
    QStringList extraLibs;
    QStringList ccFlags;
    QStringList ldFlags;
    QMap<QString, QString> otherVars;
    
    BuildInfo() {}
};

class CommonMkParser
{
public:
    CommonMkParser() {}
    
    BuildInfo parseFile(const QString& filePath)
    {
        BuildInfo info;
        info.path = filePath;
        
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return info;
        }
        
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        
        // Remove comments and blank lines
        QStringList lines = content.split('\n');
        QStringList cleanLines;
        
        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines.at(i);
            
            // Remove comments
            int commentPos = line.indexOf('#');
            if (commentPos >= 0) {
                line = line.left(commentPos);
            }
            
            line = line.trimmed();
            if (!line.isEmpty()) {
                cleanLines.append(line);
            }
        }
        
        // Parse variables
        for (int i = 0; i < cleanLines.size(); ++i) {
            QString line = cleanLines.at(i);
            
            // Handle multi-line definitions (ending with backslash)
            while (line.endsWith("\\") && i + 1 < cleanLines.size()) {
                line = line.left(line.length() - 1).trimmed();
                i++;
                line += " " + cleanLines.at(i).trimmed();
            }
            
            parseVariable(line, info);
        }
        
        return info;
    }
    
    void printReport(const QMap<QString, BuildInfo>& buildInfos)
    {
        QTextStream out(stdout);
        
        out << QString(100, '=') << endl;
        out << "COMMON.MK BUILD METADATA ANALYSIS" << endl;
        out << QString(100, '=') << endl << endl;
        
        out << "Total projects: " << buildInfos.size() << endl << endl;
        
        QMap<QString, BuildInfo>::const_iterator it;
        for (it = buildInfos.constBegin(); it != buildInfos.constEnd(); ++it) {
            const BuildInfo& info = it.value();
            
            out << QString(100, '-') << endl;
            out << "Project: " << info.path << endl;
            out << QString(100, '-') << endl;
            
            if (!info.name.isEmpty()) {
                out << "  NAME:        " << info.name << endl;
            }
            
            if (!info.usageMsg.isEmpty()) {
                out << "  USEMSG:      " << info.usageMsg << endl;
            }
            
            if (!info.installDir.isEmpty()) {
                out << "  INSTALLDIR:  " << info.installDir << endl;
            }
            
            if (!info.description.isEmpty()) {
                out << "  DESCRIPTION: " << info.description << endl;
            }
            
            if (!info.libs.isEmpty()) {
                out << "  LIBS:        " << info.libs.join(" ") << endl;
            }
            
            if (!info.extraLibs.isEmpty()) {
                out << "  EXTRA_LIBS:  " << info.extraLibs.join(" ") << endl;
            }
            
            if (!info.ccFlags.isEmpty()) {
                out << "  CCFLAGS:     " << info.ccFlags.join(" ") << endl;
            }
            
            if (!info.ldFlags.isEmpty()) {
                out << "  LDFLAGS:     " << info.ldFlags.join(" ") << endl;
            }
            
            if (!info.otherVars.isEmpty()) {
                QMap<QString, QString>::const_iterator varIt;
                for (varIt = info.otherVars.constBegin(); 
                     varIt != info.otherVars.constEnd(); ++varIt) {
                    out << "  " << qSetFieldWidth(12) << left << varIt.key() 
                        << qSetFieldWidth(0) << ": " << varIt.value() << endl;
                }
            }
            
            out << endl;
        }
        
        out << QString(100, '=') << endl;
        
        // Summary statistics
        printSummary(buildInfos);
    }
    
private:
    void parseVariable(const QString& line, BuildInfo& info)
    {
        // Match variable assignment: VAR = value or VAR := value or VAR += value
        QRegExp varAssign("^([A-Za-z0-9_]+)\\s*([:+]?=)\\s*(.*)$");
        
        if (varAssign.indexIn(line) != 0) {
            return;
        }
        
        QString varName = varAssign.cap(1);
        QString op = varAssign.cap(2);
        QString value = varAssign.cap(3).trimmed();
        
        // Special handling for known variables
        if (varName == "NAME") {
            info.name = value;
        }
        else if (varName == "USEMSG") {
            info.usageMsg = value;
        }
        else if (varName == "INSTALLDIR") {
            info.installDir = value;
        }
        else if (varName == "LIBS") {
            info.libs = value.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        }
        else if (varName == "EXTRA_LIBS") {
            info.extraLibs = value.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        }
        else if (varName == "CCFLAGS") {
            info.ccFlags = value.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        }
        else if (varName == "LDFLAGS") {
            info.ldFlags = value.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        }
        else if (varName == "PINFO") {
            // PINFO often contains DESCRIPTION=...
            QRegExp descPattern("DESCRIPTION\\s*=\\s*(.+)");
            if (descPattern.indexIn(value) >= 0) {
                info.description = descPattern.cap(1).trimmed();
            }
        }
        else {
            // Store other variables
            if (!value.isEmpty()) {
                info.otherVars[varName] = value;
            }
        }
    }
    
    void printSummary(const QMap<QString, BuildInfo>& buildInfos)
    {
        QTextStream out(stdout);
        
        out << "SUMMARY" << endl;
        out << QString(100, '-') << endl;
        
        // Count by install directory
        QMap<QString, int> installDirCounts;
        QMap<QString, QStringList> libUsage;
        
        QMap<QString, BuildInfo>::const_iterator it;
        for (it = buildInfos.constBegin(); it != buildInfos.constEnd(); ++it) {
            const BuildInfo& info = it.value();
            
            if (!info.installDir.isEmpty()) {
                installDirCounts[info.installDir]++;
            }
            
            for (int i = 0; i < info.libs.size(); ++i) {
                QString lib = info.libs.at(i);
                if (!libUsage[lib].contains(info.name)) {
                    libUsage[lib].append(info.name);
                }
            }
        }
        
        out << endl << "Install Directories:" << endl;
        QMap<QString, int>::const_iterator dirIt;
        for (dirIt = installDirCounts.constBegin(); 
             dirIt != installDirCounts.constEnd(); ++dirIt) {
            out << "  " << qSetFieldWidth(40) << left << dirIt.key() 
                << qSetFieldWidth(0) << ": " << dirIt.value() << " projects" << endl;
        }
        
        out << endl << "Most Used Libraries:" << endl;
        QList<QPair<int, QString> > libsByCount;
        QMap<QString, QStringList>::const_iterator libIt;
        for (libIt = libUsage.constBegin(); libIt != libUsage.constEnd(); ++libIt) {
            libsByCount.append(qMakePair(libIt.value().size(), libIt.key()));
        }
        qSort(libsByCount);
        
        for (int i = libsByCount.size() - 1; i >= 0 && i >= libsByCount.size() - 20; --i) {
            out << "  " << qSetFieldWidth(30) << left << libsByCount.at(i).second 
                << qSetFieldWidth(0) << ": used by " 
                << libsByCount.at(i).first << " projects" << endl;
        }
        
        out << QString(100, '=') << endl;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);
    
    if (argc < 2) {
        err << "Usage: " << argv[0] << " <root_directory>" << endl;
        err << "Extracts build metadata from all common.mk files." << endl;
        return 1;
    }
    
    QString rootPath = QString::fromLocal8Bit(argv[1]);
    
    out << "Scanning: " << rootPath << endl;
    out << "Looking for common.mk files..." << endl << endl;
    
    CommonMkParser parser;
    QMap<QString, BuildInfo> buildInfos;
    int fileCount = 0;
    
    QStringList nameFilters;
    nameFilters << "common.mk";
    
    QDirIterator it(rootPath, nameFilters, QDir::Files, 
                    QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        
        QFileInfo fileInfo(filePath);
        QString relativePath = fileInfo.absolutePath().remove(rootPath);
        if (relativePath.startsWith('/')) {
            relativePath = relativePath.mid(1);
        }
        
        BuildInfo info = parser.parseFile(filePath);
        buildInfos[relativePath] = info;
        fileCount++;
    }
    
    out << "Processed " << fileCount << " file(s)." << endl << endl;
    
    parser.printReport(buildInfos);
    
    return 0;
}

