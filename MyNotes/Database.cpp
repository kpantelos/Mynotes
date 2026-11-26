#include "Database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

Database::Database()
{
    const QString connectionName = "MyNotesConnection";

    if (QSqlDatabase::contains(connectionName))
        db = QSqlDatabase::database(connectionName);
    else
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
}

bool Database::open(QString* error)
{
    QCoreApplication::setOrganizationName("MyNotes");
    QCoreApplication::setApplicationName("MyNotes");

    QString folder = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation
    );

    if (!QDir().mkpath(folder)) {
        if (error)
            *error = "Δεν δημιουργήθηκε ο φάκελος δεδομένων.";
        return false;
    }

    db.setDatabaseName(folder + "/MyNotes.db");

    if (!db.open()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS notes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL DEFAULT 'Νέα σημείωση',
            content TEXT NOT NULL DEFAULT '',
            tags TEXT NOT NULL DEFAULT '',
            favorite INTEGER NOT NULL DEFAULT 0,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )")) {
        if (error)
            *error = query.lastError().text();
        return false;
    }

    return true;
}

QList<Note> Database::loadNotes(QString* error)
{
    QList<Note> notes;
    QSqlQuery query(db);

    if (!query.exec(
        "SELECT id, title, content, tags, favorite "
        "FROM notes ORDER BY updated_at DESC, id DESC"
    )) {
        if (error)
            *error = query.lastError().text();
        return notes;
    }

    while (query.next()) {
        Note note;

        note.id = query.value(0).toInt();
        note.title = query.value(1).toString();
        note.content = query.value(2).toString();
        note.tags = query.value(3).toString();
        note.favorite = query.value(4).toBool();

        notes.append(note);
    }

    return notes;
}

bool Database::saveNote(Note& note, QString* error)
{
    QSqlQuery query(db);

    QString title = note.title.isNull()
        ? QStringLiteral("Νέα σημείωση") : note.title;
    QString content = note.content.isNull()
        ? QStringLiteral("") : note.content;
    QString tags = note.tags.isNull()
        ? QStringLiteral("") : note.tags;

    if (note.id < 0) {
        query.prepare(R"(
            INSERT INTO notes (title, content, tags, favorite)
            VALUES (:title, :content, :tags, :favorite)
        )");
    } else {
        query.prepare(R"(
            UPDATE notes
            SET title = :title,
                content = :content,
                tags = :tags,
                favorite = :favorite,
                updated_at = CURRENT_TIMESTAMP
            WHERE id = :id
        )");
        query.bindValue(":id", note.id);
    }

    query.bindValue(":title", title);
    query.bindValue(":content", content);
    query.bindValue(":tags", tags);
    query.bindValue(":favorite", note.favorite ? 1 : 0);

    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }

    if (note.id < 0) {
        note.id = query.lastInsertId().toInt();
        if (note.id <= 0) {
            if (error)
                *error = "Δεν επιστράφηκε έγκυρο ID σημείωσης.";
            return false;
        }
    }

    note.title = title;
    note.content = content;
    note.tags = tags;
    return true;
}

bool Database::deleteNote(int id, QString* error)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM notes WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }
    return true;
}
