#include "user.h"
#include "dbManager.h"
#include <time.h>
#include <cjson/cJSON.h>

#define SECRET_KEY "MY_SUPER_SECRET_KEY_1234"

#define SECRET_KEY "MY_SUPER_SECRET_KEY_1234"

void get_json_value(const char* json, const char* key, char* out_buf, int buf_size) {
    char search_key[64];
    sprintf_s(search_key, sizeof(search_key), "\"%s\"", key);

    char* start = strstr(json, search_key);
    if (start == NULL) {
        out_buf[0] = '\0';
        return;
    }
    start = strchr(start, ':');
    if (start == NULL) return;
    start = strchr(start, '\"');
    if (start == NULL) return;
    start++;

    int i = 0;
    while (start[i] != '\"' && start[i] != '\0' && i < buf_size - 1) {
        out_buf[i] = start[i];
        i++;
    }
    out_buf[i] = '\0';
}

void base64_encode_url_safe(const unsigned char* input, int length, char* output) {
    BIO* bmem, * b64;
    BUF_MEM* bptr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);

    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    int j = 0;
    for (int i = 0; i < bptr->length; i++) {
        char c = bptr->data[i];
        if (c == '+') output[j++] = '-';
        else if (c == '/') output[j++] = '_';
        else if (c == '=') continue;
        else output[j++] = c;
    }
    output[j] = '\0';

    BIO_free_all(b64);
}

void hash_password_openssl(const char* raw_pw, const char* salt, char* out_hex) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    char combined[128];
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    sprintf_s(combined, sizeof(combined), "%s%s", raw_pw, salt);

    if (ctx != NULL) {
        EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
        EVP_DigestUpdate(ctx, combined, strlen(combined));
        EVP_DigestFinal_ex(ctx, hash, &hash_len);
        EVP_MD_CTX_free(ctx);
    }

    for (unsigned int i = 0; i < hash_len; i++) {
        sprintf_s(out_hex + (i * 2), 3, "%02x", hash[i]);
    }
    out_hex[hash_len * 2] = '\0';
}

void create_jwt_openssl(const char* nickname, char* token_buf) {
    char header[] = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
    char payload[256];
    char header_b64[256], payload_b64[512];
    char signature_input[1024];
    unsigned char signature_binary[32];
    char signature_b64[256];
    unsigned int len;

    sprintf_s(payload, sizeof(payload), "{\"sub\":\"%s\",\"iat\":%ld}", nickname, time(NULL));

    base64_encode_url_safe((unsigned char*)header, (int)strlen(header), header_b64);
    base64_encode_url_safe((unsigned char*)payload, (int)strlen(payload), payload_b64);

    sprintf_s(signature_input, sizeof(signature_input), "%s.%s", header_b64, payload_b64);

    HMAC(EVP_sha256(), SECRET_KEY, (int)strlen(SECRET_KEY),
        (unsigned char*)signature_input, strlen(signature_input),
        signature_binary, &len);

    base64_encode_url_safe(signature_binary, len, signature_b64);

    sprintf_s(token_buf, 2048, "%s.%s", signature_input, signature_b64);
}

