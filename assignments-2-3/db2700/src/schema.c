/***********************************************************
 * Schema for assignments in the Databases course INF-2700 *
 * UIT - The Arctic University of Norway                   *
 * Author: Weihai Yu                                       *
 ************************************************************/

#include "schema.h"
#include "pmsg.h"
#include <string.h>

/** @brief Field descriptor */
typedef struct field_desc_struct {
  char *name;        /**< field name */
  field_type type;   /**< field type */
  int len;           /**< field length (number of bytes) */
  int offset;        /**< offset from the beginning of the record */
  field_desc_p next; /**< next field_desc of the table, NULL if no more */
} field_desc_struct;

/** @brief Table/record schema */
/** A schema is a linked list of @ref field_desc_struct "field descriptors".
    All records of a table are of the same length.
*/
typedef struct schema_struct {
  char *name;           /**< schema (table) name */
  field_desc_p first;   /**< first field_desc */
  field_desc_p last;    /**< last field_desc */
  int num_fields;       /**< number of fields in the table */
  int len;              /**< record length */
  tbl_p tbl;            /**< table descriptor */
} schema_struct;

/** @brief Table descriptor */
/** A table descriptor allows us to find the schema and
    run-time infomation about the table.
 */
typedef struct tbl_desc_struct {
  schema_p sch;      /**< schema of this table. */
  int num_records;   /**< number of records this table has. */
  page_p current_pg; /**< current page being accessed. */
  tbl_p next;        /**< next tbl_desc in the database. */
} tbl_desc_struct;


/** @brief Database tables*/
tbl_p db_tables; /**< a linked list of table descriptors */

void put_field_info(pmsg_level level, field_desc_p f) {
  if (!f) {
    put_msg(level,  "  empty field\n");
    return;
  }
  put_msg(level, "  \"%s\", ", f->name);
  if (is_int_field(f))
    append_msg(level,  "int ");
  else
    append_msg(level,  "str ");
  append_msg(level, "field, len: %d, offset: %d, ", f->len, f->offset);
  if (f->next)
    append_msg(level,  ", next field: %s\n", f->next->name);
  else
    append_msg(level,  "\n");
}

void put_schema_info(pmsg_level level, schema_p s) {
  if (!s) {
    put_msg(level,  "--empty schema\n");
    return;
  }
  field_desc_p f;
  put_msg(level, "--schema %s: %d field(s), totally %d bytes\n",
          s->name, s->num_fields, s->len);
  for (f = s->first; f; f = f->next)
    put_field_info(level, f);
  put_msg(level, "--\n");
}

void put_tbl_info(pmsg_level level, tbl_p t) {
  if (!t) {
    put_msg(level,  "--empty tbl desc\n");
    return;
  }
  put_schema_info(level, t->sch);
  put_file_info(level, t->sch->name);
  put_msg(level, " %d blocks, %d records\n",
          file_num_blocks(t->sch->name), t->num_records);
  put_msg(level, "----\n");
}

void put_record_info(pmsg_level level, record r, schema_p s) {
  field_desc_p f;
  size_t i = 0;
  put_msg(level, "Record: ");
  for (f = s->first; f; f = f->next, i++) {
    if (is_int_field(f))
      append_msg(level,  "%d", *(int *)r[i]);
    else
      append_msg(level,  "%s", (char *)r[i]);

    if (f->next)
      append_msg(level,  " | ");
  }
  append_msg(level,  "\n");
}

void put_db_info(pmsg_level level) {
  char *db_dir = system_dir();
  if (!db_dir) return;
  put_msg(level, "======Database at %s:\n", db_dir);
  for (tbl_p tbl = db_tables; tbl; tbl = tbl->next)
    put_tbl_info(level, tbl);
  put_msg(level, "======\n");
}

field_desc_p new_int_field(char const* name) {
  field_desc_p res = malloc(sizeof (field_desc_struct));
  res->name = strdup(name);
  res->type = INT_TYPE;
  res->len = INT_SIZE;
  res->offset = 0;
  res->next = 0;
  return res;
}

