#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QList>
#include <QSqlDatabase>

struct Note
{
    int id = -1;
    QString title;
    QString content;
    QString tags;
    bool favorite = false;
};

class Database
{
public:
    Database();

    bool open(QString* error = nullptr);

    QList<Note> loadNotes(QString* error = nullptr);

    bool saveNote(Note& note, QString* error = nullptr);

    bool deleteNote(int id, QString* error = nullptr);

private:
    QSqlDatabase db;
};

#endif
