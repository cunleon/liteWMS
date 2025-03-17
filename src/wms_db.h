#pragma once

#include "clib/cJson.h"
#ifdef __cplusplus
extern "C"
{
#endif
/////////////////////////////////////////////////////////////
int database_opened();
int database_open(const char* filename);
int database_close();

cJSON* db_get_items_category();
cJSON* db_get_items();
cJSON* db_get_sectors();
cJSON* db_get_places();
cJSON* db_get_vendors();
cJSON* db_save_vendor(const char* str_id, const char* name, const char* phone);

cJSON* db_add_item(const char* str, int len);
cJSON* db_save_recode(const char* str, int len);
cJSON* db_get_stock_recodes_last(int max_id, int top);
cJSON* db_get_stock_recodes(int month, int item_id, int b_stock_in, const char* who);
cJSON* db_get_stock_remaining();


/////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif