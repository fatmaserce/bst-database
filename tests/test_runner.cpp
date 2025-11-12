// tests/test_runner.cpp
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "../BST.h"
#include "../Record.h"
#include "../Engine.h"  


struct TestSuite {
    int total = 0;
    int passed = 0;
    std::vector<std::string> failures;

    void check(bool cond, const std::string &msg) {
        ++total;
        if (cond) {
            ++passed;
        } else {
            failures.push_back(msg);
            std::cerr << "[FAIL] " << msg << "\n";
        }
    }

    void check_eq_int(int actual, int expected, const std::string &what) {
        ++total;
        if (actual == expected) {
            ++passed;
        } else {
            failures.push_back(what + " (expected " + std::to_string(expected) +
                               ", got " + std::to_string(actual) + ")");
            std::cerr << "[FAIL] " << failures.back() << "\n";
        }
    }

    int summarize() const {
        std::cout << "\n===== TEST SUMMARY =====\n";
        std::cout << "Passed: " << passed << "/" << total
                  << "  Failed: " << (total - passed) << "\n";
        if (!failures.empty()) {
            std::cout << "Failures:\n";
            for (const auto &m : failures) std::cout << "  - " << m << "\n";
        } else {
            std::cout << "ALL TESTS PASSED\n";
        }
        // Return non-zero if any failed (so CI fails appropriately).
        return failures.empty() ? 0 : 1;
    }
};

int main() {
    TestSuite ts;
    Engine eng;

    // --- Seed data (IDs strictly increasing -> right-skewed ID BST) ---
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
        ts.check(rec != nullptr, "findById should return a record for existing id");
        if (rec) ts.check(rec->last == "Gonzalez", "findById returns correct record (last name)");
        // Right-skewed ID tree; path length = 3 nodes => 2+2+1 = 5 comparisons
        ts.check_eq_int(cmp, 5, "comparisons for findById(1000789)");
    }

    // --- Test: findById (missing, larger than all keys) ---
    {
        int cmp = 0;
        auto rec = eng.findById(9999999, cmp);
        ts.check(rec == nullptr, "findById returns nullptr for missing id");
        // Traverses all 7 nodes in right-skewed tree => 7 * 2 = 14
        ts.check_eq_int(cmp, 14, "comparisons for findById(9999999)");
    }

    // --- Test: rangeById ---
    {
        int cmp = 0;
        auto rows = eng.rangeById(1000400, 1001000, cmp); // expect 1000456, 1000789, 1000811
        ts.check((int)rows.size() == 3, "rangeById returns 3 rows in [1000400..1001000]");
        bool saw_456=false, saw_789=false, saw_811=false;
        for (auto* r : rows) {
            if (r->id == 1000456) saw_456 = true;
            if (r->id == 1000789) saw_789 = true;
            if (r->id == 1000811) saw_811 = true;
        }
        ts.check(saw_456 && saw_789 && saw_811, "rangeById contains expected ids");
        // Visits nodes 1000123,1000456,1000789,1000811,1001022 (5 nodes), ~3 comps/node => 15
        ts.check_eq_int(cmp, 15, "comparisons for rangeById(1000400..1001000)");
    }

    // --- Test: prefixByLast (case-insensitive) ---
    {
        int cmp = 0;
        auto rows = eng.prefixByLast("smi", cmp); // should match last=="Smith" (two RIDs)
        int cnt = 0;
        for (auto* r : rows) if (r->last.rfind("Smith", 0) == 0) ++cnt;
        ts.check(cnt == 2, "prefixByLast('smi') returns 2 Smith records");
        // Visits 'nguyen' -> 'patel' -> 'smith' (3 nodes) => ~3 comps/node = 9
        ts.check_eq_int(cmp, 9, "comparisons for prefixByLast('smi')");
    }

    // --- Test: deleteById + verify not found ---
    {
        bool ok = eng.deleteById(1000811);  // delete Smith, Riley
        ts.check(ok, "deleteById succeeds for existing id");

        int cmp = 0;
        auto rec = eng.findById(1000811, cmp);
        ts.check(rec == nullptr, "deleted record is not findable");
        // After deletion, search descends to 1001022 then left=null: 4 nodes * 2 = 8
        ts.check_eq_int(cmp, 8, "comparisons for findById(1000811) after delete");

        auto rows = eng.prefixByLast("smith", cmp);
        int cnt = 0;
        for (auto* r : rows) if (r->last.rfind("Smith", 0) == 0) ++cnt;
        ts.check(cnt == 1, "prefixByLast after delete returns 1 Smith record");
        ts.check_eq_int(cmp, 9, "comparisons for prefixByLast('smith') after delete");
    }

    // --- Test: insertRecord + prefix again ---
    {
        int cmp = 0;
        eng.insertRecord({1003000, "Smith", "Jordan", "Math", 3.80, false});
        auto rows = eng.prefixByLast("SMI", cmp); // test case-insensitive again
        int cnt = 0;
        for (auto* r : rows) if (r->last.rfind("Smith", 0) == 0) ++cnt;
        ts.check(cnt == 2, "prefixByLast after insert returns 2 Smith records");
        ts.check_eq_int(cmp, 9, "comparisons for prefixByLast('SMI') after insert");
    }

    return ts.summarize();
}
