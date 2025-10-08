#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QFileDialog>
#include <QDirIterator>
#include <QFile>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QStack>

class XmlElement
{
public:
    QString name;
    QString text;
    QMap<QString, QString> attributes;
    QList<XmlElement*> children;
    
    XmlElement() {}
    
    ~XmlElement()
    {
        for (int i = 0; i < children.size(); ++i) {
            delete children[i];
        }
    }
};

class ModuleData
{
public:
    QString groupOwner;
    QString classification;
    QString moduleName;
    QString relativePath;
    XmlElement* rootElement;
    
    ModuleData() : rootElement(0) {}
    
    ~ModuleData()
    {
        delete rootElement;
    }
};

class XmlTreeItem
{
public:
    QString elementName;
    QString elementValue;
    QMap<QString, QString> attributes;
    QTreeWidgetItem* treeItem;
    
    XmlTreeItem() : treeItem(0) {}
};

class ModuleViewer : public QMainWindow
{
    Q_OBJECT
    
public:
    ModuleViewer(QWidget* parent = 0) : QMainWindow(parent)
    {
        setupUi();
        setWindowTitle("Module Template Viewer");
        resize(1200, 800);
    }
    
    ~ModuleViewer()
    {
        clearModules();
    }

    void loadModules(const QString& path)
    {
        rootPath = path;
        loadModules();
    }
    
private slots:
    void onSelectDirectory()
    {
        QString dir = QFileDialog::getExistingDirectory(
            this, "Select Root Directory", "",
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        
        if (!dir.isEmpty()) {
            rootPath = dir;
            loadModules();
        }
    }
    
    void onTreeItemSelected()
    {
        QTreeWidgetItem* current = tree->currentItem();
        if (!current) {
            attributeTable->setRowCount(0);
            return;
        }
        
        XmlTreeItem* item = itemMap.value(current, 0);
        if (!item) {
            attributeTable->setRowCount(0);
            return;
        }
        
        attributeTable->setRowCount(item->attributes.size());
        
        int row = 0;
        QMap<QString, QString>::const_iterator it;
        for (it = item->attributes.constBegin(); 
             it != item->attributes.constEnd(); ++it) {
            
            QTableWidgetItem* nameItem = new QTableWidgetItem(it.key());
            QTableWidgetItem* valueItem = new QTableWidgetItem(it.value());
            
            attributeTable->setItem(row, 0, nameItem);
            attributeTable->setItem(row, 1, valueItem);
            row++;
        }
        
        attributeTable->resizeColumnsToContents();
    }
    
private:
    void setupUi()
    {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(5, 5, 5, 5);
        mainLayout->setSpacing(5);
        mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        buttonLayout->setSizeConstraint(QLayout::SetMinimumSize);

        QPushButton* selectBtn = new QPushButton("Select Directory...", this);
        connect(selectBtn, SIGNAL(clicked()), this, SLOT(onSelectDirectory()));
        buttonLayout->addWidget(selectBtn);
        buttonLayout->addStretch();
        
        statusLabel = new QLabel("No directory selected", this);
        buttonLayout->addWidget(statusLabel);

        mainLayout->addLayout(buttonLayout);
        
        QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

        tree = new QTreeWidget(this);
        tree->setColumnCount(2);
        QStringList headers;
        headers << "Element" << "Type/Value" << "Path/Attrs";
        tree->setHeaderLabels(headers);
        tree->setAlternatingRowColors(true);
        connect(tree, SIGNAL(itemSelectionChanged()), 
                this, SLOT(onTreeItemSelected()));
        
        splitter->addWidget(tree);
        
        attributeTable = new QTableWidget(this);
        attributeTable->setColumnCount(2);
        QStringList attrHeaders;
        attrHeaders << "Attribute" << "Value";
        attributeTable->setHorizontalHeaderLabels(attrHeaders);
        attributeTable->horizontalHeader()->setStretchLastSection(true);
        attributeTable->setAlternatingRowColors(true);
        attributeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        attributeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        
        splitter->addWidget(attributeTable);
        
        splitter->setStretchFactor(0, 3);
        splitter->setStretchFactor(1, 1);
        
        mainLayout->addWidget(splitter, 1);
    }
    
    void clearModules()
    {
        for (int i = 0; i < modules.size(); ++i) {
            delete modules[i];
        }
        modules.clear();
    }
    
    void loadModules()
    {
        tree->clear();
        itemMap.clear();
        clearModules();
        
        statusLabel->setText("Scanning...");
        QApplication::processEvents();
        
        QStringList nameFilters;
        nameFilters << "module.tmpl";
        
        QDirIterator it(rootPath, nameFilters, QDir::Files, 
                        QDirIterator::Subdirectories);
        
        int fileCount = 0;
        while (it.hasNext()) {
            QString filePath = it.next();
            parseModuleFile(filePath);
            fileCount++;
        }
        
        buildTree();
        
        statusLabel->setText(QString("Loaded %1 module(s) from %2")
                            .arg(fileCount).arg(rootPath));
    }
    
    XmlElement* parseXmlElement(QXmlStreamReader& xml)
    {
        XmlElement* element = new XmlElement();
        element->name = xml.name().toString();
        
        QXmlStreamAttributes attrs = xml.attributes();
        for (int i = 0; i < attrs.size(); ++i) {
            element->attributes[attrs.at(i).name().toString()] = 
                attrs.at(i).value().toString();
        }
        
        while (!xml.atEnd()) {
            xml.readNext();
            
            if (xml.isEndElement()) {
                break;
            }
            
            if (xml.isStartElement()) {
                XmlElement* child = parseXmlElement(xml);
                element->children.append(child);
            }
            else if (xml.isCharacters() && !xml.isWhitespace()) {
                QString text = xml.text().toString().trimmed();
                if (!text.isEmpty()) {
                    element->text = text;
                }
            }
        }
        
        return element;
    }
    
    void parseModuleFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        
        ModuleData* data = new ModuleData();
        QDir rootDir(rootPath);
        data->relativePath = rootDir.relativeFilePath(filePath);
        data->relativePath.chop(12); // "/module.tmpl"
        
        QXmlStreamReader xml(&file);
        
        while (!xml.atEnd()) {
            xml.readNext();
            
            if (xml.isStartElement()) {
                QString elementName = xml.name().toString();
                
                if (elementName == "module") {
                    data->rootElement = parseXmlElement(xml);
                    
                    if (data->rootElement->attributes.contains("name")) {
                        data->moduleName = data->rootElement->attributes["name"];
                    }
                    
                    extractTopLevelFields(data->rootElement, data);
                    break;
                }
            }
        }
        
        file.close();
        
        if (data->groupOwner.isEmpty()) {
            data->groupOwner = "(no GroupOwner)";
        }
        if (data->classification.isEmpty()) {
            data->classification = "(no classification)";
        }
        if (data->moduleName.isEmpty()) {
            data->moduleName = "(no name)";
        }
        
        modules.append(data);
    }
    
