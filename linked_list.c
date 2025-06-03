#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "memory_manager.h"

// Node definition: stores a 16-bit integer and a pointer to the next node
typedef struct Node {
    uint16_t data;     // Stores the data as an unsigned 16-bit integer
    struct Node* next; // A pointer to the next node in the list
} Node;

// Initializes the list and the memory manager
void list_init(Node** head, size_t size) {
    if (head == NULL) {
        return;
    }
    // Set the head to NULL and initialize the memory manager
    *head = NULL;
    mem_init(size);
}

// Inserts a new node at the end of the list
void list_insert(Node** head, uint16_t data) {
    if (head == NULL) {
        return;
    }

    // Allocate memory for the new node
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        return;
    }
    new_node->data = data;
    new_node->next = NULL;

    // If the list is empty, make this node the head
    if (*head == NULL) {
        *head = new_node;
        return;
    }

    // Otherwise, traverse to the last node and append the new node
    Node* current = *head;
    while (current->next) {
        current = current->next;
    }
    current->next = new_node;
}

// Inserts a new node immediately after the given node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (!prev_node) {
        return;
    }
    // Allocate and link the new node after 'prev_node'
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        return;
    }
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

// Inserts a new node immediately before the given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (!head || !next_node) {
        return;
    }
    // Allocate memory for the new node
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        return;
    }
    new_node->data = data;

    // If 'next_node' is the head, insert the new node before it
    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
        return;
    }

    // Find the node preceding 'next_node'
    Node* current = *head;
    while (current && current->next != next_node) {
        current = current->next;
    }
    if (!current) {
        // If 'next_node' isn't found, free the new node and return
        mem_free(new_node);
        return;
    }
    new_node->next = next_node;
    current->next = new_node;
}

// Deletes the first node with the given data
void list_delete(Node** head, uint16_t data) {
    if (!head || !*head) {
        return;
    }
    Node* current = *head;
    Node* prev = NULL;

    // Traverse until the node with the matching data is found
    while (current && current->data != data) {
        prev = current;
        current = current->next;
    }
    // If no matching node, return
    if (!current) {
        return;
    }
    // If the node to delete is the head
    if (!prev) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }
    // Free the node
    mem_free(current);
}

// Searches for a node with the given data
Node* list_search(Node** head, uint16_t data) {
    if (!head) {
        return NULL;
    }
    // Start from the head
    Node* current = *head;
    // Traverse until the data is found
    while (current) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }
    // Return NULL if not found
    return NULL;
}

// Displays all elements in the list
void list_display(Node** head) {
    if (!head) {
        return;
    }
    printf("[");
    Node* current = *head;
    // Print each node's data, separated by commas
    while (current) {
        printf("%u", current->data);
        if (current->next) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}

// Displays elements within a specified range of nodes
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    if (!head) {
        return;
    }
    printf("[");
    Node* current = *head;

    // Move to start_node if provided
    if (start_node) {
        while (current && current != start_node) {
            current = current->next;
        }
        if (!current) {
            printf("]");
            return;
        }
    }
    // Print until end_node or the end of the list
    bool first = true;
    while (current) {
        if (first) {
            first = false;
        } else {
            printf(", ");
        }
        printf("%u", current->data);
        if (current == end_node) {
            break;
        }
        current = current->next;
    }
    printf("]");
}

// Counts how many nodes are in the list
int list_count_nodes(Node** head) {
    if (!head) {
        return 0;
    }
    int count = 0;
    Node* current = *head;
    // Traverse the list and count each node
    while (current) {
        count++;
        current = current->next;
    }
    printf("Total nodes in the list: %d\n", count);
    return count;
}

// Cleans up the list by freeing all nodes
void list_cleanup(Node** head) {
    if (!head) {
        return;
    }
    Node* current = *head;
    // Free each node in the list
    while (current) {
        Node* temp = current;
        current = current->next;
        mem_free(temp);
    }
    // Reset the head pointer and deinitialize memory
    *head = NULL;
    mem_deinit();
}