field_desc_p new_str_field(char const* name, int len) {
  field_desc_p res = malloc(sizeof (field_desc_struct));
  res->name = strdup(name);
  res->type = STR_TYPE;
  res->len = len;
  res->offset = 0;
  res->next = 0;
  return res;
}

static void release_field_desc(field_desc_p f) {
  if (f) {
    free(f->name);
    free(f);
    f = 0;
  }
}

int is_int_field(field_desc_p f) {
  return f ? (f->type == INT_TYPE) : 0;
}

field_desc_p field_desc_next(field_desc_p f) {
  if (f)
    return f->next;
  else {
    put_msg(ERROR, "field_desc_next: NULL field_desc_next.\n");
    return 0;
  }
}

static schema_p make_schema(char const* name) {
  schema_p res = malloc(sizeof (schema_struct));
  res->name = strdup(name);
  res->first = 0;
  res->last = 0;
  res->num_fields = 0;
  res->len = 0;
  return res;
}

/** Release the memory allocated for the schema and its field descriptors.*/
static void release_schema(schema_p sch) {
  if (!sch) return;

  field_desc_p f, nextf;
  f = sch->first;
  while (f) {
    nextf = f->next;
    release_field_desc(f);
    f = nextf;
  }
  free(sch->name);
  free(sch);
}

char const* const schema_name(schema_p sch) {
  if (sch)
    return sch->name;
  else {
    put_msg(ERROR, "schema_name: NULL schema.\n");
    return 0;
  }
}

field_desc_p schema_first_fld_desc(schema_p sch) {
  if (sch)
    return sch->first;
  else {
    put_msg(ERROR, "schema_first_fld_desc: NULL schema.\n");
    return 0;
  }
}

field_desc_p schema_last_fld_desc(schema_p sch) {
  if (sch)
    return sch->last;
  else {
    put_msg(ERROR, "schema_last_fld_desc: NULL schema.\n");
    return 0;
  }
}

int schema_num_flds(schema_p sch) {
  if (sch)
    return sch->num_fields;
  else {
    put_msg(ERROR, "schema_num_flds: NULL schema.\n");
    return -1;
  }
}

int schema_len(schema_p sch) {
  if (sch)
    return sch->len;
  else {
    put_msg(ERROR, "schema_len: NULL schema.\n");
    return -1;
  }
}

const char tables_desc_file[] = "db.db"; /***< File holding table descriptors */

static char* concat_names(char const* name1, char const* sep, char const* name2) {
  char *res = malloc((strlen(name1)) + (strlen(sep)) + (strlen(name2)) + 1);
  strcpy(res, name1);
  strcat(res, sep);
  strcat(res, name2);
  return res;
}

static void save_tbl_desc(FILE *fp, tbl_p tbl) {
  schema_p sch = tbl->sch;
  fprintf(fp, "%s %d\n", sch->name, sch->num_fields);
  field_desc_p fld = schema_first_fld_desc(sch);
  while (fld) {
    fprintf(fp, "%s %d %d %d\n",
            fld->name, fld->type, fld->len, fld->offset);
    fld = fld->next;
  }
  fprintf(fp, "%d\n", tbl->num_records);
}

static void save_tbl_descs() {
  /* backup the descriptors first in case we need some manual investigation */
  char *tbl_desc_backup = concat_names("__backup", "_", tables_desc_file);
  rename(tables_desc_file, tbl_desc_backup);
  free(tbl_desc_backup);

  FILE *dbfile = fopen(tables_desc_file, "w");
  tbl_p tbl = db_tables, next_tbl = 0;
  while (tbl) {
    save_tbl_desc(dbfile, tbl);
    release_schema(tbl->sch);
    next_tbl = tbl->next;
    free(tbl);
    tbl = next_tbl;
  }
  fclose(dbfile);
}

