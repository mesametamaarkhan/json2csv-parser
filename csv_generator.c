#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "csv_generator.h"

/* Helper function to create directory if it doesn't exist */
static int ensure_directory_exists(const char* dir) {
    struct stat st = {0};
    
    if (stat(dir, &st) == -1) {
        #ifdef _WIN32
        /* Windows implementation */
        if (mkdir(dir) == -1) {
            fprintf(stderr, "Error creating directory '%s': %s\n", dir, strerror(errno));
            return 0;
        }
        #else
        /* Unix/Linux implementation */
        if (mkdir(dir, 0755) == -1) {
            fprintf(stderr, "Error creating directory '%s': %s\n", dir, strerror(errno));
            return 0;
        }
        #endif
    }
    
    return 1;
}

/* CSV escaping function */
static char* escape_csv_field(const char* str) {
    if (str == NULL) return strdup("");
    
    /* Count characters that need escaping */
    int len = strlen(str);
    int escape_count = 0;
    int needs_quotes = 0;
    
    for (int i = 0; i < len; i++) {
        if (str[i] == '"') {
            escape_count++;
        }
        if (str[i] == ',' || str[i] == '\n' || str[i] == '"') {
            needs_quotes = 1;
        }
    }
    
    /* If no escaping or quoting needed, return a simple duplicate */
    if (escape_count == 0 && !needs_quotes) {
        return strdup(str);
    }
    
    /* Allocate memory for escaped string */
    char* escaped = malloc(len + escape_count + (needs_quotes ? 2 : 0) + 1);
    int pos = 0;
    
    /* Add opening quote if needed */
    if (needs_quotes) {
        escaped[pos++] = '"';
    }
    
    /* Copy and escape string */
    for (int i = 0; i < len; i++) {
        if (str[i] == '"') {
            escaped[pos++] = '"';  /* Double quotes */
            escaped[pos++] = '"';
        } else {
            escaped[pos++] = str[i];
        }
    }
    
    /* Add closing quote if needed */
    if (needs_quotes) {
        escaped[pos++] = '"';
    }
    
    escaped[pos] = '\0';
    return escaped;
}

/* Initialize CSV generation context */
CSVContext* init_csv_context(const char* output_dir) {
    CSVContext* context = malloc(sizeof(CSVContext));
    
    if (output_dir == NULL || strlen(output_dir) == 0) {
        context->output_dir = strdup("./csv_output");
    } else {
        context->output_dir = strdup(output_dir);
    }
    
    /* Create output directory if it doesn't exist */
    if (!ensure_directory_exists(context->output_dir)) {
        free(context->output_dir);
        free(context);
        return NULL;
    }
    
    context->next_id = 1;
    return context;
}

/* Free CSV context */
void free_csv_context(CSVContext* context) {
    if (context) {
        free(context->output_dir);
        free(context);
    }
}

/* Helper to write a node value to a CSV field */
static void write_node_value(FILE* file, Node* node) {
    if (!node) {
        fprintf(file, "");
        return;
    }
    
    char* escaped = NULL;
    char buffer[64]; /* For number conversion */
    
    switch (node->type) {
        case NODE_STRING:
            escaped = escape_csv_field(node->data.string_value);
            fprintf(file, "%s", escaped);
            free(escaped);
            break;
            
        case NODE_NUMBER:
            snprintf(buffer, sizeof(buffer), "%g", node->data.number_value);
            fprintf(file, "%s", buffer);
            break;
            
        case NODE_BOOLEAN:
            fprintf(file, "%s", node->data.boolean_value ? "true" : "false");
            break;
            
        case NODE_NULL:
            /* Empty field for null values */
            break;
            
        case NODE_OBJECT:
        case NODE_ARRAY:
            /* Complex types are processed separately */
            fprintf(file, "");
            break;
    }
}

/* Writing CSV header row (column names) */
static void write_csv_header(FILE* file, Table* table) {
    fprintf(file, "%s", table->columns[0]); /* First column (ID) */
    
    for (int i = 1; i < table->column_count; i++) {
        fprintf(file, ",%s", table->columns[i]);
    }
    
    fprintf(file, "\n");
}

