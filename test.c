#include <stdio.h>

typedef struct Node{
    int data;
    struct Node* next;
} Node;

Node *createNode(int data) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

Node *createList(int n) {
    Node *head = NULL;
    Node *current = NULL;

    for(int i = 0; i < n; i++) {
        int data;
        scanf("%d", &data);
        Node *newNode = createNode(data);
        if(head == NULL) {
            head = newNode;
            current = newNode;
        } else {
            current->next = newNode;
            current = newNode;
        }
    }
    return head;
}

int main() {
    int n;
    scanf("%d", &n);
    Node *head = createList(n);
    return 0;
}