void handle_signup(SOCKET clnt_sock, HttpRequest* req) {
    char nickname[64] = { 0 }, password[64] = { 0 };
    char salt[32] = "somesalt";
    char password_hash[65];
    char* json_response = NULL;
    char response_header[512];

    if (req->body == NULL || strlen(req->body) == 0) {
        char* msg = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    cJSON* json = cJSON_Parse(req->body);
    if (json == NULL) {
        char* msg = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid JSON";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    cJSON* nick_item = cJSON_GetObjectItemCaseSensitive(json, "nickname");
    cJSON* pw_item = cJSON_GetObjectItemCaseSensitive(json, "password");

    if (cJSON_IsString(nick_item) && (nick_item->valuestring != NULL)) {
        strcpy_s(nickname, sizeof(nickname), nick_item->valuestring);
    }
    if (cJSON_IsString(pw_item) && (pw_item->valuestring != NULL)) {
        strcpy_s(password, sizeof(password), pw_item->valuestring);
    }

    cJSON_Delete(json);

    hash_password_openssl(password, salt, password_hash);

    sqlite3* db = get_db_connection();
    cJSON* res_json = cJSON_CreateObject();

    if (db) {
        const char* sql = "INSERT INTO Users (nickname, password_hash, salt) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, nickname, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                cJSON_AddStringToObject(res_json, "message", "User created");

                json_response = cJSON_PrintUnformatted(res_json);
                sprintf_s(response_header, sizeof(response_header),
                    "HTTP/1.1 201 Created\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n",
                    (int)strlen(json_response));
            }
            else {
                cJSON_AddStringToObject(res_json, "error", "User exists");

                json_response = cJSON_PrintUnformatted(res_json);
                sprintf_s(response_header, sizeof(response_header),
                    "HTTP/1.1 409 Conflict\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n",
                    (int)strlen(json_response));
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    send(clnt_sock, response_header, (int)strlen(response_header), 0);
    send(clnt_sock, json_response, (int)strlen(json_response), 0);

    cJSON_Delete(res_json);
    free(json_response);
}

void handle_signin(SOCKET clnt_sock, HttpRequest* req) {
    char nickname[64] = { 0 }, password[64] = { 0 };
    char db_hash[65] = { 0 }, db_salt[32] = { 0 };
    char input_hash[65];
    int login_success = 0;

    if (req->body == NULL || strlen(req->body) == 0) {
        char* msg = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        send(clnt_sock, msg, (int)strlen(msg), 0);
        return;
    }

    cJSON* json = cJSON_Parse(req->body);
    if (json == NULL) return;

    cJSON* nick_item = cJSON_GetObjectItemCaseSensitive(json, "nickname");
    cJSON* pw_item = cJSON_GetObjectItemCaseSensitive(json, "password");

    if (cJSON_IsString(nick_item) && nick_item->valuestring != NULL)
        strcpy_s(nickname, sizeof(nickname), nick_item->valuestring);
    if (cJSON_IsString(pw_item) && pw_item->valuestring != NULL)
        strcpy_s(password, sizeof(password), pw_item->valuestring);

    cJSON_Delete(json);

    sqlite3* db = get_db_connection();
    if (db) {
        const char* sql = "SELECT password_hash, salt FROM Users WHERE nickname = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, nickname, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                strcpy_s(db_hash, sizeof(db_hash), (const char*)sqlite3_column_text(stmt, 0));
                strcpy_s(db_salt, sizeof(db_salt), (const char*)sqlite3_column_text(stmt, 1));

                hash_password_openssl(password, db_salt, input_hash);
                if (strcmp(input_hash, db_hash) == 0) login_success = 1;
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    cJSON* res_json = cJSON_CreateObject();
    char* json_res_str = NULL;
    char header[512];

    if (login_success) {
        char jwt[2048];
        create_jwt_openssl(nickname, jwt);

        cJSON_AddStringToObject(res_json, "message", "Login Success");
        cJSON_AddStringToObject(res_json, "token", jwt);

        json_res_str = cJSON_PrintUnformatted(res_json);
        sprintf_s(header, sizeof(header),
            "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n",
            (int)strlen(json_res_str));
    }
    else {
        cJSON_AddStringToObject(res_json, "error", "Invalid credentials");

        json_res_str = cJSON_PrintUnformatted(res_json);
        sprintf_s(header, sizeof(header),
            "HTTP/1.1 401 Unauthorized\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n",
            (int)strlen(json_res_str));
    }

    send(clnt_sock, header, (int)strlen(header), 0);
    send(clnt_sock, json_res_str, (int)strlen(json_res_str), 0);

    cJSON_Delete(res_json);
    free(json_res_str);
}

int base64_decode_url_safe(char* encoded, unsigned char* decoded) {
    char temp[1024];
    strcpy_s(temp, sizeof(temp), encoded);

    int len = (int)strlen(temp);
    for (int i = 0; i < len; i++) {
        if (temp[i] == '-') temp[i] = '+';
        else if (temp[i] == '_') temp[i] = '/';
    }
    while (strlen(temp) % 4 != 0) {
        strcat_s(temp, sizeof(temp), "=");
    }

    BIO* b64, * bmem;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new_mem_buf(temp, -1);
    bmem = BIO_push(b64, bmem);

    int decoded_len = BIO_read(bmem, decoded, len);
    decoded[decoded_len] = '\0';

    BIO_free_all(b64);
    return decoded_len;
}

int get_user_id_from_token(const char* auth_header) {
    if (auth_header == NULL || strlen(auth_header) < 10) return -1;
    printf("여기서 뻑남 1");
    const char* token = NULL;
    if (strncmp(auth_header, "Bearer ", 7) == 0) {
        token = auth_header + 7;   
        printf("토큰 %d", token);
    }
    else {
        printf("여기서 뻑남 2");
        return -1;
    }

    char token_copy[1024];
    strcpy_s(token_copy, sizeof(token_copy), token);

    char* header = token_copy;
    char* payload = strchr(header, '.');
    if (!payload) return -1;
    *payload = '\0'; payload++;

    char* signature = strchr(payload, '.');
    if (!signature) return -1;
    *signature = '\0'; signature++;

    char check_input[1024];
    unsigned char check_sig_bin[32];
    char check_sig_b64[256];
    unsigned int len;

    sprintf_s(check_input, sizeof(check_input), "%s.%s", header, payload);

    HMAC(EVP_sha256(), SECRET_KEY, (int)strlen(SECRET_KEY),
        (unsigned char*)check_input, strlen(check_input),
        check_sig_bin, &len);

    base64_encode_url_safe(check_sig_bin, len, check_sig_b64);

    if (strcmp(signature, check_sig_b64) != 0) {
        printf("[Auth] Invalid Signature\n");
        return -1;
    }
    printf("여기서 뻑남 3");
    unsigned char decoded_payload[1024];
    base64_decode_url_safe(payload, decoded_payload);

    char nickname[64] = { 0 };
    cJSON* json = cJSON_Parse((char*)decoded_payload);
    if (json) {
        cJSON* sub = cJSON_GetObjectItem(json, "sub");
        if (sub && sub->valuestring) {
            strcpy_s(nickname, sizeof(nickname), sub->valuestring);
        }
        cJSON_Delete(json);
    }

    if (strlen(nickname) == 0) return -1;

    int user_id = -1;
    sqlite3* db = get_db_connection();
    if (db) {
        const char* sql = "SELECT id FROM Users WHERE nickname = ?";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, nickname, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                user_id = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    return user_id;
}