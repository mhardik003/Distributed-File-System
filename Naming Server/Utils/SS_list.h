#ifndef SS_LIST_H
#define SS_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

// Define the Node structure
typedef struct ListNode
{
    char ip[16]; // To store the IP address
    int nm_port; // To store the Port number
    int client_port;
    struct ListNode *next;
} ListNode;

// Function to create a new node
ListNode *createListNode(const char *ip, int nm_port, int client_port)
{
    ListNode *newNode = (ListNode *)malloc(sizeof(ListNode));
    if (newNode == NULL)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }
    strncpy(newNode->ip, ip, 15);
    newNode->ip[15] = '\0'; // Ensuring string termination
    newNode->nm_port = nm_port;
    newNode->client_port = client_port;
    newNode->next = NULL;
    return newNode;
}

// Function to insert a node at the beginning of the list
void insertNode(ListNode **head, const char *ip, int nm_port, int client_port)
{
    ListNode *newNode = createListNode(ip, nm_port, client_port);
    newNode->next = *head;
    *head = newNode;
}

// Function to delete a node by IP and port
void deleteNode(ListNode **head, const char *ip, int nm_port)
{
    ListNode *temp = *head, *prev = NULL;

    // If head node itself holds the key
    if (temp != NULL && strcmp(temp->ip, ip) == 0 && temp->nm_port == nm_port)
    {
        *head = temp->next;
        free(temp);
        return;
    }

    // Search for the key to be deleted
    while (temp != NULL && !(strcmp(temp->ip, ip) == 0 && temp->nm_port == nm_port))
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL)
        return;

    // Unlink the node from linked list
    prev->next = temp->next;
    free(temp);
}

// Function to display the list
void displayList(ListNode *node)
{
    while (node != NULL)
    {
        printf("IP: %s, Ports: %d %d\n", node->ip, node->nm_port, node->client_port);
        node = node->next;
    }
}

#endif