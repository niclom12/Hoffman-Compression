#include <stdlib.h>
#include "list.h"

Node* create_list() {
    Node* head = NULL;
    for (int i = 255; i >= 0; --i) {
        Node* new_node = (Node*)malloc(sizeof(Node));
        new_node->symbol = (unsigned char)i;
        new_node->next = head;
        head = new_node;
    }
    return head;
}

// Move symbol to front, return index it was found at
int move_to_front(Node** head, unsigned char symbol) {
    Node *prev = NULL, *curr = *head;
    int index = 0;

    while (curr && curr->symbol != symbol) {
        prev = curr;
        curr = curr->next;
        index++;
    }

    if (!curr) return -1; // Not found

    if (prev) {
        prev->next = curr->next;
        curr->next = *head;
        *head = curr;
    }

    return index;
}

// Get symbol at index and move to front
unsigned char get_symbol(Node** head, int index) {
    if (!head || index < 0) {
        return 0;
    }
    Node *prev = NULL, *curr = *head;
    for (int i = 0; i < index && curr; i++) {
        prev = curr;
        curr = curr->next;
    }
    if (!curr) {
        return 0;
    }
    unsigned char sym = curr->symbol;
    if (prev) {
        prev->next = curr->next;
        curr->next = *head;
        *head = curr;
    }

    return sym;
}

void free_list(Node* head) {
    while (head) {
        Node* tmp = head;
        head = head->next;
        free(tmp);
    }
}
