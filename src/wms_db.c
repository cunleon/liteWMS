#define _CRT_SECURE_NO_WARNINGS 1 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clib/sqlite3.h"
#include "wms_db.h"

#define set_error_go_exit(code,error_str)       ret = code;error=error_str;goto exit;

sqlite3* db = NULL;
char     err_msg[2048] = "";
int      err_len = 2048;

int database_opened()
{
    if (db == NULL)
        return 0;
    else
        return 1;
}
//初始化必要的表
static int db_tables_init(sqlite3* db)
{
    extern const char* create_table_sql;
    const char* sql = create_table_sql;
    char* err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err_msg);
        return 1;
    }
    return 0;
}
static int test_db_inited(sqlite3* db)
{
    int ret = 0;
    const char* errMsg;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT * FROM 系统参数 WHERE key='init_time';";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    //sqlite3_bind_int(stmt, 1, id);
    ret = sqlite3_step(stmt);
    if (ret == SQLITE_ROW)
        return 1;
    //int row = sqlite3_column_count(stmt);
    return 0;
}
int database_open(const char* filename)
{
    int rc = sqlite3_open(filename, &db);
    if (rc != SQLITE_OK){
        sprintf(err_msg, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
        return -1;
    }
    //rc = sqlite3_key(db, const void* pKey, int nKey);
    /* Invoked by PRAGMA key='x' */
    //SQLITE_API int sqlite3_key(sqlite3 * db,const void* pKey, int nKey);
    //SQLITE_API int sqlite3_rekey(sqlite3 * db,const void* pKey, int nKey);
    //sqlite3_key_v2() and sqlite3_rekey_v2()

    if (test_db_inited(db) != 1){
        db_tables_init(db);
    }
	return 0;
}
int database_close()
{
    sqlite3_close(db);
    return 0;
}
int db_get_stock(int item_id, int max_recode_id)
{
    int ret = 0;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT sum(stock_in),sum(stock_out) FROM stock_recodes WHERE item_id=? AND id<=?;";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    sqlite3_bind_int(stmt, 1, item_id);
    sqlite3_bind_int(stmt, 2, max_recode_id);
    ret = sqlite3_step(stmt);
    if (ret == SQLITE_ROW) {
        int stock_in = sqlite3_column_int(stmt, 0);
        int stock_out = sqlite3_column_int(stmt, 1);
        return stock_in - stock_out;
    }
    return 0x80000000;//负的最小值
}

cJSON* db_get_items_category()
{
    int ret = 0;
    sqlite3_stmt* stmt;
    cJSON* json_ret = cJSON_CreateArray(); //cJSON_CreateObject();
    const char* sql = "SELECT id,name FROM categorys;";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    //sqlite3_bind_int(stmt, 1, id);
    ret = sqlite3_step(stmt);
    while (ret == SQLITE_ROW) {
        int row = sqlite3_column_count(stmt);
        int id = sqlite3_column_int(stmt, 0);
        const char * name = sqlite3_column_text(stmt, 1);
        cJSON* json_row = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_row,"id",id);
        cJSON_AddStringToObject(json_row, "name", name);

        cJSON_AddItemToArray(json_ret, json_row);

        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    return json_ret;
}
cJSON* db_get_items()
{
    int ret = 0;
    sqlite3_stmt* stmt;
    cJSON* json_ret = cJSON_CreateArray(); //cJSON_CreateObject();
    const char* sql = "SELECT a.id,a.name,a.cat_id,a.frequency,a.unit,a.place_id,a.amount,a.alarm,b.name FROM items AS a \
        JOIN categorys AS b ON a.cat_id=b.id \
        ORDER BY cat_id,frequency DESC;";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    //sqlite3_bind_int(stmt, 1, id);
    ret = sqlite3_step(stmt);
    while (ret == SQLITE_ROW) {
        int row = sqlite3_column_count(stmt);
        int         id   = sqlite3_column_int(stmt, 0);
        const char* name = sqlite3_column_text(stmt, 1);
        int  category_id = sqlite3_column_int(stmt, 2);
        int  frequency   = sqlite3_column_int(stmt, 3);
        const char* unit = sqlite3_column_text(stmt, 4);
        int  place_id    = sqlite3_column_int(stmt, 5);
        int  amount = sqlite3_column_int(stmt, 6);
        int  alarm = sqlite3_column_int(stmt, 7);
        const char* cat_name = sqlite3_column_text(stmt, 8);

        cJSON* json_row = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_row, "id", id);
        cJSON_AddStringToObject(json_row, "name", name);
        cJSON_AddNumberToObject(json_row, "category_id", category_id);
        cJSON_AddNumberToObject(json_row, "frequency", frequency);
        cJSON_AddStringToObject(json_row, "unit", unit);
        cJSON_AddNumberToObject(json_row, "place_id", place_id);
        cJSON_AddNumberToObject(json_row, "amount", amount);
        cJSON_AddNumberToObject(json_row, "alarm", alarm);
        cJSON_AddStringToObject(json_row, "cat_name", cat_name);
        cJSON_AddItemToArray(json_ret, json_row);

        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    return json_ret;
}
cJSON* db_get_sectors()
{
    int ret = 0;
    sqlite3_stmt* stmt;
    cJSON* json_ret = cJSON_CreateArray(); //cJSON_CreateObject();
    const char* sql = "SELECT id,name FROM sectors WHERE useing=1;";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    //sqlite3_bind_int(stmt, 1, id);
    ret = sqlite3_step(stmt);
    while (ret == SQLITE_ROW) {
        int row = sqlite3_column_count(stmt);
        int id = sqlite3_column_int(stmt, 0);
        const char* name = sqlite3_column_text(stmt, 1);
        cJSON* json_row = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_row, "id", id);
        cJSON_AddStringToObject(json_row, "name", name);

        cJSON_AddItemToArray(json_ret, json_row);

        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    return json_ret;
}
cJSON* db_get_places()
{
    int ret = 0;
    sqlite3_stmt* stmt;
    cJSON* json_ret = cJSON_CreateArray(); //cJSON_CreateObject();
    const char* sql = "SELECT id,sort,name,note FROM places ORDER BY sort;";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    //sqlite3_bind_int(stmt, 1, id);
    ret = sqlite3_step(stmt);
    while (ret == SQLITE_ROW) {
        cJSON* json_row = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_row, "id", sqlite3_column_int(stmt, 0));
        cJSON_AddNumberToObject(json_row, "sort", sqlite3_column_int(stmt, 1));
        cJSON_AddStringToObject(json_row, "name", sqlite3_column_text(stmt, 2));
        cJSON_AddStringToObject(json_row, "note", sqlite3_column_text(stmt, 3));
        cJSON_AddItemToArray(json_ret, json_row);
        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    return json_ret;
}
cJSON* db_get_vendors()
{
    int ret = 0;
    sqlite3_stmt* stmt;
    cJSON* json_ret = cJSON_CreateArray(); //cJSON_CreateObject();
    const char* sql = "SELECT id,name,phone FROM vendors WHERE useing=1;";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    //sqlite3_bind_int(stmt, 1, id);
    ret = sqlite3_step(stmt);
    while (ret == SQLITE_ROW) {
        cJSON* json_row = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_row, "id", sqlite3_column_int(stmt, 0));
        cJSON_AddStringToObject(json_row, "name", sqlite3_column_text(stmt, 1));
        cJSON_AddStringToObject(json_row, "phone", sqlite3_column_text(stmt, 2));

        cJSON_AddItemToArray(json_ret, json_row);

        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    return json_ret;
}
cJSON* db_save_vendor(const char * str_id, const char* name, const char* phone)
{
    const char* error = "no";
    int ret = 0;
    int id = atoi(str_id);
    const char* sql = "";
    if (id == 0) {
        sql = "INSERT INTO vendors (name,phone) VALUES (?,?)";
    }
    else {
        sql = "UPDATE vendors SET name=?,phone=? WHERE id=?;";
    }
    sqlite3_stmt* stmt;
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, name, (int)strlen(name), NULL);
    sqlite3_bind_text(stmt, 2, phone, (int)strlen(phone), NULL);
    if (id != 0)
        sqlite3_bind_int(stmt, 3, id);

    sqlite3_step(stmt);
    ret = sqlite3_finalize(stmt);

exit:
    ret = 0;
    cJSON* json_ret = cJSON_CreateObject();
    cJSON_AddNumberToObject(json_ret, "result", ret);
    cJSON_AddStringToObject(json_ret, "error", error);
    return json_ret;

    return 0;
}
cJSON* db_get_stock_recodes(int month, int item_id,int b_stock_in,const char * who)
{
    int ret = 0;
    sqlite3_stmt* stmt;
    cJSON* json_ret = cJSON_CreateArray();
    char temp[256] = "";

    const char sql[1024] =
        "SELECT a.id,a.date,a.word,a.code,a.who,a.item_id,a.stock_in,a.stock_out,a.amount,a.note,b.name,b.cat_id,b.unit,c.name "
         "FROM stock_recodes AS a JOIN items AS b ON a.item_id=b.id JOIN categorys AS c ON b.cat_id=c.id WHERE 1=1 ";
    if (month != 0) {
        sprintf(temp, "AND STRFTIME('%%m', a.date)='%02d' ", month);
        strcat(sql, temp);
    }
    if (item_id != 0) {
        sprintf(temp, "AND a.item_id = %d  ", item_id);
        strcat(sql, temp);
    }
    if (b_stock_in == 1) {
        strcat(sql, "AND a.stock_out > 0  ");
    }
    else if (b_stock_in == 2)
    {
        strcat(sql, "AND a.stock_in > 0 ");
    }
    if (strlen(who) != 0) {
        sprintf(temp, "AND a.who = '%s'  ", who);
        strcat(sql, temp);
    }
    strcat(sql, " ORDER BY a.id; ");

    const char* error = "";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, &error);
 
    ret = sqlite3_step(stmt);
    while (ret == SQLITE_ROW) {
        int row_id = sqlite3_column_int(stmt, 0);
        int row_item_id = sqlite3_column_int(stmt, 5);
        cJSON* json_row = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_row, "id", row_id);
        cJSON_AddStringToObject(json_row, "date", sqlite3_column_text(stmt, 1));
        cJSON_AddStringToObject(json_row, "word", sqlite3_column_text(stmt, 2));
        cJSON_AddNumberToObject(json_row, "code", sqlite3_column_int(stmt, 3));
        cJSON_AddStringToObject(json_row, "who", sqlite3_column_text(stmt, 4));
        cJSON_AddNumberToObject(json_row, "item_id", row_item_id);
        cJSON_AddNumberToObject(json_row, "stock_in", sqlite3_column_int(stmt, 6));
        cJSON_AddNumberToObject(json_row, "stock_out", sqlite3_column_int(stmt, 7));
        cJSON_AddNumberToObject(json_row, "amount", sqlite3_column_int(stmt, 8));
        cJSON_AddStringToObject(json_row, "note", sqlite3_column_text(stmt, 9));
        cJSON_AddStringToObject(json_row, "item_name", sqlite3_column_text(stmt, 10));
        cJSON_AddNumberToObject(json_row, "cat_id", sqlite3_column_int(stmt, 11));
        cJSON_AddStringToObject(json_row, "unit", sqlite3_column_text(stmt, 12));
        cJSON_AddStringToObject(json_row, "cat_name", sqlite3_column_text(stmt, 13));
        cJSON_AddNumberToObject(json_row, "stock", db_get_stock(row_item_id,row_id));
        cJSON_AddItemToArray(json_ret, json_row);
        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    return json_ret;
}
cJSON* db_get_stock_recodes_last(int max_id, int limit)
{
    int ret = 0;
    sqlite3_stmt* stmt;
    cJSON* json_ret = cJSON_CreateArray();
    //const char sql[1024] =
    //    "SELECT id,date,word,code,who,item_id,stock_in,stock_out,amount,note \
    //     FROM stock_recodes  ORDER BY id DESC LIMIT 10";


    const char sql[1024] =
        "SELECT a.id,a.date,a.word,a.code,a.who,a.item_id,a.stock_in,a.stock_out,a.amount,a.note,b.name,b.cat_id,b.unit,c.name \
         FROM stock_recodes AS a JOIN items AS b ON a.item_id=b.id JOIN categorys AS c ON b.cat_id=c.id WHERE a.id<=? ORDER BY a.id DESC LIMIT ? ;";
    const char* error = "";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, &error);
    sqlite3_bind_int(stmt, 1, max_id);
    sqlite3_bind_int(stmt, 2, limit);

    ret = sqlite3_step(stmt);
    while (ret == SQLITE_ROW) {
        int row_id = sqlite3_column_int(stmt, 0);
        int row_item_id = sqlite3_column_int(stmt, 5);
        cJSON* json_row = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_row, "id", row_id);
        cJSON_AddStringToObject(json_row, "date", sqlite3_column_text(stmt, 1));
        cJSON_AddStringToObject(json_row, "word", sqlite3_column_text(stmt, 2));
        cJSON_AddNumberToObject(json_row, "code", sqlite3_column_int(stmt, 3));
        cJSON_AddStringToObject(json_row, "who", sqlite3_column_text(stmt, 4));
        cJSON_AddNumberToObject(json_row, "item_id", row_item_id);
        cJSON_AddNumberToObject(json_row, "stock_in", sqlite3_column_int(stmt, 6));
        cJSON_AddNumberToObject(json_row, "stock_out", sqlite3_column_int(stmt, 7));
        cJSON_AddNumberToObject(json_row, "amount", sqlite3_column_int(stmt, 8));
        cJSON_AddStringToObject(json_row, "note", sqlite3_column_text(stmt, 9));
        cJSON_AddStringToObject(json_row, "item_name", sqlite3_column_text(stmt, 10));
        cJSON_AddNumberToObject(json_row, "cat_id", sqlite3_column_int(stmt, 11));
        cJSON_AddStringToObject(json_row, "unit", sqlite3_column_text(stmt, 12));
        cJSON_AddStringToObject(json_row, "cat_name", sqlite3_column_text(stmt, 13));
        cJSON_AddNumberToObject(json_row, "stock", db_get_stock(row_item_id, row_id));
        cJSON_AddItemToArray(json_ret, json_row);
        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    return json_ret;
}
//获取所有物品的库存剩余量
cJSON* db_get_stock_remaining()
{
    int ret = 0;
    sqlite3_stmt* stmt;
    cJSON* json_ret = cJSON_CreateArray();

    const char sql[1024] =
        "SELECT items.id,items.name,\
        (SUM(stock_in) - SUM(stock_out)) AS difference \
        FROM stock_recodes \
        JOIN items ON stock_recodes.item_id = items.id \
        GROUP BY stock_recodes.item_id \
        ORDER BY difference; ";
    const char* error = "";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, &error);

    ret = sqlite3_step(stmt);
    while (ret == SQLITE_ROW) {
        int row_id = sqlite3_column_int(stmt, 0);
        int row_item_id = sqlite3_column_int(stmt, 5);
        cJSON* json_row = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_row, "id",   sqlite3_column_int(stmt, 0));
        cJSON_AddStringToObject(json_row, "name", sqlite3_column_text(stmt, 1));
        cJSON_AddNumberToObject(json_row, "total", sqlite3_column_int(stmt, 2));
        cJSON_AddItemToArray(json_ret, json_row);
        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    return json_ret;
}
cJSON* db_save_recode(const char* str, int len)
{
    const char* error = "";
    int ret = -1;

    cJSON* json = cJSON_ParseWithLength(str, len);
    if (!json) {
        set_error_go_exit(-1, "cJSON_Parse error");
    }
    const char * date = cJSON_GetObjectItem(json, "date")->valuestring;    
    const char* word = cJSON_GetObjectItem(json, "word")->valuestring;
    int code = cJSON_GetObjectItem(json, "code")->valueint;
    const char* note = cJSON_GetObjectItem(json, "note")->valuestring;
    const char* who = cJSON_GetObjectItem(json, "who")->valuestring;
    int item_id = cJSON_GetObjectItem(json, "item_id")->valueint;
    int stock_in = cJSON_GetObjectItem(json, "stock_in")->valueint;
    int stock_out = cJSON_GetObjectItem(json, "stock_out")->valueint;
    int amount = cJSON_GetObjectItem(json, "amount")->valueint;
    int recode_id = cJSON_GetObjectItem(json, "recode_id")->valueint;

    const char* sql = "";
    if (recode_id == 0) {
        sql = "INSERT INTO stock_recodes (date,word,code,who,item_id,stock_in,stock_out,amount,note) \
                                              VALUES (?,?,?,?,?,?,?,?,?)";
    }
    else {
        sql = "UPDATE stock_recodes SET date=?,word=?,code=?,who=?,item_id=?,stock_in=?,stock_out=?,amount=?,note=? WHERE id=?;";
    }
    sqlite3_stmt* stmt;
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);

    sqlite3_bind_text(stmt, 1, date, (int)strlen(date), NULL);
    sqlite3_bind_text(stmt, 2, word, (int)strlen(word), NULL);
    sqlite3_bind_int(stmt, 3, code);
    sqlite3_bind_text(stmt, 4, who, (int)strlen(who), NULL);
    sqlite3_bind_int(stmt, 5, item_id);
    sqlite3_bind_int(stmt, 6, stock_in);
    sqlite3_bind_int(stmt, 7, stock_out);
    sqlite3_bind_int(stmt, 8, amount);
    sqlite3_bind_text(stmt, 9, note, (int)strlen(note), NULL);
    if(recode_id != 0)
        sqlite3_bind_int(stmt, 10, recode_id);

    sqlite3_step(stmt);
    ret = sqlite3_finalize(stmt);