static void read_tbl_descs() {
  FILE *fp = fopen(tables_desc_file, "r");
  if (!fp) return;
  char name[30] = "";
  schema_p sch = NULL;
  field_desc_p fld = NULL;
  int num_flds = 0, fld_type, fld_len;
  while (!feof(fp)) {
    if (fscanf(fp, "%s %d\n", name, &num_flds) < 2) {
      fclose(fp);
      return;
    }
    sch = new_schema(name);
    for (size_t i = 0; i < num_flds; i++) {
      fscanf(fp, "%s %d %d", name, &(fld_type), &(fld_len));
      switch (fld_type) {
      case INT_TYPE:
        fld = new_int_field(name);
        break;
      case STR_TYPE:
        fld = new_str_field(name, fld_len);
        break;
      }
      fscanf(fp, "%d\n", &(fld->offset));
      add_field(sch, fld);
    }
    fscanf(fp, "%d\n", &(sch->tbl->num_records));
  }
  db_tables = sch->tbl;
  fclose(fp);
}

int open_db(void) {
  pager_terminate(); /* first clean up for a fresh start */
  pager_init();
  read_tbl_descs();
  return 1;
}

void close_db(void) {
  save_tbl_descs();
  db_tables = 0;
  pager_terminate();
}

schema_p new_schema(char const* name) {
  tbl_p tbl = malloc(sizeof (tbl_desc_struct));
  tbl->sch = make_schema(name);
  tbl->sch->tbl = tbl;
  tbl->num_records = 0;
  tbl->current_pg = 0;
  tbl->next = db_tables;
  db_tables = tbl;
  return tbl->sch;
}

tbl_p get_table(char const* name) {
  for (tbl_p tbl = db_tables; tbl; tbl = tbl->next)
    if (strcmp(name, tbl->sch->name) == 0)
      return tbl;
  return 0;
}

schema_p get_schema(char const* name) {
  tbl_p tbl = get_table(name);
  if (tbl) return tbl->sch;
  else return 0;
}

void remove_table(tbl_p t) {
  if (!t) return;

  for (tbl_p tbl = db_tables, prev = 0;
       tbl;
       prev = tbl, tbl = tbl->next)
    if (tbl == t) {
      if (t == db_tables)
        db_tables = t->next;
      else
        prev->next = t->next;

      close_file(t->sch->name);
      char *tbl_backup = concat_names("_", "_", t->sch->name);
      rename(t->sch->name, tbl_backup);
      free(tbl_backup);
      release_schema(t->sch);
      free(t);
      return;
    }
}

void remove_schema(schema_p s) {
  if (s) remove_table(s->tbl);
}

static field_desc_p dup_field(field_desc_p f) {
  field_desc_p res = malloc(sizeof (field_desc_struct));
  res->name = strdup(f->name);
  res->type = f->type;
  res->len = f->len;
  res->offset = 0;
  res->next = 0;
  return res;
}

static schema_p copy_schema(schema_p s, char const* dest_name) {
  if (!s) return 0;
  schema_p dest = new_schema(dest_name);
  for (field_desc_p f = s->first; f; f = f->next)
    add_field(dest, dup_field(f));
  return dest;
}

static field_desc_p get_field(schema_p s, char const* name) {
  for (field_desc_p f = s->first; f; f = f->next)
    if (strcmp(f->name, name) == 0) return f;
  return 0;
}

static char* tmp_schema_name(char const* op_name, char const* name) {
  char *res = malloc((strlen(op_name)) + (strlen(name)) + 10);
  int i = 0;
  do
    sprintf(res, "%s__%s_%d", op_name, name, i++);
  while (get_schema(res));

  return res;
}

static schema_p make_sub_schema(schema_p s, int num_fields, char *fields[]) {
  if (!s) return 0;

  char *sub_sch_name = tmp_schema_name("project", s->name);
  schema_p res = new_schema(sub_sch_name);
  free(sub_sch_name);
  
  field_desc_p f = 0;
  for (size_t i= 0; i < num_fields; i++) {
    f = get_field(s, fields[i]);
    if (f)
      add_field(res, dup_field(f));
    else {
      put_msg(ERROR, "\"%s\" has no \"%s\" field\n",
              s->name, fields[i]);
      remove_schema(res);
      return 0;
    }
  }
  return res;
}

