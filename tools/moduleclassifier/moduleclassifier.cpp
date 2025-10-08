#include <QCoreApplication>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamReader>

class ModuleAnalyzer
{
public:
    ModuleAnalyzer() : incompleteCount(0) {}
    
    void analyzeFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream err(stderr);
            err << "Warning: Could not open file: " << filePath << endl;
            return;
        }
        
        QXmlStreamReader xml(&file);
        
        QString groupOwner;
        QString classification;
        QString type;
        QString moduleName;
        
        while (!xml.atEnd()) {
            xml.readNext();
            
            if (xml.isStartElement()) {
                QString elementName = xml.name().toString();
                
                if (elementName == "GroupOwner") {
                    groupOwner = xml.readElementText().trimmed();
                }
                else if (elementName == "classification") {
                    classification = xml.readElementText().trimmed();
                }
                else if (elementName == "type") {
                    type = xml.readElementText().trimmed();
                }
                else if (elementName == "module") {
                    QXmlStreamAttributes attributes = xml.attributes();
                    if (attributes.hasAttribute("name")) {
                        moduleName = attributes.value("name").toString().trimmed();
                    }
                }
            }
        }
        
        if (xml.hasError()) {
            QTextStream err(stderr);
            err << "Warning: XML parse error in " << filePath << ": " 
                << xml.errorString() << endl;
        }
        
        file.close();
        
        // Check for missing fields and log them
        bool hasIssues = false;
        QTextStream err(stderr);
        
        if (groupOwner.isEmpty()) {
            err << "Warning: Missing GroupOwner in " << filePath << endl;
            groupOwner = "(no GroupOwner)";
            hasIssues = true;
        }
        if (classification.isEmpty()) {
            err << "Warning: Missing classification in " << filePath << endl;
            classification = "(no classification)";
            hasIssues = true;
        }
        if (type.isEmpty()) {
            err << "Warning: Missing type in " << filePath << endl;
            type = "(no type)";
            hasIssues = true;
        }
        if (moduleName.isEmpty()) {
            err << "Warning: Missing module name attribute in " << filePath << endl;
            moduleName = "(no module name)";
            hasIssues = true;
        }
        
        if (hasIssues) {
            incompleteCount++;
        }
        
        // Always add to hierarchy (using placeholders if needed)
        addToHierarchy(groupOwner, classification, type, moduleName);
    }
    
    void printHierarchy()
    {
        QTextStream out(stdout);
        
        if (incompleteCount > 0) {
            out << endl << "WARNING: Found " << incompleteCount 
                << " file(s) with incomplete data (see warnings above)." 
                << endl << endl;
        }
        
        out << QString(80, '=') << endl;
        out << "MODULE HIERARCHY" << endl;
        out << QString(80, '=') << endl << endl;
        
        // Iterate through GroupOwners (Level 0)
        QMap<QString, QMap<QString, QMap<QString, QMap<QString, int> > > >::const_iterator ownerIter;
        
        for (ownerIter = hierarchy.constBegin(); 
             ownerIter != hierarchy.constEnd(); ++ownerIter) {
            
            QString groupOwner = ownerIter.key();
            const QMap<QString, QMap<QString, QMap<QString, int> > >& classificationMap = ownerIter.value();
            
            // Calculate total count for this GroupOwner
            int ownerTotal = calculateTotal(classificationMap);
            
            out << groupOwner << " [" << ownerTotal << "]" << endl;
            
            // Iterate through classifications (Level 1)
            QMap<QString, QMap<QString, QMap<QString, int> > >::const_iterator classIter;
            
            for (classIter = classificationMap.constBegin(); 
                 classIter != classificationMap.constEnd(); ++classIter) {
                
                QString classification = classIter.key();
                const QMap<QString, QMap<QString, int> >& typeMap = classIter.value();
                
                // Calculate total count for this classification
                int classTotal = calculateTotal(typeMap);
                
                out << "  " << classification << " [" << classTotal << "]" << endl;
                
                // Iterate through types (Level 2)
                QMap<QString, QMap<QString, int> >::const_iterator typeIter;
                
                for (typeIter = typeMap.constBegin(); 
                     typeIter != typeMap.constEnd(); ++typeIter) {
                    
                    QString type = typeIter.key();
                    const QMap<QString, int>& moduleMap = typeIter.value();
                    
                    // Calculate total count for this type
                    int typeTotal = calculateTotal(moduleMap);
                    
                    out << "    " << type << " [" << typeTotal << "]" << endl;
                    
                    // Iterate through module names (Level 3)
                    QMap<QString, int>::const_iterator moduleIter;
                    
                    for (moduleIter = moduleMap.constBegin(); 
                         moduleIter != moduleMap.constEnd(); ++moduleIter) {
                        
                        QString moduleName = moduleIter.key();
                        int count = moduleIter.value();
                        
                        out << "      " << moduleName << " [" << count << "]" << endl;
                    }
                }
            }
            
            out << endl;
        }
        
        out << QString(80, '=') << endl;
    }
    
