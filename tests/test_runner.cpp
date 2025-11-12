// tests/test_runner.cpp
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "../BST.h"
#include "../Record.h"
#include "../Engine.h"  // students implement parts here

using std::cout;
using std::endl;

static void expect(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "TEST FAIL: " << msg << std::endl;
        std::exit(1);
    }
}

static void expect_eq_int(int actual, int expected, const char* what) {
    if (actual != expected) {
        std::cerr << "TEST FAIL: " << what << " (expected " << expected
                  << ", got " << actual << ")\n";
        std::exit(1);
    }
}

static void printRecord(const Record& r) {
    cout << r.id << "," << r.last << "," << r.first << "," << r.major << "," << r.gpa << "\n";
}

int main() {
    Engine eng;

    // --- Seed data (note: IDs strictly increasing -> right-skewed ID BST) ---
    std::vector<Record> seed = {
        {1000123, "Nguyen",   "Anya",   "CS",   3.87, false},
        {1000456, "Patel",    "Dev",    "Math", 3.55, false},
        {1000789, "Gonzalez", "Lucia",  "EE",   3.92, false},
        {1000811, "Smith",    "Riley",  "CS",   2.98, false},
        {1001022, "Ali",      "Omar",   "Bio",  3.20, false},
        {1001099, "Smith",    "Avery",  "EE",   3.70, false},
        {1002042, "Green",    "Jamie",  "CS",   3.65, false}
    };
    for (const auto& r : seed) eng.insertRecord(r);

    // --- Test: findById (existing) ---
    {
        int cmp = 0;
        auto rec = eng.findById(1000789, cmp);
        expect(rec != nullptr, "findById should return a record for existing id");
        expect(rec->last == "Gonzalez", "findById returned wrong record (last name)");
        // Right-skewed ID tree; path length = 3 nodes => 2+2+1 = 5 comparisons
        expect_eq_int(cmp, 5, "comparisons for findById(1000789)");
    }

    // --- Test: findById (missing, larger than all keys) ---
    {
        int cmp = 0;
        auto rec = eng.findById(9999999, cmp);
        expect(rec == nullptr, "findById should return nullptr for missing id");
        // Traverses all 7 nodes in right-skewed tree => 7 * 2 = 14
        expect_eq_int(cmp, 14, "comparisons for findById(9999999)");
    }

    // --- Test: rangeById ---
    {
        int cmp = 0;
        auto rows = eng.rangeById(1000400, 1001000, cmp); // expect 1000456, 1000789, 1000811
        expect(rows.size() == 3, "rangeById should return 3 rows in range [1000400..1001000]");
        bool saw_456=false, saw_789=false, saw_811=false;
        for (auto* r : rows) {
            if (r->id == 1000456) saw_456 = true;
            if (r->id == 1000789) saw_789 = true;
            if (r->id == 1000811) saw_811 = true;
        }
        expect(saw_456 && saw_789 && saw_811, "rangeById missing expected ids");
        // Visits nodes 1000123, 1000456, 1000789, 1000811, 1001022 (5 nodes), 3 comps/node => 15
        expect_eq_int(cmp, 15, "comparisons for rangeById(1000400..1001000)");
    }

    // --- Test: prefixByLast (case-insensitive) ---
    {
        int cmp = 0;
        auto rows = eng.prefixByLast("smi", cmp); // should match last=="Smith" (two RIDs)
        int cnt = 0;
        for (auto* r : rows) if (r->last.rfind("Smith", 0) == 0) ++cnt;
        expect(cnt == 2, "prefixByLast('smi') should return 2 Smith records");
        // Visits 'nguyen' -> 'patel' -> 'smith' (3 nodes) => 3 comps/node = 9
        expect_eq_int(cmp, 9, "comparisons for prefixByLast('smi')");
    }

    // --- Test: deleteById + verify not found ---
    {
        bool ok = eng.deleteById(1000811);  // delete Smith, Riley
        expect(ok, "deleteById should succeed for existing id");

        int cmp = 0;
        auto rec = eng.findById(1000811, cmp);
        expect(rec == nullptr, "deleted record should not be findable");
        // After deletion, search descends to 1001022 then left=null: 4 nodes * 2 = 8
        expect_eq_int(cmp, 8, "comparisons for findById(1000811) after delete");

        // After delete, prefix query should have 1 Smith (Avery) remaining
        auto rows = eng.prefixByLast("smith", cmp);
        int cnt = 0;
        for (auto* r : rows) if (r->last.rfind("Smith", 0) == 0) ++cnt;
        expect(cnt == 1, "prefixByLast after delete should return 1 Smith record");
        // Same last-name tree path ('nguyen' -> 'patel' -> 'smith') => 9
        expect_eq_int(cmp, 9, "comparisons for prefixByLast('smith') after delete");
    }

    // --- Test: insertRecord + prefix again ---
    {
        int cmp = 0;
        eng.insertRecord({1003000, "Smith", "Jordan", "Math", 3.80, false});
        auto rows = eng.prefixByLast("SMI", cmp); // test case-insensitive again
        int cnt = 0;
        for (auto* r : rows) if (r->last.rfind("Smith", 0) == 0) ++cnt;
        expect(cnt == 2, "prefixByLast after insert should return 2 Smith records");
        // Tree shape for last names unchanged on key placement for 'smith' => 9
        expect_eq_int(cmp, 9, "comparisons for prefixByLast('SMI') after insert");
    }

    cout << "ALL TESTS PASSED\n";
    return 0;
}