int add_field(schema_p s, field_desc_p f) {
  if (!s) return 0;
  if (s->len + f->len > BLOCK_SIZE - PAGE_HEADER_SIZE) {
    put_msg(ERROR,
            "schema already has %d bytes, adding %d will exceed limited %d bytes.\n",
            s->len, f->len, BLOCK_SIZE - PAGE_HEADER_SIZE);
    return 0;
  }
  if (s->num_fields == 0) {
    s->first = f;
    f->offset = 0;
  }
  else {
    s->last->next = f;
    f->offset = s->len;
  }
  s->last = f;
  s->num_fields++;
  s->len += f->len;
  return s->num_fields;
}

record new_record(schema_p s) {
  if (!s) {
    put_msg(ERROR,  "new_record: NULL schema!\n");
    exit(EXIT_FAILURE);
  }
  record res = malloc((sizeof (void *)) * s->num_fields);

  /* allocate memory for the fields */
  field_desc_p f;
  size_t i = 0;
  for (f = s->first; f; f = f->next, i++) {
    res[i] =  malloc(f->len);
  }
  return res;
}

void release_record(record r, schema_p s) {
  if (!(r && s)) {
    put_msg(ERROR,  "release_record: NULL record or schema!\n");
    return;
  }
  for (size_t i = 0; i < s->num_fields; i++)
    free(r[i]);
  free(r);
  r = 0;
}

void assign_int_field(void const* field_p, int int_val) {
  *(int *)field_p = int_val;
}

void assign_str_field(void* field_p, char const* str_val) {
  strcpy(field_p, str_val);
}

int fill_record(record r, schema_p s, ...) {
  if (!(r && s)) {
    put_msg(ERROR,  "fill_record: NULL record or schema!\n");
    return 0;
  }
  va_list vals;
  va_start(vals, s);
  field_desc_p f;
  size_t i = 0;
  for (f = s->first; f; f = f->next, i++) {
    if (is_int_field(f))
      assign_int_field(r[i], va_arg(vals, int));
    else
      assign_str_field(r[i], va_arg(vals, char*));
  }
  return 1;
}

static void fill_sub_record(record dest_r, schema_p dest_s,
                            record src_r, schema_p src_s) {
  field_desc_p src_f, dest_f;
  size_t i = 0, j = 0;
  for (dest_f = dest_s->first; dest_f; dest_f = dest_f->next, i++) {
    for (j = 0, src_f = src_s->first;
         strcmp(src_f->name, dest_f->name) != 0;
         j++, src_f = src_f->next)
      ;
    if (is_int_field(dest_f))
      assign_int_field(dest_r[i], *(int *)src_r[j]);
    else
      assign_str_field(dest_r[i], (char *)src_r[j]);
  }
}

int equal_record(record r1, record r2, schema_p s) {
  if (!(r1 && r2 && s)) {
    put_msg(ERROR,  "equal_record: NULL record or schema!\n");
    return 0;
  }

  field_desc_p fd;
  size_t i = 0;;
  for (fd = s->first; fd; fd = fd->next, i++) {
    if (is_int_field(fd)) {
      if (*(int *)r1[i] != *(int *)r2[i])
        return 0;
    }
    else {
      if (strcmp((char *)r1[i], (char *)r2[i]) != 0)
        return 0;
    }
  }
  return 1;
}

void set_tbl_position(tbl_p t, tbl_position pos) {
  switch (pos) {
  case TBL_BEG:
    {
      t->current_pg = get_page(t->sch->name, 0);
      page_set_pos_begin(t-> current_pg);
    }
    break;
  case TBL_END:
    t->current_pg = get_page_for_append(t->sch->name);
  }
}

int eot(tbl_p t) {
  return (peof(t->current_pg));
}

/** check if the the current position is valid */
static int page_valid_pos_for_get_with_schema(page_p p, schema_p s) {
  return (page_valid_pos_for_get(p, page_current_pos(p))
          && (page_current_pos(p) - PAGE_HEADER_SIZE) % s->len == 0);
}

/** check if the the current position is valid */
static int page_valid_pos_for_put_with_schema(page_p p, schema_p s) {
  return (page_valid_pos_for_put(p, page_current_pos(p), s->len)
          && (page_current_pos(p) - PAGE_HEADER_SIZE) % s->len == 0);
}

