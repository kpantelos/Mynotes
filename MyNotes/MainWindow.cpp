#include "MainWindow.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QInputDialog>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowTitle("My Notes");
    resize(1400, 850);
    setMinimumSize(1050, 650);

    QString dbError;
    if (!database.open(&dbError)) {
        QMessageBox::critical(this, "Database Error",
            "Δεν ήταν δυνατό να ανοίξει η βάση:\n" + dbError);
        return;
    }

    QWidget* central = ui.centralWidget;
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* topBar = new QWidget;
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(65);
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(22, 8, 22, 8);

    auto* logo = new QLabel("My Notes");
    logo->setObjectName("logo");
    auto* topSearch = new QLineEdit;
    topSearch->setPlaceholderText("⌕  Αναζήτηση σημειώσεων...");
    topSearch->setMaximumWidth(420);
    auto* newButton = new QPushButton("+ Νέα σημείωση");
    newButton->setObjectName("primaryButton");
    topLayout->addWidget(logo);
    topLayout->addStretch();
    topLayout->addWidget(topSearch);
    topLayout->addSpacing(15);
    topLayout->addWidget(newButton);
    mainLayout->addWidget(topBar);

    auto* splitter = new QSplitter(Qt::Horizontal);

    auto* sidebar = new QWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setMinimumWidth(205);
    sidebar->setMaximumWidth(260);
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(15, 20, 15, 15);

    auto* navigationTitle = new QLabel("ΧΩΡΟΣ ΕΡΓΑΣΙΑΣ");
    navigationTitle->setObjectName("sectionLabel");
    auto* allNotes = new QPushButton("▤   Όλες οι σημειώσεις");
    auto* workButton = new QPushButton("▰   Εργασία");
    auto* favorites = new QPushButton("☆   Αγαπημένες");
    allNotes->setObjectName("activeNav");
    workButton->setObjectName("navButton");
    favorites->setObjectName("navButton");

    auto* foldersTitle = new QLabel("ΦΑΚΕΛΟΙ");
    foldersTitle->setObjectName("sectionLabel");
    auto* folderTree = new QTreeWidget;
    folderTree->setHeaderHidden(true);
    auto* workFolder = new QTreeWidgetItem(folderTree);
    workFolder->setText(0, "📁  Εργασία");
    workFolder->setExpanded(true);
    auto* addFolder = new QPushButton("+ Νέος φάκελος");
    addFolder->setObjectName("secondaryButton");

    sideLayout->addWidget(navigationTitle);
    sideLayout->addWidget(allNotes);
    sideLayout->addWidget(workButton);
    sideLayout->addWidget(favorites);
    sideLayout->addSpacing(25);
    sideLayout->addWidget(foldersTitle);
    sideLayout->addWidget(folderTree);
    sideLayout->addWidget(addFolder);

    auto* notesPanel = new QWidget;
    notesPanel->setObjectName("notesPanel");
    notesPanel->setMinimumWidth(280);
    auto* notesLayout = new QVBoxLayout(notesPanel);
    notesLayout->setContentsMargins(18, 22, 18, 15);
    auto* notesHeading = new QLabel("Οι σημειώσεις μου");
    notesHeading->setObjectName("heading");
    auto* notesSearch = new QLineEdit;
    notesSearch->setPlaceholderText("⌕  Φιλτράρισμα...");
    auto* notesList = new QListWidget;
    notesList->setObjectName("notesList");
    notesLayout->addWidget(notesHeading);
    notesLayout->addWidget(notesSearch);
    notesLayout->addWidget(notesList);

    auto* editorPanel = new QWidget;
    editorPanel->setObjectName("editorPanel");
    auto* editorLayout = new QVBoxLayout(editorPanel);
    editorLayout->setContentsMargins(35, 22, 35, 20);
    editorLayout->setSpacing(12);
    auto* editorToolbar = new QHBoxLayout;
    auto* saveStatus = new QLabel("●  Έτοιμο");
    saveStatus->setObjectName("saveStatus");
    auto* favoriteButton = new QPushButton("☆");
    auto* deleteButton = new QPushButton("Διαγραφή");
    editorToolbar->addWidget(saveStatus);
    editorToolbar->addStretch();
    editorToolbar->addWidget(favoriteButton);
    editorToolbar->addWidget(deleteButton);
    auto* titleEdit = new QLineEdit;
    titleEdit->setObjectName("titleEdit");
    titleEdit->setPlaceholderText("Τίτλος σημείωσης");
    auto* dateLabel = new QLabel("Δημιούργησε ή επίλεξε μια σημείωση");
    auto* tagsEdit = new QLineEdit;
    tagsEdit->setPlaceholderText("Ετικέτες: projects, πελάτες, εργασία...");
    auto* contentEdit = new QTextEdit;
    contentEdit->setObjectName("contentEdit");
    contentEdit->setPlaceholderText("Ξεκίνα να γράφεις τις σημειώσεις σου εδώ...");
    editorLayout->addLayout(editorToolbar);
    editorLayout->addSpacing(20);
    editorLayout->addWidget(titleEdit);
    editorLayout->addWidget(dateLabel);
    editorLayout->addWidget(tagsEdit);
    editorLayout->addWidget(contentEdit);

    splitter->addWidget(sidebar);
    splitter->addWidget(notesPanel);
    splitter->addWidget(editorPanel);
    splitter->setSizes({230, 350, 820});
    mainLayout->addWidget(splitter);

    auto saveCurrentNote = [=]() {
        auto* item = notesList->currentItem();
        if (!item)
            return;
        Note note;
        note.id = item->data(Qt::UserRole + 4).toInt();
        note.title = titleEdit->text();
        note.content = contentEdit->toPlainText();
        note.tags = tagsEdit->text();
        note.favorite = item->data(Qt::UserRole + 3).toBool();
        QString error;
        if (!database.saveNote(note, &error)) {
            saveStatus->setText("●  Σφάλμα αποθήκευσης");
            qWarning() << error;
            return;
        }
        item->setData(Qt::UserRole, note.title);
        item->setData(Qt::UserRole + 1, note.content);
        item->setData(Qt::UserRole + 2, note.tags);
        item->setData(Qt::UserRole + 3, note.favorite);
        item->setData(Qt::UserRole + 4, note.id);
        item->setText(note.title.isEmpty() ? "Χωρίς τίτλο" : note.title);
        saveStatus->setText("●  Αποθηκεύτηκε");
    };

    QString loadError;
    QList<Note> savedNotes = database.loadNotes(&loadError);
    if (!loadError.isEmpty()) {
        QMessageBox::warning(this, "Database", "Σφάλμα φόρτωσης:\n" + loadError);
    }
    for (const Note& note : savedNotes) {
        auto* item = new QListWidgetItem(note.title);
        item->setData(Qt::UserRole, note.title);
        item->setData(Qt::UserRole + 1, note.content);
        item->setData(Qt::UserRole + 2, note.tags);
        item->setData(Qt::UserRole + 3, note.favorite);
        item->setData(Qt::UserRole + 4, note.id);
        notesList->addItem(item);
    }

    auto createNote = [=]() {
        Note note;
        note.title = "Νέα σημείωση";
        QString error;
        if (!database.saveNote(note, &error)) {
            QMessageBox::warning(this, "Σφάλμα",
                "Δεν αποθηκεύτηκε η σημείωση:\n" + error);
            return;
        }
        auto* item = new QListWidgetItem(note.title);
        item->setData(Qt::UserRole, note.title);
        item->setData(Qt::UserRole + 1, note.content);
        item->setData(Qt::UserRole + 2, note.tags);
        item->setData(Qt::UserRole + 3, note.favorite);
        item->setData(Qt::UserRole + 4, note.id);
        notesList->insertItem(0, item);
        notesList->setCurrentItem(item);
        titleEdit->setFocus();
        titleEdit->selectAll();
        dateLabel->setText("Δημιουργήθηκε: " +
            QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
    };
    connect(newButton, &QPushButton::clicked, this, createNote);

    connect(notesList, &QListWidget::currentItemChanged, this,
        [=](QListWidgetItem* current, QListWidgetItem*) {
            if (!current) {
                QSignalBlocker b1(titleEdit);
                QSignalBlocker b2(contentEdit);
                QSignalBlocker b3(tagsEdit);
                titleEdit->clear();
                contentEdit->clear();
                tagsEdit->clear();
                favoriteButton->setText("☆");
                return;
            }
            QSignalBlocker b1(titleEdit);
            QSignalBlocker b2(contentEdit);
            QSignalBlocker b3(tagsEdit);
            titleEdit->setText(current->data(Qt::UserRole).toString());
            contentEdit->setPlainText(current->data(Qt::UserRole + 1).toString());
            tagsEdit->setText(current->data(Qt::UserRole + 2).toString());
            favoriteButton->setText(
                current->data(Qt::UserRole + 3).toBool() ? "★" : "☆");
            dateLabel->setText("Η σημείωση είναι αποθηκευμένη.");
            saveStatus->setText("●  Αποθηκευμένο");
        });

    connect(titleEdit, &QLineEdit::textChanged, this,
        [=](const QString&) { saveCurrentNote(); });
    connect(contentEdit, &QTextEdit::textChanged, this,
        [=]() { saveCurrentNote(); });
    connect(tagsEdit, &QLineEdit::textChanged, this,
        [=](const QString&) { saveCurrentNote(); });

    connect(deleteButton, &QPushButton::clicked, this, [=]() {
        auto* item = notesList->currentItem();
        if (!item)
            return;
        int id = item->data(Qt::UserRole + 4).toInt();
        QString error;
        if (!database.deleteNote(id, &error)) {
            QMessageBox::warning(this, "Σφάλμα διαγραφής", error);
            return;
        }
        delete notesList->takeItem(notesList->row(item));
        titleEdit->clear();
        contentEdit->clear();
        tagsEdit->clear();
        saveStatus->setText("●  Έτοιμο");
    });

    connect(favoriteButton, &QPushButton::clicked, this, [=]() {
        auto* item = notesList->currentItem();
        if (!item)
            return;
        bool starred = !item->data(Qt::UserRole + 3).toBool();
        item->setData(Qt::UserRole + 3, starred);
        favoriteButton->setText(starred ? "★" : "☆");
        saveCurrentNote();
    });

    connect(notesSearch, &QLineEdit::textChanged, this,
        [=](const QString& text) {
            for (int i = 0; i < notesList->count(); ++i) {
                auto* item = notesList->item(i);
                QString title = item->data(Qt::UserRole).toString();
                QString content = item->data(Qt::UserRole + 1).toString();
                QString tags = item->data(Qt::UserRole + 2).toString();
                bool match = title.contains(text, Qt::CaseInsensitive) ||
                    content.contains(text, Qt::CaseInsensitive) ||
                    tags.contains(text, Qt::CaseInsensitive);
                item->setHidden(!match);
            }
        });
    connect(topSearch, &QLineEdit::textChanged, this,
        [=](const QString& text) { notesSearch->setText(text); });

    connect(addFolder, &QPushButton::clicked, this, [=]() {
        bool ok = false;
        QString name = QInputDialog::getText(this, "Νέος φάκελος",
            "Όνομα φακέλου:", QLineEdit::Normal, "", &ok);
        if (ok && !name.trimmed().isEmpty()) {
            auto* parent = folderTree->currentItem();
            if (!parent)
                parent = workFolder;
            auto* folder = new QTreeWidgetItem(parent);
            folder->setText(0, "📁  " + name.trimmed());
            parent->setExpanded(true);
        }
    });

    connect(allNotes, &QPushButton::clicked, this, [=]() {
        notesSearch->clear();
        notesHeading->setText("Όλες οι σημειώσεις");
    });
    connect(workButton, &QPushButton::clicked, this, [=]() {
        notesSearch->clear();
        notesHeading->setText("Εργασία");
    });
    connect(favorites, &QPushButton::clicked, this, [=]() {
        notesSearch->clear();
        notesHeading->setText("Αγαπημένες");
        for (int i = 0; i < notesList->count(); ++i) {
            auto* item = notesList->item(i);
            item->setHidden(!item->data(Qt::UserRole + 3).toBool());
        }
    });

    setStyleSheet(R"(
        QMainWindow { background: #f5f6f8; }
        QWidget { font-family: "Segoe UI"; font-size: 10pt; color: #252932; }
        #topBar { background: white; border-bottom: 1px solid #e7e9ee; }
        #logo { font-size: 21pt; font-weight: bold; color: #315fce; }
        #sidebar, #notesPanel { background: white; border-right: 1px solid #e7e9ee; }
        #editorPanel { background: white; }
        #sectionLabel { color: #89919d; font-size: 9pt; font-weight: bold; padding: 8px 4px; }
        #heading { font-size: 17pt; font-weight: bold; }
        QPushButton { background: white; border: 1px solid #e2e5eb; border-radius: 7px; padding: 9px; }
        QPushButton:hover { background: #edf2ff; color: #315fce; }
        #primaryButton, #activeNav { background: #315fce; color: white; border: none; border-radius: 7px; padding: 10px; }
        #navButton, #secondaryButton { text-align: left; border: none; }
        QLineEdit { background: #f7f8fa; border: 1px solid #e7e9ee; border-radius: 7px; padding: 10px; }
        #titleEdit { background: white; border: none; font-size: 25pt; font-weight: bold; padding: 8px 0; }
        #contentEdit { background: white; border: none; font-size: 11pt; padding: 12px 0; }
        #dateLabel, #saveStatus { color: #858d99; font-size: 9pt; }
        QListWidget, QTreeWidget { background: white; border: none; outline: none; }
        QListWidget::item { padding: 15px 10px; border-bottom: 1px solid #eef0f3; border-radius: 7px; }
        QListWidget::item:selected { background: #edf2ff; color: #315fce; }
        QTreeWidget::item { padding: 7px; }
        QTreeWidget::item:selected { background: #edf2ff; color: #315fce; }
        QSplitter::handle { background: #e7e9ee; width: 2px; }
    )");
    statusBar()->showMessage("My Notes | Επαγγελματική οργάνωση");
}

MainWindow::~MainWindow()
{
}