    void extractTopLevelFields(XmlElement* root, ModuleData* data)
    {
        if (!root) return;
        
        for (int i = 0; i < root->children.size(); ++i) {
            XmlElement* child = root->children.at(i);

            if (child->name == "GroupOwner") {
                data->groupOwner = child->text;
            }
            else if (child->name == "classification") {
                data->classification = child->text;
            }
        }
    }
    
    void buildTree()
    {
        QMap<QString, QMap<QString, QList<ModuleData*> > > hierarchy;
        
        for (int i = 0; i < modules.size(); ++i) {
            ModuleData* mod = modules.at(i);
            hierarchy[mod->groupOwner][mod->classification].append(mod);
        }
        
        QMap<QString, QMap<QString, QList<ModuleData*> > >::const_iterator ownerIt;
        
        for (ownerIt = hierarchy.constBegin(); 
             ownerIt != hierarchy.constEnd(); ++ownerIt) {
            
            QString groupOwner = ownerIt.key();
            QTreeWidgetItem* ownerItem = new QTreeWidgetItem(tree);
            ownerItem->setText(0, groupOwner);
            ownerItem->setText(1, "GroupOwner");
            
            XmlTreeItem* ownerData = new XmlTreeItem();
            ownerData->elementName = "GroupOwner";
            ownerData->elementValue = groupOwner;
            ownerData->treeItem = ownerItem;
            itemMap[ownerItem] = ownerData;
            
            const QMap<QString, QList<ModuleData*> >& classifications = ownerIt.value();
            QMap<QString, QList<ModuleData*> >::const_iterator classIt;
            
            for (classIt = classifications.constBegin(); 
                 classIt != classifications.constEnd(); ++classIt) {
                
                QString classification = classIt.key();
                QTreeWidgetItem* classItem = new QTreeWidgetItem(ownerItem);
                classItem->setText(0, classification);
                classItem->setText(1, "classification");
                
                XmlTreeItem* classData = new XmlTreeItem();
                classData->elementName = "classification";
                classData->elementValue = classification;
                classData->treeItem = classItem;
                itemMap[classItem] = classData;
                
                const QList<ModuleData*>& moduleList = classIt.value();
                
                for (int i = 0; i < moduleList.size(); ++i) {
                    ModuleData* mod = moduleList.at(i);
                    
                    QTreeWidgetItem* modItem = new QTreeWidgetItem(classItem);
                    modItem->setText(0, mod->moduleName);
                    modItem->setText(1, "module");
                    modItem->setText(2, mod->relativePath);

                    XmlTreeItem* modData = new XmlTreeItem();
                    modData->elementName = "module";
                    modData->elementValue = mod->moduleName;
                    if (mod->rootElement) {
                        modData->attributes = mod->rootElement->attributes;
                    }
                    //modData->attributes["path"] = mod->relativePath;
                    modData->treeItem = modItem;
                    itemMap[modItem] = modData;
                    
                    if (mod->rootElement) {
                        addXmlElementsToTree(mod->rootElement, modItem);
                    }
                }
                classItem->sortChildren(0, Qt::AscendingOrder);
            }
        }
        
        tree->expandToDepth(1);
        tree->resizeColumnToContents(0);
    }
    
