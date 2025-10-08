#include <QCoreApplication>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QStack>
#include <QTextStream>
#include <QXmlStreamReader>

class XmlAnalyzer
{
public:
    XmlAnalyzer() {}
    
    void analyzeFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream err(stderr);
            err << "Warning: Could not open file: " << filePath << endl;
            return;
        }
        
        QXmlStreamReader xml(&file);
        QStack<QString> elementStack;
        
        while (!xml.atEnd()) {
            xml.readNext();
            
            if (xml.isStartElement()) {
                QString elementName = xml.name().toString();
                
                // Count this element
                if (elementCounts.contains(elementName)) {
                    elementCounts[elementName] = elementCounts[elementName] + 1;
                } else {
                    elementCounts[elementName] = 1;
                }
                
                // Track parent-child relationship
                if (!elementStack.isEmpty()) {
                    QString parentElement = elementStack.top();
                    
                    if (!elementChildren.contains(parentElement)) {
                        elementChildren[parentElement] = QMap<QString, int>();
                    }
                    
                    QMap<QString, int>& children = elementChildren[parentElement];
                    if (children.contains(elementName)) {
                        children[elementName] = children[elementName] + 1;
                    } else {
                        children[elementName] = 1;
                    }
                }
                
                // Process attributes
                QXmlStreamAttributes attributes = xml.attributes();
                for (int i = 0; i < attributes.size(); ++i) {
                    QString attrName = attributes.at(i).name().toString();
                    
                    // Count this attribute globally
                    if (attributeCounts.contains(attrName)) {
                        attributeCounts[attrName] = attributeCounts[attrName] + 1;
                    } else {
                        attributeCounts[attrName] = 1;
                    }
                    
                    // Track which element has which attribute
                    if (!elementAttributes.contains(elementName)) {
                        elementAttributes[elementName] = QMap<QString, int>();
                    }
                    
                    QMap<QString, int>& attrs = elementAttributes[elementName];
                    if (attrs.contains(attrName)) {
                        attrs[attrName] = attrs[attrName] + 1;
                    } else {
                        attrs[attrName] = 1;
                    }
                }
                
                // Push current element onto stack
                elementStack.push(elementName);
                
            } else if (xml.isEndElement()) {
                // Pop element from stack
                if (!elementStack.isEmpty()) {
                    elementStack.pop();
                }
            }
        }
        
        if (xml.hasError()) {
            QTextStream err(stderr);
            err << "Warning: XML parse error in " << filePath << ": " 
                << xml.errorString() << endl;
        }
        
        file.close();
    }
    
    void printResults()
    {
        QTextStream out(stdout);
        
        out << QString(70, '=') << endl;
        out << "XML ANALYSIS RESULTS" << endl;
        out << QString(70, '=') << endl << endl;
        
        // Print element counts
        out << "ALL XML ELEMENTS DISCOVERED:" << endl;
        out << QString(70, '-') << endl;
        QMap<QString, int>::const_iterator elemIter;
        for (elemIter = elementCounts.constBegin(); 
             elemIter != elementCounts.constEnd(); ++elemIter) {
            out << qSetFieldWidth(40) << left << elemIter.key() 
                << qSetFieldWidth(0) << ": " 
                << elemIter.value() << " instances" << endl;
        }
        out << endl;
        
        // Print attribute counts
        out << "ALL ATTRIBUTES DISCOVERED:" << endl;
        out << QString(70, '-') << endl;
        QMap<QString, int>::const_iterator attrIter;
        for (attrIter = attributeCounts.constBegin(); 
             attrIter != attributeCounts.constEnd(); ++attrIter) {
            out << qSetFieldWidth(40) << left << attrIter.key() 
                << qSetFieldWidth(0) << ": " 
                << attrIter.value() << " instances" << endl;
        }
        out << endl;
        
        // Print hierarchical structure
        out << "HIERARCHICAL STRUCTURE:" << endl;
        out << QString(70, '-') << endl;
        
        QMap<QString, QMap<QString, int> >::const_iterator parentIter;
        
        // First, print element children relationships
        for (parentIter = elementChildren.constBegin(); 
             parentIter != elementChildren.constEnd(); ++parentIter) {
            
            QString parentElement = parentIter.key();
            const QMap<QString, int>& children = parentIter.value();
            
            out << endl << "Element: " << parentElement << endl;
            
            if (!children.isEmpty()) {
                out << "  Contains child elements:" << endl;
                QMap<QString, int>::const_iterator childIter;
                for (childIter = children.constBegin(); 
                     childIter != children.constEnd(); ++childIter) {
                    out << "    - " << qSetFieldWidth(35) << left << childIter.key() 
                        << qSetFieldWidth(0) << ": " 
                        << childIter.value() << " instances" << endl;
                }
            }
            
            // Print attributes for this element
            if (elementAttributes.contains(parentElement)) {
                const QMap<QString, int>& attrs = elementAttributes[parentElement];
                if (!attrs.isEmpty()) {
                    out << "  Has attributes:" << endl;
                    QMap<QString, int>::const_iterator attrIt;
                    for (attrIt = attrs.constBegin(); 
                         attrIt != attrs.constEnd(); ++attrIt) {
                        out << "    @ " << qSetFieldWidth(35) << left << attrIt.key() 
                            << qSetFieldWidth(0) << ": " 
                            << attrIt.value() << " instances" << endl;
                    }
                }
            }
        }
        
        // Print elements that have attributes but no children
        for (parentIter = elementAttributes.constBegin(); 
             parentIter != elementAttributes.constEnd(); ++parentIter) {
            
            QString elementName = parentIter.key();
            
            // Skip if already printed above
            if (elementChildren.contains(elementName)) {
                continue;
            }
            
            const QMap<QString, int>& attrs = parentIter.value();
            
            out << endl << "Element: " << elementName << endl;
            if (!attrs.isEmpty()) {
                out << "  Has attributes:" << endl;
                QMap<QString, int>::const_iterator attrIt;
                for (attrIt = attrs.constBegin(); 
                     attrIt != attrs.constEnd(); ++attrIt) {
                    out << "    @ " << qSetFieldWidth(35) << left << attrIt.key() 
                        << qSetFieldWidth(0) << ": " 
                        << attrIt.value() << " instances" << endl;
                }
            }
        }
        
        out << endl << QString(70, '=') << endl;
        out << "SUMMARY:" << endl;
        out << "  Total unique elements: " << elementCounts.size() << endl;
        out << "  Total unique attributes: " << attributeCounts.size() << endl;
        out << QString(70, '=') << endl;
    }
    
private:
    QMap<QString, int> elementCounts;
    QMap<QString, int> attributeCounts;
    QMap<QString, QMap<QString, int> > elementChildren;
    QMap<QString, QMap<QString, int> > elementAttributes;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);
    
    // Check if root directory argument is provided
    if (argc < 2) {
        err << "Usage: " << argv[0] << " <root_directory>" << endl;
        err << "Analyzes all module.tmpl files in the specified directory tree." << endl;
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
    
    XmlAnalyzer analyzer;
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
    
    // Print analysis results
    analyzer.printResults();
    
    return 0;
}

