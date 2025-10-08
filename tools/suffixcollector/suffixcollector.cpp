#include <QCoreApplication>
#include <QDirIterator>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);
    
    // Check if root directory argument is provided
    if (argc < 2) {
        err << "Usage: " << argv[0] << " <root_directory>" << endl;
        return 1;
    }
    
    QString rootPath = QString::fromLocal8Bit(argv[1]);
    QFileInfo rootInfo(rootPath);
    
    // Verify that the root directory exists and is a directory
    if (!rootInfo.exists()) {
        err << "Error: Directory does not exist: " << rootPath << endl;
        return 1;
    }
    
    if (!rootInfo.isDir()) {
        err << "Error: Path is not a directory: " << rootPath << endl;
        return 1;
    }
    
    // Map to store suffix/basename counts (automatically sorted by key)
    QMap<QString, int> suffixCounts;
    
    // Iterate recursively through all files in the directory tree
    QDirIterator it(rootPath, QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        
        QString key;
        QString suffix = fileInfo.suffix();
        
        if (suffix.isEmpty()) {
            // No suffix, use base name instead
            key = fileInfo.baseName();
        } else {
            // Use the suffix
            key = suffix;
        }
        
        // Increment the count for this suffix/basename
        if (suffixCounts.contains(key)) {
            suffixCounts[key] = suffixCounts[key] + 1;
        } else {
            suffixCounts[key] = 1;
        }
    }
    
    // Output results alphabetically (QMap keeps keys sorted)
    out << "File suffix/basename statistics for: " << rootPath << endl;
    out << QString(60, '-') << endl;
    
    QMap<QString, int>::const_iterator iter;
    for (iter = suffixCounts.constBegin(); iter != suffixCounts.constEnd(); ++iter) {
        out << qSetFieldWidth(30) << left << iter.key() 
            << qSetFieldWidth(0) << ": " 
            << iter.value() << endl;
    }
    
    out << QString(60, '-') << endl;
    out << "Total unique suffixes/basenames: " << suffixCounts.size() << endl;
    
    return 0;
}

