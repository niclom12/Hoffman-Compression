// Linked list logic for manipulating the mtf structure
#ifndef LIST_H
#define LIST_H

typedef struct Node {
    unsigned char symbol;
    struct Node* next;
} Node;

Node* create_list();
int move_to_front(Node** head, unsigned char symbol);
unsigned char get_symbol(Node** head, int index);
void free_list(Node* head);

#endif