exit:
    if(json != NULL)
        cJSON_free(json);

    cJSON* json_ret = cJSON_CreateObject();
    cJSON_AddNumberToObject(json_ret, "result", ret);
    cJSON_AddStringToObject(json_ret, "error", error);
    return json_ret;
}
cJSON* db_add_item(const char* str, int len)
{
    //添加一种物品信息
    const char* error = "";
    int ret = 0;
    sqlite3_stmt* stmt = NULL;
    cJSON* item = cJSON_ParseWithLength(str, len);
    if (item == NULL) {
        set_error_go_exit(-1, "json string parse error");
    }
    const char* sql = "INSERT INTO items (name,cat_id,frequency,unit,place_id,amount,alarm) VALUES (?,?,0,?,?,0,10000);";
    ret = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);
    if (ret != 0) {
        set_error_go_exit(-1, "sqlite3_prepare_v2 error");
    }
    const char* item_name = cJSON_GetObjectItem(item, "name")->valuestring;
    const char* unit = cJSON_GetObjectItem(item, "unit")->valuestring;

    sqlite3_bind_text(stmt, 1, item_name, (int)strlen(item_name), NULL);
    sqlite3_bind_int(stmt, 2, cJSON_GetObjectItem(item, "category_id")->valueint);
    sqlite3_bind_text(stmt, 3, unit, (int)strlen(unit), NULL);
    sqlite3_bind_int(stmt, 4, cJSON_GetObjectItem(item, "place_id")->valueint);
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        set_error_go_exit(-1, sqlite3_errmsg(db));
    }
    ret = 0;
exit:
    if(stmt)
        sqlite3_finalize(stmt);
    if(item)
        cJSON_free(item);

    cJSON* json_ret = cJSON_CreateObject();
    cJSON_AddNumberToObject(json_ret, "result", ret);
    cJSON_AddStringToObject(json_ret, "error", error);
    return json_ret;
}
