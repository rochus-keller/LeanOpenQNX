#include <QCoreApplication>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QSet>
#include <QMap>
#include <QString>
#include <QStringList>

class MakefileAnalyzer
{
public:
    MakefileAnalyzer() {}
    
    void analyzeFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        
        // Extract variable references: $(VAR) or ${VAR}
        QRegExp varPattern("\\$\\(([A-Za-z0-9_]+)\\)|\\$\\{([A-Za-z0-9_]+)\\}");
        int pos = 0;
        while ((pos = varPattern.indexIn(content, pos)) != -1) {
            QString var = varPattern.cap(1);
            if (var.isEmpty()) {
                var = varPattern.cap(2);
            }
            variablesUsed.insert(var);
            pos += varPattern.matchedLength();
        }
        
        // Extract variable definitions: VAR = value or VAR := value or VAR ?= value
        QRegExp defPattern("^([A-Za-z0-9_]+)\\s*[:?]?=", Qt::CaseSensitive);
        defPattern.setMinimal(false);
        
        QStringList lines = content.split('\n');
        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines.at(i).trimmed();
            
            // Skip comments
            if (line.startsWith('#')) {
                continue;
            }
            
            // Check for variable definition
            if (defPattern.indexIn(line) == 0) {
                QString var = defPattern.cap(1);
                variablesDefined.insert(var);
            }
            
            // Extract tool/command invocations from recipe lines (start with tab)
            if (lines.at(i).startsWith('\t')) {
                extractCommands(line);
            }
            
            // Extract from $(shell ...) constructs
            QRegExp shellPattern("\\$\\(shell\\s+([^)]+)\\)");
            int shellPos = 0;
            while ((shellPos = shellPattern.indexIn(line, shellPos)) != -1) {
                QString shellCmd = shellPattern.cap(1).trimmed();
                extractCommands(shellCmd);
                shellPos += shellPattern.matchedLength();
            }
        }
    }
    
    void printReport(const QString& moduleListPath)
    {
        QTextStream out(stdout);
        
        // Load module names from module.tmpl data if provided
        QSet<QString> knownModules;
        if (!moduleListPath.isEmpty()) {
            loadModuleNames(moduleListPath, knownModules);
        }
        
        out << QString(80, '=') << endl;
        out << "MAKEFILE ANALYSIS REPORT" << endl;
        out << QString(80, '=') << endl << endl;
        
        // Calculate undefined variables
        QSet<QString> undefined = variablesUsed;
        undefined.subtract(variablesDefined);
        
        // Remove common built-in make variables
        QStringList builtins;
        builtins << "MAKE" << "MAKEFILE_LIST" << "MAKEFLAGS" << "SHELL"
                 << "CC" << "CXX" << "LD" << "AR" << "AS" << "CPP"
                 << "CFLAGS" << "CXXFLAGS" << "LDFLAGS" << "ARFLAGS"
                 << "TARGET" << "CURDIR" << ".DEFAULT_GOAL";
        
        for (int i = 0; i < builtins.size(); ++i) {
            undefined.remove(builtins.at(i));
        }
        
        // Report undefined variables
        out << "UNDEFINED VARIABLES (" << undefined.size() << "):" << endl;
        out << QString(80, '-') << endl;
        
        QStringList undefinedList = undefined.toList();
        qSort(undefinedList);
        for (int i = 0; i < undefinedList.size(); ++i) {
            out << "  " << undefinedList.at(i) << endl;
        }
        out << endl;
        
        // Report all defined variables
        out << "DEFINED VARIABLES (" << variablesDefined.size() << "):" << endl;
        out << QString(80, '-') << endl;
        QStringList definedList = variablesDefined.toList();
        qSort(definedList);
        for (int i = 0; i < definedList.size(); ++i) {
            out << "  " << definedList.at(i) << endl;
        }
        out << endl;
        
        // Report commands/tools
        out << "COMMANDS/TOOLS INVOKED (" << commandsUsed.size() << "):" << endl;
        out << QString(80, '-') << endl;
        
        QStringList cmdList = commandsUsed.toList();
        qSort(cmdList);
        
        for (int i = 0; i < cmdList.size(); ++i) {
            QString cmd = cmdList.at(i);
            out << "  " << qSetFieldWidth(30) << left << cmd << qSetFieldWidth(0);
            
            // Check if this is a known QNX module
            if (!knownModules.isEmpty()) {
                if (knownModules.contains(cmd)) {
                    out << " [QNX module]";
                } else {
                    out << " [unknown]";
                }
            }
            out << endl;
        }
        
        out << endl << QString(80, '=') << endl;
    }
    
private:
    void extractCommands(const QString& line)
    {
        // Remove variable references for cleaner command extraction
        QString cleaned = line;
        cleaned.replace(QRegExp("\\$\\([^)]+\\)"), "");
        cleaned.replace(QRegExp("\\$\\{[^}]+\\}"), "");
        cleaned = cleaned.trimmed();
        
        if (cleaned.isEmpty()) {
            return;
        }
        
        // Extract first command (before pipe, semicolon, or &&)
        QRegExp cmdSep("[|;&]");
        int sepPos = cmdSep.indexIn(cleaned);
        if (sepPos != -1) {
            cleaned = cleaned.left(sepPos).trimmed();
        }
        
        // Get first word (the command)
        QStringList words = cleaned.split(QRegExp("\\s+"));
        if (!words.isEmpty()) {
            QString cmd = words.at(0);
            
            // Remove common shell prefixes
            if (cmd == "@" || cmd == "-" || cmd == "+" || cmd == "@-" || cmd == "-@") {
                if (words.size() > 1) {
                    cmd = words.at(1);
                }
            }
            
            // Skip common shell built-ins
            if (cmd != "cd" && cmd != "echo" && cmd != "test" && 
                cmd != "if" && cmd != "for" && cmd != "while" &&
                cmd != "case" && cmd != "export" && cmd != "set") {
                commandsUsed.insert(cmd);
            }
        }
    }
    
    void loadModuleNames(const QString& moduleListPath, QSet<QString>& modules)
    {
        // Simple parser to extract module names from your previous analysis
        // Expects lines with module names
        QFile file(moduleListPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty() && !line.startsWith('#')) {
                modules.insert(line);
            }
        }
        file.close();
    }
    
    QSet<QString> variablesUsed;
    QSet<QString> variablesDefined;
    QSet<QString> commandsUsed;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);
    
    if (argc < 2) {
        err << "Usage: " << argv[0] << " <root_directory> [module_list.txt]" << endl;
        err << "Analyzes all *.mk files for undefined variables and tool dependencies." << endl;
        return 1;
    }
    
    QString rootPath = QString::fromLocal8Bit(argv[1]);
    QString moduleList;
    
    if (argc >= 3) {
        moduleList = QString::fromLocal8Bit(argv[2]);
    }
    
    out << "Scanning: " << rootPath << endl;
    out << "Looking for *.mk files..." << endl << endl;
    
    MakefileAnalyzer analyzer;
    int fileCount = 0;
    
    QStringList nameFilters;
    nameFilters << "*.mk";
    
    QDirIterator it(rootPath, nameFilters, QDir::Files, 
                    QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        out << "Processing: " << filePath << endl;
        analyzer.analyzeFile(filePath);
        fileCount++;
    }
    
    out << endl << "Processed " << fileCount << " file(s)." << endl << endl;
    
    analyzer.printReport(moduleList);
    
    return 0;
}

