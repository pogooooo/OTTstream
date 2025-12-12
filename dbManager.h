#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include "common.h"

int init_db();
sqlite3* get_db_connection();

#endif