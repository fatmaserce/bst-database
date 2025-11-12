#ifndef RECORD_H
#define RECORD_H

#include <iostream>
using namespace std;

// ================== Record ==================
// Represents one "row" or entry in our simple in-memory database.
struct Record {
    int id = 0;             // unique student ID (used as primary key)
    string last;            // last name
    string first;           // first name
    string major;           // major field of study
    double gpa = 0.0;       // GPA value
    bool deleted = false;   // "tombstone" flag for soft deletion
};

#endif
