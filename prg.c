#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char str[100];
int curr=0;

void invalid(){ printf("----ERROR!----\n"); exit(0); }
void valid(){ printf("----SUCCESS!----\n"); exit(0); }

// ---------------- Grammar 1 ----------------

void S1(); void T1();

void S1(){
    if(str[curr]=='a'){ curr++; }
    else if(str[curr]=='>'){ curr++; }
    else if(str[curr]=='('){
        curr++; T1();
        if(str[curr]==')') curr++;
        else invalid();
    }
    else invalid();
}

void T1(){
    S1();
    if(str[curr]==','){
        curr++;
        T1();
    }
}

// ---------------- Grammar 2 ----------------
void S2(); void U2(); void V2(); void W2();

void S2(){ U2(); V2(); W2(); }

void U2(){
    if(str[curr]=='('){
        curr++; S2();
        if(str[curr]==')') curr++;
        else invalid();
    }
    else if(str[curr]=='a'){
        curr++; S2();
        if(str[curr]=='b') curr++;
        else invalid();
    }
    else if(str[curr]=='d'){ curr++; }
    else invalid();
}

void V2(){
    if(str[curr]=='a'){ curr++; V2(); }
    // epsilon otherwise
}

void W2(){
    if(str[curr]=='c'){ curr++; W2(); }
    // epsilon otherwise
}

// ---------------- Grammar 3 ----------------
void S3(); void A3(); void B3();

void S3(){
    if(str[curr]=='a'){
        curr++; A3();
        if(str[curr]=='c'){ curr++; B3(); }
        else invalid();
        if(str[curr]=='e') curr++;
        else invalid();
    }
    else invalid();
}

void A3(){
    if(str[curr]=='b'){
        curr++;
        if(str[curr]=='b'){ A3(); } // recursion for Ab
    }
    else invalid();
}

void B3(){
    if(str[curr]=='d') curr++;
    else invalid();
}

/* ---------------- Main ---------------- */
int main(){
    int choice;
    printf("Choose Grammar (1/2/3): ");
    scanf("%d",&choice);
    printf("Enter string: ");
    scanf("%s",str);
    curr=0;

    if(choice==1) S1();
    else if(choice==2) S2();
    else if(choice==3) S3();
    else { printf("Invalid choice\n"); return 0; }

    if(str[curr]=='$') valid();
    else invalid();
}
