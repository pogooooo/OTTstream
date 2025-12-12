#include "video.h"
#include "dbManager.h"
#include <time.h>
#include <direct.h>
#include <io.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cjson/cJSON.h>

#define UPLOAD_DIR "data/video/"
#define THUMB_DIR "data/thumb/"

extern int get_user_id_from_token(const char* auth_header);
extern sqlite3* get_db_connection();

void ensure_directory(const char* path) {
    if (_access(path, 0) == -1) {
        _mkdir(path);
    }
}

void init_video_system() {
    ensure_directory("data");
    ensure_directory("data/video");
    ensure_directory("data/thumb");
}

void extract_thumbnail(const char* video_path, char* out_thumb_path) {
    char cmd[1024];

    char filename_only[128];
    char* p = strrchr(video_path, '/');
    if (p == NULL) p = (char*)video_path; else p++;

    strcpy_s(filename_only, sizeof(filename_only), p);
    char* ext = strrchr(filename_only, '.');
    if (ext) *ext = '\0';

    sprintf_s(out_thumb_path, 256, "%s%s.jpg", THUMB_DIR, filename_only);

    sprintf_s(cmd, sizeof(cmd), "ffmpeg -y -i \"%s\" -ss 00:00:01 -vframes 1 -s 320x180 \"%s\" -loglevel quiet",
        video_path, out_thumb_path);

    printf("[FFmpeg] Executing: %s\n", cmd);
    system(cmd);
}

// [수정] 쿼리 추출 함수 제거됨

