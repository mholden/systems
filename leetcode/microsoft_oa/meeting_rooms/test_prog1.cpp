#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

static void dump_imap(multimap<int, int>& imap) {
    for (auto it = imap.begin(); it != imap.end(); it++)
        cout << "{ " << it->first << ", " << it->second << " }" << endl;
}

static void dump_rooms(vector<pair<int, int>>& crooms) {
    cout << "rooms: " << endl;
    for (auto it = crooms.begin(); it != crooms.end(); it++) {
        cout << "{" << it->first << " " << it->second << ")";
    }
    cout << endl;
}

int minMeetingRooms(vector<vector<int>>& intervals) {
    multimap<int, int> imap;
    vector<pair<int, int>> crooms; // current rooms
    pair<int, int> meeting;
    int max, nrooms;
    
    for (int i = 0; i < intervals.size(); i++)
        imap.insert({intervals[i][0], intervals[i][1]});

    //dump_imap(imap);
    
    max = 0;
    nrooms = 0;
    for (auto it = imap.begin(); it != imap.end(); it++) {
        meeting = {it->first, it->second};
        // remove any rooms that are 'out-of-session'
        for (auto it = crooms.begin(); it != crooms.end(); ) {
            if (it->second <= meeting.first) {
                it = crooms.erase(it);
                nrooms--;
            } else
                it++;
        }
        //cout << "meeting " << meeting.first << " " << meeting.second << endl;
        //dump_rooms(crooms);
        crooms.push_back(meeting);
        nrooms++;
        if (nrooms > max)
            max = nrooms;
    }
    
    return max;
}

static void test_prog1_specific_case1(void) {
    vector<vector<int>> times;
    
    times = { {0, 10}, {5, 35}, {10, 40}, {50, 80}, {80, 90} };
    assert(minMeetingRooms(times) == 2);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