static page_p get_page_for_next_record(schema_p s) {
  page_p pg = s->tbl->current_pg;
  if (peof(pg)) return 0;
  if (eop(pg)) {
    unpin(pg);
    pg = get_next_page(pg);
    if (!pg) {
      put_msg(FATAL, "get_page_for_next_record failed at block %d\n",
              page_block_nr(pg) + 1);
      exit(EXIT_FAILURE);
    }
    page_set_pos_begin(pg);
    s->tbl->current_pg = pg;
  }
  return pg;
}

static int get_page_record(page_p p, record r, schema_p s) {
  if (!p) return 0;
  if (!page_valid_pos_for_get_with_schema(p, s)) {
    put_msg(FATAL, "try to get record at invalid position.\n");
    exit(EXIT_FAILURE);
  }
  field_desc_p fld_desc;
  size_t i = 0;
  for (fld_desc = s->first; fld_desc;
       fld_desc = fld_desc->next, i++)
    if (is_int_field(fld_desc))
      assign_int_field(r[i], page_get_int(p));
    else
      page_get_str(p, r[i], fld_desc->len);
  return 1;
}

int get_record(record r, schema_p s) {
  page_p pg = get_page_for_next_record(s);
  return pg ? get_page_record(pg, r, s) : 0;
}

static int int_equal(int x, int y) {
  return x == y;
}
static int int_less(int x, int y) {
  return x > y;
}
static int int_less_equal(int x, int y) {
  return x >= y;
}
static int int_greater(int x, int y) {
  return x < y;
}
static int int_greater_equal(int x, int y) {
  return x <= y;
}
static int int_not_equal(int x, int y) {
  return x != y;
}

static int find_record_int_val(record r, schema_p s, int offset,
                               int (*op) (int, int), int val) {
  page_p pg = get_page_for_next_record(s);
  if (!pg) return 0;
  int pos, rec_val;
  for (; pg; pg = get_page_for_next_record(s)) {
    pos = page_current_pos(pg);
    rec_val = page_get_int_at (pg, pos + offset);
    if ((*op) (val, rec_val)) {
      page_set_current_pos(pg, pos);
      get_page_record(pg, r, s);
      return 1;
    }
    else
      page_set_current_pos(pg, pos + s->len);
  }
  return 0;
}

static int put_page_record(page_p p, record r, schema_p s) {
  if (!page_valid_pos_for_put_with_schema(p, s))
    return 0;

  field_desc_p fld_desc;
  size_t i = 0;
  for (fld_desc = s->first; fld_desc;
       fld_desc = fld_desc->next, i++)
    if (is_int_field(fld_desc))
      page_put_int(p, *(int *)r[i]);
    else
      page_put_str(p, (char *)r[i], fld_desc->len);
  return 1;
}

int put_record(record r, schema_p s) {
  page_p p = s->tbl->current_pg;

  if (!page_valid_pos_for_put_with_schema(p, s))
    return 0;

  field_desc_p fld_desc;
  size_t i = 0;
  for (fld_desc = s->first; fld_desc;
       fld_desc = fld_desc->next, i++)
    if (is_int_field(fld_desc))
      page_put_int(p, *(int *)r[i]);
    else
      page_put_str(p, (char *)r[i], fld_desc->len);
  return 1;
}

void append_record(record r, schema_p s) {
  tbl_p tbl = s->tbl;
  page_p pg = get_page_for_append(s->name);
  if (!pg) {
    put_msg(FATAL, "Failed to get page for appending to \"%s\".\n",
            s->name);
    exit(EXIT_FAILURE);
  }
  if (!put_page_record(pg, r, s)) {
    /* not enough space in the current page */
    unpin(pg);
    pg = get_next_page(pg);
    if (!pg) {
      put_msg(FATAL, "Failed to get page for \"%s\" block %d.\n",
              s->name, page_block_nr(pg) + 1);
      exit(EXIT_FAILURE);
    }
    if (!put_page_record(pg, r, s)) {
      put_msg(FATAL, "Failed to put record to page for \"%s\" block %d.\n",
              s->name, page_block_nr(pg) + 1);
      exit(EXIT_FAILURE);
    }
  }
  tbl->current_pg = pg;
  tbl->num_records++;
}