/* Find a pair by key in an object node */
static Pair* find_pair_by_key(Node* obj_node, const char* key) {
    if (obj_node->type != NODE_OBJECT) return NULL;
    
    for (int i = 0; i < obj_node->data.object.pair_count; i++) {
        if (strcmp(obj_node->data.object.pairs[i]->key, key) == 0) {
            return obj_node->data.object.pairs[i];
        }
    }
    
    return NULL;
}

/* Process an object and write it to CSV */
static void process_object(Node* obj_node, Table* table, FILE* file, int id, Schema* schema, CSVContext* context);

/* Process array nodes, find all objects and process them */
static void process_array(Node* array_node, Table* table, FILE* file, Schema* schema, CSVContext* context) {
    if (array_node->type != NODE_ARRAY) return;
    
    for (int i = 0; i < array_node->data.array.element_count; i++) {
        Node* element = array_node->data.array.elements[i];
        
        if (element->type == NODE_OBJECT) {
            /* Process object within array with a new ID */
            process_object(element, table, file, context->next_id++, schema, context);
        }
    }
}

/* Process a single object node and write it to CSV */
static void process_object(Node* obj_node, Table* table, FILE* file, int id, Schema* schema, CSVContext* context) {
    if (obj_node->type != NODE_OBJECT) return;
    
    /* Start with ID column */
    fprintf(file, "%d", id);
    
    /* Process all scalar columns in order */
    for (int i = 1; i < table->column_count; i++) {
        fprintf(file, ",");
        
        Pair* pair = find_pair_by_key(obj_node, table->columns[i]);
        if (pair) {
            write_node_value(file, pair->value);
            
            /* Process nested objects and arrays */
            if (pair->value->type == NODE_OBJECT || pair->value->type == NODE_ARRAY) {
                /* Find matching table for this nested structure */
                Table* nested_table = schema->tables;
                while (nested_table) {
                    if (strcmp(nested_table->name, table->columns[i]) == 0) {
                        /* Found matching table, process nested structure */
                        char filepath[512];
                        snprintf(filepath, sizeof(filepath), "%s/%s.csv", context->output_dir, nested_table->name);
                        
                        FILE* nested_file = fopen(filepath, "a");
                        if (!nested_file) {
                            fprintf(stderr, "Failed to open nested file %s\n", filepath);
                            continue;
                        }
                        
                        if (pair->value->type == NODE_OBJECT) {
                            process_object(pair->value, nested_table, nested_file, context->next_id++, schema, context);
                        } else if (pair->value->type == NODE_ARRAY) {
                            process_array(pair->value, nested_table, nested_file, schema, context);
                        }
                        
                        fclose(nested_file);
                        break;
                    }
                    nested_table = nested_table->next;
                }
            }
        }
    }
    
    fprintf(file, "\n");
}

/* Generate CSV files from AST */
void generate_csv(Node* root, Schema* schema, CSVContext* context) {
    if (!root || !schema || !context) return;
    
    /* Process each table in schema */
    Table* current_table = schema->tables;
    while (current_table) {
        /* Create CSV file for this table */
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s.csv", context->output_dir, current_table->name);
        
        FILE* file = fopen(filepath, "w");
        if (!file) {
            fprintf(stderr, "Failed to create file %s\n", filepath);
            current_table = current_table->next;
            continue;
        }
        
        /* Write header row */
        write_csv_header(file, current_table);
        
        /* Process root node if it's an object */
        if (root->type == NODE_OBJECT) {
            process_object(root, current_table, file, context->next_id++, schema, context);
        }
        /* Process root node if it's an array of objects */
        else if (root->type == NODE_ARRAY) {
            for (int i = 0; i < root->data.array.element_count; i++) {
                Node* element = root->data.array.elements[i];
                if (element->type == NODE_OBJECT) {
                    process_object(element, current_table, file, context->next_id++, schema, context);
                }
            }
        }
        
        fclose(file);
        current_table = current_table->next;
    }
}