void handle_video_upload(SOCKET clnt_sock, HttpRequest* req) {
    char response[1024];
    char file_uuid[64];
    char file_path[256];
    char thumb_path[256] = { 0 };
    char video_title[256] = "Uploaded Video";
    int uploader_id;

    printf("[DEBUG] --- Starting handle_video_upload ---\n");

    uploader_id = get_user_id_from_token(req->authorization);
    printf("req : %d", req->authorization);
    printf("uploader_id : %d", uploader_id);
    if (uploader_id == -1) {
        char* msg = "HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        printf("[DEBUG] Auth failed. Returning 401.\n");
        return;
    }
    printf("[DEBUG] User ID: %d, Content-Length: %lld\n", uploader_id, req->content_length);

    if (strlen(req->video_title) > 0) {
        strcpy_s(video_title, sizeof(video_title), req->video_title);
    }
    printf("[DEBUG] Video Title: %s\n", video_title);

    srand((unsigned int)time(NULL));
    sprintf_s(file_uuid, sizeof(file_uuid), "%ld_%d", time(NULL), rand() % 1000);
    sprintf_s(file_path, sizeof(file_path), "%s%s.mp4", UPLOAD_DIR, file_uuid);

    printf("[DEBUG] Trying fopen_s for file path: %s\n", file_path);
    FILE* fp = NULL;
    fopen_s(&fp, file_path, "wb");
    if (!fp) {
        char* msg = "HTTP/1.1 500 Server Error\r\nContent-Length: 0\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        printf("[ERROR] Failed to open file: %s\n", file_path);
        return;
    }

    long long total_written = 0;
    int success = 1;

    // 1. 초기 청크 쓰기
    if (req->body && req->body_len > 0) {
        if (fwrite(req->body, 1, req->body_len, fp) != req->body_len) {
            success = 0;
        }
        total_written += req->body_len;
        printf("[DEBUG] Initial chunk written. Size: %d\n", req->body_len);
    }

    char buffer[65536];
    int read_len;
    long long remaining = req->content_length - total_written;

    // 2. 루프를 돌며 나머지 데이터 수신
    printf("[DEBUG] Starting recv loop. Remaining bytes: %lld\n", remaining);
    while (remaining > 0 && success) {
        int to_read = (remaining > sizeof(buffer)) ? sizeof(buffer) : (int)remaining;

        read_len = recv(clnt_sock, buffer, to_read, 0);

        if (read_len <= 0) {
            printf("[ERROR] Recv failed or disconnected. Read size: %d, Socket error: %d\n", read_len, WSAGetLastError());
            success = 0;
            break;
        }

        if (fwrite(buffer, 1, read_len, fp) != read_len) {
            printf("[ERROR] Fwrite failed during loop.\n");
            success = 0;
            break;
        }
        remaining -= read_len;
        total_written += read_len;
    }
    printf("[DEBUG] Recv loop finished. Total written: %lld\n", total_written);

    // 3. 파일 닫기 및 정리
    printf("[DEBUG] Trying fclose(fp).\n");
    fclose(fp);

    if (!success || remaining > 0) {
        printf("[ERROR] Upload failed or incomplete! Removing file.\n");
        remove(file_path);
        char* msg = "HTTP/1.1 500 Server Error\r\nContent-Length: 0\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    // 4. 썸네일 추출 (FFmpeg)
    printf("[DEBUG] Starting extract_thumbnail (FFmpeg).\n");
    extract_thumbnail(file_path, thumb_path);
    printf("[DEBUG] extract_thumbnail finished.\n");

    // 5. DB 연결 및 저장
    printf("[DEBUG] Starting DB transaction.\n");
    sqlite3* db = get_db_connection();
    if (db) {
        const char* sql = "INSERT INTO Videos (title, uploader_id, file_size, mime_type, file_path, thumbnail_path) VALUES (?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, video_title, -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, uploader_id);
            sqlite3_bind_int64(stmt, 3, total_written);
            sqlite3_bind_text(stmt, 4, "video/mp4", -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, file_path, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 6, thumb_path, -1, SQLITE_STATIC);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        printf("[DEBUG] DB transaction complete.\n");
    }

    // 6. 성공 응답 전송
    sprintf_s(response, sizeof(response),
        "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"message\": \"Upload Success\", \"id\": \"%s\"}", file_uuid);
    send(clnt_sock, response, (int)strlen(response), 0);
    printf("[DEBUG] Success response sent. --- Ending handle_video_upload ---\n");
}

void handle_video_play(SOCKET clnt_sock, HttpRequest* req) {
    int video_id = -1;
    char* param = strchr(req->url, '=');
    if (param) video_id = atoi(param + 1);

    if (video_id == -1) {
        char* msg = "HTTP/1.1 400 Bad Request\r\n\r\nMissing ID";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    char file_path[256] = { 0 };
    long long total_size = 0;

    sqlite3* db = get_db_connection();
    if (db) {
        sqlite3_stmt* stmt;
        const char* sql = "SELECT file_path, file_size FROM Videos WHERE id = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, video_id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                strcpy_s(file_path, sizeof(file_path), (const char*)sqlite3_column_text(stmt, 0));
                total_size = sqlite3_column_int64(stmt, 1);
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    if (strlen(file_path) == 0) {
        char* msg = "HTTP/1.1 404 Not Found\r\n\r\nVideo not found";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    long long start = 0, end = total_size - 1;
    if (strlen(req->range) > 0) {
        char* range_val = req->range;
        if (strncmp(range_val, "bytes=", 6) == 0) {
            range_val += 6;
        }

        char* dash = strchr(range_val, '-');
        if (dash) {
            *dash = '\0';
            start = atoll(range_val);

            if (*(dash + 1) != '\0') {
                end = atoll(dash + 1);
            }
        }
    }

    long long chunk_size = end - start + 1;
    if (chunk_size > 1024 * 1024) {
        chunk_size = 1024 * 1024;
        end = start + chunk_size - 1;
    }

    FILE* fp = NULL;
    fopen_s(&fp, file_path, "rb");
    if (!fp) {
        char* msg = "HTTP/1.1 500 Server Error\r\n\r\nFile Open Failed";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    fseek(fp, (long)start, SEEK_SET);

    char header[512];
    sprintf_s(header, sizeof(header),
        "HTTP/1.1 206 Partial Content\r\n"
        "Content-Type: video/mp4\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Range: bytes %lld-%lld/%lld\r\n"
        "Content-Length: %lld\r\n"
        "\r\n",
        start, end, total_size, chunk_size);
    send(clnt_sock, header, (int)strlen(header), 0);

    char* buffer = (char*)malloc((size_t)chunk_size);
    if (buffer) {
        size_t read_len = fread(buffer, 1, (size_t)chunk_size, fp);
        send(clnt_sock, buffer, (int)read_len, 0);
        free(buffer);
    }

    fclose(fp);
}

void handle_save_progress(SOCKET clnt_sock, HttpRequest* req) {
    int user_id = get_user_id_from_token(req->authorization);

    if (user_id == -1) {
        char* msg = "HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    int video_id = 0;
    int position = 0;

    if (req->body) {
        cJSON* json = cJSON_Parse(req->body);
        if (json) {
            cJSON* v_item = cJSON_GetObjectItem(json, "video_id");
            cJSON* p_item = cJSON_GetObjectItem(json, "position");
            if (v_item) video_id = v_item->valueint;
            if (p_item) position = p_item->valueint;
            cJSON_Delete(json);
        }
    }

    if (video_id == 0) return;

    sqlite3* db = get_db_connection();
    if (db) {
        const char* sql = "INSERT OR REPLACE INTO ViewHistory (video_id, user_id, last_position) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, video_id);
            sqlite3_bind_int(stmt, 2, user_id);
            sqlite3_bind_int(stmt, 3, position);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    char* msg = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(clnt_sock, msg, (int)strlen(msg), 0);
}

void handle_get_progress(SOCKET clnt_sock, HttpRequest* req) {
    int user_id = get_user_id_from_token(req->authorization);

    if (user_id == -1) {
        char* msg = "HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    int video_id = -1;
    int last_pos = 0;

    char* param = strstr(req->url, "id=");
    if (param) video_id = atoi(param + 3);

    if (video_id != -1) {
        sqlite3* db = get_db_connection();
        if (db) {
            const char* sql = "SELECT last_position FROM ViewHistory WHERE video_id = ? AND user_id = ?;";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, video_id);
                sqlite3_bind_int(stmt, 2, user_id);
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    last_pos = sqlite3_column_int(stmt, 0);
                }
            }
            sqlite3_finalize(stmt);
            sqlite3_close(db);
        }
    }

    char body[128];
    sprintf_s(body, sizeof(body), "{\"video_id\": %d, \"last_position\": %d}", video_id, last_pos);

    char header[512];
    sprintf_s(header, sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n",
        (int)strlen(body));

    send(clnt_sock, header, (int)strlen(header), 0);
    send(clnt_sock, body, (int)strlen(body), 0);
}

void handle_video_list(SOCKET clnt_sock, HttpRequest* req) {
    sqlite3* db = get_db_connection();
    cJSON* root = cJSON_CreateArray();

    if (db) {
        const char* sql =
            "SELECT V.id, V.title, U.nickname, V.views, V.thumbnail_path, V.upload_date, V.length "
            "FROM Videos V "
            "LEFT JOIN Users U ON V.uploader_id = U.id "
            "ORDER BY V.id DESC;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                cJSON* item = cJSON_CreateObject();

                cJSON_AddNumberToObject(item, "id", sqlite3_column_int(stmt, 0));
                cJSON_AddStringToObject(item, "title", (const char*)sqlite3_column_text(stmt, 1));

                const char* nick = (const char*)sqlite3_column_text(stmt, 2);
                cJSON_AddStringToObject(item, "uploader", nick ? nick : "Unknown");

                cJSON_AddNumberToObject(item, "views", sqlite3_column_int(stmt, 3));

                cJSON_AddStringToObject(item, "thumbnail", (const char*)sqlite3_column_text(stmt, 4));
                cJSON_AddStringToObject(item, "uploadDate", (const char*)sqlite3_column_text(stmt, 5));

                cJSON_AddItemToArray(root, item);
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    char* json_str = cJSON_PrintUnformatted(root);
    char header[512];
    sprintf_s(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n",
        (int)strlen(json_str));

    send(clnt_sock, header, (int)strlen(header), 0);
    send(clnt_sock, json_str, (int)strlen(json_str), 0);

    cJSON_Delete(root);
    free(json_str);
}

void handle_video_delete(SOCKET clnt_sock, HttpRequest* req) {
    int user_id = get_user_id_from_token(req->authorization);
    if (user_id == -1) {
        char* msg = "HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    int video_id = -1;
    char* param = strstr(req->url, "id=");
    if (param) video_id = atoi(param + 3);

    if (video_id == -1) {
        char* msg = "HTTP/1.1 400 Bad Request\r\n\r\nMissing ID";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    sqlite3* db = get_db_connection();
    if (db) {
        char file_path[256] = { 0 };
        char thumb_path[256] = { 0 };

        const char* sql_select = "SELECT file_path, thumbnail_path FROM Videos WHERE id = ? AND uploader_id = ?";
        sqlite3_stmt* stmt;

        int found = 0;
        if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, video_id);
            sqlite3_bind_int(stmt, 2, user_id);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                strcpy_s(file_path, sizeof(file_path), (const char*)sqlite3_column_text(stmt, 0));
                const char* thumb = (const char*)sqlite3_column_text(stmt, 1);
                if (thumb) strcpy_s(thumb_path, sizeof(thumb_path), thumb);
                found = 1;
            }
        }
        sqlite3_finalize(stmt);

        if (found) {
            const char* sql_del_hist = "DELETE FROM ViewHistory WHERE video_id = ?";
            sqlite3_prepare_v2(db, sql_del_hist, -1, &stmt, 0);
            sqlite3_bind_int(stmt, 1, video_id);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);

            const char* sql_del = "DELETE FROM Videos WHERE id = ?";
            sqlite3_prepare_v2(db, sql_del, -1, &stmt, 0);
            sqlite3_bind_int(stmt, 1, video_id);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);

            remove(file_path);
            if (strlen(thumb_path) > 0) remove(thumb_path);

            char* msg = "HTTP/1.1 200 OK\r\n\r\nDeleted";
            send(clnt_sock, msg, (int)strlen(msg), 0);
        }
        else {
            char* msg = "HTTP/1.1 403 Forbidden\r\n\r\nNot Allowed or Not Found";
            send(clnt_sock, msg, (int)strlen(msg), 0);
        }
        sqlite3_close(db);
    }
}

void handle_video_edit(SOCKET clnt_sock, HttpRequest* req) {
    int user_id = get_user_id_from_token(req->authorization);
    if (user_id == -1) {
        char* msg = "HTTP/1.1 401 Unauthorized\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    int video_id = 0;
    char new_title[256] = { 0 };

    if (req->body) {
        cJSON* json = cJSON_Parse(req->body);
        if (json) {
            cJSON* id_item = cJSON_GetObjectItem(json, "id");
            cJSON* title_item = cJSON_GetObjectItem(json, "title");

            if (id_item) video_id = id_item->valueint;
            if (title_item && title_item->valuestring) {
                strcpy_s(new_title, sizeof(new_title), title_item->valuestring);
            }
            cJSON_Delete(json);
        }
    }

    if (video_id == 0 || strlen(new_title) == 0) {
        char* msg = "HTTP/1.1 400 Bad Request\r\n\r\nMissing Data";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    sqlite3* db = get_db_connection();
    if (db) {
        const char* sql = "UPDATE Videos SET title = ? WHERE id = ? AND uploader_id = ?";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, new_title, -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, video_id);
            sqlite3_bind_int(stmt, 3, user_id);

            sqlite3_step(stmt);

            if (sqlite3_changes(db) > 0) {
                char* msg = "HTTP/1.1 200 OK\r\n\r\nUpdated";
                send(clnt_sock, msg, (int)strlen(msg), 0);
            }
            else {
                char* msg = "HTTP/1.1 403 Forbidden\r\n\r\nNo Permission or Not Found";
                send(clnt_sock, msg, (int)strlen(msg), 0);
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
}