static void display_tbl_header(tbl_p t) {
  if (!t) {
    put_msg(INFO,  "Trying to display non-existant table.\n");
    return;
  }
  schema_p s = t->sch;
  for (field_desc_p f = s->first; f; f = f->next)
    put_msg(FORCE, "%20s", f->name);
  put_msg(FORCE, "\n");
  for (field_desc_p f = s->first; f; f = f->next) {
    for (size_t i = 0; i < 20 - strlen(f->name); i++)
      put_msg(FORCE, " ");
    for (size_t i = 0; i < strlen(f->name); i++)
      put_msg(FORCE, "-");
  }
  put_msg(FORCE, "\n");
}

static void display_record(record r, schema_p s) {
  field_desc_p f = s->first;
  for (size_t i = 0; f; f = f->next, i++) {
    if (is_int_field(f))
      put_msg(FORCE, "%20d", *(int *)r[i]);
    else
      put_msg(FORCE, "%20s", (char *)r[i]);
  }
  put_msg(FORCE, "\n");
}

void table_display(tbl_p t) {
  if (!t) return;
  display_tbl_header(t);

  schema_p s = t->sch;
  record rec = new_record(s);
  set_tbl_position(t, TBL_BEG);
  while (get_record(rec, s)) {
    display_record(rec, s);
  }
  put_msg(FORCE, "\n");

  release_record(rec, s);
}

/* Returns value at an index in schema.
   Offset detemins the field.
   Stores the values page position in pos. */
int get_value_at_idx(int idx, schema_p s, int *pos, int offset) {
  int rec_size = s->num_fields * sizeof(int);
  int rec_per_blk = (BLOCK_SIZE - PAGE_HEADER_SIZE) / rec_size;
  int blk_nr = idx / (rec_per_blk);
  int rec_pos = ((idx % rec_per_blk) * rec_size) + PAGE_HEADER_SIZE;
  *pos = rec_pos;

  page_p page = get_page(s->name, blk_nr);
  s->tbl->current_pg = page;

  return page_get_int_at(page, rec_pos + offset);
}

int binarySearch(record r, schema_p s, int offset, int val) {
  int start = 0;
  int end = s->tbl->num_records - 1;
  int pos;

  /* Binary search loop */
  while (start <= end) {
    int middle = (start + end) / 2;
        int middle_val = get_value_at_idx(middle, s, &pos, offset);
 
    /* Check if middle value is equal to target value */
    if (val == middle_val) {
            page_set_current_pos(s->tbl->current_pg, pos);
            get_page_record(s->tbl->current_pg, r, s);
            return 1;
        }

    /* Limit search bellow middle */
    else if (val < middle_val)
      end = middle - 1;

    /* Limit search above middle */
    else if (val > middle_val)
      start = middle + 1;
  }

    return 0;
}

/* We restrict ourselves to equality search on an int attribute */
tbl_p table_search(tbl_p t, char const* attr, char const* op, int val) {
  if (!t) return 0;
  int doBinarySearch = 0;

  int (*cmp_op)() = 0;

  if (strcmp(op, "=") == 0) {
    cmp_op = int_equal;
      doBinarySearch = 1;
  }
  if (strcmp(op, "<") == 0)
    cmp_op = int_less;

  if (strcmp(op, "<=") == 0)
    cmp_op = int_less_equal;

  if (strcmp(op, ">") == 0)
    cmp_op = int_greater;

  if (strcmp(op, ">=") == 0)
    cmp_op = int_greater_equal;

  if (strcmp(op, "!=") == 0)
    cmp_op = int_not_equal;

  if (!cmp_op) {
    put_msg(ERROR, "unknown comparison operator \"%s\".\n", op);
    return 0;
  }

  schema_p s = t->sch;
  field_desc_p f;
  size_t i = 0;
  for (f = s->first; f; f = f->next, i++)
    if (strcmp(f->name, attr) == 0) {
      if (f->type != INT_TYPE) {
        put_msg(ERROR, "\"%s\" is not an integer field.\n", attr);
        return 0;
      }
      break;
    }
  if (!f) return 0;

  char *tmp_name = tmp_schema_name("select", s->name);
  schema_p res_sch = copy_schema(s, tmp_name);
  free(tmp_name);

  record rec = new_record(s);

  /* pager_profiler_reset(); */

  set_tbl_position(t, TBL_BEG);

  doBinarySearch = 0;

  /* If equal operand do binary search
     else do linear search */
  if (doBinarySearch) {
    /* printf("\nBinary search:\n"); */
      if (binarySearch(rec, s, f->offset, val))
      append_record(rec, res_sch);
  }
  else {
    /* printf("\nLinear search:\n"); */
    while (find_record_int_val(rec, s, f->offset, cmp_op, val)) {
      put_record_info(DEBUG, rec, s);
      append_record(rec, res_sch);
    }
  }
  /* put_pager_profiler_info(INFO);  */

  release_record(rec, s);

  return res_sch->tbl;
}