    void addXmlElementsToTree(XmlElement* xmlElement, QTreeWidgetItem* parentItem)
    {
        for (int i = 0; i < xmlElement->children.size(); ++i) {
            XmlElement* child = xmlElement->children.at(i);

            QTreeWidgetItem* childItem = new QTreeWidgetItem(parentItem);
            childItem->setText(0, child->name);
            
            QString displayValue = child->text;
            if (displayValue.isEmpty() && child->children.isEmpty()) {
                // displayValue = "(empty)";
            }
            childItem->setText(1, displayValue);
            if( !child->attributes.isEmpty() )
                childItem->setText(2, QString::number(child->attributes.size()));

            XmlTreeItem* itemData = new XmlTreeItem();
            itemData->elementName = child->name;
            itemData->elementValue = child->text;
            itemData->attributes = child->attributes;
            itemData->treeItem = childItem;
            itemMap[childItem] = itemData;
            
            if (!child->children.isEmpty()) {
                addXmlElementsToTree(child, childItem);
            }
        }
    }
    
    QTreeWidget* tree;
    QTableWidget* attributeTable;
    QLabel* statusLabel;
    QString rootPath;
    QList<ModuleData*> modules;
    QMap<QTreeWidgetItem*, XmlTreeItem*> itemMap;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ModuleViewer viewer;
    viewer.showMaximized();

    if( app.arguments().size() > 1 )
    {
        QFileInfo info(app.arguments()[1]);
        if( info.isDir() )
            viewer.loadModules(info.filePath());
    }
    
    return app.exec();
}

#include "modulenavigator.moc"

