#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

typedef struct fifo_entry {
    vector<int> schedule;
    int difficulty;
    int days_left;
} fe_t;

static void dump_fe(fe_t& fe) {
    cout << "days left " << fe.days_left << " difficulty " << fe.difficulty <<  " schedule: ";
    for (int i = 0; i < fe.schedule.size(); i++) {
        cout << fe.schedule[i] << " ";
    }
    cout << endl;
}

static int day_difficulty(int sind, int eind, vector<int>& jd) {
    int max = 0;
    for (int i = sind + 1; i <= eind; i++) {
        if (jd[i] > max)
            max = jd[i];
    }
    return max;
}

static int schedule_difficulty(vector<int>& s, vector<int>& jd) {
    int sd = 0;
    sd = day_difficulty(-1, s[0], jd);
    for (int i = 1; i < s.size(); i++) {
        sd += day_difficulty(s[i-1], s[i], jd);
    }
    return sd;
}

int minDifficulty(vector<int>& jd, int d) {
    queue<fe_t> fifo;
    fe_t fe, new_fe;
    vector<int> schedule;
    int min = -1, days_left, last;
    
    if (jd.size() < d) // no solution
        goto out;

    days_left = d;
    
    // add all first day options:
    if (days_left == 1) {
        schedule.push_back(jd.size() - 1);
        min = schedule_difficulty(schedule, jd);
        goto out;
    }
    for (int i = 0; i < jd.size() - days_left + 1; i++) {
        schedule.clear();
        schedule.push_back(i);
        fe.schedule = schedule;
        fe.difficulty = schedule_difficulty(schedule, jd);
        fe.days_left = days_left - 1;
        fifo.push(fe);
    }
    
    // explore all schedules:
    while (!fifo.empty()) {
        fe = fifo.front(); fifo.pop();
        dump_fe(fe);
        if (fe.days_left == 1) {
            fe.schedule.push_back(jd.size() - 1);
            fe.difficulty += day_difficulty(fe.schedule[fe.schedule.size() - 2], fe.schedule[fe.schedule.size() - 1], jd);
            if (min < 0 || fe.difficulty < min)
                min = fe.difficulty;
        } else {
            days_left = fe.days_left;
            last = fe.schedule[fe.schedule.size() - 1];
            for (int i = last + 1; i < jd.size() - days_left + 1; i++) {
                new_fe.schedule = fe.schedule;
                new_fe.schedule.push_back(i);
                new_fe.days_left = days_left - 1;
                new_fe.difficulty = fe.difficulty + day_difficulty(last, i, jd);
                fifo.push(new_fe);
            }
        }
    }
    
    assert(min > 0);
    
out:
    return min;
}

static void test_prog1_specific_case1(void) {
    vector<int> v;
    
    v = {6, 5, 4, 3, 2, 1};
    assert(minDifficulty(v, 2) == 7);
    
    v = {10, 6, 4, 1, 12, 2, 14};
    printf("md %d\n", minDifficulty(v, 3));
    
    v = {380,302,102,681,863,676,243,671,651,612,162,561,394,856,601,30,6,257,921,405,716,126,158,476,889,699,668,930,139,164,641,801,480,756,797,915,275,709,161,358,461,938,914,557,121,964,315};
    printf("md %d\n", minDifficulty(v, 10));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