tbl_p table_project(tbl_p t, int num_fields, char* fields[]) {
  schema_p s = t->sch;
  schema_p dest = make_sub_schema(s, num_fields, fields);
  if (!dest) return 0;

  record rec = new_record(s), rec_dest = new_record(dest);

  set_tbl_position(t, TBL_BEG);
  while (get_record(rec, s)) {
    fill_sub_record(rec_dest, dest, rec, s);
    put_record_info(DEBUG, rec_dest, dest);
    append_record(rec_dest, dest);
  }

  release_record(rec, s);
  release_record(rec_dest, dest);

  return dest->tbl;
}

int idx_to_pos(int idx, int rec_size) {
  /* int rec_per_blk = (BLOCK_SIZE - PAGE_HEADER_SIZE) / rec_size; */
  int pos = ((idx) * rec_size) + PAGE_HEADER_SIZE;
  return pos;
}

int idx_to_record(int idx, record rec, int rec_size, page_p page, schema_p s) {
  page_set_current_pos(page, idx_to_pos(idx, rec_size));

  /* Check if last record of table */
  if (peof(page))
    return OUT_OF_RECORDS;

  get_page_record(page, rec, s);
  return 1;
}

void natural_join_fields(schema_p schLeft, schema_p schRight, schema_p schRes, int *leftOffset, int *rightOffset) {
    /* Add all left fields */
  for (field_desc_p leftField = schLeft->first; leftField; leftField = leftField->next) {
    if (leftField->type == INT_TYPE)
      add_field(schRes, new_int_field(leftField->name));
    if (leftField->type == STR_TYPE)
      add_field(schRes, new_str_field(leftField->name, leftField->len));
  }

  int leftIdx = 0, rightIdx = 0;
  /* Add unique right fields */
  for (field_desc_p rightField = schRight->first; rightField; rightField = rightField->next) {
    int different = 1;

    /* Compare right field with all added left fields */
    for (field_desc_p resField = schRes->first; resField; resField = resField->next) {
      if (!strncmp(resField->name, rightField->name, rightField->len)) {
        different = 0;
        *leftOffset = leftIdx;
        *rightOffset = rightIdx;
      }
      leftIdx++;
    }
    leftIdx = 0;
    rightIdx++;

    /* Add field if its different */
    if (different) {
      if (rightField->type == INT_TYPE)
        add_field(schRes, new_int_field(rightField->name));
      if (rightField->type == STR_TYPE)
        add_field(schRes, new_str_field(rightField->name, rightField->len));
    }
  }
}

