# JSON to CSV Mini Project

## Overview
This project is a C-based tool that parses a structured JSON file and generates CSV files for each top-level array (table) in the JSON. It is designed for educational purposes, demonstrating compiler construction techniques such as lexical analysis, parsing, AST construction, and code generation.

## Features
- Parses JSON files with nested arrays and objects.
- Automatically detects tables (arrays of objects) and generates a CSV file for each.
- Handles scalar fields (strings, numbers, booleans) as CSV columns.
- Outputs CSV files to an `output/` directory.
- `expected_outputs` is the directory that contains results for the data files to be tested

## Prerequisites
- GCC (tested with gcc (Debian 14.2.0-19) 14.2.0)
- Flex (tested with flex 2.6.4)
- Bison (tested with bison++ Version 1.21.9-1)

## Build Instructions
1. Generate the parser and scanner (if not already present):
   ```sh
   flex scanner.l
   bison -d parser.y
   ```
2. Compile the project:
   ```sh
   gcc -o csv_parser main.c ast.c csv_generator.c parser.tab.c lex.yy.c -lfl
   ```

## Usage
1. Place your input JSON file in the project directory (e.g., `input.json`, `data1.json` etc.). (All 5 test files are already present in the project directory, all you need to do is change the filename below to test it.)
2. Run the parser:
   ```sh
   ./csv_parser data1.json
   ```
3. The generated CSV files will be found in the `output/` directory.

## Example
### Sample `input.json`
```json
{
  "users": [
    { "id": 1, "name": "Alice Johnson", "email": "alice.j@example.com", "age": 29, "isActive": true, "country": "USA" },
    { "id": 2, "name": "Bob Smith", "email": "bob.smith@sample.org", "age": 34, "isActive": false, "country": "UK" }
  ],
  "posts": [
    { "id": 101, "userId": 1, "title": "Exploring the Mountains", "content": "Had an amazing hike in the Rockies!", "createdAt": "2024-11-01T10:00:00Z" },
    { "id": 102, "userId": 2, "title": "City Food Tour", "content": "Discovered some incredible eats downtown.", "createdAt": "2024-12-05T14:30:00Z" }
  ],
  "comments": [
    { "id": 201, "postId": 101, "userId": 2, "comment": "Looks like a great trip!", "createdAt": "2024-11-02T08:15:00Z" },
    { "id": 202, "postId": 102, "userId": 1, "comment": "I need to check out those places!", "createdAt": "2024-12-06T09:00:00Z" }
  ],
  "products": [
    { "id": 301, "name": "Wireless Headphones", "price": 89.99, "stock": 120, "category": "Electronics" },
    { "id": 302, "name": "Yoga Mat", "price": 25.50, "stock": 200, "category": "Fitness" }
  ],
  "orders": [
    { "id": 401, "userId": 1, "productId": 301, "quantity": 1, "orderDate": "2025-01-10T12:00:00Z", "status": "shipped" },
    { "id": 402, "userId": 2, "productId": 302, "quantity": 2, "orderDate": "2025-02-20T15:45:00Z", "status": "processing" }
  ]
}
```

### Sample Output
#### `output/users.csv`
```
id,name,email,age,isActive,country
1,Alice Johnson,alice.j@example.com,29,true,USA
2,Bob Smith,bob.smith@sample.org,34,false,UK
```
#### `output/posts.csv`
```
id,userId,title,content,createdAt
3,1,Exploring the Mountains,Had an amazing hike in the Rockies!,2024-11-01T10:00:00Z
4,2,City Food Tour,Discovered some incredible eats downtown.,2024-12-05T14:30:00Z
```
#### `output/comments.csv`
```
id,postId,userId,comment,createdAt
5,101,2,Looks like a great trip!,2024-11-02T08:15:00Z
6,102,1,I need to check out those places!,2024-12-06T09:00:00Z
```
#### `output/products.csv`
```
id,name,price,stock,category
7,Wireless Headphones,89.99,120,Electronics
8,Yoga Mat,25.5,200,Fitness
```
#### `output/orders.csv`
```
id,userId,productId,quantity,orderDate,status
9,1,301,1,2025-01-10T12:00:00Z,shipped
10,2,302,2,2025-02-20T15:45:00Z,processing
```

## Notes
- The tool currently expects the input JSON to have top-level arrays of objects for each table.
- The `output/root.csv` file is generated for the root object and can usually be ignored if there is no object name for a single table json file.
- Only scalar fields (string, number, boolean) are included as CSV columns; nested objects/arrays are not flattened.
- For large or deeply nested JSON, further enhancements may be needed.

## License
This project is for educational use. 