private:
    void addToHierarchy(const QString& groupOwner, 
                       const QString& classification,
                       const QString& type, 
                       const QString& moduleName)
    {
        // Navigate/create the nested structure
        if (!hierarchy.contains(groupOwner)) {
            hierarchy[groupOwner] = QMap<QString, QMap<QString, QMap<QString, int> > >();
        }
        
        QMap<QString, QMap<QString, QMap<QString, int> > >& classificationMap = hierarchy[groupOwner];
        
        if (!classificationMap.contains(classification)) {
            classificationMap[classification] = QMap<QString, QMap<QString, int> >();
        }
        
        QMap<QString, QMap<QString, int> >& typeMap = classificationMap[classification];
        
        if (!typeMap.contains(type)) {
            typeMap[type] = QMap<QString, int>();
        }
        
        QMap<QString, int>& moduleMap = typeMap[type];
        
        // Increment count for this module name
        if (moduleMap.contains(moduleName)) {
            moduleMap[moduleName] = moduleMap[moduleName] + 1;
        } else {
            moduleMap[moduleName] = 1;
        }
    }
    
    // Helper function to calculate totals at different levels
    int calculateTotal(const QMap<QString, int>& map) const
    {
        int total = 0;
        QMap<QString, int>::const_iterator iter;
        for (iter = map.constBegin(); iter != map.constEnd(); ++iter) {
            total += iter.value();
        }
        return total;
    }
    
    int calculateTotal(const QMap<QString, QMap<QString, int> >& map) const
    {
        int total = 0;
        QMap<QString, QMap<QString, int> >::const_iterator iter;
        for (iter = map.constBegin(); iter != map.constEnd(); ++iter) {
            total += calculateTotal(iter.value());
        }
        return total;
    }
    
    int calculateTotal(const QMap<QString, QMap<QString, QMap<QString, int> > >& map) const
    {
        int total = 0;
        QMap<QString, QMap<QString, QMap<QString, int> > >::const_iterator iter;
        for (iter = map.constBegin(); iter != map.constEnd(); ++iter) {
            total += calculateTotal(iter.value());
        }
        return total;
    }
    
    // 4-level nested hierarchy: GroupOwner -> classification -> type -> module name -> count
    QMap<QString, QMap<QString, QMap<QString, QMap<QString, int> > > > hierarchy;
    int incompleteCount;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);
    
    // Check if root directory argument is provided
    if (argc < 2) {
        err << "Usage: " << argv[0] << " <root_directory>" << endl;
        err << "Analyzes all module.tmpl files and displays hierarchical module structure." << endl;
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
    
    out << "Scanning directory: " << rootPath << endl;
    out << "Looking for module.tmpl files..." << endl << endl;
    
    ModuleAnalyzer analyzer;
    int fileCount = 0;
    
    // Create name filter for module.tmpl files
    QStringList nameFilters;
    nameFilters << "module.tmpl";
    
    // Iterate recursively through all module.tmpl files
    QDirIterator it(rootPath, nameFilters, QDir::Files, 
                    QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        out << "Processing: " << filePath << endl;
        analyzer.analyzeFile(filePath);
        fileCount++;
    }
    
    out << endl << "Processed " << fileCount << " file(s)." << endl << endl;
    
    if (fileCount == 0) {
        out << "No module.tmpl files found in the specified directory." << endl;
        return 0;
    }
    
    // Print hierarchical structure
    analyzer.printHierarchy();
    
    return 0;
}

