#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cassert>
using namespace std;
struct Node {
   string data;
   struct Node *next;
};
struct Node* top = NULL;
void push(string val) {
   struct Node* newnode = (struct Node*) malloc(sizeof(struct Node));
   newnode->data = val;
   newnode->next = top;
   top = newnode;
}
void pop() {
   if(top==NULL)
   cout<<"Stack Underflow"<<endl;
   else {
    //   cout<<"The popped element is "<< top->data <<endl;
      free(top);
      top = top->next;
   }
}

string peek(){
    cout << "OUTPUT: ";
    if(top != NULL){
    return top->data + "\n";
    }
    return "Stack is empty\n";
}


int main(){
    string res;
    push("hello");
    res = peek();
    assert(res.compare("hello\n") == 0);
    push("hi");
    pop();
    res = peek();
    assert(res.compare("hello\n") == 0);
    pop();
    res = peek();
    assert(res.compare("Stack is empty\n") == 0);
}

