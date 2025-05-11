#ifndef CSV_GENERATOR_H
#define CSV_GENERATOR_H

#include "ast.h"

typedef struct {
    char* output_dir;  /* Directory for CSV files */
    int next_id;       /* Counter for generating unique IDs */
} CSVContext;

/* Initialize CSV generation context */
CSVContext* init_csv_context(const char* output_dir);

/* Generate CSV files from AST */
void generate_csv(Node* root, Schema* schema, CSVContext* context);

/* Free CSV context */
void free_csv_context(CSVContext* context);

#endif /* CSV_GENERATOR_H */