tbl_p table_block_natural_join(tbl_p left, tbl_p right) {
  if (!(left && right)) {
    put_msg(ERROR, "no table found!\n");
    return 0;
  }
  
  schema_p sch = new_schema("blockJoin");
  tbl_p res = sch->tbl;

  int leftNumBlks = file_num_blocks(left->sch->name);
  int rightNumBlks = file_num_blocks(right->sch->name);
  int leftRecPerBlk = (BLOCK_SIZE - PAGE_HEADER_SIZE) / left->sch->len;
  int rightRecPerBlk = (BLOCK_SIZE - PAGE_HEADER_SIZE) / right->sch->len;
  int leftOffset, rightOffset;

  /* Create the fields for the result table */
  natural_join_fields(left->sch, right->sch, res->sch, &leftOffset, &rightOffset);

  /* Get left page */
  for (int blkL = 0; blkL < leftNumBlks; blkL++) {
    page_p pageL = get_page(left->sch->name, blkL);
    /* Get right page */
    for (int blkR = 0; blkR < rightNumBlks; blkR++) {
      page_p pageR = get_page(right->sch->name, blkR);

      /* Get left record */
      for (int idxRecL = 0; idxRecL < leftRecPerBlk; idxRecL++) {
        record recL = new_record(left->sch);
        if (idx_to_record(idxRecL, recL, left->sch->len, pageL, left->sch) == OUT_OF_RECORDS) {
          break;
        }

        /* Get right record */
        for (int idxRecR = 0; idxRecR < rightRecPerBlk; idxRecR++) {
          record recR = new_record(right->sch);
          if (idx_to_record(idxRecR, recR, right->sch->len, pageR, right->sch) == OUT_OF_RECORDS){
            break;
          }

          // Create and append reccords
          if (*(int *)recL[leftOffset] == *(int *)recR[rightOffset]) {
            record recRes = new_record(res->sch);

            assign_int_field(recRes[0], *(int *)recL[0]);
            assign_str_field(recRes[1], (char *)recL[1]);
            assign_str_field(recRes[2], (char *)recR[1]);

            append_record(recRes, res->sch);
            release_record(recRes, res->sch);
          }
          release_record(recR, right->sch);
        }

        release_record(recL, left->sch);
      }
      unpin(pageR);
    }
    unpin(pageL);
  }
  
  return res;
}

tbl_p table_natural_join(tbl_p left, tbl_p right) {
  if (!(left && right)) {
    put_msg(ERROR, "no table found!\n");
    return 0;
  }
  
  schema_p sch = new_schema("nestedJoin");
  tbl_p res = sch->tbl;

  int leftNumBlks = file_num_blocks(left->sch->name);
  int rightNumBlks = file_num_blocks(right->sch->name);
  int leftRecPerBlk = (BLOCK_SIZE - PAGE_HEADER_SIZE) / left->sch->len;
  int rightRecPerBlk = (BLOCK_SIZE - PAGE_HEADER_SIZE) / right->sch->len;
  int leftOffset, rightOffset;

  /* Create the fields for the result table */
  natural_join_fields(left->sch, right->sch, res->sch, &leftOffset, &rightOffset);

  /* Get left page */
  for (int blkL = 0; blkL < leftNumBlks; blkL++) {
    page_p pageL = get_page(left->sch->name, blkL);

    /* Get left record */
    for (int idxRecL = 0; idxRecL < leftRecPerBlk; idxRecL++) {
      record recL = new_record(left->sch);
      if (idx_to_record(idxRecL, recL, left->sch->len, pageL, left->sch) == OUT_OF_RECORDS) {
        break;
      }
      /* Get right page */
      for (int blkR = 0; blkR < rightNumBlks; blkR++) {
        page_p pageR = get_page(right->sch->name, blkR);

        /* Get right record */
        for (int idxRecR = 0; idxRecR < rightRecPerBlk; idxRecR++) {
          record recR = new_record(right->sch);
          if (idx_to_record(idxRecR, recR, right->sch->len, pageR, right->sch) == OUT_OF_RECORDS){
            break;
          }

          // Create and append reccords
          if (*(int *)recL[leftOffset] == *(int *)recR[rightOffset]) {
            record recRes = new_record(res->sch);

            assign_int_field(recRes[0], *(int *)recL[0]);
            assign_str_field(recRes[1], (char *)recL[1]);
            assign_str_field(recRes[2], (char *)recR[1]);

            append_record(recRes, res->sch);
            release_record(recRes, res->sch);
          }
          release_record(recR, right->sch);
        }

        unpin(pageR);
      }
      release_record(recL, left->sch);
    }

    unpin(pageL);
  }
  
  return res;
}