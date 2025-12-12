#include "dbManager.h"

static const char* DB_NAME = "server.db";

int init_db() {
    sqlite3* db;
    char* err_msg = 0;
    int rc;

    rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS Users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nickname TEXT NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL, "
        "profile_image TEXT, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);"

        "CREATE TABLE IF NOT EXISTS Videos ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "uploader_id INTEGER NOT NULL, "
        "upload_date DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "length INTEGER, "
        "file_size INTEGER NOT NULL, "
        "mime_type TEXT, "
        "views INTEGER DEFAULT 0, "
        "thumbnail_path TEXT, "
        "file_path TEXT NOT NULL, "
        "FOREIGN KEY(uploader_id) REFERENCES Users(id));"

        "CREATE TABLE IF NOT EXISTS ViewHistory ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "video_id INTEGER NOT NULL, "
        "user_id INTEGER NOT NULL, "
        "viewed_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY(video_id) REFERENCES Videos(id), "
        "FOREIGN KEY(user_id) REFERENCES Users(id));";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return -1;
    }

    printf("[DB] Database and tables initialized successfully.\n");
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", 0, 0, 0);
    sqlite3_close(db);
    return 0;
}

sqlite3* get_db_connection() {
    sqlite3* db;
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        return NULL;
    }
    return db;
}